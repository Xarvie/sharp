/* types.c — implementation of the type module.
 *
 * Interning strategy:
 *   - Primitives: one entry per TypeKind, looked up in a fixed array.
 *   - Pointers / refs: keyed on (kind, base).
 *   - Named without targs: keyed on the interned name string.
 *   - Named with targs: keyed on (name, targs[...]).
 *
 * We use a single open-addressed hash table keyed on a small "shape"
 * descriptor. The descriptor is a fixed-size stack struct for fast
 * lookup; on hit we return the canonical Type*; on miss we allocate
 * (in the module's arena), fill in, and insert.
 *
 * Thread safety: none — Sharp's compiler is single-threaded. If that
 * ever changes, wrap this file's globals in a mutex.
 */

#include "sharp.h"
#include "types.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* ===================================================================== *
 *   Module state
 * ===================================================================== */

static Arena* s_arena = NULL;

/* Prim singletons: indexed by TypeKind. Populated lazily. */
static Type* s_prim[TY_USIZE + 1];

/* Open-addressed hash table for composite types. */
typedef struct {
    uint64_t hash;
    Type*    type;
} Bucket;

/* Global SymTable reference for typedef resolution during type checking.
 * Set by sema_build before the type-checking pass. */
static struct SymTable* g_symtable = NULL;

void ty_set_symtable(struct SymTable* st) { g_symtable = st; }

static Bucket* s_table     = NULL;
static size_t  s_table_cap = 0;
static size_t  s_table_len = 0;

/* Interned (arena-copied) names: one canonical const char* per distinct
 * name string. Used so name-equality reduces to pointer-equality, making
 * name-bearing Type shapes cheap to hash / compare. */
typedef struct NameBucket {
    uint64_t            hash;
    const char*         str;
    int                 len;
    struct NameBucket*  next;
} NameBucket;

static NameBucket* s_name_buckets[1024];  /* small fixed-bucket hash */

/* Render cache. We allocate the rendered string in the arena on first
 * use and cache it on the Type itself via a side table — we can't
 * extend Type* without breaking ABI with sharp.h's struct definition. */
typedef struct RenderCacheEntry {
    const Type*                 key;
    const char*                 rendered;
    const char*                 mangled;
    struct RenderCacheEntry*    next;
} RenderCacheEntry;

static RenderCacheEntry* s_render_cache[512];

/* ===================================================================== *
 *   FNV-1a hashing helpers
 * ===================================================================== */

static inline uint64_t fnv_init(void)       { return 1469598103934665603ULL; }
static inline uint64_t fnv_mix (uint64_t h, uint64_t x) {
    return (h ^ x) * 1099511628211ULL;
}
static uint64_t fnv_bytes(uint64_t h, const void* p, size_t n) {
    const unsigned char* b = (const unsigned char*)p;
    for (size_t i = 0; i < n; i++) h = fnv_mix(h, b[i]);
    return h;
}

/* ===================================================================== *
 *   Name interning
 * ===================================================================== */

static const char* intern_name(const char* s, int len) {
    if (!s) return NULL;
    uint64_t h = fnv_bytes(fnv_init(), s, (size_t)len);
    unsigned slot = (unsigned)(h & 1023);
    for (NameBucket* b = s_name_buckets[slot]; b; b = b->next) {
        if (b->hash == h && b->len == len && memcmp(b->str, s, (size_t)len) == 0)
            return b->str;
    }
    /* Allocate and copy into the module arena so lifetime is tied to us. */
    char* copy = (char*)arena_alloc(&s_arena, (size_t)len + 1);
    memcpy(copy, s, (size_t)len);
    copy[len] = 0;
    NameBucket* nb = (NameBucket*)arena_alloc(&s_arena, sizeof(NameBucket));
    nb->hash = h;
    nb->str  = copy;
    nb->len  = len;
    nb->next = s_name_buckets[slot];
    s_name_buckets[slot] = nb;
    return copy;
}

static const char* intern_name_z(const char* s) {
    return s ? intern_name(s, (int)strlen(s)) : NULL;
}

/* ===================================================================== *
 *   Type hashing / equality (structural, for intern lookup only)
 * ===================================================================== */

static uint64_t type_shape_hash(TypeKind k, Type* base,
                                const char* name, Type** targs, int ntargs,
                                bool is_const) {
    uint64_t h = fnv_init();
    h = fnv_mix(h, (uint64_t)k);
    if (base) h = fnv_mix(h, (uint64_t)(uintptr_t)base);
    if (name) h = fnv_mix(h, (uint64_t)(uintptr_t)name);   /* interned ptr */
    h = fnv_mix(h, (uint64_t)ntargs);
    for (int i = 0; i < ntargs; i++)
        h = fnv_mix(h, (uint64_t)(uintptr_t)targs[i]);
    h = fnv_mix(h, (uint64_t)(is_const ? 1 : 0));
    return h;
}

static bool type_shape_eq(const Type* t, TypeKind k, Type* base,
                          const char* name, Type** targs, int ntargs,
                          bool is_const) {
    if (t->kind != k)            return false;
    if (t->base != base)         return false;
    if (t->name != name)         return false;     /* interned → ptr eq */
    if (t->ntargs != ntargs)     return false;
    for (int i = 0; i < ntargs; i++)
        if (t->targs[i] != targs[i]) return false;
    if (t->is_const != is_const) return false;
    return true;
}

static void table_grow(void) {
    size_t new_cap = s_table_cap ? s_table_cap * 2 : 128;
    Bucket* new_tbl = (Bucket*)calloc(new_cap, sizeof(Bucket));
    if (!new_tbl) fatal("types: out of memory");
    for (size_t i = 0; i < s_table_cap; i++) {
        if (!s_table[i].type) continue;
        size_t j = (size_t)(s_table[i].hash & (new_cap - 1));
        while (new_tbl[j].type) j = (j + 1) & (new_cap - 1);
        new_tbl[j] = s_table[i];
    }
    free(s_table);
    s_table     = new_tbl;
    s_table_cap = new_cap;
}

static Type* intern_lookup_or_insert(uint64_t hash, TypeKind k, Type* base,
                                     const char* name,
                                     Type** targs, int ntargs,
                                     bool is_const) {
    if (!s_table_cap || s_table_len * 2 >= s_table_cap) table_grow();

    size_t mask = s_table_cap - 1;
    size_t slot = (size_t)(hash & mask);
    while (s_table[slot].type) {
        if (s_table[slot].hash == hash &&
            type_shape_eq(s_table[slot].type, k, base, name, targs, ntargs, is_const))
            return s_table[slot].type;
        slot = (slot + 1) & mask;
    }

    /* Insert. */
    Type* t = (Type*)arena_alloc(&s_arena, sizeof(Type));
    memset(t, 0, sizeof(*t));
    t->kind      = k;
    t->base      = base;
    t->name      = name;
    t->ntargs    = ntargs;
    t->is_const  = is_const;
    if (ntargs > 0) {
        t->targs = (Type**)arena_alloc(&s_arena, (size_t)ntargs * sizeof(Type*));
        memcpy(t->targs, targs, (size_t)ntargs * sizeof(Type*));
    } else {
        t->targs = NULL;
    }
    s_table[slot].hash = hash;
    s_table[slot].type = t;
    s_table_len++;
    return t;
}

/* ===================================================================== *
 *   Lifecycle
 * ===================================================================== */

void ty_init(void) {
    if (s_arena) return;    /* idempotent */
    s_arena = NULL;
    memset(s_prim, 0, sizeof(s_prim));
    memset(s_name_buckets, 0, sizeof(s_name_buckets));
    memset(s_render_cache, 0, sizeof(s_render_cache));
    s_table     = NULL;
    s_table_cap = 0;
    s_table_len = 0;
    /* Touch the arena so a subsequent `if (s_arena)` check works. */
    (void)arena_alloc(&s_arena, 1);
}

void ty_shutdown(void) {
    if (s_arena) { arena_free_all(&s_arena); s_arena = NULL; }
    free(s_table);
    s_table = NULL; s_table_cap = 0; s_table_len = 0;
    memset(s_prim,         0, sizeof(s_prim));
    memset(s_name_buckets, 0, sizeof(s_name_buckets));
    memset(s_render_cache, 0, sizeof(s_render_cache));
}

/* ===================================================================== *
 *   Construction
 * ===================================================================== */

Type* ty_prim(TypeKind k) {
    if (!s_arena) ty_init();
    if ((int)k < 0 || (int)k > TY_WCHAR_T) return NULL;
    if (k == TY_WCHAR_T) {
        /* wchar_t is a non-primitive (no single C type); return NULL so caller uses named type. */
        return NULL;
    }
    if (s_prim[k]) return s_prim[k];
    Type* t = (Type*)arena_alloc(&s_arena, sizeof(Type));
    memset(t, 0, sizeof(*t));
    t->kind = k;
    s_prim[k] = t;
    return t;
}

Type* ty_ptr(Type* base) {
    if (!s_arena) ty_init();
    uint64_t h = type_shape_hash(TY_PTR, base, NULL, NULL, 0, false);
    return intern_lookup_or_insert(h, TY_PTR, base, NULL, NULL, 0, false);
}

Type* ty_named(const char* name) {
    if (!s_arena) ty_init();
    const char* n = intern_name_z(name);
    uint64_t h = type_shape_hash(TY_NAMED, NULL, n, NULL, 0, false);
    return intern_lookup_or_insert(h, TY_NAMED, NULL, n, NULL, 0, false);
}

Type* ty_generic(const char* name, Type** targs, int ntargs) {
    if (!s_arena) ty_init();
    if (ntargs == 0)
        return ty_named(name);
    const char* n = intern_name_z(name);
    uint64_t h = type_shape_hash(TY_NAMED, NULL, n, targs, ntargs, false);
    return intern_lookup_or_insert(h, TY_NAMED, NULL, n, targs, ntargs, false);
}

/* ===================================================================== *
 *   Const qualifier
 * ===================================================================== */

Type* ty_const(Type* base) {
    if (!s_arena) ty_init();
    if (!base) return NULL;
    if (base->is_const) return base;   /* already const, return same ptr */

    uint64_t h = type_shape_hash(base->kind, base->base, base->name,
                                 base->targs, base->ntargs, true);
    return intern_lookup_or_insert(h, base->kind, base->base, base->name,
                                   base->targs, base->ntargs, true);
}

bool ty_is_const(const Type* t) {
    return t && t->is_const;
}

/* ===================================================================== *
 *   Queries
 * ===================================================================== */

TypeKind    ty_kind  (const Type* t) { return t ? t->kind : TY_VOID; }
Type*       ty_base  (const Type* t) { return t ? t->base : NULL; }
const char* ty_name  (const Type* t) { return t ? t->name : NULL; }
int         ty_ntargs(const Type* t) { return t ? t->ntargs : 0; }
Type*       ty_targ  (const Type* t, int i) {
    return (t && i >= 0 && i < t->ntargs) ? t->targs[i] : NULL;
}

size_t ty_hash(const Type* t) {
    return (size_t)(uintptr_t)t * 2654435761u;
}

/* ===================================================================== *
 *   Predicates
 * ===================================================================== */

bool ty_is_primitive(const Type* t) {
    if (!t) return false;
    return t->kind >= TY_VOID && t->kind <= TY_USIZE;
}

bool ty_is_integer(const Type* t) {
    if (!t) return false;
    switch (t->kind) {
        case TY_I8: case TY_I16: case TY_I32: case TY_I64:
        case TY_U8: case TY_U16: case TY_U32: case TY_U64:
        case TY_ISIZE: case TY_USIZE:
        case TY_WCHAR_T:
            return true;
        default:
            return false;
    }
}

bool ty_is_signed_integer(const Type* t) {
    if (!t) return false;
    switch (t->kind) {
        case TY_I8: case TY_I16: case TY_I32: case TY_I64: case TY_ISIZE:
            return true;
        default:
            return false;
    }
}

bool ty_is_float(const Type* t) {
    return t && (t->kind == TY_F32 || t->kind == TY_F64);
}

bool ty_is_numeric(const Type* t) {
    return ty_is_integer(t) || ty_is_float(t);
}

bool ty_is_bool(const Type* t)         { return t && t->kind == TY_BOOL; }
bool ty_is_void(const Type* t)         { return t && t->kind == TY_VOID; }
bool ty_is_ptr (const Type* t)         { return t && t->kind == TY_PTR;  }
bool ty_is_ref (const Type* t)         { (void)t; return false; }  /* removed */
bool ty_is_named(const Type* t)        { return t && t->kind == TY_NAMED; }
bool ty_is_pointer_like(const Type* t) {
    return t && t->kind == TY_PTR;
}

/* ===================================================================== *
 *   Substitution
 * ===================================================================== */

Type* ty_subst(Type* t, const char** names, Type** repls, int n) {
    if (!t || n == 0) return t;
    switch (t->kind) {
        case TY_PTR: {
            Type* b = ty_subst(t->base, names, repls, n);
            Type* result = (b == t->base) ? t : ty_ptr(b);
            /* Preserve is_const if we created a new pointer. */
            if (t->is_const && result != t)
                result = ty_const(result);
            return result;
        }
        case TY_NAMED: {
            /* A bare named type with no targs may itself be a type-parameter. */
            if (t->ntargs == 0 && t->name) {
                for (int i = 0; i < n; i++)
                    if (strcmp(t->name, names[i]) == 0) {
                        Type* r = repls[i];
                        /* Preserve is_const on the original type. */
                        if (t->is_const && r && !r->is_const)
                            r = ty_const(r);
                        return r;
                    }
                return t;
            }
            if (t->ntargs == 0) return t;
            Type* stack_targs[16];
            Type** nta = (t->ntargs <= 16)
                ? stack_targs
                : (Type**)malloc((size_t)t->ntargs * sizeof(Type*));
            bool changed = false;
            for (int i = 0; i < t->ntargs; i++) {
                nta[i] = ty_subst(t->targs[i], names, repls, n);
                if (nta[i] != t->targs[i]) changed = true;
            }
            Type* out = changed ? ty_generic(t->name, nta, t->ntargs) : t;
            if (t->is_const && out != t)
                out = ty_const(out);
            if (nta != stack_targs) free(nta);
            return out;
        }
        default:
            return t;
    }
}

/* ===================================================================== *
 *   Assignability
 * ===================================================================== */

bool ty_assignable(const Type* to, const Type* from) {
    if (!to || !from) return true;   /* unknown → don't double-diagnose */
    if (ty_eq(to, from)) return true;

    /* NULL literal has type void* — assignable to any pointer. */
    if (from->kind == TY_PTR && from->base && from->base->kind == TY_VOID) {
        if (to->kind == TY_PTR) return true;
    }

    /* Const discard check: you cannot assign const T* to T*. */
    if (to->kind == TY_PTR && from->kind == TY_PTR) {
        /* Pointee: const T* → T* is not allowed (discard const). */
        if (from->base && to->base) {
            if (from->base->is_const && !to->base->is_const) {
                return false;
            }
        }
        /* If both pointees are compatible (same kind), allow T* → const T*. */
        if (ty_eq(from->base, to->base)) return true;
    }

    /* Numeric ↔ numeric (C's usual arithmetic conversions). */
    if (ty_is_numeric(to) && ty_is_numeric(from)) return true;
    if (to->kind == TY_BOOL && ty_is_integer(from)) return true;
    if (ty_is_integer(to) && from->kind == TY_BOOL) return true;

    return false;
}

/* ===================================================================== *
 *   Rendering
 * ===================================================================== */

static RenderCacheEntry* cache_find(const Type* t, bool insert_if_missing) {
    if (!t) return NULL;
    unsigned slot = (unsigned)(ty_hash(t) & 511);
    for (RenderCacheEntry* e = s_render_cache[slot]; e; e = e->next)
        if (e->key == t) return e;
    if (!insert_if_missing) return NULL;
    RenderCacheEntry* e = (RenderCacheEntry*)arena_alloc(&s_arena, sizeof(*e));
    e->key      = t;
    e->rendered = NULL;
    e->mangled  = NULL;
    e->next     = s_render_cache[slot];
    s_render_cache[slot] = e;
    return e;
}

static void render_into(StrBuf* sb, const Type* t) {
    if (!t) { sb_puts(sb, "<unknown>"); return; }
    switch (t->kind) {
        case TY_VOID:
            if (t->is_const) sb_puts(sb, "const void");
            else             sb_puts(sb, "void");
            return;
        case TY_BOOL:
            if (t->is_const) sb_puts(sb, "const bool");
            else             sb_puts(sb, "bool");
            return;
        case TY_I8:
            if (t->is_const) sb_puts(sb, "const i8");
            else             sb_puts(sb, "i8");
            return;
        case TY_I16:
            if (t->is_const) sb_puts(sb, "const i16");
            else             sb_puts(sb, "i16");
            return;
        case TY_I32:
            if (t->is_const) sb_puts(sb, "const i32");
            else             sb_puts(sb, "i32");
            return;
        case TY_I64:
            if (t->is_const) sb_puts(sb, "const i64");
            else             sb_puts(sb, "i64");
            return;
        case TY_U8:
            if (t->is_const) sb_puts(sb, "const u8");
            else             sb_puts(sb, "u8");
            return;
        case TY_U16:
            if (t->is_const) sb_puts(sb, "const u16");
            else             sb_puts(sb, "u16");
            return;
        case TY_U32:
            if (t->is_const) sb_puts(sb, "const u32");
            else             sb_puts(sb, "u32");
            return;
        case TY_U64:
            if (t->is_const) sb_puts(sb, "const u64");
            else             sb_puts(sb, "u64");
            return;
        case TY_F32:
            if (t->is_const) sb_puts(sb, "const f32");
            else             sb_puts(sb, "f32");
            return;
        case TY_F64:
            if (t->is_const) sb_puts(sb, "const f64");
            else             sb_puts(sb, "f64");
            return;
        case TY_ISIZE:
            if (t->is_const) sb_puts(sb, "const isize");
            else             sb_puts(sb, "isize");
            return;
        case TY_USIZE:
            if (t->is_const) sb_puts(sb, "const usize");
            else             sb_puts(sb, "usize");
            return;
        case TY_WCHAR_T:
            if (t->is_const) sb_puts(sb, "const wchar_t");
            else             sb_puts(sb, "wchar_t");
            return;
        case TY_PTR:
            render_into(sb, t->base);
            sb_putc(sb, '*');
            if (t->is_const) sb_puts(sb, " const");
            return;
        case TY_NAMED:
            if (t->is_const) sb_puts(sb, "const ");
            sb_puts(sb, t->name ? t->name : "<anon>");
            if (t->ntargs > 0) {
                sb_putc(sb, '<');
                for (int i = 0; i < t->ntargs; i++) {
                    if (i) sb_puts(sb, ", ");
                    render_into(sb, t->targs[i]);
                }
                sb_putc(sb, '>');
            }
            return;
    }
}

static void mangle_into(StrBuf* sb, const Type* t) {
    if (!t) { sb_puts(sb, "Q"); return; }
    const char* cp = t->is_const ? "c" : "";
    switch (t->kind) {
        case TY_VOID:  sb_puts(sb, cp); sb_puts(sb, "void");   return;
        case TY_BOOL:  sb_puts(sb, cp); sb_puts(sb, "bool");   return;
        case TY_I8:    sb_puts(sb, cp); sb_puts(sb, "i8");     return;
        case TY_I16:   sb_puts(sb, cp); sb_puts(sb, "i16");    return;
        case TY_I32:   sb_puts(sb, cp); sb_puts(sb, "i32");    return;
        case TY_I64:   sb_puts(sb, cp); sb_puts(sb, "i64");    return;
        case TY_U8:    sb_puts(sb, cp); sb_puts(sb, "u8");     return;
        case TY_U16:   sb_puts(sb, cp); sb_puts(sb, "u16");    return;
        case TY_U32:   sb_puts(sb, cp); sb_puts(sb, "u32");    return;
        case TY_U64:   sb_puts(sb, cp); sb_puts(sb, "u64");    return;
        case TY_F32:   sb_puts(sb, cp); sb_puts(sb, "f32");    return;
        case TY_F64:   sb_puts(sb, cp); sb_puts(sb, "f64");    return;
        case TY_ISIZE: sb_puts(sb, cp); sb_puts(sb, "isize");  return;
        case TY_USIZE: sb_puts(sb, cp); sb_puts(sb, "usize");  return;
        case TY_WCHAR_T: sb_puts(sb, cp); sb_puts(sb, "wchar_t"); return;
        case TY_PTR:
            if (t->is_const) { sb_puts(sb, "cp_"); mangle_into(sb, t->base); }
            else               { sb_puts(sb, "p_");  mangle_into(sb, t->base); }
            return;
        case TY_NAMED:
            if (t->is_const) sb_puts(sb, "c");
            sb_puts(sb, t->name ? t->name : "Q");
            for (int i = 0; i < t->ntargs; i++) {
                sb_putc(sb, '_');
                mangle_into(sb, t->targs[i]);
            }
            return;
    }
}

const char* ty_render(const Type* t) {
    if (!t) return "<unknown>";
    if (!s_arena) ty_init();
    RenderCacheEntry* e = cache_find(t, true);
    if (!e->rendered) {
        StrBuf sb; sb_init(&sb);
        render_into(&sb, t);
        char* copy = (char*)arena_alloc(&s_arena, sb.len + 1);
        memcpy(copy, sb.data, sb.len + 1);
        sb_free(&sb);
        e->rendered = copy;
    }
    return e->rendered;
}

const char* ty_mangle(const Type* t) {
    if (!t) return "Q";
    if (!s_arena) ty_init();
    RenderCacheEntry* e = cache_find(t, true);
    if (!e->mangled) {
        StrBuf sb; sb_init(&sb);
        mangle_into(&sb, t);
        char* copy = (char*)arena_alloc(&s_arena, sb.len + 1);
        memcpy(copy, sb.data, sb.len + 1);
        sb_free(&sb);
        e->mangled = copy;
    }
    return e->mangled;
}

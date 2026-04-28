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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* ===================================================================== *
 *   Module state
 * ===================================================================== */

static Arena* s_arena = NULL;

/* Prim singletons: indexed by TypeKind. Populated lazily. */
static Type* s_prim[TY_DOUBLE + 1];

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

static void table_grow(void);  /* forward declaration */

/* Ensure the hash table has room before inserting func/bitfield types */
static void table_ensure(void) {
    if (!s_table_cap || s_table_len * 2 >= s_table_cap) table_grow();
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
    if ((int)k < 0 || k >= TY_PTR) return NULL;
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

Type* ty_func(Type* ret, Type** params, int nparams, bool variadic) {
    if (!s_arena) ty_init();
    table_ensure();
    uint64_t h = fnv_init();
    h = fnv_mix(h, (uint64_t)TY_FUNC);
    h = fnv_mix(h, (uint64_t)(uintptr_t)ret);
    h = fnv_mix(h, (uint64_t)nparams);
    h = fnv_mix(h, (uint64_t)(variadic ? 1 : 0));
    for (int i = 0; i < nparams; i++)
        h = fnv_mix(h, (uint64_t)(uintptr_t)params[i]);
    size_t mask = s_table_cap - 1;
    size_t slot = (size_t)(h & mask);
    while (s_table[slot].type) {
        Type* t = s_table[slot].type;
        if (t->kind == TY_FUNC && t->base == ret && t->nfunc_params == nparams &&
            t->func_variadic == variadic) {
            bool same = true;
            for (int i = 0; i < nparams; i++)
                if (t->func_params[i] != params[i]) { same = false; break; }
            if (same) return t;
        }
        slot = (slot + 1) & mask;
    }
    /* Insert new func type */
    Type* t = (Type*)arena_alloc(&s_arena, sizeof(Type));
    memset(t, 0, sizeof(*t));
    t->kind = TY_FUNC;
    t->base = ret;
    t->nfunc_params = nparams;
    t->func_variadic = variadic;
    if (nparams > 0) {
        t->func_params = (Type**)arena_alloc(&s_arena, (size_t)nparams * sizeof(Type*));
        memcpy(t->func_params, params, (size_t)nparams * sizeof(Type*));
    }
    s_table[slot].hash = h;
    s_table[slot].type = t;
    s_table_len++;
    return t;
}

Type* ty_bitfield(Type* base, int width) {
    if (!s_arena) ty_init();
    table_ensure();
    uint64_t h = fnv_init();
    h = fnv_mix(h, (uint64_t)TY_BITFIELD);
    h = fnv_mix(h, (uint64_t)(uintptr_t)base);
    h = fnv_mix(h, (uint64_t)width);
    size_t mask = s_table_cap - 1;
    size_t slot = (size_t)(h & mask);
    while (s_table[slot].type) {
        Type* t = s_table[slot].type;
        if (t->kind == TY_BITFIELD && t->base == base && t->bit_width == width)
            return t;
        slot = (slot + 1) & mask;
    }
    /* Insert new bitfield type */
    Type* t = (Type*)arena_alloc(&s_arena, sizeof(Type));
    memset(t, 0, sizeof(*t));
    t->kind = TY_BITFIELD;
    t->base = base;
    t->bit_width = width;
    s_table[slot].hash = h;
    s_table[slot].type = t;
    s_table_len++;
    return t;
}

Type* ty_array(Type* elem, int size) {
    if (!s_arena) ty_init();
    table_ensure();
    uint64_t h = fnv_init();
    h = fnv_mix(h, (uint64_t)TY_ARRAY);
    h = fnv_mix(h, (uint64_t)(uintptr_t)elem);
    h = fnv_mix(h, (uint64_t)size);
    size_t mask = s_table_cap - 1;
    size_t slot = (size_t)(h & mask);
    while (s_table[slot].type) {
        Type* t = s_table[slot].type;
        if (t->kind == TY_ARRAY && t->base == elem && t->array_size == size)
            return t;
        slot = (slot + 1) & mask;
    }
    Type* t = (Type*)arena_alloc(&s_arena, sizeof(Type));
    memset(t, 0, sizeof(*t));
    t->kind = TY_ARRAY;
    t->base = elem;
    t->array_size = size;
    s_table[slot].hash = h;
    s_table[slot].type = t;
    s_table_len++;
    return t;
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
    return t->kind >= TY_VOID && t->kind <= TY_DOUBLE;
}

bool ty_is_integer(const Type* t) {
    if (!t) return false;
    switch (t->kind) {
        case TY_CHAR: case TY_SHORT: case TY_INT: case TY_LONG: case TY_LONGLONG:
            return true;
        case TY_BITFIELD:
            return ty_is_integer(t->base);
        default:
            return false;
    }
}

bool ty_is_signed_integer(const Type* t) {
    if (!t) return false;
    switch (t->kind) {
        case TY_CHAR: case TY_SHORT: case TY_INT: case TY_LONG: case TY_LONGLONG:
            return true;
        default:
            return false;
    }
}

bool ty_is_float(const Type* t) {
    return t && (t->kind == TY_FLOAT || t->kind == TY_DOUBLE);
}

bool ty_is_numeric(const Type* t) {
    return ty_is_integer(t) || ty_is_float(t);
}

bool ty_is_bool(const Type* t)         { return t && t->kind == TY_BOOL; }
bool ty_is_void(const Type* t)         { return t && t->kind == TY_VOID; }
bool ty_is_ptr (const Type* t)         { return t && t->kind == TY_PTR;  }
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
        case TY_FUNC: {
            Type* r = ty_subst(t->base, names, repls, n);
            Type* stack_params[16];
            Type** nps = (t->nfunc_params <= 16)
                ? stack_params
                : (Type**)malloc((size_t)t->nfunc_params * sizeof(Type*));
            bool changed = (r != t->base);
            for (int i = 0; i < t->nfunc_params; i++) {
                nps[i] = ty_subst(t->func_params[i], names, repls, n);
                if (nps[i] != t->func_params[i]) changed = true;
            }
            Type* out = changed ? ty_func(r, nps, t->nfunc_params, t->func_variadic) : t;
            if (t->is_const && out != t)
                out = ty_const(out);
            if (nps != stack_params) free(nps);
            return out;
        }
        case TY_BITFIELD: {
            Type* b = ty_subst(t->base, names, repls, n);
            Type* result = (b == t->base) ? t : ty_bitfield(b, t->bit_width);
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

    /* Resolve C primitive names for compatibility checks.
     * Handles TY_NAMED("int") ↔ TY_PRIM(TY_INT) equivalence,
     * including pointer base types like int* ↔ TY_PTR(TY_PRIM(TY_INT)). */
    Type to_tmp[1], from_tmp[1];
    const Type* to_r = to;
    const Type* from_r = from;

    if (to->kind == TY_PTR && to->base && to->base->kind == TY_NAMED && to->base->name) {
        Type* sub = ty_resolve_c_named(to->base->name);
        if (sub) {
            to_tmp[0] = *to;
            to_tmp[0].base = sub;
            to_r = to_tmp;
        }
    } else if (to->kind == TY_NAMED && to->name) {
        Type* r = ty_resolve_c_named(to->name);
        if (r) to_r = r;
    }

    if (from->kind == TY_PTR && from->base && from->base->kind == TY_NAMED && from->base->name) {
        Type* sub = ty_resolve_c_named(from->base->name);
        if (sub) {
            from_tmp[0] = *from;
            from_tmp[0].base = sub;
            from_r = from_tmp;
        }
    } else if (from->kind == TY_NAMED && from->name) {
        Type* r = ty_resolve_c_named(from->name);
        if (r) from_r = r;
    }

    if (ty_eq(to_r, from_r)) return true;

    /* NULL literal has type void* — assignable to any pointer (including function pointers).
     * Also allow int literal 0 → any pointer (C's null pointer constant rule). */
    if (from_r->kind == TY_PTR && from_r->base && from_r->base->kind == TY_VOID) {
        if (to_r->kind == TY_PTR) return true;
    }
    if (from_r->kind == TY_INT && to_r->kind == TY_PTR) return true;

    /* C void* compatibility: any pointer → void* or const void* */
    if (to_r->kind == TY_PTR && to_r->base && to_r->base->kind == TY_VOID) {
        if (from_r->kind == TY_PTR) return true;
    }

    /* Const discard check: you cannot assign const T* to T*. */
    if (to_r->kind == TY_PTR && from_r->kind == TY_PTR) {
        /* Pointee: const T* → T* is not allowed (discard const). */
        if (from_r->base && to_r->base) {
            if (from_r->base->is_const && !to_r->base->is_const) {
                return false;
            }
        }
        /* If both pointees are compatible (same kind), allow T* → const T*. */
        if (ty_eq(from_r->base, to_r->base)) return true;
    }


    /* Numeric ↔ numeric (C's usual arithmetic conversions). */
    if (ty_is_numeric(to_r) && ty_is_numeric(from_r)) return true;
    if (to_r->kind == TY_BOOL && ty_is_integer(from_r)) return true;
    if (ty_is_integer(to_r) && from_r->kind == TY_BOOL) return true;

    return false;
}

/* ===================================================================== *
 *   C primitive name resolution
 * ===================================================================== */

Type* ty_resolve_c_named(const char* name) {
    if (!name) return NULL;
    if (!strcmp(name, "int"))               return ty_prim(TY_INT);
    if (!strcmp(name, "unsigned int"))      return ty_prim(TY_LONG);
    if (!strcmp(name, "long"))              return ty_prim(TY_LONG);
    if (!strcmp(name, "unsigned long"))      return ty_prim(TY_LONG);
    if (!strcmp(name, "long long"))         return ty_prim(TY_LONGLONG);
    if (!strcmp(name, "unsigned long long"))return ty_prim(TY_LONGLONG);
    if (!strcmp(name, "short"))             return ty_prim(TY_SHORT);
    if (!strcmp(name, "unsigned short"))     return ty_prim(TY_SHORT);
    if (!strcmp(name, "char"))              return ty_prim(TY_CHAR);
    if (!strcmp(name, "unsigned char"))     return ty_prim(TY_CHAR);
    if (!strcmp(name, "signed int"))        return ty_prim(TY_INT);
    if (!strcmp(name, "signed long"))       return ty_prim(TY_LONG);
    if (!strcmp(name, "signed long long"))  return ty_prim(TY_LONGLONG);
    if (!strcmp(name, "signed short"))      return ty_prim(TY_SHORT);
    if (!strcmp(name, "signed char"))       return ty_prim(TY_CHAR);
    if (!strcmp(name, "float"))             return ty_prim(TY_FLOAT);
    if (!strcmp(name, "double"))            return ty_prim(TY_DOUBLE);
    if (!strcmp(name, "__int64"))           return ty_prim(TY_LONGLONG);
    if (!strcmp(name, "unsigned __int64"))   return ty_prim(TY_LONGLONG);
    if (!strcmp(name, "signed __int64"))    return ty_prim(TY_LONGLONG);
    if (!strcmp(name, "_Bool"))             return ty_prim(TY_BOOL);
    return NULL;
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
        case TY_CHAR:
            if (t->is_const) sb_puts(sb, "const char");
            else             sb_puts(sb, "char");
            return;
        case TY_SHORT:
            if (t->is_const) sb_puts(sb, "const short");
            else             sb_puts(sb, "short");
            return;
        case TY_INT:
            if (t->is_const) sb_puts(sb, "const int");
            else             sb_puts(sb, "int");
            return;
        case TY_LONG:
            if (t->is_const) sb_puts(sb, "const long");
            else             sb_puts(sb, "long");
            return;
        case TY_LONGLONG:
            if (t->is_const) sb_puts(sb, "const long long");
            else             sb_puts(sb, "long long");
            return;
        case TY_FLOAT:
            if (t->is_const) sb_puts(sb, "const float");
            else             sb_puts(sb, "float");
            return;
        case TY_DOUBLE:
            if (t->is_const) sb_puts(sb, "const double");
            else             sb_puts(sb, "double");
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
        case TY_FUNC:
            if (t->base) render_into(sb, t->base);
            else sb_puts(sb, "void");
            sb_puts(sb, "(*)");
            sb_putc(sb, '(');
            for (int i = 0; i < t->nfunc_params; i++) {
                if (i) sb_puts(sb, ", ");
                render_into(sb, t->func_params[i]);
            }
            if (t->func_variadic) {
                if (t->nfunc_params > 0) sb_puts(sb, ", ");
                sb_puts(sb, "...");
            }
            sb_putc(sb, ')');
            return;
        case TY_BITFIELD:
            if (t->base) render_into(sb, t->base);
            else sb_puts(sb, "int");
            sb_puts(sb, " : ");
            {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d", t->bit_width);
                sb_puts(sb, buf);
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
        case TY_CHAR:  sb_puts(sb, cp); sb_puts(sb, "char");    return;
        case TY_SHORT: sb_puts(sb, cp); sb_puts(sb, "short");   return;
        case TY_INT:   sb_puts(sb, cp); sb_puts(sb, "int");     return;
        case TY_LONG:  sb_puts(sb, cp); sb_puts(sb, "long");    return;
        case TY_LONGLONG: sb_puts(sb, cp); sb_puts(sb, "ll");   return;
        case TY_FLOAT: sb_puts(sb, cp); sb_puts(sb, "float");   return;
        case TY_DOUBLE: sb_puts(sb, cp); sb_puts(sb, "double"); return;
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
        case TY_FUNC:
            sb_puts(sb, "func_");
            if (t->base) mangle_into(sb, t->base);
            else sb_puts(sb, "void");
            sb_putc(sb, '_');
            for (int i = 0; i < t->nfunc_params; i++) {
                if (i) sb_putc(sb, '_');
                mangle_into(sb, t->func_params[i]);
            }
            if (t->func_variadic) sb_puts(sb, "_var");
            return;
        case TY_BITFIELD:
            sb_puts(sb, "bf_");
            if (t->base) mangle_into(sb, t->base);
            else sb_puts(sb, "int");
            sb_putc(sb, '_');
            {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d", t->bit_width);
                sb_puts(sb, buf);
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

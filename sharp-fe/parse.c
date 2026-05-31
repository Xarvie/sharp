/*
 * parse.c — Sharp Frontend: Phase 3 Recursive-Descent Parser.
 *
 * Processes SharpTok[] (from lex.h) into AstNode* (from ast.h).
 * No semantic analysis — pure syntax.
 *
 * Key implementation notes:
 *   - Pratt parser for expressions (precedence climbing).
 *   - '>>'/STOK_GTGT split inside parse_generic_args() via pending_close.
 *   - Error recovery syncs to ';' or '}' after each bad statement.
 *   - 'auto x;' (no initialiser) is a parse-phase error.
 *   - Nested defer rejected at parse time.
 */

#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* =========================================================================
 * ParseState
 * ====================================================================== */

/* Typedef-name set, populated at parse time so that the C-correct
 * disambiguation of `( IDENT )` (sub-declarator vs. parameter list with
 * a single typedef-typed parameter) can be made.
 *
 * Scope is per parse_file invocation; the set lives on the PS and is
 * pushed-into every time a `typedef ... NAME;` declarator is built (see
 * parse_top_decl and parse_init_declarator_list).  We do not pop on block
 * exit — once a typedef name is seen at file scope it stays valid for
 * the rest of the translation unit, which is the only context where
 * this disambiguation matters in practice.  Block-scope typedefs are
 * rare and never appear in Lua's headers.
 *
 * Implementation: open-addressing hash table with linear probing,
 * djb2 string hash, automatic 2x grow at 50% load.  Names are stored
 * as heap-owned copies so the set outlives the SharpTok backing
 * buffer. */
typedef struct {
    char  **slots;   /* NULL = empty slot */
    size_t  cap;     /* always a power of two, 0 before first insert */
    size_t  len;
} TdSet;

/* =========================================================================
 * Typedef name set (inline slant hash table)
 * ====================================================================== */

static unsigned td_hash(const char *s) {
    unsigned h = 5381;
    for (; *s; s++) h = ((h << 5) + h) + (unsigned char)*s;
    return h;
}

static int td_slot(char **slots, size_t cap, const char *name) {
    /* Returns the slot index where `name` lives, or where it would be
     * inserted (an empty slot).  Caller distinguishes by checking
     * slots[i] == NULL. */
    unsigned h = td_hash(name);
    size_t mask = cap - 1;
    for (size_t i = h & mask; ; i = (i + 1) & mask) {
        if (!slots[i] || strcmp(slots[i], name) == 0) return (int)i;
    }
}

static void td_grow(TdSet *s) {
    size_t new_cap = s->cap ? s->cap * 2 : 32;
    char **new_slots = calloc(new_cap, sizeof *new_slots);
    if (!new_slots) return;  /* OOM: silently disable further inserts */
    if (s->slots) {
        for (size_t i = 0; i < s->cap; i++) {
            if (!s->slots[i]) continue;
            int j = td_slot(new_slots, new_cap, s->slots[i]);
            new_slots[j] = s->slots[i];
        }
        free(s->slots);
    }
    s->slots = new_slots;
    s->cap   = new_cap;
}

static void td_add(TdSet *s, const char *name) {
    if (!name) return;
    if (!s->cap || s->len * 2 >= s->cap) td_grow(s);
    if (!s->slots) return;
    int i = td_slot(s->slots, s->cap, name);
    if (!s->slots[i]) {
        s->slots[i] = cpp_xstrdup(name);
        if (s->slots[i]) s->len++;
    }
}

static bool td_has(const TdSet *s, const char *name) {
    if (!s->cap || !name) return false;
    int i = td_slot(s->slots, s->cap, name);
    return s->slots[i] != NULL;
}

static void td_remove(TdSet *s, const char *name) {
    if (!s->cap || !name) return;
    int i = td_slot(s->slots, s->cap, name);
    if (s->slots[i]) {
        free((void*)s->slots[i]);
        s->slots[i] = NULL;
        s->len--;
    }
}

/* Same as td_has but takes a pointer-and-length pair, avoiding the
 * malloc that the caller would otherwise need to NUL-terminate a token
 * text slice.  This is the hot-path lookup inside the declarator
 * disambiguator and runs at every `(` position. */
static bool td_has_n(const TdSet *s, const char *name, size_t len) {
    if (!s->cap || !name) return false;
    /* Hash the slice without copying. */
    unsigned h = 5381;
    for (size_t i = 0; i < len; i++)
        h = ((h << 5) + h) + (unsigned char)name[i];
    size_t mask = s->cap - 1;
    for (size_t i = h & mask; ; i = (i + 1) & mask) {
        char *slot = s->slots[i];
        if (!slot) return false;
        /* Match: slot length equals `len` and bytes match. */
        if (strncmp(slot, name, len) == 0 && slot[len] == '\0') return true;
    }
}

static void td_free(TdSet *s) {
    if (!s->slots) return;
    for (size_t i = 0; i < s->cap; i++) free(s->slots[i]);
    free(s->slots);
    s->slots = NULL; s->cap = 0; s->len = 0;
}

/* Add a name given as (text, length) — no NUL-termination required. */
static void td_add_slice(TdSet *s, const char *text, size_t len) {
    if (!text || !len || len >= 256) return;
    char tmp[256];
    memcpy(tmp, text, len);
    tmp[len] = '\0';
    td_add(s, tmp);
}

typedef struct {
    const SharpTok *toks;
    size_t          ntoks;
    size_t          pos;           /* index of current token               */
    const char     *filename;
    FeDiagArr      *diags;
    bool            in_defer;      /* true while inside a defer body       */
    int             pending_close; /* leftover '>' from a split '>>'       */
    /* Multi-variable declarations: parse_init_declarator_list pushes the *first*
     * decl as the function's "return value" and queues the rest here.
     * parse_file / parse_block / for-init drain this before re-entering
     * the recogniser.  This lets multi-decls work without changing every
     * call site's return type. */
    AstVec          pending_decls;
    /* Typedef-name set, populated as we parse typedef declarations.
     * Used by dd_paren_is_subdeclarator to disambiguate `( IDENT )`. */
    TdSet           typedefs;
    /* Struct/union/enum tag names.  Populated alongside typedefs so that
     * bare tag names are recognised as types in Sharp mode, but can be
     * excluded by the IDENT-RPAREN cast heuristic — bare (tag_name) is
     * never a valid cast even in C-superset mode. */
    TdSet           tag_names;
    /* Explicit typedef alias names — names from `typedef ... Name;`.
     * These are always valid for bare casts in IDENT-RPAREN context,
     * even when the struct/union tag has the same name (e.g.
     * `typedef union UValue { ... } UValue;`). */
    TdSet           typedef_aliases;
    /* ── Phase G: Generic name set ─────────────────────────────────────
     * Populated by prescan_generic_defs() before the first top-level
     * declaration is parsed.  Contains the name of every struct/union/
     * function whose definition used generic parameters (< T, U, … >).
     *
     * Use: when the parser sees  IDENT '<'  in an expression context it
     * checks generic_names first.  If the name is present the '<' is
     * unambiguously a generic argument list opener; if it is absent the
     * '<' must be a comparison operator.  This eliminates all speculation
     * and prevents false-positive or false-negative disambiguation.
     *
     * Lifetime: allocated by prescan_generic_defs, freed by parse_file. */
    TdSet           generic_names;
    /* Counter for anonymous struct/union synthetic names.  Used when a
     * `typedef struct { ... } Name;` form is encountered: the inner
     * struct has no source-level tag, so we synthesise one from this
     * counter and emit it as a normal AST_STRUCT_DEF.  The C compiler
     * sees a tagged struct in the generated output, semantically
     * identical to the original anonymous form. */
    unsigned        anon_struct_counter;
    /* Nesting depth inside struct/union bodies.  When > 0, struct
     * definitions encountered in tspec_try_consume are nested (e.g.
     * `struct sColMap { ... } aCol[1]` inside `struct FKey { ... }`). */
    int             struct_body_depth;
    /* attribute text captured by parse_declarator when an
     * __attribute__ follows the declarator name.  Picked up by parse_top_decl
     * to detect transparent_union typedefs. Owned, freed after each use. */
    char           *last_decl_attr;
    /* C3: set to true by parse_param_list_inner when `()` (empty parens)
     * is seen, so the caller (parse_top_decl / parse_declarator) can set
     * func_def.params_unspecified on the resulting AST node. */
    bool            last_params_unspecified;
    /* C8: set when the declarator name came from a paren group `(name)`. */
    bool            last_name_paren;
} PS;

/* =========================================================================
 * Phase G — Pre-scan: build generic_names set.
 *
 * A single O(n) pass over the complete token stream (post-CPP, so all
 * #include-expanded tokens are present) that identifies every struct,
 * union, and function defined with generic parameters.  The result is
 * stored in ps->generic_names and used by parse_primary to resolve the
 * IDENT '<' ambiguity without any speculation / backtracking.
 *
 * Patterns detected:
 *
 *   Pattern 1 — generic struct/union:
 *     (struct|union) NAME '<' IDENT (',' | '>' | '>>') …
 *     → NAME is a generic type name
 *
 *   Pattern 2 — prefix-style generic free function:
 *     At brace-depth 0, '<' IDENT (',' | '>' | '>>') found at a
 *     declaration boundary (after ';' / '}' / at file start).
 *     We then scan forward past the closing '>' and the return-type
 *     tokens to find the first IDENT followed by '(' — that is the
 *     function name.
 *     → that IDENT is a generic function name
 *
 * The pre-scan intentionally does NOT parse — it only pattern-matches
 * at the token level.  It may miss contrived edge cases (typedef-
 * aliased generic structs, function pointers to generic functions) but
 * covers every normal Sharp generic definition.
 * ====================================================================== */
static void prescan_generic_defs(PS *ps) {
    size_t saved = ps->pos;
    ps->pos = 0;
    int brace_depth = 0;

    /* Track whether we are at a "declaration-start" position: just after
     * ';' / '}' / at position 0 (i.e. top-level context at depth 0). */
    bool at_decl_start = true;

    while (ps->pos < ps->ntoks) {
        SharpTok t = ps->toks[ps->pos];
        if (t.kind == STOK_EOF) break;

        /* ── brace depth bookkeeping ── */
        if (t.kind == STOK_LBRACE) {
            brace_depth++;
            at_decl_start = false;
            ps->pos++;
            continue;
        }
        if (t.kind == STOK_RBRACE) {
            if (brace_depth > 0) brace_depth--;
            at_decl_start = (brace_depth == 0);  /* after '}' at top level */
            ps->pos++;
            continue;
        }
        if (t.kind == STOK_SEMI) {
            at_decl_start = (brace_depth == 0);
            ps->pos++;
            continue;
        }

        /* ── Pattern 1: (struct | union) NAME '<' IDENT (',' | '>' | '>>') ── */
        if ((t.kind == STOK_STRUCT || t.kind == STOK_UNION || t.kind == STOK_CLASS) &&
            ps->pos + 4 < ps->ntoks) {
            SharpTok t1 = ps->toks[ps->pos + 1];
            SharpTok t2 = ps->toks[ps->pos + 2];
            SharpTok t3 = ps->toks[ps->pos + 3];
            SharpTokKind k4 = ps->toks[ps->pos + 4].kind;
            if (t1.kind == STOK_IDENT && t2.kind == STOK_LT &&
                t3.kind == STOK_IDENT &&
                (k4 == STOK_COMMA || k4 == STOK_GT || k4 == STOK_GTGT)) {
                td_add_slice(&ps->generic_names, t1.text, t1.len);
            }
        }

        /* ── Pattern 2: prefix generic function  '<' IDENT … at depth 0 ── */
        if (at_decl_start && brace_depth == 0 &&
            t.kind == STOK_LT && ps->pos + 2 < ps->ntoks) {
            SharpTok t1 = ps->toks[ps->pos + 1];
            SharpTokKind k2 = ps->toks[ps->pos + 2].kind;
            if (t1.kind == STOK_IDENT &&
                (k2 == STOK_COMMA || k2 == STOK_GT || k2 == STOK_GTGT)) {
                /* Looks like a generic-param list.  Find the matching '>',
                 * then skip past the return type to find  IDENT '(' . */
                size_t j = ps->pos + 1;
                int depth = 1;
                while (j < ps->ntoks && depth > 0 &&
                       ps->toks[j].kind != STOK_EOF) {
                    SharpTokKind k = ps->toks[j].kind;
                    if (k == STOK_LT) depth++;
                    else if (k == STOK_GT)   { depth--; }
                    else if (k == STOK_GTGT) { depth -= 2; }
                    j++;
                }
                /* j is now just past the closing '>'.
                 * Walk forward skipping type tokens until we hit IDENT '('. */
                size_t k_pos = j;
                while (k_pos + 1 < ps->ntoks) {
                    SharpTokKind ka = ps->toks[k_pos].kind;
                    SharpTokKind kb = ps->toks[k_pos + 1].kind;
                    if (ka == STOK_IDENT && kb == STOK_LPAREN) {
                        td_add_slice(&ps->generic_names,
                                     ps->toks[k_pos].text,
                                     ps->toks[k_pos].len);
                        break;
                    }
                    /* Stop at tokens that can't be part of a return type */
                    if (ka == STOK_LBRACE || ka == STOK_SEMI ||
                        ka == STOK_RBRACE || ka == STOK_EOF)
                        break;
                    k_pos++;
                }
            }
        }

        at_decl_start = false;
        ps->pos++;
    }

    ps->pos = saved;
}

/* =========================================================================
 * Primitives
 * ====================================================================== */

static SharpTok ps_peek(const PS *ps) {
    size_t i = ps->pos < ps->ntoks ? ps->pos : ps->ntoks;
    return ps->toks[i];
}
static SharpTok ps_peek2(const PS *ps) {
    size_t i = (ps->pos + 1) < ps->ntoks ? ps->pos + 1 : ps->ntoks;
    return ps->toks[i];
}
static SharpTok ps_peek3(const PS *ps) {
    size_t i = (ps->pos + 2) < ps->ntoks ? ps->pos + 2 : ps->ntoks;
    return ps->toks[i];
}
/* Peek at an arbitrary offset from current position. */
static SharpTok ps_peek_at(const PS *ps, int offset) {
    size_t i = (ps->pos + offset) < ps->ntoks ? ps->pos + offset : ps->ntoks;
    return ps->toks[i];
}
static SharpTok ps_advance(PS *ps) {
    SharpTok t = ps_peek(ps);
    if (ps->pos < ps->ntoks) ps->pos++;
    return t;
}
static bool ps_at(const PS *ps, SharpTokKind k) {
    return ps_peek(ps).kind == k;
}
static bool ps_match(PS *ps, SharpTokKind k) {
    if (ps_at(ps, k)) { ps_advance(ps); return true; }
    return false;
}

static void ps_error(PS *ps, CppLoc loc, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *msg = malloc((size_t)(n + 1));
    if (!msg) { perror("sharp-fe parse"); abort(); }
    va_start(ap, fmt);
    vsnprintf(msg, (size_t)(n + 1), fmt, ap);
    va_end(ap);
    CppDiag d = { CPP_DIAG_ERROR, loc, msg };
    fe_diag_push(ps->diags, d);
}

static SharpTok ps_expect(PS *ps, SharpTokKind k, const char *ctx) {
    SharpTok t = ps_peek(ps);
    if (t.kind == k) { ps_advance(ps); return t; }
    ps_error(ps, t.loc, "expected %s %s, got '%.*s'",
             lex_tok_kind_name(k), ctx, (int)t.len, t.text);
    /* return a synthetic token of the expected kind at current position */
    SharpTok fake = t;
    fake.kind = k;
    return fake;
}

/* Sync past the next ';' or '}' for error recovery.
 * Also stops on declaration introducers to avoid eating the next decl. */
static void ps_sync(PS *ps) {
    while (!ps_at(ps, STOK_EOF)) {
        SharpTokKind k = ps_peek(ps).kind;
        if (k == STOK_SEMI) { ps_advance(ps); return; }
        if (k == STOK_RBRACE) return;   /* don't consume the '}' */
        /* declaration introducers — stop before the next top-level decl */
        if (k == STOK_TYPEDEF || k == STOK_EXTERN  || k == STOK_STATIC ||
            k == STOK_STRUCT  || k == STOK_UNION   || k == STOK_ENUM   ||
            k == STOK_CLASS)
            return;
        ps_advance(ps);
    }
}

static void rollback_diags(PS *ps, size_t save_len) {
    if (ps->diags && ps->diags->len > save_len) {
        for (size_t i = save_len; i < ps->diags->len; i++)
            free(ps->diags->data[i].msg);
        ps->diags->len = save_len;
    }
}

static void skip_balanced_parens(PS *ps) {
    int depth = 0;
    while (!ps_at(ps, STOK_EOF)) {
        SharpTokKind k = ps_peek(ps).kind;
        if (k == STOK_LPAREN) { depth++; ps_advance(ps); }
        else if (k == STOK_RPAREN) {
            if (depth == 0) { ps_advance(ps); break; }
            depth--; ps_advance(ps);
        } else { ps_advance(ps); }
    }
}

static int collect_balanced_call_body(PS *ps, char *buf, int buf_size) {
    int blen = 0;
    int depth = 0;
    while (!ps_at(ps, STOK_EOF)) {
        SharpTok tk = ps_peek(ps);
        if (tk.kind == STOK_LPAREN) {
            depth++;
            if (blen + (int)tk.len + 1 < buf_size) {
                memcpy(buf + blen, tk.text, tk.len);
                blen += tk.len;
            }
            ps_advance(ps);
        } else if (tk.kind == STOK_RPAREN) {
            if (depth == 0) {
                blen += snprintf(buf + blen, buf_size - blen, ")");
                ps_advance(ps);
                break;
            }
            depth--;
            if (blen + (int)tk.len + 1 < buf_size) {
                memcpy(buf + blen, tk.text, tk.len);
                blen += tk.len;
            }
            ps_advance(ps);
        } else {
            if (blen + (int)tk.len + 2 < buf_size) {
                if (blen > 0) {
                    char last = buf[blen - 1];
                    if (last != '(' && last != ',')
                        buf[blen++] = ' ';
                }
                memcpy(buf + blen, tk.text, tk.len);
                blen += tk.len;
            }
            ps_advance(ps);
        }
    }
    if (blen >= buf_size) blen = buf_size - 1;
    buf[blen] = '\0';
    return blen;
}

/* Check if a name is a known type (typedef or struct/union tag).
 * Needed because is_type_start must distinguish `Vec<int> vi;` (declaration)
 * from `swap<int>(&x, &y)` (generic function call). */
static bool name_is_known_type(PS *ps, const char *name, size_t len) {
    if (td_has_n(&ps->typedefs, name, len))
        return true;
    for (size_t i = 0; i < ps->pending_decls.len; i++) {
        AstNode *pd = ps->pending_decls.data[i];
        if (pd->kind == AST_STRUCT_DEF &&
            pd->u.struct_def.name &&
            strlen(pd->u.struct_def.name) == len &&
            memcmp(pd->u.struct_def.name, name, len) == 0)
            return true;
    }
    return false;
}

/* =========================================================================
 * is_type_start: can the current token begin a type expression
 *                or a declaration (with storage-class specifier)?
 * ====================================================================== */
static bool is_type_start(PS *ps) {
    SharpTokKind k  = ps_peek(ps).kind;
    SharpTokKind k2 = ps_peek2(ps).kind;
    switch (k) {
    /* Storage-class specifiers begin a declaration but not a type
     * expression.  parse_decl_specifiers handles them; pure type-name
     * positions (cast, sizeof, generic args) call parse_type which
     * never sees these tokens because parse_top_decl / parse_stmt
     * have already routed past this check. */
    case STOK_STATIC: case STOK_EXTERN: case STOK_REGISTER:
    case STOK_TYPEDEF: case STOK_INLINE: case STOK_VOLATILE:
    case STOK_RESTRICT: case STOK__ATOMIC: case STOK_TYPEOF:
    /* Clang nullability annotations — macOS / iOS system headers */
    case STOK__NULLABLE: case STOK__NONNULL: case STOK__NULL_UNSPEC:
    /* GCC `__attribute__((…))` may legally lead a declaration:
     *   `__attribute__((unused)) static int counter;`
     * Treat it as a decl-specifier opener so parse_top_decl /
     * parse_stmt route into the declarator path. */
    case STOK_ATTRIBUTE:
    case STOK_DECLSPEC:
    case STOK_ASM:
    /* C11 keywords that can lead a declaration */
    case STOK__NORETURN: case STOK__ALIGNAS: case STOK__THREAD_LOCAL:
    /* C23: constexpr can lead a declaration */
    case STOK_CONSTEXPR:
        return true;
    case STOK_CONST: case STOK_STRUCT: case STOK_UNION: case STOK_ENUM: case STOK_CLASS:
    case STOK_VOID:  case STOK_INT:   case STOK_CHAR:  case STOK_LONG:
    case STOK_SHORT: case STOK_FLOAT: case STOK_DOUBLE:
    case STOK_SIGNED:case STOK_UNSIGNED: case STOK__BOOL:
    case STOK_AUTO:
        return true;
    case STOK_IDENT:
        /* IDENT IDENT          → user type, var name (Buffer buf)
         * IDENT *              → user type, pointer (Buffer *p)
         * IDENT operator …     → free-function operator
         * IDENT < TYPE-LIKE …  → generic type (Vec<int>, Pair<K,V>)
         * IDENT )              → only when IDENT is a typedef name —
         *                        this is the cast `(typedef_name)expr`
         *                        form.  Without the typedef-set probe
         *                        we'd misparse `(x)y` where x is just
         *                        a variable.  The typedef set is
         *                        populated as parse encounters typedef
         *                        declarations earlier in the same TU.
         *
         * The `<` case must distinguish generic types from comparisons
         * like `i < 5`.  Heuristic: the token after `<` must look like
         * the start of a type — a primitive-type keyword, IDENT, or
         * `const`.  Numeric literals, `(`, etc. fall through. */
        if (k2 == STOK_IDENT || k2 == STOK_STAR || k2 == STOK_OPERATOR)
            return true;
        /* IDENT < TYPE-LIKE … → generic type (Vec<int>, Pair<K,V>).
         * Distinguish from comparisons: the token after '<' must look
         * like the start of a type, AND the IDENT must be a known typedef.
         * Without the typedef check, statements like `swap<float>(...)`
         * would be misparsed as declarations. */
        if (k2 == STOK_LT && td_has_n(&ps->typedefs, ps_peek(ps).text, ps_peek(ps).len)) {
            SharpTok k3 = ps_peek3(ps);
            if (k3.kind == STOK_IDENT || k3.kind == STOK_CONST ||
                k3.kind == STOK_VOLATILE || k3.kind == STOK_AUTO ||
                k3.kind == STOK_INT || k3.kind == STOK_CHAR ||
                k3.kind == STOK_SHORT || k3.kind == STOK_LONG ||
                k3.kind == STOK_FLOAT || k3.kind == STOK_DOUBLE ||
                k3.kind == STOK__BOOL || k3.kind == STOK_VOID ||
                k3.kind == STOK_SIGNED || k3.kind == STOK_UNSIGNED)
                return true;
        }
        if (k2 == STOK_RPAREN) {
            SharpTok t = ps_peek(ps);
            /* An explicit typedef alias (e.g. from `typedef struct
             * { ... } Foo;`) is always safe as a cast target —
             * `(Foo)expr` is a valid C cast. */
            if (td_has_n(&ps->typedef_aliases, t.text, t.len))
                return true;
            /* A bare struct/union/enum tag with no explicit typedef
             * alias is never a valid cast target without the
             * 'struct'/'union' keyword.  e.g. `struct epollop {…}`;
             * `return (epollop);` — parenthesised var, NOT a cast.
             *
             * In Sharp, as in standard C:
             *   - struct name is only in tag_names, not in typedefs
             *   - only typedef brings it into typedefs
             * So if it's in tag_names but NOT in typedefs → not a type
             * If it's in both → it is a type (from typedef) */
            if (td_has_n(&ps->tag_names, t.text, t.len) &&
                !td_has_n(&ps->typedefs, t.text, t.len))
                return false;
            return td_has_n(&ps->typedefs, t.text, t.len);
        }
        /* IDENT '(' — only a type-start when the IDENT is a
         * typedef-name and the parenthesis opens a parenthesised
         * declarator (e.g. `sqlite3_syscall_ptr (*x)(args);` — typedef
         * as the return type of a function-pointer field).  Without
         * the typedef-set probe we'd consume calls like `f(x)` as
         * declarations.  sqlite3.c hits this dozens of times. */
        if (k2 == STOK_LPAREN) {
            SharpTok t2 = ps_peek(ps);
            return name_is_known_type(ps, t2.text, t2.len);
        }
        /* IDENT followed by a type-qualifier (const, volatile,
         * restrict) — this is the postfix-qualifier form `Mt const *p`,
         * equivalent to `const Mt *p` per ISO C99 §6.7.3.  Same gate
         * as IDENT '(' / IDENT ')': only a type-start when IDENT is a
         * known typedef-name.  sqlite3.c has many `T const *` fields
         * and casts (e.g. `sqlite3_io_methods const *pMethod`). */
        if (k2 == STOK_CONST || k2 == STOK_VOLATILE ||
            k2 == STOK_RESTRICT || k2 == STOK_ATTRIBUTE ||
            k2 == STOK_ASM || k2 == STOK_DECLSPEC) {
            SharpTok t3 = ps_peek(ps);
            return name_is_known_type(ps, t3.text, t3.len);
        }
        if (k2 == STOK_LT) {
            SharpTok t4 = ps_peek(ps);
            if (!name_is_known_type(ps, t4.text, t4.len))
                return false;
            SharpTokKind k3 = ps_peek3(ps).kind;
            switch (k3) {
            case STOK_CONST: case STOK_STRUCT: case STOK_CLASS:
            case STOK_VOID:  case STOK_INT:   case STOK_CHAR:
            case STOK_LONG:  case STOK_SHORT: case STOK_FLOAT:
            case STOK_DOUBLE:case STOK_SIGNED:case STOK_UNSIGNED:
            case STOK__BOOL: case STOK_IDENT:
                return true;
            default:
                return false;  /* `i < 5`, `x < (a+b)` etc. */
            }
        }
        return false;
    default:
        return false;
    }
}

/* =========================================================================
 * Forward declarations
 * ====================================================================== */
static AstNode *parse_type(PS *ps);
static AstNode *parse_type_unqual(PS *ps);
/* needed by parse_type's abstract function-pointer
 * declarator handling (defined later in the file). */
/* parse_direct_declarator needed by apply_suffix in parse_type
 * for doubly-nested abstract declarators: void(*(*)(T*))(U). */
static AstNode *parse_direct_declarator(PS *ps, AstNode *base, char **out_name);
static void parse_param_list_inner(PS *ps, AstVec *out);
static AstNode *parse_struct_def(PS *ps);
static AstNode *parse_enum_def(PS *ps);
static AstNode *parse_expr(PS *ps);
static AstNode *parse_expr_prec(PS *ps, int min_prec);
static AstNode *parse_stmt(PS *ps);
static AstNode *parse_block(PS *ps);
static AstNode *parse_top_decl(PS *ps);

/* Multi-variable / array-suffix declaration helper.
 *
 * Caller has already parsed `base_ty` (e.g. the `int` in `int x = 3, y = 7;`)
 * and the cursor is positioned at the first declarator's IDENT (no name yet
 * consumed).  Behaviour:
 *   - Loops over comma-separated declarators.
 *   - For each declarator: parses IDENT, optional `[size]` array suffix
 *     (multi-dim allowed), and optional `= initialiser`.
 *   - Initialisers are parsed at precedence 2 to *exclude* the comma operator
 *     (otherwise `int x = 3, y = 7` would be one comma expression).
 *   - The first declarator owns `base_ty`; subsequent declarators each get a
 *     deep clone via ast_clone_type() so ast_node_free() does not double-free.
 *   - Consumes the trailing ';'.
 *
 * Return value:
 *   - The first declarator's node (caller's "primary" result).
 *   - All *additional* declarators are pushed to ps->pending_decls so that
 *     parse_file / parse_block can drain them before parsing the next item.
 *
 * If `stmt_wrap` is true, decls are wrapped in AST_DECL_STMT (statement
 * context); otherwise the bare AST_VAR_DECL is produced (file/top-level). */
static AstNode *parse_array_suffix(PS *ps, AstNode *inner_ty);
static AstNode *parse_init_list(PS *ps);
static AstNode *finish_func(PS *ps, AstNode *ret_type, SharpTok name_tok, bool is_operator);

/* =========================================================================
 * GCC `__attribute__((…))` and `__asm__("name")` swallow
 *
 * GCC and Clang accept attribute specifiers in many declarator positions:
 *
 *   __attribute__((noreturn)) void abort(void);
 *   void *malloc(size_t) __attribute__((malloc));
 *   struct { int x __attribute__((aligned(8))); } s;
 *   typedef int int4 __attribute__((vector_size(16)));
 *
 * They also accept `__asm__("name")` (and the equivalent `asm("name")`,
 * `__asm("name")`) as a declarator suffix that renames the linker
 * symbol — used heavily by glibc's `<sys/cdefs.h>`:
 *
 *   extern int foo(int) __asm__("__xpg_foo");
 *
 * `__attribute__((...))` text is now optionally captured rather
 * than silently discarded.  When `out_attrs` is non-NULL, the full text
 * of every `__attribute__((...))` found is appended to `*out_attrs`
 * (a heap-allocated, space-separated string; NULL on first append).
 * The text is reconstructed by concatenating the raw `.text` fields of
 * the consumed tokens — no reformatting, so the result is semantically
 * identical to the original source text.
 *
 * `__asm__("name")` linker-rename suffixes are always silently discarded
 * regardless of `out_attrs`; they name linker symbols that the generated
 * C resolves from its own headers and must not be re-emitted.
 *
 * Attribute syntax: `__attribute__` `(` `(` BALANCED `)` `)`
 * Asm-name syntax:  `__asm__` `(` BALANCED `)`
 * Both are eaten in any order, repeatedly, at the call site.
 *
 * Returns true if at least one specifier was eaten. */

/* Append the raw text of tokens ps->toks[from..to) to *out, allocating
 * or growing the heap string as needed.  A space separator is inserted
 * between successive calls. */
static void attrs_append_tokens(char **out, const PS *ps,
                                size_t from, size_t to) {
    /* Build attribute text: no spaces adjacent to ( ) , for compact form. */
    size_t chunk_len = 0;
    for (size_t i = from; i < to; i++) {
        chunk_len += ps->toks[i].len;
        if (i + 1 < to) {
            SharpTokKind ci = ps->toks[i].kind, ni = ps->toks[i+1].kind;
            bool no_sp = (ci == STOK_LPAREN || ci == STOK_RPAREN ||
                          ni == STOK_LPAREN || ni == STOK_RPAREN ||
                          ci == STOK_COMMA  || ni == STOK_COMMA);
            if (!no_sp) chunk_len++;
        }
    }
    if (chunk_len == 0) return;
    char *buf = malloc(chunk_len + 1);
    size_t off = 0;
    for (size_t i = from; i < to; i++) {
        memcpy(buf + off, ps->toks[i].text, ps->toks[i].len);
        off += ps->toks[i].len;
        if (i + 1 < to) {
            SharpTokKind ci = ps->toks[i].kind, ni = ps->toks[i+1].kind;
            bool no_sp = (ci == STOK_LPAREN || ci == STOK_RPAREN ||
                          ni == STOK_LPAREN || ni == STOK_RPAREN ||
                          ci == STOK_COMMA  || ni == STOK_COMMA);
            if (!no_sp) buf[off++] = ' ';
        }
    }
    buf[off] = '\0';
    if (!*out) {
        *out = buf;
    } else {
        size_t ex = strlen(*out);
        *out = realloc(*out, ex + 1 + chunk_len + 1);
        if (!*out) abort();
        (*out)[ex] = ' ';
        memcpy(*out + ex + 1, buf, chunk_len + 1);
        free(buf);
    }
}

static bool eat_attribute_specifiers(PS *ps, char **out_attrs) {
    bool ate_any = false;
    for (;;) {
        if (ps_at(ps, STOK_ATTRIBUTE)) {
            size_t start = ps->pos;
            ps_advance(ps);
            if (!ps_match(ps, STOK_LPAREN)) {
                if (out_attrs) attrs_append_tokens(out_attrs, ps, start, ps->pos);
                ate_any = true; continue;
            }
            if (!ps_match(ps, STOK_LPAREN)) {
                if (out_attrs) attrs_append_tokens(out_attrs, ps, start, ps->pos);
                ate_any = true; continue;
            }
            skip_balanced_parens(ps);
            ps_match(ps, STOK_RPAREN);
            if (out_attrs) attrs_append_tokens(out_attrs, ps, start, ps->pos);
            ate_any = true; continue;
        }
        if (ps_at(ps, STOK_ASM)) {
            size_t asm_start = ps->pos;
            ps_advance(ps);
            if (!ps_match(ps, STOK_LPAREN)) { ate_any = true; continue; }
            skip_balanced_parens(ps);
            /* p46: preserve asm rename text verbatim */
            if (out_attrs) attrs_append_tokens(out_attrs, ps, asm_start, ps->pos);
            ate_any = true; continue;
        }
        if (ps_at(ps, STOK_DECLSPEC)) {
            size_t decl_start = ps->pos;
            ps_advance(ps);
            if (!ps_match(ps, STOK_LPAREN)) { ate_any = true; continue; }
            skip_balanced_parens(ps);
            if (out_attrs) attrs_append_tokens(out_attrs, ps, decl_start, ps->pos);
            ate_any = true; continue;
        }
        break;
    }
    return ate_any;
}


/* =========================================================================
 * Type parsing
 * ====================================================================== */

/* Parse generic argument list <T1, T2, ...>.
 * pending_close: how many extra '>' are available from a split '>>'.
 * Returns true + fills args on success, false (no advance) on failure. */
static bool parse_generic_args(PS *ps, AstVec *args) {
    /* expect '<' */
    if (!ps_at(ps, STOK_LT)) return false;
    size_t save_pos = ps->pos;
    int save_pending = ps->pending_close;
    /* Snapshot diagnostics length so we can roll back any errors emitted
     * during the speculative parse_type() inside this trial. */
    size_t save_diag_len = ps->diags ? ps->diags->len : 0;
    ps_advance(ps);  /* consume '<' */

    for (;;) {
        /* empty arg list '<>' not valid; must have at least one type */
        if (ps_at(ps, STOK_GT) || ps_at(ps, STOK_EOF)) break;

        /* Cheap pre-check: only attempt parse_type() if the next token can
         * actually start a type.  This prevents `i < 5` from being mis-
         * parsed as a generic-arg list (which would otherwise produce a
         * spurious "expected type, got '5'" diagnostic). */
        SharpTokKind k = ps_peek(ps).kind;
        bool ok_start = (k == STOK_CONST  || k == STOK_STRUCT || k == STOK_CLASS ||
                         k == STOK_VOID   || k == STOK_INT    ||
                         k == STOK_CHAR   || k == STOK_LONG   ||
                         k == STOK_SHORT  || k == STOK_FLOAT  ||
                         k == STOK_DOUBLE || k == STOK_SIGNED ||
                         k == STOK_UNSIGNED || k == STOK__BOOL ||
                         k == STOK_AUTO   || k == STOK_IDENT);
        if (!ok_start) break;

        AstNode *arg = parse_type(ps);
        if (!arg) { ps->pos = save_pos; astvec_free(args); return false; }
        astvec_push(args, arg);

        if (ps_at(ps, STOK_COMMA)) { ps_advance(ps); continue; }

        /* close: '>' or '>>' (split) */
        if (ps_at(ps, STOK_GT)) { ps_advance(ps); return true; }
        if (ps_at(ps, STOK_GTGT)) {
            /* Phase 3 P1.8: split '>>'.
             * One '>' closes this level; one goes to ps->pending_close. */
            ps_advance(ps);
            ps->pending_close++;
            return true;
        }
        /* Consume a '>' left over from an enclosing '>>' split. */
        if (ps->pending_close > 0) {
            ps->pending_close--;
            return true;
        }
        break;  /* unexpected token — bail */
    }
    /* rollback on failure */
    ps->pos = save_pos;
    ps->pending_close = save_pending;
    for (size_t i = 0; i < args->len; i++) ast_node_free(args->data[i]);
    astvec_free(args);
    *args = (AstVec){0};
    /* Roll back any diagnostics emitted during the failed trial parse. */
    rollback_diags(ps, save_diag_len);
    return false;
}

/* =========================================================================
 * Type parsing
 * ====================================================================== */

static AstNode *parse_type(PS *ps) {
    SharpTok t = ps_peek(ps);
    AstNode *base = NULL;

    /* GCC __attribute__(…) may lead a cast or sizeof type expression, e.g.
     *   ((ATTR int(*)(void)) ptr)()   where ATTR = __attribute__((__noinline__))
     * Consume and discard any leading attributes before parsing the actual type.
     * The attribute text is not carried into the AST here (cg will not see it
     * from the cast-type node, but that is acceptable for C pass-through). */
    if (t.kind == STOK_ATTRIBUTE) {
        eat_attribute_specifiers(ps, NULL);
        t = ps_peek(ps);
    }

    /* const T — const wraps only the base type; pointer suffixes apply to
     * the const-qualified result.  Examples:
     *   const char*   → PTR(CONST(char))    — pointer to const char
     *   char* const   → CONST(PTR(char))    — const pointer to char
     * We parse: CONST( base-type ), then handle '*' and 'const' suffixes. */
    if (t.kind == STOK_CONST) {
        ps_advance(ps);
        /* Handle `const volatile T`: volatile follows const
         * Consume a trailing volatile and nest it INSIDE the const so that
         * `const volatile int` → CONST(VOLATILE(int)) → emits `const volatile int`
         * (source order preserved).  Without this, `parse_type_unqual` sees
         * `volatile` and errors; volatile would be lost or order reversed. */
        bool has_volatile = ps_at(ps, STOK_VOLATILE);
        if (has_volatile) ps_advance(ps);
        AstNode *inner = parse_type_unqual(ps);
        if (has_volatile) {
            AstNode *vn = ast_node_new(AST_TYPE_VOLATILE, t.loc);
            vn->u.type_volatile.base = inner;
            inner = vn;
        }
        AstNode *cn = ast_node_new(AST_TYPE_CONST, t.loc);
        cn->u.type_const.base = inner;
        base = cn;
        goto apply_suffix;
    }

    /* volatile T — symmetric with const.  Sharp transpiles volatile
     * straight through to the generated C; semantically it has no effect
     * on Sharp's interned Type* but is preserved for correctness.
     * recurse into parse_type (not parse_type_unqual) to handle
     * `volatile const T`, `volatile restrict T`, etc. */
    if (t.kind == STOK_VOLATILE) {
        ps_advance(ps);
        AstNode *inner = parse_type(ps);  /* allow volatile const T */
        AstNode *vn = ast_node_new(AST_TYPE_VOLATILE, t.loc);
        vn->u.type_volatile.base = inner;
        return vn;  /* apply_suffix already done by recursive parse_type */
    }

    /* typeof / __typeof__(expr) — GCC typeof extension.
     * In a type context, typeof(E) means "the type of expression E".
     * We collect all tokens up to the matching ')' and store the
     * expression verbatim as an AST_TYPE_NAME node.  The sema phase
     * will try to resolve the type; if it can't, we fall back to int.
     * This handles the common case: `__typeof__(fread) *` → function
     * pointer type.  For our purposes, we store the whole `typeof(expr)`
     * as a verbatim type name so cc can resolve it in the generated C. */
    if (t.kind == STOK_TYPEOF) {
        CppLoc loc = t.loc;
        ps_advance(ps);  /* eat typeof/__typeof__ */
        if (ps_at(ps, STOK_LPAREN)) {
            /* Collect everything from '(' to matching ')' verbatim */
            char buf[256]; int blen = 0;
            blen += snprintf(buf + blen, sizeof buf - blen, "__typeof__(");
            ps_advance(ps);  /* eat '(' */
            int depth = 0;
            while (!ps_at(ps, STOK_EOF)) {
                SharpTok tk = ps_peek(ps);
                if (tk.kind == STOK_LPAREN) {
                    depth++;
                    if (blen + (int)tk.len + 1 < (int)sizeof buf) {
                        memcpy(buf + blen, tk.text, tk.len); blen += tk.len;
                    }
                    ps_advance(ps);
                } else if (tk.kind == STOK_RPAREN) {
                    if (depth == 0) {
                        if (blen + 2 < (int)sizeof buf) buf[blen++] = ')';
                        ps_advance(ps); break;
                    }
                    depth--;
                    if (blen + (int)tk.len + 1 < (int)sizeof buf) {
                        memcpy(buf + blen, tk.text, tk.len); blen += tk.len;
                    }
                    ps_advance(ps);
                } else {
                    if (blen > 0 && blen + (int)tk.len + 2 < (int)sizeof buf) {
                        if (buf[blen-1] != '(' && buf[blen-1] != ',')
                            buf[blen++] = ' ';
                        memcpy(buf + blen, tk.text, tk.len); blen += tk.len;
                    } else if (blen + (int)tk.len + 1 < (int)sizeof buf) {
                        memcpy(buf + blen, tk.text, tk.len); blen += tk.len;
                    }
                    ps_advance(ps);
                }
            }
            buf[blen] = '\0';
            AstNode *tn = ast_node_new(AST_TYPE_NAME, loc);
            tn->u.type_name.name = cpp_xstrdup(buf);
            base = tn;
            goto apply_suffix;
        }
        /* No '(' — treat as void (shouldn't happen in valid C) */
        base = ast_node_new(AST_TYPE_VOID, loc);
        goto apply_suffix;
    }

    /* C11 _Atomic qualifier — preserved verbatim in the C output. */
    if (t.kind == STOK__ATOMIC) {
        ps_advance(ps);
        AstNode *inner;
        bool is_paren = false;
        if (ps_at(ps, STOK_LPAREN)) {
            is_paren = true;
            ps_advance(ps);
            inner = parse_type(ps);
            ps_expect(ps, STOK_RPAREN, "_Atomic(T) closing ')'");
        } else {
            inner = parse_type(ps);
        }
        AstNode *an = ast_node_new(AST_TYPE_ATOMIC, t.loc);
        an->u.type_atomic.base     = inner;
        an->u.type_atomic.is_paren  = is_paren;
        base = an;
        goto apply_suffix;
    }

    /* auto */
    if (t.kind == STOK_AUTO) {
        ps_advance(ps);
        return ast_node_new(AST_TYPE_AUTO, t.loc);
    }

    /* void.  The trailing `*` and qualifier suffixes still apply
     * (`void *p`, `void * const restrict`), so we route through
     * apply_suffix rather than returning immediately. */
    if (t.kind == STOK_VOID) {
        ps_advance(ps);
        base = ast_node_new(AST_TYPE_VOID, t.loc);
        goto apply_suffix;
    }

    /* struct / union as a type-specifier.  Three shapes accepted:
     *
     *   struct Tag                     — bare tag reference; the type
     *                                    resolves through scope to an
     *                                    earlier AST_STRUCT_DEF.
     *   struct Tag { fields… }         — inline tagged definition;
     *                                    parsed via parse_struct_def
     *                                    and queued to pending_decls so
     *                                    it surfaces at file scope.
     *   struct { fields… }             — inline anonymous definition;
     *                                    parse_struct_def synthesises a
     *                                    unique tag.
     *
     * The same three apply to `union`.  All three appear in real C code:
     * libc's <bits/mbstate_t.h> uses `union { ... } __value;` inside a
     * struct body; Lua's lobject.h uses `struct { ... } tbclist;` inside
     * a union; and `typedef struct Tag { ... } Alias;` is everywhere. */
    if (t.kind == STOK_STRUCT || t.kind == STOK_UNION || t.kind == STOK_CLASS) {
        bool is_union = (t.kind == STOK_UNION);
        SharpTokKind k2 = ps_peek2(ps).kind;
        SharpTokKind k3 = ps_peek3(ps).kind;
        /* Inline body — anonymous `struct {` or named `struct Tag {` */
        bool inline_anon  = (k2 == STOK_LBRACE);
        bool inline_named = (k2 == STOK_IDENT && k3 == STOK_LBRACE);
        if (inline_anon || inline_named) {
            AstNode *sd = parse_struct_def(ps);
            AstNode *n  = ast_node_new(AST_TYPE_NAME, t.loc);
            n->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
            n->u.type_name.is_struct_tag = true; /* struct/union keyword was explicit */
            astvec_push(&ps->pending_decls, sd);
            base = n;
            goto try_generic;
        }
        /* Bare tag reference: `struct Tag` */
        ps_advance(ps);
        SharpTok name_tok = ps_expect(ps, STOK_IDENT,
                                       is_union ? "after 'union'"
                                                : "after 'struct'");
        AstNode *n = ast_node_new(AST_TYPE_NAME, t.loc);
        n->u.type_name.name = cpp_xstrndup(name_tok.text, name_tok.len);
        n->u.type_name.is_struct_tag = true; /* struct keyword used */
        base = n;
        goto try_generic;
    }

    /* `enum Tag` as a type-specifier in cast/sizeof context.
     * `(enum dupstring)0` is a valid C cast.  We map enum types to int
     * (same as our type system: Sharp collapses enums to int). */
    if (t.kind == STOK_ENUM) {
        ps_advance(ps);  /* eat 'enum' */
        if (ps_at(ps, STOK_IDENT)) {
            ps_advance(ps);  /* eat Tag name — discard, collapse to int */
        }
        AstNode *n = ast_node_new(AST_TYPE_NAME, t.loc);
        n->u.type_name.name = cpp_xstrdup("int");
        base = n;
        goto apply_suffix;
    }

    /* Named type (identifier or C keyword used as type).  For multi-token
     * primitive specifiers (`unsigned char`, `long long`, …) we compose
     * a canonical space-separated name so that ty_from_name() can
     * recognise the combined type. */
    if (t.kind == STOK_IDENT     ||
        t.kind == STOK_INT       || t.kind == STOK_CHAR   ||
        t.kind == STOK_LONG      || t.kind == STOK_SHORT  ||
        t.kind == STOK_FLOAT     || t.kind == STOK_DOUBLE ||
        t.kind == STOK_SIGNED    || t.kind == STOK_UNSIGNED ||
        t.kind == STOK__BOOL) {
        ps_advance(ps);
        char namebuf[64];
        size_t off = 0;
        if (t.len < sizeof namebuf) {
            memcpy(namebuf, t.text, t.len);
            off = t.len;
        }
        /* Absorb trailing primitive specifiers into the canonical name.
         * Note: STOK_IDENT cannot combine with primitives, so we only
         * extend when the lead token is itself a primitive. */
        /*  C8: preserve __int128 verbatim (GCC native type). */
        if (t.kind == STOK_IDENT && t.len == 8 && memcmp(t.text, "__int128", 8) == 0) {
            memcpy(namebuf, "__int128", 8); off = 8;
        } else if (t.kind != STOK_IDENT) {
            while (ps_at(ps, STOK_LONG) || ps_at(ps, STOK_INT)    ||
                   ps_at(ps, STOK_SHORT)|| ps_at(ps, STOK_CHAR)   ||
                   ps_at(ps, STOK_DOUBLE)||
                   ps_at(ps, STOK_SIGNED)|| ps_at(ps, STOK_UNSIGNED)) {
                SharpTok x = ps_advance(ps);
                if (off + 1 + x.len < sizeof namebuf) {
                    namebuf[off++] = ' ';
                    memcpy(namebuf + off, x.text, x.len);
                    off += x.len;
                }
            }
            /* C8: preserve (signed|unsigned) __int128 verbatim */
            if (ps_at(ps, STOK_IDENT)) {
                SharpTok nx = ps_peek(ps);
                if (nx.len == 8 && memcmp(nx.text, "__int128", 8) == 0) {
                    ps_advance(ps);
                    /* append " __int128" to whatever prefix we have */
                    if (off + 9 < (int)sizeof namebuf) {
                        namebuf[off++] = ' ';
                        memcpy(namebuf + off, "__int128", 8);
                        off += 8;
                    }
                }
            }
        }
        AstNode *n = ast_node_new(AST_TYPE_NAME, t.loc);
        n->u.type_name.name = cpp_xstrndup(namebuf, off);
        base = n;
        goto try_generic;
    }

    ps_error(ps, t.loc, "expected type, got '%.*s'", (int)t.len, t.text);
    {
        AstNode *err = ast_node_new(AST_TYPE_NAME, t.loc);
        err->u.type_name.name = cpp_xstrdup("?");
        return err;  /* synthetic error node — name set so downstream
                      * sema / cg code can call strcmp, scope_lookup,
                      * etc. without null-deref. */
    }

try_generic:
    /* Attach <T1, T2, ...> generic args to a type name.
     * parse_generic_args uses safe speculation: if the '<' is not followed
     * by a valid generic argument list it backtracks without consuming any
     * tokens.  This means we can always try — no pre-scan gate needed. */
    if (ps_at(ps, STOK_LT) && base && base->kind == AST_TYPE_NAME) {
        AstVec args = {0};
        if (parse_generic_args(ps, &args)) {
            AstNode *gen = ast_node_new(AST_TYPE_GENERIC, base->loc);
            gen->u.type_generic.name =
                cpp_xstrdup(base->u.type_name.name);
            gen->u.type_generic.args = args;
            ast_node_free(base);
            base = gen;
        }
    }

apply_suffix:
    /* postfix qualifiers on the base type — `T const`, `T
     * volatile` are equivalent to `const T` / `volatile T` per ISO C99
     * §6.7.3.  Casts like `(unsigned char const *)p` are valid and used
     * by sqlite3.c's UTF-8 helpers.  Eat any trailing const /
     * volatile here, then proceed to the pointer / array suffix loop. */
    for (;;) {
        if (ps_at(ps, STOK_CONST)) {
            CppLoc cl = ps_advance(ps).loc;
            AstNode *cn = ast_node_new(AST_TYPE_CONST, cl);
            cn->u.type_const.base       = base;
            cn->u.type_const.is_postfix = true;  /* p64: T const — postfix */
            base = cn;
        } else if (ps_at(ps, STOK_VOLATILE)) {
            CppLoc vl = ps_advance(ps).loc;
            AstNode *vn = ast_node_new(AST_TYPE_VOLATILE, vl);
            vn->u.type_volatile.base       = base;
            vn->u.type_volatile.is_postfix = true;  /* T volatile — postfix, symmetric with T const */
            base = vn;
        } else if (ps_at(ps, STOK_RESTRICT)) {
            ps_advance(ps);  /* discard — restrict is a hint, no AST node needed */
        } else if (ps_at(ps, STOK__ATOMIC)) {
            CppLoc al = ps_advance(ps).loc;
            AstNode *an = ast_node_new(AST_TYPE_ATOMIC, al);
            an->u.type_atomic.base     = base;
            an->u.type_atomic.is_paren  = false;  /* postfix form: T _Atomic */
            base = an;
        } else {
            break;
        }
    }
    /* pointer suffix: T* or T**  with optional pointer-qualifiers after */
    while (ps_at(ps, STOK_STAR)) {
        ps_advance(ps);
        AstNode *ptr = ast_node_new(AST_TYPE_PTR, base->loc);
        ptr->u.type_ptr.base         = base;
        ptr->u.type_ptr.ptr_const    = false;
        ptr->u.type_ptr.ptr_volatile = false;
        ptr->u.type_ptr.ptr_restrict = false;
        base = ptr;
        /* Pointer-side qualifiers: T * const, T * volatile, T * restrict */
        for (;;) {
            if (ps_at(ps, STOK_CONST)) {
                ps_advance(ps);
                ptr->u.type_ptr.ptr_const = true;
            } else if (ps_at(ps, STOK_VOLATILE)) {
                ps_advance(ps);
                ptr->u.type_ptr.ptr_volatile = true;
            } else if (ps_at(ps, STOK_RESTRICT)) {
                SharpTok rk = ps_advance(ps);
                ptr->u.type_ptr.ptr_restrict = true;
                if (!ptr->u.type_ptr.restrict_kw)
                    ptr->u.type_ptr.restrict_kw = cpp_xstrndup(rk.text, rk.len);
            } else if (ps_at(ps, STOK__ATOMIC)) {
                ps_advance(ps);
                /* T * _Atomic p  →  the _Atomic qualifies the pointee.
                 * Represent as PTR(ATOMIC(base)) so CG emits correctly. */
                AstNode *an = ast_node_new(AST_TYPE_ATOMIC, ps_peek(ps).loc);
                an->u.type_atomic.base     = ptr->u.type_ptr.base;
                an->u.type_atomic.is_paren  = false;
                ptr->u.type_ptr.base = an;
            } else if (ps_at(ps, STOK__NULLABLE) || ps_at(ps, STOK__NONNULL) ||
                       ps_at(ps, STOK__NULL_UNSPEC)) {
                SharpTok nk = ps_advance(ps);
                ptr->u.type_ptr.nullability = cpp_xstrndup(nk.text, nk.len);
            } else if (ps_at(ps, STOK_ATTRIBUTE) || ps_at(ps, STOK_ASM) || ps_at(ps, STOK_DECLSPEC)) {
                eat_attribute_specifiers(ps, NULL);
            } else {
                break;
            }
        }
    }
    /* abstract function-pointer declarator in a type-name
     * position — `RetT (*)(args)` or `RetT (* const)(args)`.  This is
     * the cast form used to call a function-pointer field through a
     * void-ish base, e.g. sqlite3.c's syscall-table dispatch:
     *
     *   ((int (*)(int, uid_t, gid_t)) aSyscall[20].pCurrent)(fd, uid, gid)
     *
     * parse_direct_declarator handles arbitrary nesting via its
     * placeholder-splice mechanism, including doubly-nested forms like
     * `void(*(*)(void*,const char*))(void)` used in sqlite3's dlsym wrapper. */
    if (ps_at(ps, STOK_LPAREN) && ps_peek2(ps).kind == STOK_STAR) {
        char *dummy_name = NULL;
        base = parse_direct_declarator(ps, base, &dummy_name);
        /* dummy_name is always NULL for abstract declarators (no IDENT
         * inside the `(*...)` group).  No free needed — it was never set. */
    }
    /* Also handle `(ATTR *)` — attribute before `*` inside the grouped
     * function-pointer declarator, e.g. `int(ATTR *)(void)`.
     * dd_paren_is_subdeclarator already recognises this pattern;
     * parse_direct_declarator handles the full form. */
    else if (ps_at(ps, STOK_LPAREN) && ps_peek2(ps).kind == STOK_ATTRIBUTE) {
        /* Verify there's actually a `*` past the attribute(s).
         * ps_peek_at(ps, 1) == ATTRIBUTE (same as ps_peek2).
         * Walk forward past each __attribute__((...)) sequence. */
        int _pi = 1;
        while (ps_peek_at(ps, _pi).kind == STOK_ATTRIBUTE) {
            _pi++;  /* skip __attribute__ keyword */
            if (ps_peek_at(ps, _pi).kind != STOK_LPAREN) goto _no_fnptr;
            int _d = 0;
            while (1) {
                SharpTokKind _pk = ps_peek_at(ps, _pi).kind;
                if (_pk == STOK_EOF) goto _no_fnptr;
                if (_pk == STOK_LPAREN) { _d++; _pi++; }
                else if (_pk == STOK_RPAREN) { _d--; _pi++; if (_d == 0) break; }
                else _pi++;
            }
        }
        if (ps_peek_at(ps, _pi).kind == STOK_STAR) {
            char *dummy_name2 = NULL;
            base = parse_direct_declarator(ps, base, &dummy_name2);
        }
        _no_fnptr:;
    }
    /* Array suffix in a type-name position — `(char[32]){0}`
     * compound literals.  After base type + pointer qualifiers, eat `[N]`. */
    while (ps_at(ps, STOK_LBRACKET)) {
        CppLoc aloc = ps_advance(ps).loc;  /* eat '[' */
        AstNode *arr = ast_node_new(AST_TYPE_ARRAY, aloc);
        arr->u.type_array.base = base;
        arr->u.type_array.size = NULL;
        if (!ps_at(ps, STOK_RBRACKET))
            arr->u.type_array.size = parse_expr_prec(ps, 2);
        ps_expect(ps, STOK_RBRACKET, "']' in array type");
        base = arr;
    }
    return base;
}

/* =========================================================================
 * parse_type_unqual — base type without pointer/const suffixes.
 * Used so that "const char*" → PTR(CONST(char)), not CONST(PTR(char)).
 * ====================================================================== */
static AstNode *parse_type_unqual(PS *ps) {
    SharpTok t = ps_peek(ps);
    AstNode *base = NULL;
    if (t.kind == STOK_AUTO) { ps_advance(ps); return ast_node_new(AST_TYPE_AUTO, t.loc); }
    if (t.kind == STOK_VOID) { ps_advance(ps); return ast_node_new(AST_TYPE_VOID, t.loc); }
    if (t.kind == STOK_STRUCT) {
        ps_advance(ps);
        SharpTok nm = ps_expect(ps, STOK_IDENT, "after 'struct'");
        base = ast_node_new(AST_TYPE_NAME, t.loc);
        base->u.type_name.name = cpp_xstrndup(nm.text, nm.len);
        base->u.type_name.is_struct_tag = true;  /* Preserve struct keyword in cast types */
    } else if (t.kind == STOK_CLASS) {
        ps_advance(ps);
        SharpTok nm = ps_expect(ps, STOK_IDENT, "after 'class'");
        base = ast_node_new(AST_TYPE_NAME, t.loc);
        base->u.type_name.name = cpp_xstrndup(nm.text, nm.len);
        /* class names are bare type names (typedef auto-injected) */
    } else if (t.kind == STOK_IDENT || t.kind == STOK_INT || t.kind == STOK_CHAR ||
               t.kind == STOK_LONG  || t.kind == STOK_SHORT || t.kind == STOK_FLOAT ||
               t.kind == STOK_DOUBLE|| t.kind == STOK_SIGNED || t.kind == STOK_UNSIGNED ||
               t.kind == STOK__BOOL) {
        ps_advance(ps);
        char namebuf[64];
        size_t off = 0;
        /* C8: preserve __int128 verbatim (GCC native type). */
        if (t.kind == STOK_IDENT && t.len == 8 && memcmp(t.text, "__int128", 8) == 0) {
            base = ast_node_new(AST_TYPE_NAME, t.loc);
            base->u.type_name.name = cpp_xstrdup("__int128");
            return base;
        }
        if (t.len < sizeof namebuf) {
            memcpy(namebuf, t.text, t.len);
            off = t.len;
        }
        if (t.kind != STOK_IDENT) {
            while (ps_at(ps, STOK_LONG) || ps_at(ps, STOK_INT) ||
                   ps_at(ps, STOK_SHORT)|| ps_at(ps, STOK_CHAR)||
                   ps_at(ps, STOK_DOUBLE)||
                   ps_at(ps, STOK_SIGNED)|| ps_at(ps, STOK_UNSIGNED)) {
                SharpTok x = ps_advance(ps);
                if (off + 1 + x.len < sizeof namebuf) {
                    namebuf[off++] = ' ';
                    memcpy(namebuf + off, x.text, x.len);
                    off += x.len;
                }
            }
            /* C8: preserve (signed|unsigned) __int128 verbatim */
            if (ps_at(ps, STOK_IDENT)) {
                SharpTok nx = ps_peek(ps);
                if (nx.len == 8 && memcmp(nx.text, "__int128", 8) == 0) {
                    ps_advance(ps);
                    if (off + 9 < (int)sizeof namebuf) {
                        namebuf[off++] = ' ';
                        memcpy(namebuf + off, "__int128", 8);
                        off += 8;
                    }
                }
            }
        }
        base = ast_node_new(AST_TYPE_NAME, t.loc);
        base->u.type_name.name = cpp_xstrndup(namebuf, off);
    } else {
        ps_error(ps, t.loc, "expected base type, got '%.*s'", (int)t.len, t.text);
        base = ast_node_new(AST_TYPE_NAME, t.loc);
        base->u.type_name.name = cpp_xstrndup("?", 1);
        return base;
    }
    /* Attach generic args to a type name. */
    if (ps_at(ps, STOK_LT) && base && base->kind == AST_TYPE_NAME) {
        AstVec args = {0};
        if (parse_generic_args(ps, &args)) {
            AstNode *gen = ast_node_new(AST_TYPE_GENERIC, base->loc);
            gen->u.type_generic.name =
                cpp_xstrdup(base->u.type_name.name);
            gen->u.type_generic.args = args;
            ast_node_free(base);
            base = gen;
        }
    }
    return base;
}

/* =========================================================================
 * Generic parameter list parsing: <T, U, ...>
 * ====================================================================== */
static void parse_generic_params(PS *ps, AstVec *params) {
    if (!ps_at(ps, STOK_LT)) return;
    ps_advance(ps);
    while (!ps_at(ps, STOK_GT) && !ps_at(ps, STOK_EOF)) {
        SharpTok nm = ps_expect(ps, STOK_IDENT, "generic parameter name");
        AstNode *gp = ast_node_new(AST_GENERIC_PARAM, nm.loc);
        gp->u.generic_param.name = cpp_xstrndup(nm.text, nm.len);
        astvec_push(params, gp);
        if (!ps_match(ps, STOK_COMMA)) break;
    }
    ps_expect(ps, STOK_GT, "closing '>' of generic params");
}

/* =========================================================================
 * S1: DeclSpecs + parse_declarator — ISO C declaration model.
 *
 * The pre-S1 parser used a "type-prefix" model (`parse_type` consumed a
 * single contiguous type expression, then a separate helper consumed the
 * variable name and array suffix).  That model cannot represent ISO C
 * declarators with sub-grouping such as `int (*signal(int, void(*)(int)))(int)`,
 * nor does it have a place for storage-class specifiers (static / extern /
 * inline / register / volatile).
 *
 * S1 introduces two functions:
 *
 *   parse_decl_specifiers()  consumes  storage-class*  type-qualifier*
 *                            type-specifier+   in any order, composing
 *                            multi-token type specifiers (`unsigned long
 *                            long`) into a canonical TYPE_NAME string
 *                            recognised by ty_from_name().
 *
 *   parse_declarator()       consumes  pointer?  direct-declarator,
 *                            where direct-declarator may recurse through
 *                            parenthesised sub-declarators.  The "outer
 *                            base type plugs in here" position is marked
 *                            by a sentinel placeholder node identified by
 *                            pointer equality, then spliced in once the
 *                            full declarator tree is known.
 *
 * Sharp's older "value" model — `int* p, q;` makes BOTH p and q of type
 * int* — is dropped.  S1 conforms to ISO semantics: the leftmost `*`
 * binds to the FIRST declarator only.  Idiomatic multi-pointer becomes
 * `int *p, *q;` (which now works) or `typedef int* IntP; IntP p, q;`.
 * ====================================================================== */

typedef struct {
    StorageClass storage;        /* SC_NONE / SC_STATIC / ...                */
    bool         is_inline;
    bool         is_const;       /* type qualifier on the base specifier    */
    bool         is_volatile;
    /* fmt: bit-field recording source-order of qualifiers/specifiers for
     * faithful C output reproduction.  Use FMTF_* constants from parse.h. */
    FmtFlags     fmt;
    char        *post_type_attr;  /* __attribute__ appearing after type specifier */
    /* C8: _Atomic qualifier — preserved verbatim in C output. */
    bool         is_atomic;
    /* C11 _Noreturn — function specifier. */
    bool         is_noreturn;
    /* C11 _Alignas(expr) — alignment specifier.
     * The raw token text inside the parens is preserved for faithful
     * C output reproduction (e.g. "_Alignas(16)", "_Alignas(double)"). */
    char        *alignas_text;     /* text inside _Alignas(...) */
    /* C11 _Thread_local — orthogonal to storage class (can
     * combine with static or extern per ISO 6.7.1).  Recorded as its
     * own flag rather than a SC_THREAD_LOCAL because the existing
     * StorageClass enum is single-valued.  stb_image: `static
     * _Thread_local const char *xx;`. */
    bool         is_thread_local;
    AstNode     *base_ty;        /* raw type-specifier AST                  */
    CppLoc       loc;            /* location of the first specifier token   */
    bool         empty;          /* true iff no decl-spec tokens consumed   */
    /* GCC __attribute__((...)) text accumulated from the
     * decl-specifier sequence (leading attributes such as
     * `__attribute__((always_inline)) static inline`).  Heap-allocated,
     * may be NULL.  Transferred to the produced AST node; caller must
     * NOT free it separately once transferred. */
    char        *gcc_attrs;
    /* p45: original GCC alias keyword spellings */
    const char  *inline_kw;
    const char  *const_kw;
    const char  *volatile_kw;
    /* C8-fix: when an inline `enum { ... }` body appears in a decl-spec,
     * base_ty is set to a fresh TYPE_NAME("int") (owned) and this field
     * holds a NON-owning back-reference to the AST_ENUM_DEF that was
     * pushed to pending_decls.  The typedef code uses this to emit the
     * enum body inline without causing a double-free. */
    AstNode     *enum_body_ref;
    /* C23: constexpr specifier */
    bool         is_constexpr;
} DeclSpecs;

/* Type-specifier accumulator.  Combines repeated tokens into a single
 * canonical type name, e.g. "unsigned" + "long" + "long" -> "unsigned long
 * long".  All counts default to zero. */
typedef struct {
    int      saw_void, saw_bool, saw_char, saw_short, saw_int;
    int      saw_long_count;
    int      saw_float, saw_double;
    int      saw_signed, saw_unsigned;
    int      saw_complex;
    AstNode *user_ty;
    const char *signed_kw;   /* p45: "__signed" / "__signed__" / "signed" */
    AstNode *enum_body_ref;  /* C8-fix: non-owning ref to inline enum def */
} TSpec;

static bool tspec_has_anything(const TSpec *t) {
    return t->saw_void || t->saw_bool || t->saw_char || t->saw_short ||
           t->saw_int  || t->saw_long_count > 0 ||
           t->saw_float|| t->saw_double || t->saw_signed || t->saw_unsigned ||
           t->saw_complex || t->user_ty;
}

/* Resolve TSpec into a single AstNode type tree.  The resulting node is
 * always one of: AST_TYPE_NAME (for primitives), AST_TYPE_VOID, or the
 * user_ty node passed through unchanged. */
static AstNode *tspec_resolve(PS *ps, TSpec *ts, CppLoc loc) {
    if (ts->user_ty) {
        /* User-defined type (struct tag / IDENT / generic) — primitive
         * tokens cannot legally combine with it.  We don't enforce that
         * here; the C compiler will reject illegal mixes.
         * C8: exception — signed/unsigned __int128: preserve prefix in name. */
        AstNode *out = ts->user_ty;
        ts->user_ty = NULL;
        if (out->kind == AST_TYPE_NAME && out->u.type_name.name &&
            strcmp(out->u.type_name.name, "__int128") == 0 &&
            (ts->saw_signed || ts->saw_unsigned)) {
            char *new_nm;
            if (ts->saw_unsigned)
                new_nm = cpp_xstrdup("unsigned __int128");
            else
                new_nm = cpp_xstrdup("__signed__ __int128");
            free(out->u.type_name.name);
            out->u.type_name.name = new_nm;
        }
        return out;
    }
    if (ts->saw_void) {
        return ast_node_new(AST_TYPE_VOID, loc);
    }

    /* Build the canonical name string. */
    char buf[64];
    size_t off = 0;
    #define PUSH(s) do { \
        size_t _len = strlen(s); \
        if (off > 0 && off + 1 < sizeof buf) { buf[off++] = ' '; } \
        if (off + _len < sizeof buf) { memcpy(buf+off, s, _len); off += _len; } \
    } while (0)

    if (ts->saw_signed && !ts->saw_unsigned) PUSH("signed");
    if (ts->saw_unsigned)                     PUSH("unsigned");
    if (ts->saw_complex)                      PUSH("_Complex");
    if (ts->saw_char)                          PUSH("char");
    else if (ts->saw_short)                    PUSH("short");
    else if (ts->saw_long_count == 2)          PUSH("long long");
    else if (ts->saw_long_count == 1 && ts->saw_double) {
        /* `long double` — overwrite to a single canonical form. */
        off = 0;
        PUSH("long double");
    }
    else if (ts->saw_long_count == 1)          PUSH("long");
    else if (ts->saw_float)                    PUSH("float");
    else if (ts->saw_double)                   PUSH("double");
    else if (ts->saw_bool)                     PUSH("_Bool");
    /* A4: when `int` is explicitly written, always include it in the name
     * so "signed int" stays "signed int" and not just "signed". */
    if (ts->saw_int) PUSH("int");

    /* `int` is the implicit specifier for short/long/signed/unsigned.
     * We append it only when the canonical form needs disambiguation —
     * actually our table in ty_from_name() already accepts both "long"
     * and "long int" so we can omit the trailing "int" for compactness. */

    if (off == 0) {
        /* No primitive specifier at all — implicit-int (pre-C99) or a
         * truly empty decl-specifier (which the caller handles). */
        ps_error(ps, loc, "missing type specifier");
        memcpy(buf, "int", 4); off = 3;
    }
    buf[off] = '\0';

    AstNode *n = ast_node_new(AST_TYPE_NAME, loc);
    n->u.type_name.name = cpp_xstrndup(buf, off);
    /* p45: if signed_kw is a GCC alias (__signed/__signed__), store display form */
    if (ts->signed_kw && strcmp(ts->signed_kw, "signed") != 0) {
        /* Build display name by replacing "signed" with the alias */
        char dbuf[128] = {0};
        size_t doff = 0;
        #define DPUSH(s) do { size_t _l=strlen(s); if(doff+_l<sizeof dbuf){memcpy(dbuf+doff,s,_l);doff+=_l;} } while(0)
        DPUSH(ts->signed_kw);
        /* Append the rest of buf after "signed" */
        if (strncmp(buf, "signed", 6) == 0)
            DPUSH(buf + 6);
        dbuf[doff] = '\0';
        #undef DPUSH
        if (doff > 0) n->u.type_name.display_name = cpp_xstrndup(dbuf, doff);
    }
    return n;
    #undef PUSH
}

/* Try to extend `ts` with the current token.  Returns true and advances
 * `ps` on success; returns false and leaves `ps` unchanged otherwise. */
/* Forward declaration — defined near the end of this file.              */
static AstNode *typeof_parse_expr(SharpTok *tokens, int ntoks, int *pos);
static AstNode *typeof_parse_unary(SharpTok *tokens, int ntoks, int *pos);

static bool tspec_try_consume(PS *ps, TSpec *ts) {
    SharpTok t = ps_peek(ps);
    switch (t.kind) {
    case STOK_VOID:    ts->saw_void++;     ps_advance(ps); return true;
    case STOK__BOOL:   ts->saw_bool++;     ps_advance(ps); return true;
    case STOK_CHAR:    ts->saw_char++;     ps_advance(ps); return true;
    case STOK_SHORT:   ts->saw_short++;    ps_advance(ps); return true;
    case STOK_INT:     ts->saw_int++;      ps_advance(ps); return true;
    case STOK_LONG:    ts->saw_long_count++; ps_advance(ps); return true;
    case STOK_FLOAT:   ts->saw_float++;    ps_advance(ps); return true;
    case STOK_DOUBLE:  ts->saw_double++;   ps_advance(ps); return true;
    case STOK_SIGNED: {
        SharpTok sk = ps_advance(ps);
        ts->saw_signed++;
        if (!ts->signed_kw) ts->signed_kw = cpp_xstrndup(sk.text, sk.len); /* p45 */
        return true;
    }
    case STOK_UNSIGNED:ts->saw_unsigned++; ps_advance(ps); return true;
    case STOK__COMPLEX: ts->saw_complex++; ps_advance(ps); return true;

    case STOK_STRUCT:
    case STOK_UNION:
    case STOK_CLASS:
    case STOK_ENUM: {
        /* `struct Tag` / `union Tag` / `enum Tag` — a tag reference.  We
         * do NOT consume a struct/union/enum BODY here; full definitions
         * appear at top level and are dispatched by parse_top_decl
         * before reaching this code path.  For C-superset code, the
         * tag-reference form is the common case for type-specifier use.
         *
         * Exception: `struct {...}` and `union {...}` with NO tag —
         * common in `typedef struct { ... } Name;`.  We can't defer
         * those to parse_top_decl because the surrounding declaration
         * is already in flight.  Instead, parse the anonymous body
         * inline via parse_struct_def, push the resulting AST_STRUCT_DEF
         * to pending_decls so it surfaces at file scope, and reference
         * it by its synthetic tag.  Enum has no analogous idiom in
         * production code we target, so it stays diagnostic-only.
         *
         * For struct/union, the resulting type AST is a TYPE_NAME that
         * resolves to the registered SYM_TYPE; the `is_union` bit lives
         * on the AST_STRUCT_DEF the symbol points at.  For enum, the
         * type collapses to int at the Sharp Type* layer. */
        if (ts->user_ty) return false;
        SharpTokKind kw_kind = t.kind;

        /* Anonymous struct/union body: `struct {` or `union {` */
        if ((kw_kind == STOK_STRUCT || kw_kind == STOK_UNION || kw_kind == STOK_CLASS) &&
            ps_peek2(ps).kind == STOK_LBRACE) {
            AstNode *sd = parse_struct_def(ps);
            /* parse_struct_def synthesises a unique tag for anonymous
             * forms; reference that tag via TYPE_NAME and queue the
             * definition for emission at file scope. */
            ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
            ts->user_ty->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
            astvec_push(&ps->pending_decls, sd);
            return true;
        }

        /* Tagged inline definition: `struct Tag { ... }` or `union Tag
         * { ... }` appearing in a decl-specifier context — common in
         *   typedef struct Foo { ... } Foo;
         *   typedef union  Bar { ... } Bar;
         * Also handles generic structs: `struct Pair<K,V> { ... }`.
         * also handles attributes between keyword and tag:
         *   typedef union __attribute__((packed)) Tag { ... } Tag;
         * The struct body must be parsed inline; we route to
         * parse_struct_def, queue the resulting AST_STRUCT_DEF to
         * pending_decls so it surfaces at file scope, and resolve the
         * type-specifier as a TYPE_NAME referring to the tag. */
        if ((kw_kind == STOK_STRUCT || kw_kind == STOK_UNION || kw_kind == STOK_CLASS) &&
            ps_peek2(ps).kind == STOK_IDENT) {
            SharpTokKind k3 = ps_peek3(ps).kind;
            if (k3 == STOK_LBRACE || k3 == STOK_LT) {
                AstNode *sd = parse_struct_def(ps);
                /* Mark as nested when defined inside another struct body */
                if (ps->struct_body_depth > 0)
                    sd->u.struct_def.is_nested_in_struct = true;
                ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
                ts->user_ty->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
                ts->user_ty->u.type_name.is_struct_tag = true;
                astvec_push(&ps->pending_decls, sd);
                return true;
            }
        }
        /* struct/union/enum with attributes before tag name:
         * `union __attribute__((packed)) Tag { ... }` */
        if (kw_kind == STOK_STRUCT || kw_kind == STOK_UNION || kw_kind == STOK_CLASS) {
            /* Look past optional __attribute__((...)) sequences.
             * ps_peek_at(ps, 1) is the token right after struct/union.
             * Each attribute is: STOK_ATTRIBUTE '(' '(' ... ')' ')'
             * We must skip ALL tokens of each attribute, not just the
             * keyword, because __attribute__((packed)) is 6 tokens. */
            int peek_idx = 1;
            for (;;) {
                if (ps_peek_at(ps, peek_idx).kind != STOK_ATTRIBUTE) break;
                peek_idx++;  /* skip __attribute__ keyword */
                /* skip the balanced ((...)) that follows */
                if (ps_peek_at(ps, peek_idx).kind != STOK_LPAREN) break;
                int depth = 0;
                while (1) {
                    SharpTokKind pk = ps_peek_at(ps, peek_idx).kind;
                    if (pk == STOK_EOF) goto after_attr_skip;
                    if (pk == STOK_LPAREN)  { depth++; peek_idx++; }
                    else if (pk == STOK_RPAREN) {
                        depth--; peek_idx++;
                        if (depth == 0) break;
                    } else { peek_idx++; }
                }
            }
            after_attr_skip:
            if (ps_peek_at(ps, peek_idx).kind == STOK_IDENT) {
                SharpTokKind after_ident = ps_peek_at(ps, peek_idx + 1).kind;
                if (after_ident == STOK_LBRACE || after_ident == STOK_LT) {
                    AstNode *sd = parse_struct_def(ps);
                    if (ps->struct_body_depth > 0)
                        sd->u.struct_def.is_nested_in_struct = true;
                    ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
                    ts->user_ty->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
                    ts->user_ty->u.type_name.is_struct_tag = true;
                    astvec_push(&ps->pending_decls, sd);
                    return true;
                }
            }
        }

        /* Anonymous enum body: `enum {` (used by `typedef enum { ... }
         * Alias;` everywhere — Lua's ltm.h, glibc's many internal
         * tag-enums).  Parse via parse_enum_def, which already accepts
         * the no-name shape; queue to pending_decls and resolve the
         * spec to int (Sharp collapses enum types to int).  The named
         * `enum Tag {` form is handled identically since parse_enum_def
         * picks up the optional tag itself. */
        if (kw_kind == STOK_ENUM &&
            (ps_peek2(ps).kind == STOK_LBRACE ||
             (ps_peek2(ps).kind == STOK_IDENT &&
              ps_peek3(ps).kind == STOK_LBRACE))) {
            AstNode *ed = parse_enum_def(ps);
            astvec_push(&ps->pending_decls, ed);
            /* C8-fix: pending_decls owns ed; give user_ty a fresh TYPE_NAME
             * so FIELD_DECL / VAR_DECL do not share ownership of ed and
             * cause a double-free on ast_node_free.
             * For anonymous enums Sharp collapses to "int".  Named enums
             * also collapse to "int" at the type level; the enum body is
             * emitted separately via pending_decls.
             * Store a non-owning back-reference in enum_body_ref so the
             * typedef path (parse_top_decl SC_TYPEDEF) can still emit the
             * enum body inline (the C8 p90 requirement). */
            ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
            ts->user_ty->u.type_name.name =
                (ed->u.enum_def.name && ed->u.enum_def.name[0])
                    ? cpp_xstrdup(ed->u.enum_def.name)
                    : cpp_xstrdup("int");
            ts->enum_body_ref = ed;  /* non-owning */
            return true;
        }

        ps_advance(ps);
        /* handle attributes between struct/union/enum keyword
         * and the tag name: `union __attribute__((packed)) Tag { ... }` */
        while (ps_at(ps, STOK_ATTRIBUTE)) {
            char *attr_text = NULL;
            eat_attribute_specifiers(ps, &attr_text);
            free(attr_text);
        }
        if (!ps_at(ps, STOK_IDENT)) {
            ps_error(ps, ps_peek(ps).loc,
                "expected %s tag name",
                kw_kind == STOK_STRUCT ? "struct" :
                kw_kind == STOK_UNION  ? "union"  :
                kw_kind == STOK_CLASS  ? "class"  : "enum");
            ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
            ts->user_ty->u.type_name.name = cpp_xstrdup("?");
            return true;
        }
        SharpTok nm = ps_advance(ps);
        ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
        ts->user_ty->u.type_name.name = cpp_xstrndup(nm.text, nm.len);
        /* mark when the struct/union keyword was explicit.
         * Used by ty_from_ast to prefer the struct tag over a same-named
         * typedef (e.g. `struct h1_tunnel_state` vs `typedef enum {} h1_tunnel_state`). */
        if ((kw_kind == STOK_STRUCT || kw_kind == STOK_UNION) &&
            !ps_at(ps, STOK_LBRACE))
            ts->user_ty->u.type_name.is_struct_tag = true;
        /* class names are bare type names — no struct tag prefix needed */
        /* For enum bare-tag references (`enum E`, no body), mark is_enum_tag
         * so the CG emits `enum E` faithfully without needing a scope lookup.
         * Without this the CG sees just "E" (unknown name) and emits "E *p"
         * which zig cc / clang rejects as an unknown type name.             */
        if (kw_kind == STOK_ENUM && !ps_at(ps, STOK_LBRACE))
            ts->user_ty->u.type_name.is_enum_tag = true;
        /* optional generic args after a struct tag (Sharp generic types) */
        if ((kw_kind == STOK_STRUCT || kw_kind == STOK_CLASS) && ps_at(ps, STOK_LT)) {
            AstVec args = {0};
            if (parse_generic_args(ps, &args)) {
                AstNode *gen = ast_node_new(AST_TYPE_GENERIC, t.loc);
                gen->u.type_generic.name =
                    cpp_xstrdup(ts->user_ty->u.type_name.name);
                gen->u.type_generic.args = args;
                ast_node_free(ts->user_ty);
                ts->user_ty = gen;
            }
        }
        return true;
    }

    case STOK_TYPEOF: {
        /*  typeof-root-fix: typeof(expr) / __typeof__(expr)
         * as a type specifier.  Build the verbatim string for cg round-
         * trip AND save inner tokens to parse a lightweight expression
         * AST.  ty_from_ast uses the expr AST to infer the concrete C
         * type; if parsing fails, falls back to opaque struct. */
        if (ts->user_ty) return false;
        CppLoc loc = t.loc;
        ps_advance(ps);  /* eat typeof token */
        char buf[256]; int blen = 0;
        SharpTok saved_toks[256]; int ntoks = 0;
        blen += snprintf(buf + blen, sizeof buf - blen, "__typeof__(");
        if (ps_at(ps, STOK_LPAREN)) {
            ps_advance(ps);
            int depth = 0;
            while (!ps_at(ps, STOK_EOF) && ntoks < 255) {
                SharpTok tk = ps_peek(ps);
                saved_toks[ntoks++] = tk;
                if (tk.kind == STOK_LPAREN) { depth++; }
                else if (tk.kind == STOK_RPAREN) {
                    if (depth == 0) {
                        if (blen + 2 < (int)sizeof buf) buf[blen++] = ')';
                        ps_advance(ps); break;
                    }
                    depth--;
                }
                if (blen + (int)tk.len + 2 < (int)sizeof buf) {
                    if (blen > 10) buf[blen++] = ' ';
                    memcpy(buf + blen, tk.text, tk.len); blen += tk.len;
                }
                ps_advance(ps);
            }
        }
        buf[blen] = '\0';
        AstNode *te = ast_node_new(AST_TYPEOF_TYPE, loc);
        te->u.typeof_type.verbatim = cpp_xstrdup(buf);
        if (ntoks > 0) {
            int tpos = 0;
            te->u.typeof_type.expr = typeof_parse_expr(saved_toks, ntoks, &tpos);
        } else {
            te->u.typeof_type.expr = NULL;
        }
        ts->user_ty = te;
        return true;
    }

    case STOK_IDENT: {
        /* User-named type (IDENT possibly followed by `<...>`).  Only
         * accepted when it's the FIRST type-specifier seen — otherwise
         * it must be the start of a declarator (the variable name). */
        /*  C8: __int128 is a GCC/Clang built-in 128-bit integer type.
         * It can follow `signed` or `unsigned` as `signed __int128`.
         * Preserve the verbatim name so faithful C output keeps `__int128`.
         * (Downstream GCC handles __int128 natively.) */
        if (t.len == 8 && memcmp(t.text, "__int128", 8) == 0) {
            ps_advance(ps);
            if (!ts->user_ty) {
                ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
                ts->user_ty->u.type_name.name = cpp_xstrdup("__int128");
            }
            return true;
        }
        if (tspec_has_anything(ts)) return false;
        /* Must look-ahead to confirm this IDENT is a type, not a
         * variable name in a context where the type is implicit-int.
         * Implicit-int doesn't appear in modern code, so we trust
         * is_type_start()'s caller-level decision: by the time we get
         * here, we've already classified this position as a declaration. */
        ps_advance(ps);
        ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
        ts->user_ty->u.type_name.name = cpp_xstrndup(t.text, t.len);
        /* Phase G: only attach generic args when name is in generic_names. */
        if (ps_at(ps, STOK_LT)) {
            bool known = (ps->generic_names.len > 0)
                         ? td_has(&ps->generic_names,
                                  ts->user_ty->u.type_name.name)
                         : true;
            if (known) {
                AstVec args = {0};
                if (parse_generic_args(ps, &args)) {
                    AstNode *gen = ast_node_new(AST_TYPE_GENERIC, t.loc);
                    gen->u.type_generic.name =
                        cpp_xstrdup(ts->user_ty->u.type_name.name);
                    gen->u.type_generic.args = args;
                    ast_node_free(ts->user_ty);
                    ts->user_ty = gen;
                }
            }
        }
        return true;
    }

    default:
        return false;
    }
}

/* =========================================================================
 * Declaration specifiers and declarator parsing
 * ====================================================================== */

/* Parse a complete decl-specifier sequence.  Returns DeclSpecs.empty=true
 * if no spec tokens were consumed; the caller decides whether that's an
 * error (top-level decls) or end-of-input (parameter abstract types). */
static DeclSpecs parse_decl_specifiers(PS *ps) {
    DeclSpecs ds = {0};
    ds.loc   = ps_peek(ps).loc;
    ds.empty = true;
    TSpec ts = {0};

    for (;;) {
        SharpTok t = ps_peek(ps);

        /* Storage-class specifier — at most one allowed. */
        StorageClass new_sc = SC_NONE;
        switch (t.kind) {
        case STOK_STATIC:   new_sc = SC_STATIC;   break;
        case STOK_EXTERN:   new_sc = SC_EXTERN;   break;
        case STOK_TYPEDEF:  new_sc = SC_TYPEDEF;  break;
        case STOK_REGISTER: new_sc = SC_REGISTER; break;
        case STOK_AUTO:
            /* Sharp's `auto` is a TYPE (deduction) in declaration context.
             * We treat it as a type-specifier, not a storage class.
             * Fall through to the type-specifier branch below. */
            break;
        default: break;
        }
        if (new_sc != SC_NONE) {
            if (ds.storage != SC_NONE && ds.storage != new_sc) {
                ps_error(ps, t.loc,
                    "more than one storage-class specifier in declaration");
            }
            ds.storage = new_sc;
            ds.empty   = false;
            ps_advance(ps);
            continue;
        }

        /* Function-specifier (C99): inline. */
        if (t.kind == STOK_INLINE) {
            ds.is_inline = true;
            ds.empty     = false;
            if (!ds.inline_kw) ds.inline_kw = cpp_xstrndup(t.text, t.len);
            if (ds.storage == SC_NONE) ds.fmt |= FMTF_INLINE_LEADING;  /* p45 */
            /* Track if inline appeared AFTER gcc_attrs
             * (e.g. `static __attribute__((always_inline)) inline`).
             * cg_func uses this to preserve the original attribute-first order. */
            if (ds.gcc_attrs && !(ds.fmt & FMTF_INLINE_AFTER_ATTRS)) ds.fmt |= FMTF_INLINE_AFTER_ATTRS;
            ps_advance(ps);
            continue;
        }

        /* C23: constexpr specifier.  Handled here in parse_decl_specifiers
         * so it works as a top-level prefix and attribute specifier. */
        if (t.kind == STOK_CONSTEXPR) {
            ds.is_constexpr = true;
            ds.empty        = false;
            ps_advance(ps);
            continue;
        }

        /* C11 _Noreturn function specifier. */
        if (t.kind == STOK__NORETURN) {
            ds.is_noreturn = true;
            ds.empty       = false;
            ps_advance(ps);
            continue;
        }

        /* C11 _Alignas(expression) alignment specifier.
         * Consume _Alignas(...) and preserve the inner text verbatim. */
        if (t.kind == STOK__ALIGNAS) {
            ds.empty = false;
            ps_advance(ps);
            /* Expect '(' */
            if (ps_at(ps, STOK_LPAREN)) {
                ps_advance(ps);
                /* Collect tokens until ')' — preserve verbatim for C output */
                size_t start = ps->pos;
                int paren_depth = 1;
                while (ps->pos < ps->ntoks && paren_depth > 0) {
                    SharpTok cur = ps_peek(ps);
                    if (cur.kind == STOK_LPAREN) paren_depth++;
                    else if (cur.kind == STOK_RPAREN) paren_depth--;
                    if (paren_depth > 0) ps_advance(ps);
                }
                size_t end = ps->pos;
                /* Copy the text between start and end */
                if (end > start) {
                    SharpTok first_tok = ps->toks[start];
                    SharpTok last_tok  = ps->toks[end - 1];
                    size_t total_len = (size_t)(last_tok.text + last_tok.len - first_tok.text);
                    ds.alignas_text = cpp_xstrndup(first_tok.text, total_len);
                }
                /* Consume ')' */
                if (ps_at(ps, STOK_RPAREN)) ps_advance(ps);
            }
            continue;
        }

        /* C11 _Thread_local */
        if (t.kind == STOK__THREAD_LOCAL) {
            ds.is_thread_local = true;
            ds.empty           = false;
            ps_advance(ps);
            continue;
        }

        if (t.kind == STOK_CONST) {
            if (!ds.is_const && !ds.is_volatile) ds.fmt |= FMTF_CONST_BEFORE_VOL; /* first qualifier is const */
            ds.is_const = true;
            ds.empty = false;
            if (tspec_has_anything(&ts)) ds.fmt |= FMTF_CONST_POSTFIX;
            if (!ds.const_kw) ds.const_kw = cpp_xstrndup(t.text, t.len);
            if (ds.storage == SC_NONE && !tspec_has_anything(&ts))
                ds.fmt |= FMTF_CONST_LEADING; /* p45: __const before storage */
            ps_advance(ps);
            continue;
        }
        if (t.kind == STOK_VOLATILE) {
            ds.is_volatile = true;
            ds.empty = false;
            if (!ds.volatile_kw) ds.volatile_kw = cpp_xstrndup(t.text, t.len); /* p45 */
            if (tspec_has_anything(&ts)) ds.fmt |= FMTF_VOLATILE_POSTFIX;
            ps_advance(ps);
            continue;
        }
        if (t.kind == STOK__ATOMIC)  {
            ps_advance(ps);
            ds.empty = false;
            if (ps_at(ps, STOK_LPAREN)) {
                /* _Atomic(T) — type-specifier form (C11 6.7.2.4).
                 * Parse the inner type and wrap it; don't set ds.is_atomic
                 * since the atomic-ness is already encoded in the node. */
                ps_advance(ps);  /* consume '(' */
                AstNode *inner = parse_type(ps);
                ps_expect(ps, STOK_RPAREN, "_Atomic(T) closing ')'");
                AstNode *an = ast_node_new(AST_TYPE_ATOMIC, t.loc);
                an->u.type_atomic.base     = inner;
                an->u.type_atomic.is_paren  = true;
                ts.user_ty = an;
            } else {
                /* _Atomic alone — type qualifier form (C11 6.7.2.4p5).
                 * Applied to resolved base type later via ds.is_atomic flag. */
                ds.is_atomic = true;
            }
            continue;
        }
        if (t.kind == STOK_RESTRICT) { ds.empty = false; ps_advance(ps); continue; }

        /* Clang nullability annotations — macOS / iOS system headers use
         * these extensively (e.g. lua.h, dispatch.h).  They are type qualifiers
         * that can appear in decl-specifier position or after '*'.  We silently
         * consume them here so the parser doesn't mistake them for variable names. */
        if (t.kind == STOK__NULLABLE || t.kind == STOK__NONNULL ||
            t.kind == STOK__NULL_UNSPEC) {
            ds.empty = false;
            ps_advance(ps);
            continue;
        }

        /* GCC `__attribute__((…))` — accepted in any decl-specifier
         * position (before, between, or after the type spec).  * captured into ds.gcc_attrs for passthrough to the AST node;
         * does not toggle ds.empty since attributes never stand alone
         * as a declaration. */
        if (t.kind == STOK_ATTRIBUTE) {
            /* p43: if storage class not yet seen, this is a leading attribute */
            if (ds.storage == SC_NONE && !tspec_has_anything(&ts)) {
                eat_attribute_specifiers(ps, &ds.gcc_attrs);
                ds.fmt |= FMTF_ATTRS_LEADING;
            } else if (tspec_has_anything(&ts)) {
                /* Attribute comes AFTER the type specifier (e.g. l_noret expands to
                 * `void __attribute__((noreturn))`).  Store in post_type_attr; it will
                 * be wrapped onto the resolved base type after tspec_resolve() below. */
                eat_attribute_specifiers(ps, &ds.post_type_attr);
            } else {
                /* Attribute between storage class and type (normal trailing) */
                eat_attribute_specifiers(ps, &ds.gcc_attrs);
            }
            continue;
        }

        /* Sharp's `auto` (type deduction) is a type-specifier here — we
         * accept it only when no other type-specifier has been seen. */
        if (t.kind == STOK_AUTO) {
            if (tspec_has_anything(&ts)) {
                ps_error(ps, t.loc, "'auto' cannot combine with other type specifiers");
            }
            ts.user_ty = ast_node_new(AST_TYPE_AUTO, t.loc);
            ds.empty = false;
            ps_advance(ps);
            continue;
        }

        /* Type-specifier (primitive / struct / IDENT / generic). */
        {
            size_t pd_before = ps->pending_decls.len;
            if (tspec_try_consume(ps, &ts)) {
                /* If we're in a typedef and a struct was pushed to
                 * pending_decls, mark it as from_inline_typedef so
                 * cg can emit `typedef struct X { ... } X;` rather
                 * than separating them.
                 * Otherwise (variable/parameter context), mark as
                 * from_inline_var so cg can emit inline with the var. */
                if (ds.storage == SC_TYPEDEF) {
                    for (size_t _pi = pd_before; _pi < ps->pending_decls.len; _pi++) {
                        AstNode *_pd = ps->pending_decls.data[_pi];
                        if (_pd && _pd->kind == AST_STRUCT_DEF)
                            _pd->u.struct_def.from_inline_typedef = true;
                    }
                } else {
                    /* Non-typedef: the struct is being defined inline in a
                     * variable declaration (e.g. `struct X { } var` or
                     * `static struct X { } var`).
                     * Mark it so cg emits the struct body inline with the var.
                     *
                     * Set from_inline_var for SC_NONE too:
                     * Previously only non-SC_NONE storage classes (static, extern)
                     * triggered this, leaving `struct WindowUpdate { } aUp[]`
                     * (local var, no storage class) to emit as two separate
                     * statements — `struct WindowUpdate { };` then
                     * `WindowUpdate aUp[] = { };` — producing extra tokens
                     * `;` and `WindowUpdate` that gcc -E on the original does
                     * not have.  We now always mark inline structs as
                     * from_inline_var here; parse_declaration at the top level
                     * will clear the flag if no declarator follows (see below). */
                    for (size_t _pi = pd_before; _pi < ps->pending_decls.len; _pi++) {
                        AstNode *_pd = ps->pending_decls.data[_pi];
                        if (_pd && _pd->kind == AST_STRUCT_DEF &&
                            !_pd->u.struct_def.is_nested_in_struct)
                            _pd->u.struct_def.from_inline_var = true;
                    }
                }
                ds.empty = false;
                continue;
            }
        }

        /* Anything else ends the decl-specifier sequence. */
        break;
    }

    if (ds.empty) {
        return ds;
    }

    AstNode *base = tspec_resolve(ps, &ts, ds.loc);
    if (ds.is_const && ds.is_volatile) {
        /* Preserve qualifier order:
         * `const volatile T` → CONST(VOLATILE(T)) → emits `const volatile T`
         * `volatile const T` → VOLATILE(CONST(T)) → emits `volatile const T` */
        if (ds.fmt & FMTF_CONST_BEFORE_VOL) {
            /* const came first: CONST(VOLATILE(base)) */
            AstNode *v = ast_node_new(AST_TYPE_VOLATILE, ds.loc);
            v->u.type_volatile.base       = base;
            v->u.type_volatile.kw         = ds.volatile_kw;
            v->u.type_volatile.is_postfix = ds.fmt & FMTF_VOLATILE_POSTFIX;
            base = v;
            AstNode *c = ast_node_new(AST_TYPE_CONST, ds.loc);
            c->u.type_const.base       = base;
            c->u.type_const.is_postfix = ds.fmt & FMTF_CONST_POSTFIX;
            c->u.type_const.kw         = ds.const_kw;
            base = c;
        } else {
            /* volatile came first: VOLATILE(CONST(base)) */
            AstNode *c = ast_node_new(AST_TYPE_CONST, ds.loc);
            c->u.type_const.base       = base;
            c->u.type_const.is_postfix = ds.fmt & FMTF_CONST_POSTFIX;
            c->u.type_const.kw         = ds.const_kw;
            base = c;
            AstNode *v = ast_node_new(AST_TYPE_VOLATILE, ds.loc);
            v->u.type_volatile.base       = base;
            v->u.type_volatile.kw         = ds.volatile_kw;
            v->u.type_volatile.is_postfix = ds.fmt & FMTF_VOLATILE_POSTFIX;
            base = v;
        }
    } else if (ds.is_const) {
        AstNode *c = ast_node_new(AST_TYPE_CONST, ds.loc);
        c->u.type_const.base       = base;
        c->u.type_const.is_postfix = ds.fmt & FMTF_CONST_POSTFIX;
        c->u.type_const.kw         = ds.const_kw;    /* p45 */
        base = c;
    }
    if (ds.is_volatile && !ds.is_const) {
        /* Only apply volatile separately when const wasn't also present
         * (the const+volatile case was handled above to preserve ordering). */
        AstNode *v = ast_node_new(AST_TYPE_VOLATILE, ds.loc);
        v->u.type_volatile.base       = base;
        v->u.type_volatile.kw         = ds.volatile_kw;    /* p45 */
        v->u.type_volatile.is_postfix = ds.fmt & FMTF_VOLATILE_POSTFIX;
        base = v;
    }
    if (ds.is_atomic) {
        AstNode *a = ast_node_new(AST_TYPE_ATOMIC, ds.loc);
        a->u.type_atomic.base     = base;
        a->u.type_atomic.is_paren  = false;
        base = a;
    }
    /* post-type __attribute__ (e.g. `void __attribute__((noreturn))`):
     * wrap it onto the resolved base so it's emitted in the right position. */
    if (ds.post_type_attr) {
        AstNode *q = ast_node_new(AST_TYPE_VOLATILE, ds.loc);
        q->u.type_volatile.kw   = ds.post_type_attr;
        q->u.type_volatile.base = base;
        base = q;
        ds.post_type_attr = NULL;
    }
    ds.base_ty = base;
    ds.enum_body_ref = ts.enum_body_ref;  /* C8-fix: propagate non-owning enum ref */
    return ds;
}

/* =========================================================================
 * parse_declarator — ISO C 6.7.6 declarator.
 *
 * Recursive structure:
 *   declarator       := pointer? direct-declarator
 *   pointer          := ( '*' (const|volatile|restrict)* )+
 *   direct-declarator:= IDENT
 *                     | '(' declarator ')'
 *                     | direct-declarator '[' constant-expression? ']'
 *                     | direct-declarator '(' parameter-list ')'
 *
 * The function takes an outer base type and threads it through the inner
 * declarator using a sentinel placeholder node identified by pointer
 * equality.  After the full tree is built, the placeholder is spliced
 * out and replaced with whatever wraps the IDENT.
 * ====================================================================== */

static AstNode *parse_declarator(PS *ps, AstNode *base, char **out_name);

/* Allocate a unique sentinel node (an AST_TYPE_NAME whose name is a magic
 * marker) to be spliced out of the inner declarator's type tree later. */
static AstNode *placeholder_make(CppLoc loc) {
    AstNode *p = ast_node_new(AST_TYPE_NAME, loc);
    p->u.type_name.name = cpp_xstrdup("@@PLACEHOLDER@@");
    return p;
}

/* Splice: replace any direct child pointer in `root` that points to
 * `ph` with `replacement`.  We only need to look at the type-tree
 * structural-child positions (PTR.base, CONST.base, VOLATILE.base,
 * ARRAY.base, FUNC.ret).  Returns the (possibly new) root. */
static AstNode *splice_placeholder(AstNode *root, AstNode *ph, AstNode *replacement) {
    if (root == ph) return replacement;
    if (!root) return root;
    switch (root->kind) {
    case AST_TYPE_PTR:
        root->u.type_ptr.base =
            splice_placeholder(root->u.type_ptr.base, ph, replacement);
        break;
    case AST_TYPE_CONST:
        root->u.type_const.base =
            splice_placeholder(root->u.type_const.base, ph, replacement);
        break;
    case AST_TYPE_VOLATILE:
        root->u.type_volatile.base =
            splice_placeholder(root->u.type_volatile.base, ph, replacement);
        break;
    case AST_TYPE_ARRAY:
        root->u.type_array.base =
            splice_placeholder(root->u.type_array.base, ph, replacement);
        break;
    case AST_TYPE_FUNC:
        root->u.type_func.ret =
            splice_placeholder(root->u.type_func.ret, ph, replacement);
        break;
    default:
        break;
    }
    return root;
}

/* Decide whether `(` at the current position opens a sub-declarator
 * (e.g. `int (*p)(int)`, `int (foo)(int)`) or a parameter list
 * (e.g. `int f(int)`).
 *
 * The standard C disambiguation rule: in `( X ...`, the `(` opens a
 * sub-declarator iff X is NOT a typedef-name.  Concretely:
 *
 *   `(` `*`           -> sub-declarator  (pointer suffix on the inner)
 *   `(` `(`           -> sub-declarator  (further nesting)
 *   `(` IDENT `)`     -> sub-declarator iff IDENT is not a typedef-name
 *                        AND the close-paren is followed by something
 *                        a declarator can be followed by (`(` for fn,
 *                        `[` for array, `;` `,` `=` `{` for terminator)
 *   `(` IDENT IDENT…  -> parameter list (first IDENT is the param type)
 *   `(` `void` `)`    -> parameter list (idiomatic "no parameters")
 *   `(` <type-kw> …   -> parameter list
 *   `(` `)`           -> parameter list (empty / unspecified)
 *   anything else     -> parameter list
 *
 * The `( IDENT )` case is the one that lets `extern T (foo)(args);` —
 * Lua's macro-evasion idiom — parse correctly.
 */
static bool dd_paren_is_subdeclarator(const PS *ps) {
    if (!ps_at(ps, STOK_LPAREN)) return false;
    SharpTokKind k2 = ps_peek2(ps).kind;
    if (k2 == STOK_STAR || k2 == STOK_LPAREN) return true;
    /* `( [ ... ] )` — abstract grouped array declarator, e.g. `int ([4])`. */
    if (k2 == STOK_LBRACKET) return true;
    /* `( __attribute__(...) * )` — grouped pointer declarator with a leading
     * attribute, e.g. `int(ATTR *)(void)` cast.  Peek past the full
     * __attribute__((...)) to check for a `*`. */
    if (k2 == STOK_ATTRIBUTE) {
        int pi = 1;  /* k2 = ps_peek_at(ps,1) = ATTRIBUTE; start there */
        while (ps_peek_at(ps, pi).kind == STOK_ATTRIBUTE) {
            pi++;  /* skip __attribute__ keyword */
            if (ps_peek_at(ps, pi).kind != STOK_LPAREN) break;
            int d = 0;
            while (1) {
                SharpTokKind pk = ps_peek_at(ps, pi).kind;
                if (pk == STOK_EOF) goto attr_ptr_done;
                if (pk == STOK_LPAREN) { d++; pi++; }
                else if (pk == STOK_RPAREN) { d--; pi++; if (d == 0) break; }
                else pi++;
            }
        }
        if (ps_peek_at(ps, pi).kind == STOK_STAR) return true;
        attr_ptr_done:;
    }
    if (k2 == STOK_IDENT) {
        /* Distinguish `( typedef-name ... )` (parameter list) from
         * `( declared-name ) ...` (sub-declarator). */
        SharpTok name = ps->toks[ps->pos + 1];
        if (td_has_n(&ps->typedefs, name.text, name.len))
            return false;  /* typedef-name => parameter list */
        /* Not a known typedef name.  Confirm the shape is `( IDENT )`
         * followed by something that can follow a sub-declarator —
         * otherwise it's still safer to treat as parameter list (the
         * IDENT might be a typedef from a header we haven't fully
         * processed in some edge case). */
        SharpTokKind k3 = ps_peek3(ps).kind;
        if (k3 != STOK_RPAREN) return false;
        size_t i4 = ps->pos + 3;
        SharpTokKind k4 = i4 < ps->ntoks ? ps->toks[i4].kind : STOK_EOF;
        switch (k4) {
        case STOK_LPAREN:    /* foo(args)        — function */
        case STOK_LBRACKET:  /* foo[N]           — array    */
        case STOK_SEMI:      /* foo;             — bare decl */
        case STOK_COMMA:     /* foo, bar         — multi-decl */
        case STOK_EQ:        /* foo = init       — initialiser */
        case STOK_LBRACE:    /* foo { body }     — function definition */
            return true;
        default:
            return false;
        }
    }
    return false;
}

/* Pointer prefix: zero or more `*` each optionally followed by const /
 * volatile / restrict in any order.  Wraps `base` accordingly. */
static AstNode *parse_pointer_prefix(PS *ps, AstNode *base) {
    /* GCC allows __attribute__ before the `*` in a grouped declarator,
     * e.g. `int (ATTR *)(void)`.  Consume and discard such leading attrs. */
    while (ps_at(ps, STOK_ATTRIBUTE))
        eat_attribute_specifiers(ps, NULL);
    while (ps_at(ps, STOK_STAR)) {
        CppLoc star_loc = ps_advance(ps).loc;  /* eat '*' */
        AstNode *ptr = ast_node_new(AST_TYPE_PTR, star_loc);
        ptr->u.type_ptr.base         = base;
        ptr->u.type_ptr.ptr_const    = false;
        ptr->u.type_ptr.ptr_volatile = false;
        ptr->u.type_ptr.ptr_restrict = false;
        base = ptr;
        /* qualifiers on the pointer itself: T * const, T * volatile, T * restrict */
        for (;;) {
            if (ps_at(ps, STOK_CONST)) {
                ps_advance(ps);
                ptr->u.type_ptr.ptr_const = true;
            } else if (ps_at(ps, STOK_VOLATILE)) {
                ps_advance(ps);
                ptr->u.type_ptr.ptr_volatile = true;
            } else if (ps_at(ps, STOK_RESTRICT) || ps_at(ps, STOK__ATOMIC)) {
                SharpTok rk = ps_advance(ps);
                ptr->u.type_ptr.ptr_restrict = true;
                if (!ptr->u.type_ptr.restrict_kw)
                    ptr->u.type_ptr.restrict_kw = cpp_xstrndup(rk.text, rk.len);
            } else if (ps_at(ps, STOK__NULLABLE) || ps_at(ps, STOK__NONNULL) ||
                       ps_at(ps, STOK__NULL_UNSPEC)) {
                SharpTok nk = ps_advance(ps);
                ptr->u.type_ptr.nullability = cpp_xstrndup(nk.text, nk.len);
            } else if (ps_at(ps, STOK_ATTRIBUTE) || ps_at(ps, STOK_ASM) || ps_at(ps, STOK_DECLSPEC)) {
                eat_attribute_specifiers(ps, NULL);
            } else {
                break;
            }
        }
    }
    return base;
}

/* Parse a direct-declarator with the given outer base type.  Sets
 * *out_name to the parsed identifier, or NULL for an abstract declarator.
 *
 * Suffixes (`[N]`, `(params)`) are LEFT-recursive in the C grammar: the
 * first suffix encountered is the OUTERMOST in the resulting type tree.
 * To build the tree iteratively we keep an `inner` pointer-to-pointer
 * that always points at the slot where the *next* element type plugs in.
 * On each new suffix we wrap whatever currently lives at `*inner` and
 * then descend `inner` one level into the freshly-allocated wrapper.
 *
 * Example trace for `m[3][4]`:
 *   start:        tree=int, inner=&tree
 *   see [3]:      new = ARRAY(base=int, size=3); *inner=new;
 *                 inner=&new.base; tree=ARRAY(int,3)
 *   see [4]:      new2 = ARRAY(base=int, size=4); *inner=new2;
 *                 inner=&new2.base; tree=ARRAY(ARRAY(int,4),3)
 *   final tree:   ARRAY(size=3, base=ARRAY(size=4, base=int))  ✓
 */
static AstNode *parse_direct_declarator(PS *ps, AstNode *base, char **out_name) {
    /* Stage 1: prefix — either '(' declarator ')' or IDENT or nothing. */
    AstNode *inner_root = NULL;
    AstNode *placeholder = NULL;

    if (dd_paren_is_subdeclarator(ps)) {
        ps_advance(ps);  /* '(' */
        placeholder = placeholder_make(ps_peek(ps).loc);
        ps->last_name_paren = false;  /* C8: reset first */
        inner_root  = parse_declarator(ps, placeholder, out_name);
        ps_expect(ps, STOK_RPAREN, "closing ')' of grouped declarator");
        /* C8: only mark name_paren for simple `(name)` — not for `(*name)`,
         * `(* name(params))`, etc. which are full declarators, not just names. */
        if (inner_root && inner_root->kind == AST_TYPE_NAME)
            ps->last_name_paren = true;   /* plain (name) form */
        else
            ps->last_name_paren = false;  /* complex declarator */
    } else if (ps_at(ps, STOK_IDENT)) {
        ps->last_name_paren = false;  /* C8: plain name, not parenthesized */
        SharpTok nm = ps_advance(ps);
        *out_name = cpp_xstrndup(nm.text, nm.len);
        /* R14__attribute__ between declarator name and suffix.
         * Consume but save text for caller inspection (e.g. transparent_union). */
        if (ps_at(ps, STOK_ATTRIBUTE)) {
            char *_decl_attr = NULL;
            eat_attribute_specifiers(ps, &_decl_attr);
            /* Store on ps so parse_top_decl can pick it up */
            if (ps->last_decl_attr) free(ps->last_decl_attr);
            ps->last_decl_attr = _decl_attr;
            (void)ps->last_decl_attr; /* prevent unused warning */
        }
    } else {
        /* abstract declarator: no name, no inner */
        *out_name = NULL;
    }

    /* Stage 2: collect array / function suffixes, wrapping `base`.
     * `inner` always tracks the slot where the next deeper element type
     * lives, so consecutive suffixes nest in left-to-right order. */
    AstNode  *tree  = base;
    AstNode **inner = &tree;
    /* C8: save last_name_paren before the suffix loop — parsing params
     * clobbers it via recursive parse_direct_declarator calls. */
    bool saved_name_paren = ps->last_name_paren;
    for (;;) {
        if (ps_at(ps, STOK_LBRACKET)) {
            CppLoc lb = ps_advance(ps).loc;
            /*  C8: eat C99 VLA qualifiers inside [...].
             * Record restrict/static for faithful round-trip output. */
            bool dd_restrict = false, dd_static = false;
            for (;;) {
                if (ps_at(ps, STOK_RESTRICT) || ps_at(ps, STOK__ATOMIC) ||
                    ps_at(ps, STOK_VOLATILE)  || ps_at(ps, STOK_CONST)) {
                    if (ps_at(ps, STOK_RESTRICT)) dd_restrict = true;
                    ps_advance(ps);
                } else if (ps_at(ps, STOK_STATIC)) {
                    dd_static = true;
                    ps_advance(ps);
                } else break;
            }
            AstNode *sz = NULL;
            bool vla_star = false;
            if (ps_at(ps, STOK_STAR)) {
                ps_advance(ps);
                vla_star = true;
            } else if (!ps_at(ps, STOK_RBRACKET)) {
                sz = parse_expr_prec(ps, 2);
            }
            ps_expect(ps, STOK_RBRACKET, "array suffix ']'");
            AstNode *arr = ast_node_new(AST_TYPE_ARRAY, lb);
            arr->u.type_array.base        = *inner;
            arr->u.type_array.size        = sz;
            arr->u.type_array.has_restrict = dd_restrict;
            arr->u.type_array.has_static   = dd_static;
            arr->u.type_array.has_vla_star = vla_star;
            *inner = arr;
            inner  = &arr->u.type_array.base;
        } else if (ps_at(ps, STOK_LPAREN)) {
            CppLoc lp = ps_advance(ps).loc;
            AstNode *fn = ast_node_new(AST_TYPE_FUNC, lp);
            fn->u.type_func.ret = *inner;
            ps->last_params_unspecified = false;
            parse_param_list_inner(ps, &fn->u.type_func.params);
            /* C3: store the unspecified flag in the type node for propagation */
            fn->u.type_func.params_unspecified = ps->last_params_unspecified;
            ps->last_params_unspecified = false;
            ps_expect(ps, STOK_RPAREN, "function-declarator ')'");
            *inner = fn;
            inner  = &fn->u.type_func.ret;
        } else {
            break;
        }
    }
    /* C8: restore last_name_paren that may have been clobbered by param parsing */
    ps->last_name_paren = saved_name_paren;

    /* Stage 3: splice the placeholder if we recursed. */
    if (inner_root) {
        inner_root = splice_placeholder(inner_root, placeholder, tree);
        free(placeholder->u.type_name.name);
        free(placeholder);
        return inner_root;
    }
    return tree;
}

static AstNode *parse_declarator(PS *ps, AstNode *base, char **out_name) {
    base = parse_pointer_prefix(ps, base);
    return parse_direct_declarator(ps, base, out_name);
}

/* =========================================================================
 * Parameter list — uses parse_decl_specifiers + parse_declarator.
 * The `param_list_inner` form is shared between the function-declarator
 * suffix in parse_direct_declarator and the public parse_param_list call
 * site (function definitions).  Both forms expect the caller to consume
 * the surrounding parentheses.
 * ====================================================================== */
static void parse_param_list_inner(PS *ps, AstVec *params) {
    /* `(void)` — explicit empty parameter list (C convention). */
    if (ps_at(ps, STOK_VOID) && ps_peek2(ps).kind == STOK_RPAREN) {
        ps_advance(ps);  /* void */
        return;
    }
    /* `()` — empty list (C++ style; in C this means "unspecified", but we
     * treat it the same as (void) for transpilation).
     * C3: mark caller's func_def as params_unspecified so cg can preserve
     * the original empty-parens form in C mode. */
    if (ps_at(ps, STOK_RPAREN)) {
        /* Signal via a sentinel: push a dummy param with is_vararg=false
         * and type=NULL that cg_func will detect and remove.
         * Simpler: rely on the caller checking ps->last_params_unspecified. */
        ps->last_params_unspecified = true;
        return;
    }

    while (!ps_at(ps, STOK_RPAREN) && !ps_at(ps, STOK_EOF)) {
        if (ps_at(ps, STOK_ELLIPSIS)) {
            CppLoc el = ps_advance(ps).loc;
            AstNode *p = ast_node_new(AST_PARAM_DECL, el);
            p->u.param_decl.is_vararg = true;
            p->u.param_decl.type = ast_node_new(AST_TYPE_VOID, el);
            p->u.param_decl.name = cpp_xstrndup("...", 3);
            astvec_push(params, p);
            break;
        }

        /* Extension method self parameter: `this`. Type will be resolved
         * later in scope pass (pointer to the enclosing struct/class). */
        if (ps_at(ps, STOK_THIS)) {
            SharpTok this_tok = ps_advance(ps);
            AstNode *pd = ast_node_new(AST_PARAM_DECL, this_tok.loc);
            pd->u.param_decl.name = cpp_xstrndup("this", 4);
            pd->u.param_decl.type = NULL;  /* resolved in scope pass 2 */
            astvec_push(params, pd);
            if (ps_at(ps, STOK_COMMA)) { ps_advance(ps); continue; }
            if (ps_at(ps, STOK_RPAREN)) break;
            continue;
        }

        DeclSpecs ds = parse_decl_specifiers(ps);
        if (ds.empty) {
            ps_error(ps, ps_peek(ps).loc,
                "expected parameter type, got '%.*s'",
                (int)ps_peek(ps).len, ps_peek(ps).text);
            /* try to recover: skip to next ',' or ')' */
            while (!ps_at(ps, STOK_COMMA) &&
                   !ps_at(ps, STOK_RPAREN) &&
                   !ps_at(ps, STOK_EOF)) {
                ps_advance(ps);
            }
            if (ps_at(ps, STOK_COMMA)) { ps_advance(ps); continue; }
            return;
        }
        char *pname = NULL;
        AstNode *pty = parse_declarator(ps, ds.base_ty, &pname);

        /* C7: capture __attribute__ after parameter declarator for passthrough.
         * parse_direct_declarator already consumed any __attribute__ right
         * after the name into ps->last_decl_attr; also check for any further
         * trailing attributes. */
        char *param_attrs = ps->last_decl_attr;
        ps->last_decl_attr = NULL;
        eat_attribute_specifiers(ps, &param_attrs);

        AstNode *pd = ast_node_new(AST_PARAM_DECL, ds.loc);
        pd->u.param_decl.type      = pty;
        pd->u.param_decl.name      = pname;  /* may be NULL for prototype */
        pd->u.param_decl.gcc_attrs = param_attrs;
        pd->u.param_decl.storage   = ds.storage; /* register on params */
        astvec_push(params, pd);

        if (!ps_match(ps, STOK_COMMA)) break;
    }
}

/* Public wrapper used by function definitions: consumes the surrounding
 * '(' and ')'.  parse_direct_declarator's function-suffix path calls
 * the inner version directly because it has already eaten the '('. */
static void parse_param_list(PS *ps, AstVec *params) {
    ps_expect(ps, STOK_LPAREN, "parameter list '('");
    parse_param_list_inner(ps, params);
    ps_expect(ps, STOK_RPAREN, "parameter list ')'");
}

static bool is_kr_param_list(const AstVec *params) {
    bool all_bare = params->len > 0;
    for (size_t pi = 0; pi < params->len && all_bare; pi++) {
        AstNode *p = params->data[pi];
        if (!p || p->kind != AST_PARAM_DECL) { all_bare = false; break; }
        if (p->u.param_decl.is_vararg) continue;
        if (p->u.param_decl.name != NULL) { all_bare = false; break; }
        if (!p->u.param_decl.type ||
            p->u.param_decl.type->kind != AST_TYPE_NAME) { all_bare = false; break; }
        const char *tname = p->u.param_decl.type->u.type_name.name;
        if (!tname) { all_bare = false; break; }
        static const char * const prims[] = {
            "int","char","short","long","float","double","void","unsigned",
            "signed","_Bool","__int128", NULL };
        for (int ki = 0; prims[ki]; ki++) {
            if (strcmp(tname, prims[ki]) == 0) { all_bare = false; break; }
        }
    }
    return all_bare;
}

static void consume_kr_declarations(PS *ps, AstVec *params) {
    while (!ps_at(ps, STOK_LBRACE) && !ps_at(ps, STOK_SEMI) && !ps_at(ps, STOK_EOF)) {
        DeclSpecs kds = parse_decl_specifiers(ps);
        if (kds.empty) break;
        do {
            char *pname = NULL;
            AstNode *pty = parse_declarator(ps, ast_clone_type(kds.base_ty), &pname);
            if (pname) {
                for (size_t pi = 0; pi < params->len; pi++) {
                    AstNode *p = params->data[pi];
                    if (!p || p->kind != AST_PARAM_DECL) continue;
                    const char *kr_name = p->u.param_decl.type &&
                        p->u.param_decl.type->kind == AST_TYPE_NAME
                        ? p->u.param_decl.type->u.type_name.name : NULL;
                    if (kr_name && strcmp(kr_name, pname) == 0) {
                        ast_node_free(p->u.param_decl.type);
                        p->u.param_decl.type = pty;
                        p->u.param_decl.name = cpp_xstrdup(pname);
                        pty = NULL; break;
                    }
                }
                free(pname);
            }
            if (pty) ast_node_free(pty);
        } while (ps_match(ps, STOK_COMMA));
        ps_match(ps, STOK_SEMI);
        ast_node_free(kds.base_ty);
    }
}

/* =========================================================================
 * Function definition body (after name + generic params + param list)
 * ====================================================================== */
static AstNode *finish_func(PS *ps, AstNode *ret_type,
                             SharpTok name_tok, bool is_operator) {
    AstNode *fn = ast_node_new(AST_FUNC_DEF, name_tok.loc);
    fn->u.func_def.name        = cpp_xstrndup(name_tok.text, name_tok.len);
    fn->u.func_def.ret_type    = ret_type;
    fn->u.func_def.is_operator = is_operator;

    /* optional <T, U> */
    parse_generic_params(ps, &fn->u.func_def.generic_params);

    /* (params) */
    ps->last_params_unspecified = false;
    parse_param_list(ps, &fn->u.func_def.params);
    /* C3: record whether the source used `()` vs `(void)` */
    fn->u.func_def.params_unspecified = ps->last_params_unspecified;
    ps->last_params_unspecified = false;

    /* optional 'const' suffix (const method) */
    if (ps_match(ps, STOK_CONST)) fn->u.func_def.is_const_method = true;

    /* C8/C-superset: K&R old-style parameter type declarations.
     * Detect: after (a, b) the next token is a type specifier (not '{' or ';').
     * In K&R form, each param in (a, b) is parsed as type=TYPE_NAME{"a"} with
     * name=NULL (the identifier was consumed as the "type specifier").
     * Form: `int add(a, b) int a; int b; { ... }` */
    if (is_kr_param_list(&fn->u.func_def.params)) {
        consume_kr_declarations(ps, &fn->u.func_def.params);
    }

    /* body or ';' for forward decl */
    if (ps_at(ps, STOK_LBRACE)) {
        fn->u.func_def.body = parse_block(ps);
    } else {
        ps_match(ps, STOK_SEMI);
    }
    return fn;
}

/* =========================================================================
 * S2: parse_enum_def — `enum [Tag] { NAME [= expr], ... };`
 *
 * Sharp does not give enums a distinct type at the Type* layer; the C
 * compiler treats enum constants as ints and we follow.  We do however
 * preserve the enum body in the generated C verbatim so the C compiler
 * sees the same declaration.  Each enumerator is registered as a SYM_VAR
 * in scope.c so that bare references (`return RED;`) resolve.
 *
 * Anonymous enums (`enum { A, B };`) are accepted; the tag is NULL and
 * cg.c emits `enum { ... };`.
 * ====================================================================== */
static AstNode *parse_enum_def(PS *ps) {
    SharpTok kw = ps_expect(ps, STOK_ENUM, "enum keyword");
    AstNode  *ed = ast_node_new(AST_ENUM_DEF, kw.loc);
    ed->u.enum_def.name = NULL;
    if (ps_at(ps, STOK_IDENT)) {
        SharpTok nm = ps_advance(ps);
        ed->u.enum_def.name = cpp_xstrndup(nm.text, nm.len);
    }
    /* `enum Tag;` (forward) and `enum Tag x;` (use as type) are handled
     * elsewhere — this routine is reached only when the next token is
     * `{`.  parse_top_decl decides which path to take. */
    ps_expect(ps, STOK_LBRACE, "enum body '{'");
    while (!ps_at(ps, STOK_RBRACE) && !ps_at(ps, STOK_EOF)) {
        SharpTok inm = ps_expect(ps, STOK_IDENT, "enumerator name");
        AstNode *en  = ast_node_new(AST_ENUMERATOR, inm.loc);
        en->u.enumerator.name = cpp_xstrndup(inm.text, inm.len);
        /* GCC allows __attribute__((deprecated(...))) between
         * the enumerator name and its optional `= value`.  curl.h uses
         * this pattern extensively.  Consume and discard all consecutive
         * attributes (macOS headers can emit 4+ availability attrs). */
        while (ps_at(ps, STOK_ATTRIBUTE)) {
            ps_advance(ps);  /* eat __attribute__ */
            if (ps_at(ps, STOK_LPAREN)) {
                int depth = 0;
                while (!ps_at(ps, STOK_EOF)) {
                    SharpTok tk = ps_advance(ps);
                    if (tk.kind == STOK_LPAREN) depth++;
                    else if (tk.kind == STOK_RPAREN) {
                        depth--;
                        if (depth == 0) break;
                    }
                }
            }
        }
        if (ps_match(ps, STOK_EQ)) {
            /* prec-2 to skip the comma operator: `A = 1, B = 2`. */
            en->u.enumerator.value = parse_expr_prec(ps, 2);
        }
        astvec_push(&ed->u.enum_def.items, en);
        if (!ps_match(ps, STOK_COMMA)) break;
        /* C allows a trailing comma (C99); preserve flag for faithful re-emission. */
        if (ps_at(ps, STOK_RBRACE)) {
            ed->u.enum_def.trailing_comma = true;
            break;
        }
    }
    ps_expect(ps, STOK_RBRACE, "enum body '}'");
    /* do NOT consume the trailing `;` here.  The various
     * call sites own that token themselves:
     *   - parse_top_decl: ps_expects ';' after the returned ed
     *   - tspec_try_consume (stmt / field decl-spec context): the
     *     outer init-declarator-list / field-list expects ';'
     *   - typedef path: continues with the alias, no ';' expected
     * Eating it here was wrong in stmt context: it caused
     * `enum { W=16, H=16 }; int x = W+H;` to drop the `;` after the
     * enum body, after which the outer parse_decl_specifiers happily
     * consumed `int x` as a continuation of the same declaration —
     * the AST_VAR_DECL ended up emitted BEFORE the AST_ENUM_DEF in
     * the block, breaking compile order.  stb_driver.c hits this. */
    return ed;
}

/* =========================================================================
 * Struct definition
 * ====================================================================== */
static AstNode *parse_struct_def(PS *ps) {
    /* S2: same parser handles `struct Name { ... }` and `union Name { ... }`.
     * The kind is recorded on the AST node so cg.c can emit the right
     * keyword.  Field-access semantics are identical at the AST level
     * (both use FIELD_ACCESS); the C compiler enforces union-overlap
     * storage rules for us.
     *
     * Anonymous form `struct { ... }` (no tag) is also accepted here —
     * used by `typedef struct { ... } Name;` and similar idioms.  We
     * synthesise a unique tag like `__anon_struct_<n>` so downstream
     * code (sema, cg) can refer to the type by name; the synthetic name
     * is only visible in generated C output, which is functionally
     * equivalent to the source-level anonymous form. */
    SharpTok kw = ps_peek(ps);
    bool is_union = (kw.kind == STOK_UNION);
    bool is_class = (kw.kind == STOK_CLASS);
    if (is_union)       ps_advance(ps);
    else if (is_class)  ps_advance(ps);
    else                ps_expect(ps, STOK_STRUCT, "struct/class keyword");

    /* handle `struct __attribute__((...)) Tag { ... }`.
     * In GNU C, attributes can appear immediately after the struct/union
     * keyword, before the tag name.  Capture them for later emission. */
    char *struct_leading_attrs = NULL;
    while (ps_at(ps, STOK_ATTRIBUTE)) {
        char *attr_text = NULL;
        eat_attribute_specifiers(ps, &attr_text);
        /* Prepend to existing (in case of multiple __attribute__) */
        if (struct_leading_attrs && attr_text) {
            size_t old_len = strlen(struct_leading_attrs);
            size_t new_len = strlen(attr_text);
            char *merged = malloc(old_len + 1 + new_len + 1);
            memcpy(merged, struct_leading_attrs, old_len);
            merged[old_len] = ' ';
            memcpy(merged + old_len + 1, attr_text, new_len + 1);
            free(attr_text);
            free(struct_leading_attrs);
            struct_leading_attrs = merged;
        } else if (attr_text) {
            struct_leading_attrs = attr_text;
        }
    }

    AstNode  *sd;
    if (ps_at(ps, STOK_IDENT)) {
        SharpTok nm  = ps_advance(ps);
        sd = ast_node_new(AST_STRUCT_DEF, nm.loc);
        sd->u.struct_def.name = cpp_xstrndup(nm.text, nm.len);
    } else {
        /* Anonymous: synthesise a unique tag.  The counter lives on PS so
         * each compilation unit gets its own monotonic numbering. */
        char synth[32];
        snprintf(synth, sizeof synth, "__anon_%s_%u",
                 is_union ? "union" : "struct",
                 ps->anon_struct_counter++);
        sd = ast_node_new(AST_STRUCT_DEF, kw.loc);
        sd->u.struct_def.name = cpp_xstrdup(synth);
    }
    sd->u.struct_def.is_union = is_union;
    sd->u.struct_def.is_class = is_class;
    sd->u.struct_def.leading_attrs = struct_leading_attrs; /* */

    parse_generic_params(ps, &sd->u.struct_def.generic_params);

    /* Register tag name immediately so that self-referential fields
     * (e.g., Node<T>* next; inside struct Node<T>) and methods
     * (e.g., Stack<T> new() inside struct Stack<T>) are recognised
     * as types during body parsing.  Also registered into tag_names
     * so that is_type_start's IDENT-RPAREN heuristic can exclude
     * struct/union/enum tags from the cast-detection path — bare
     * (tag_name) is never a cast in C/C-superset.
     *
     * In Sharp (like C++), struct names are automatically usable as
     * type names without requiring an explicit typedef — this matches
     * C++ semantics. */
    if (sd->u.struct_def.name) {
        td_add(&ps->typedefs, sd->u.struct_def.name);
        td_add(&ps->tag_names, sd->u.struct_def.name);
    }

    /* optional forward decl (just 'struct Name;') */
    if (ps_match(ps, STOK_SEMI)) {
        sd->u.struct_def.has_body = false; /* forward decl, no body */
        return sd;
    }

    sd->u.struct_def.has_body = true; /* '{...}' was present */
    ps_expect(ps, STOK_LBRACE, "struct body '{'");

    ps->struct_body_depth++;
    while (!ps_at(ps, STOK_RBRACE) && !ps_at(ps, STOK_EOF)) {
        /* C8: __extension__ may prefix a field declaration inside a struct
         * (e.g. glibc ucontext.h: `__extension__ unsigned long long __x[8]`).
         * Consume it silently, mirroring parse_top_decl's handling. */
        {
            SharpTok _et = ps_peek(ps);
            if (_et.kind == STOK__EXTENSION__)
                ps_advance(ps);
        }
        if (!is_type_start(ps)) {
            ps_error(ps, ps_peek(ps).loc, "unexpected token in struct body");
            ps_advance(ps);
            continue;
        }

        /* Parse the decl-specifier sequence (base type + qualifiers).
         * For struct/union body fields we use parse_decl_specifiers so
         * that the comma-list logic can clone the BASE type (before
         * pointer/array suffixes) for each additional declarator.
         * Previously, parse_type (which applies apply_suffix and eats
         * `*`) was used here, so `struct CI *previous, *next` would
         * clone `CI*` (already pointer-wrapped) and then parse_declarator
         * would add another `*` for `*next`, producing `CI**`. */
        DeclSpecs fds = parse_decl_specifiers(ps);
        if (fds.empty || !fds.base_ty) {
            ps_error(ps, ps_peek(ps).loc, "expected field type");
            ps_advance(ps);
            continue;
        }
        AstNode *base_ty = fds.base_ty;

        /* Check for static field: `static int count;` */
        bool is_static_field = false;
        if (is_class && ps_at(ps, STOK_STATIC)) {
            is_static_field = true;
            ps_advance(ps);
        }

        SharpTok name_tok = {0};
        char    *field_name = NULL;

        /* General declarator: handles `*name`, `(*name)(args)`,
         * `name[N]`, `name[N][M]`, and combinations.  We pass
         * base_ty; parse_declarator wraps it appropriately.
         * The name comes back through *out. */
        AstNode *field_ty = parse_declarator(ps, base_ty, &field_name);
        if (!field_name) {
            /* C11 anonymous struct/union field (no name).  Legal
             * when the field's declared type is itself a struct or
             * union (typically inline-defined just above).  Members
             * of the anonymous aggregate are accessed as if they
             * were direct members of the enclosing struct, but
             * Sharp doesn't synthesise that flattening; we just
             * emit a synthetic field name.  glibc's
             * `<bits/types/struct_rusage.h>` uses this pattern.
             *
             * Detection: parse_decl_specifiers may have synthesised
             * a tag prefixed `__anon_struct_` or `__anon_union_`
             * for an inline-defined body; that's our cue. */
            bool is_anon_aggregate =
                field_ty && field_ty->kind == AST_TYPE_NAME &&
                field_ty->u.type_name.name &&
                strncmp(field_ty->u.type_name.name, "__anon_", 7) == 0;
            if (is_anon_aggregate) {
                char synth[40];
                snprintf(synth, sizeof synth,
                         "__anon_field_%u", ps->anon_struct_counter++);
                field_name = cpp_xstrdup(synth);
            } else if (ps_at(ps, STOK_COLON)) {
                /* ISO C99 anonymous bit-field — `int :32;`
                 * or `int :0;` for explicit alignment / padding. */
                field_name = cpp_xstrdup("");
            } else {
                ps_error(ps, ps_peek(ps).loc,
                    "field declaration is missing a name");
                field_name = cpp_xstrdup("?");
            }
        }
        name_tok.loc = ps_peek(ps).loc;

        /* S5: GCC `__attribute__((aligned(...))) ` etc. after a field
         * declarator.  C7: capture verbatim for C mode passthrough.
         * NOTE: parse_direct_declarator already consumed any __attribute__
         * that appeared immediately after the field name (it's stored in
         * ps->last_decl_attr).  Check both that AND any remaining trailing
         * attributes after the declarator. */
        char *trailing_field_attrs = ps->last_decl_attr;
        ps->last_decl_attr = NULL;  /* take ownership */
        eat_attribute_specifiers(ps, &trailing_field_attrs);  /* further attrs */

        /* S4: bit-field width — `unsigned int a : 3;`. */
        AstNode *bw = NULL;
        if (ps_at(ps, STOK_COLON)) {
            ps_advance(ps);
            bw = parse_expr_prec(ps, 2);
        }

        if (ps_at(ps, STOK_SEMI) || ps_at(ps, STOK_COMMA)) {
            /* field declaration; supports comma list `int x, y, z;`
             * C7: merge leading (fds.gcc_attrs) and trailing attrs. */
            char *field_attrs = trailing_field_attrs;
            if (fds.gcc_attrs && field_attrs) {
                /* Combine: "leading trailing" */
                size_t len = strlen(fds.gcc_attrs) + 1 + strlen(field_attrs) + 1;
                char *combined = malloc(len);
                if (combined) {
                    snprintf(combined, len, "%s %s", fds.gcc_attrs, field_attrs);
                    free(field_attrs);
                    field_attrs = combined;
                }
                free(fds.gcc_attrs);
            } else if (fds.gcc_attrs) {
                field_attrs = fds.gcc_attrs;
            }
            fds.gcc_attrs = NULL;
            AstNode *fd = ast_node_new(AST_FIELD_DECL, name_tok.loc);
            fd->u.field_decl.type      = field_ty;
            fd->u.field_decl.name      = field_name;
            fd->u.field_decl.bit_width = bw;
            fd->u.field_decl.gcc_attrs = field_attrs;
            fd->u.field_decl.name_paren = ps->last_name_paren; /* Preserve (name) parens from source */
            fd->u.field_decl.is_static = is_static_field;
            astvec_push(&sd->u.struct_def.fields, fd);
            /* Comma list — additional declarators share the BASE type
             * (not the full declarator result which includes pointer/
             * array suffixes).  Each gets a deep clone of base_ty so
             * parse_declarator can apply its own `*` / `[]` independently.
             * Example: `struct CI *previous, *next` — base_ty = `CI`,
             * each declarator contributes its own `*`. */
            while (ps_match(ps, STOK_COMMA)) {
                AstNode *base_copy = ast_clone_type(base_ty);
                char    *next_name = NULL;
                CppLoc   next_loc  = ps_peek(ps).loc;
                AstNode *next_ty   = parse_declarator(ps, base_copy, &next_name);
                eat_attribute_specifiers(ps, NULL); /* field attrs: cc's concern */
                if (!next_name && ps_at(ps, STOK_COLON)) {
                    next_name = cpp_xstrdup("");
                } else if (!next_name) {
                    ps_error(ps, next_loc, "field declaration is missing a name");
                    next_name = cpp_xstrdup("?");
                }
                /* parse optional bitfield width for each
                 * declarator in the comma list (e.g. `u16 a:4, b:4`). */
                AstNode *bw2 = NULL;
                if (ps_at(ps, STOK_COLON)) {
                    ps_advance(ps);
                    bw2 = parse_expr_prec(ps, 2);
                }
                AstNode *fd2 = ast_node_new(AST_FIELD_DECL, next_loc);
                fd2->u.field_decl.type         = next_ty;
                fd2->u.field_decl.name         = next_name;
                fd2->u.field_decl.bit_width    = bw2;
                fd2->u.field_decl.is_comma_cont = true;
                fd2->u.field_decl.is_static = is_static_field;
                astvec_push(&sd->u.struct_def.fields, fd2);
            }
            ps_expect(ps, STOK_SEMI, "after field declaration");
        } else {
            ps_error(ps, ps_peek(ps).loc,
                "expected ';' after field declaration");
            ps_advance(ps);
            ast_node_free(field_ty);
            free(field_name);
            free(fds.gcc_attrs); /* avoid leak on error path */
            fds.gcc_attrs = NULL;
        }
        continue;
    }
    ps->struct_body_depth--;
    ps_expect(ps, STOK_RBRACE, "struct closing '}'");
    /* Capture ALL trailing __attribute__ after '}' and before ';' or declarator.
     * This handles packed, aligned, may_alias, transparent_union, etc.
     * Previous code only detected transparent_union and lost everything else. */
    if (ps_at(ps, STOK_ATTRIBUTE)) {
        char *attr_text = NULL;
        eat_attribute_specifiers(ps, &attr_text);
        if (attr_text) {
            sd->u.struct_def.tail_attrs = attr_text;
            if (sd->u.struct_def.is_union && strstr(attr_text, "transparent_union"))
                sd->u.struct_def.is_transparent_union = true;
        }
    }
    /* The trailing `;` is owned by the caller — for top-level
     * `struct X { ... };` parse_top_decl consumes it; for inline use
     * inside a typedef or as a field type the surrounding declarator
     * path consumes its own `;`.  Eating it here would steal the
     * outer `;` and produce spurious "expected ';'" diagnostics on
     * anonymous-union fields, e.g. `struct S { union {...}; };`. */
    return sd;
}

/* Parse `[ size ]` array suffix(es) immediately after an IDENT in a declarator.
 * Wraps `inner_ty` in AST_TYPE_ARRAY for each suffix encountered.
 * Returns the (possibly unmodified) type AST.  Multi-dim like `[3][4]` is
 * supported and produces nested array types: arr[3][4] → array(array(T,4),3).
 *
 * Note on multi-dim ordering: in C, `int a[3][4]` means an array of 3
 * arrays of 4 ints.  The leftmost subscript is the outermost dimension.
 * So we must wrap left-to-right with the *first* suffix as the OUTERMOST
 * array.  We therefore parse suffixes in source order and apply them
 * left-to-right by wrapping outward — see code below. */
static AstNode *parse_array_suffix(PS *ps, AstNode *inner_ty) {
    AstNode *sizes[16];
    CppLoc   locs[16];
    bool     arr_restrict[16];
    bool     arr_static_kw[16];
    bool     arr_vla_star[16];
    size_t   nsizes = 0;
    while (ps_at(ps, STOK_LBRACKET)) {
        if (nsizes >= 16) {
            ps_error(ps, ps_peek(ps).loc, "too many array dimensions (max 16)");
            break;
        }
        SharpTok lb = ps_advance(ps);  /* '[' */
        bool arr_has_restrict = false, arr_has_static = false;
        for (;;) {
            if (ps_at(ps, STOK_RESTRICT) || ps_at(ps, STOK__ATOMIC) ||
                ps_at(ps, STOK_VOLATILE) || ps_at(ps, STOK_CONST)) {
                if (ps_at(ps, STOK_RESTRICT)) arr_has_restrict = true;
                ps_advance(ps);
            } else if (ps_at(ps, STOK_STATIC)) {
                arr_has_static = true;
                ps_advance(ps);
            } else break;
        }
        AstNode *sz = NULL;
        bool vla_star = false;
        if (ps_at(ps, STOK_STAR)) {
            ps_advance(ps);
            vla_star = true;
        } else if (!ps_at(ps, STOK_RBRACKET)) {
            sz = parse_expr_prec(ps, 2);
        }
        ps_expect(ps, STOK_RBRACKET, "array suffix ']'");
        sizes[nsizes]        = sz;
        locs[nsizes]         = lb.loc;
        arr_restrict[nsizes] = arr_has_restrict;
        arr_static_kw[nsizes]= arr_has_static;
        arr_vla_star[nsizes] = vla_star;
        nsizes++;
    }
    AstNode *t = inner_ty;
    for (size_t i = nsizes; i > 0; i--) {
        AstNode *arr = ast_node_new(AST_TYPE_ARRAY, locs[i-1]);
        arr->u.type_array.base        = t;
        arr->u.type_array.size        = sizes[i-1];
        arr->u.type_array.has_restrict = arr_restrict[i-1];
        arr->u.type_array.has_static   = arr_static_kw[i-1];
        arr->u.type_array.has_vla_star = arr_vla_star[i-1];
        t = arr;
    }
    return t;
}

/* =========================================================================
 * parse_init_declarator_list — the code:gen:line:markers:rework S1 pass.
 *
 * Replaces the old type-prefix declarator-list parser.  The caller has
 * already consumed the decl-specifiers (storage class + type qualifiers
 * + base type-specifier).  This function loops over comma-separated
 * declarators; each can have its own pointer prefix and array/function
 * suffixes.  Multi-declarator semantics now match ISO C: the first `*`
 * after the comma applies only to the next declarator (whereas pre-S1
 * Sharp shared the leading `*` across all declarators).
 *
 * Storage-class fields from `ds` are propagated to every produced
 * AST_VAR_DECL.  If `ds.storage == SC_TYPEDEF`, each declarator becomes
 * an AST_TYPEDEF_DECL instead.
 *
 * The first declarator's node is returned (caller's "primary" result);
 * the rest are pushed to ps->pending_decls and drained by the caller.
 * If `stmt_wrap` is true, VAR_DECL nodes are wrapped in AST_DECL_STMT.
 * ====================================================================== */
static void mark_transparent_union_by_name(PS *ps, const char *name) {
    for (size_t i = 0; i < ps->pending_decls.len; i++) {
        AstNode *pd = ps->pending_decls.data[i];
        if (pd && pd->kind == AST_STRUCT_DEF &&
            pd->u.struct_def.is_union && pd->u.struct_def.name &&
            strcmp(pd->u.struct_def.name, name) == 0) {
            pd->u.struct_def.is_transparent_union = true;
            break;
        }
    }
}

static AstNode *parse_init_declarator_list(PS *ps, const DeclSpecs *ds, bool stmt_wrap) {
    AstNode *first_node = NULL;
    bool     is_first   = true;

    do {
        /* Each declarator owns its own copy of the base type AST.  The
         * first uses the original; the rest get a deep clone so that
         * ast_node_free does not double-free. */
        AstNode *base_copy = is_first ? ds->base_ty : ast_clone_type(ds->base_ty);

        char    *vname = NULL;
        CppLoc   vloc  = ps_peek(ps).loc;
        AstNode *vty   = parse_declarator(ps, base_copy, &vname);
        /* GCC `__attribute__((…))` after declarator (e.g.
         * `int x __attribute__((unused)) = 0;`) — captured for passthrough.
         * For the first declarator, merge with the leading attrs from
         * ds->gcc_attrs (e.g. from `__attribute__((aligned(8))) int x`). */
        char *trailing_attrs = ps->last_decl_attr;   /* C7: from declarator name */
        ps->last_decl_attr = NULL;                    /* take ownership */
        eat_attribute_specifiers(ps, &trailing_attrs);
        /* Combine: leading attrs (from decl-specifiers) first, then trailing. */
        char *var_gcc_attrs = NULL;
        if (is_first && ds->gcc_attrs) {
            var_gcc_attrs = cpp_xstrdup(ds->gcc_attrs);
        }
        if (trailing_attrs) {
            if (var_gcc_attrs) {
                /* append space + trailing_attrs */
                size_t a = strlen(var_gcc_attrs), b = strlen(trailing_attrs);
                var_gcc_attrs = realloc(var_gcc_attrs, a + 1 + b + 1);
                if (!var_gcc_attrs) abort();
                var_gcc_attrs[a] = ' ';
                memcpy(var_gcc_attrs + a + 1, trailing_attrs, b + 1);
            } else {
                var_gcc_attrs = trailing_attrs;
                trailing_attrs = NULL;
            }
            free(trailing_attrs);
        }

        if (!vname) {
            /* No declarator name.  If the next token is `;` and the
             * type specifier was itself a struct/union/enum definition
             * (the definition was already queued to pending_decls by
             * tspec_try_consume), this is a pure type-definition
             * statement with no variable:
             *
             *   struct cD { char c; int u; };   ← no var name
             *   typedef enum { A, B } MyEnum;  ← name is the typedef alias
             *
             * Accept gracefully: consume the `;` and return NULL so
             * the caller sees no new VAR_DECL while pending_decls still
             * carries the struct/enum for scope registration. */
            if (ps_at(ps, STOK_SEMI)) {
                /* C8: if vty is a shared reference to a pending_decl (e.g.
                 * inline enum typedef target), don't free it here — it's
                 * owned by pending_decls and will be freed with the file AST. */
                bool vty_is_pending = false;
                if (vty && vty->kind == AST_ENUM_DEF) {
                    for (size_t _k = 0; _k < ps->pending_decls.len && !vty_is_pending; _k++)
                        if (ps->pending_decls.data[_k] == vty)
                            vty_is_pending = true;
                }
                if (!vty_is_pending) ast_node_free(vty);
                /* Clear from_inline_var on any struct defs
                 * that were tentatively marked by parse_declaration_specifiers.
                 * We now know no variable declarator follows (next token is `;`),
                 * so this is a standalone type definition `struct X { };` —
                 * NOT an inline struct-with-var.  Clearing the flag ensures cg
                 * emits `struct X { ... };` as a standalone statement. */
                for (size_t _k = 0; _k < ps->pending_decls.len; _k++) {
                    AstNode *_pd = ps->pending_decls.data[_k];
                    if (_pd && _pd->kind == AST_STRUCT_DEF)
                        _pd->u.struct_def.from_inline_var = false;
                }
                ps_advance(ps);   /* eat `;` */
                return NULL;
            }
            ps_error(ps, vloc, "declaration is missing a name");
            vname = cpp_xstrdup("?");
        }

        if (ds->storage == SC_TYPEDEF) {
            /* detect transparent_union attribute on typedef.
             * `typedef union {...} Name __attribute__((transparent_union))`
             * — mark the anonymous union so ty_from_ast returns first member. */
            if (var_gcc_attrs && strstr(var_gcc_attrs, "transparent_union") &&
                vty && vty->kind == AST_TYPE_NAME && vty->u.type_name.name) {
                mark_transparent_union_by_name(ps, vty->u.type_name.name);
            }
            /* `typedef T NAME;` — produce AST_TYPEDEF_DECL.  Any pointer
             * / array / function suffix has already been folded into vty
             * by parse_declarator, so the alias resolves to the full
             * declared type. */
            AstNode *td = ast_node_new(AST_TYPEDEF_DECL, vloc);
            td->u.typedef_decl.alias  = vname;
            td->u.typedef_decl.target = vty;
            td->u.typedef_decl.gcc_attrs = var_gcc_attrs;
            var_gcc_attrs = NULL;
            if (vty && vty->kind == AST_ENUM_DEF) {
                vty->u.enum_def.inline_typedef = true;
                td->u.typedef_decl.target_unowned = true;
            }
            /* C8-fix: when an inline `enum { ... }` body was present, vty is
             * now a TYPE_NAME("int") and enum_body_ref holds the actual
             * AST_ENUM_DEF (owned by pending_decls).  Switch the typedef
             * target to the enum node (non-owning) so cg_typedef_c can emit
             * `typedef enum { ... } Alias;` inline. */
            if (ds->enum_body_ref && ds->enum_body_ref->kind == AST_ENUM_DEF) {
                ast_node_free(td->u.typedef_decl.target); /* free the TYPE_NAME */
                td->u.typedef_decl.target = ds->enum_body_ref;
                td->u.typedef_decl.target_unowned = true;
                ds->enum_body_ref->u.enum_def.inline_typedef = true;
            }
            td_add(&ps->typedefs, vname);
            td_add(&ps->typedef_aliases, vname);
            /* typedef cannot have an initialiser; reject. */
            if (ps_at(ps, STOK_EQ)) {
                ps_error(ps, ps_peek(ps).loc,
                    "typedef declarations cannot have an initialiser");
                ps_advance(ps);
                ast_node_free(parse_expr_prec(ps, 2));
            }
            AstNode *out = stmt_wrap
                         ? (ast_node_new(AST_DECL_STMT, vloc))
                         : NULL;
            if (stmt_wrap) {
                out->u.decl_stmt.decl = td;
            } else {
                out = td;
            }
            if (is_first) { first_node = out; is_first = false; }
            else          astvec_push(&ps->pending_decls, out);
            continue;
        }

        AstNode *vd = ast_node_new(AST_VAR_DECL, vloc);
        vd->u.var_decl.type            = vty;
        vd->u.var_decl.name            = vname;
        vd->u.var_decl.storage         = ds->storage;
        vd->u.var_decl.is_thread_local = ds->is_thread_local;
        vd->u.var_decl.is_constexpr    = ds->is_constexpr;
        vd->u.var_decl.gcc_attrs       = var_gcc_attrs;
        vd->u.var_decl.is_comma_cont   = !is_first;
        vd->u.var_decl.fmt |= (ds->fmt & FMTF_ATTRS_LEADING);
        /* _Alignas — copy the text since ds is const */
        if (ds->alignas_text)
            vd->u.var_decl.alignas_text = cpp_xstrdup(ds->alignas_text);

        if (ps_match(ps, STOK_EQ)) {
            /* S4: `T x = {...};` — braced initializer.  Without the
             * lookahead route, `{` would fall to parse_expr where it
             * doesn't belong (would be a stmt-block). */
            vd->u.var_decl.init = ps_at(ps, STOK_LBRACE)
                                ? parse_init_list(ps)
                                : parse_expr_prec(ps, 2);
        } else if (vty && vty->kind == AST_TYPE_AUTO) {
            ps_error(ps, vloc, "'auto %s' requires an initialiser", vname);
        }

        AstNode *out_node = vd;
        if (stmt_wrap) {
            out_node = ast_node_new(AST_DECL_STMT, vloc);
            out_node->u.decl_stmt.decl = vd;
        }
        if (is_first) { first_node = out_node; is_first = false; }
        else          astvec_push(&ps->pending_decls, out_node);
    } while (ps_match(ps, STOK_COMMA));

    ps_expect(ps, STOK_SEMI, "after variable declaration");
    return first_node;
}

/* =========================================================================
 * looks_like_sharp_generic_params  —  3-token lookahead.
 *
 * After `name`, we have `<TOK1 TOK2 ...`.  This is a Sharp generic param
 * list iff TOK1 is an IDENT and TOK2 is `,` (multiple params) or `>`
 * (single param).  Anything else (e.g. `name<5` for a comparison) is
 * not a generic param list.
 * ====================================================================== */
static bool looks_like_sharp_generic_params(const PS *ps, size_t off) {
    /* off points at the `<` token. */
    if (off + 2 >= ps->ntoks) return false;
    if (ps->toks[off+1].kind != STOK_IDENT) return false;
    SharpTokKind k = ps->toks[off+2].kind;
    return k == STOK_COMMA || k == STOK_GT || k == STOK_GTGT;
}

/* =========================================================================
 * Build an AST_FUNC_DEF from a declarator-produced AST_TYPE_FUNC tree.
 *
 * Transfers ownership of `func_ty->u.type_func.ret` and `func_ty->
 * u.type_func.params` into the new FUNC_DEF, then frees the husk.  After
 * this call, `func_ty` is destroyed and must not be touched.
 * ====================================================================== */
static AstNode *build_func_def_from_decl(AstNode *func_ty, char *name, CppLoc loc) {
    AstNode *fn = ast_node_new(AST_FUNC_DEF, loc);
    fn->u.func_def.name     = name;     /* transfer */
    fn->u.func_def.ret_type = func_ty->u.type_func.ret;       /* transfer */
    fn->u.func_def.params   = func_ty->u.type_func.params;    /* transfer */
    fn->u.func_def.params_unspecified = func_ty->u.type_func.params_unspecified; /* C3 */
    /* Detach so freeing func_ty doesn't free the now-owned children. */
    func_ty->u.type_func.ret    = NULL;
    func_ty->u.type_func.params = (AstVec){0};
    ast_node_free(func_ty);
    return fn;
}

/* =========================================================================
 * Top-level declaration  (S1 — DeclSpecs / declarator model)
 * ====================================================================== */
/* =========================================================================
 * C7: Verbatim text builder for GCC extensions.
 *
 * Builds a heap-allocated NUL-terminated string from tokens [start, end).
 * Token texts are space-separated.  This gives a functionally equivalent
 * token sequence even if original whitespace is not preserved exactly.
 * Does NOT include the trailing `;` (caller adds it when emitting).
 * ======================================================================== */
static char *build_verbatim(PS *ps, size_t start, size_t end) {
    /* Build verbatim text. Preserve original asm spelling (asm/__asm__/__asm)
     * for token-identical output with gcc -E. The generated C may use either
     * form; gcc accepts both in GCC extension mode. */
    size_t total = 0;
    for (size_t i = start; i < end; i++) {
        total += ps->toks[i].len + 1;
    }
    if (total == 0) return cpp_xstrdup("");
    char *buf = malloc(total + 1);
    if (!buf) abort();
    char *p = buf;
    for (size_t i = start; i < end; i++) {
        memcpy(p, ps->toks[i].text, ps->toks[i].len); p += ps->toks[i].len;
        if (i + 1 < end) *p++ = ' ';
    }
    *p = '\0';
    return buf;
}

static bool next_ident_starts_new_decl(PS *ps) {
    if (!ps_at(ps, STOK_IDENT)) return false;
    if (td_has_n(&ps->typedefs, ps_peek(ps).text, ps_peek(ps).len)) {
        if (ps_peek2(ps).kind == STOK_IDENT ||
            ps_peek2(ps).kind == STOK_OPERATOR)
            return true;
        if (ps_peek2(ps).kind == STOK_LT) {
            size_t peek = ps->pos + 2;
            int gdep = 1;
            while (peek < ps->ntoks && gdep > 0) {
                SharpTokKind pk = ps->toks[peek].kind;
                if (pk == STOK_LT) gdep++;
                else if (pk == STOK_GT) { gdep--; if (gdep == 0) { peek++; break; } }
                else if (pk == STOK_GTGT) { gdep -= 2; if (gdep <= 0) { peek++; break; } }
                else if (pk == STOK_EOF) break;
                peek++;
            }
            if (peek < ps->ntoks) {
                SharpTokKind nextk = ps->toks[peek].kind;
                if (nextk == STOK_DOT || nextk == STOK_IDENT)
                    return true;
            }
        }
    } else {
        if (ps_peek2(ps).kind == STOK_IDENT ||
            ps_peek2(ps).kind == STOK_DOT ||
            ps_peek2(ps).kind == STOK_STAR)
            return true;
    }
    return false;
}

static AstNode *parse_top_decl(PS *ps) {
    /* reset last_decl_attr from any previous call. */
    if (ps->last_decl_attr) { free(ps->last_decl_attr); ps->last_decl_attr = NULL; }
    SharpTok t = ps_peek(ps);

    /* C8: `__extension__` — GCC no-warn prefix. The lexer maps the
     * `__extension__` keyword to STOK__EXTENSION__.  Skip it and
     * annotate the following declaration so CG can re-emit in C mode. */
    bool has_extension = (t.kind == STOK__EXTENSION__);
    if (has_extension) ps_advance(ps);

    /* `_Static_assert(cond, "msg");` at file scope.
     * C7: preserve as AST_GCC_VERBATIM so C mode can emit it faithfully.
     * Sharp mode cg silently ignores AST_GCC_VERBATIM nodes. */
    if (t.kind == STOK__STATIC_ASSERT) {
        size_t start_pos = ps->pos;
        ps_advance(ps);
        if (ps_match(ps, STOK_LPAREN)) {
            skip_balanced_parens(ps);
        }
        size_t end_pos = ps->pos;
        ps_match(ps, STOK_SEMI);
        /* Build verbatim and create node */
        char *text = build_verbatim(ps, start_pos, end_pos);
        AstNode *vn = ast_node_new(AST_GCC_VERBATIM, t.loc);
        vn->u.gcc_verbatim.text   = text;
        vn->u.gcc_verbatim.is_stmt = false;
        return vn;
    }

    /* C23: `constexpr` prefix.  Let parse_decl_specifiers handle it
     * so that ds.is_constexpr is properly set for both variables and functions. */



    /* S2: enum at top level.  Two shapes:
     *   enum [Tag] { ... };           — definition  (parse_enum_def)
     *   enum Tag x; / enum Tag *p;    — type usage  (general declarator)
     * The body presence ('{' after the optional tag) decides. */
    if (t.kind == STOK_ENUM) {
        SharpTokKind k2 = ps_peek2(ps).kind;
        SharpTokKind k3 = ps->pos + 2 < ps->ntoks ? ps->toks[ps->pos+2].kind : STOK_EOF;
        if (k2 == STOK_LBRACE || (k2 == STOK_IDENT && k3 == STOK_LBRACE)) {
            AstNode *ed = parse_enum_def(ps);
            /* top-level `enum {...};` requires a trailing
             * semicolon.  parse_enum_def no longer eats it (so stmt /
             * field paths can own it correctly); the top-level path
             * must consume it itself. */
            ps_expect(ps, STOK_SEMI, "after top-level enum definition");
            return ed;
        }
        /* `enum Tag;` — forward declaration of an opaque enum
         * (GCC extension used by libevent http-internal.h).  Consume it
         * and register a minimal enum definition. */
        if (k2 == STOK_IDENT && k3 == STOK_SEMI) {
            ps_advance(ps);  /* eat 'enum' */
            SharpTok nm = ps_advance(ps);  /* eat tag name */
            ps_advance(ps);  /* eat ';' */
            /* Return a minimal AST_ENUM_DEF with no items; scope.c
             * will register the tag so subsequent uses of 'enum Tag'
             * resolve correctly. */
            AstNode *ed = ast_node_new(AST_ENUM_DEF, nm.loc);
            ed->u.enum_def.name = cpp_xstrndup(nm.text, nm.len);
            return ed;
        }
        /* Fall through: `enum Tag x;` is a declaration whose
         * decl-specifier consumes `enum Tag` (handled by tspec). */
    }

    /* S2: union — symmetric with struct.  `union Tag { ... };` or
     * `union Tag;` here, otherwise fall through to the declarator path. */
    if (t.kind == STOK_UNION) {
        size_t save = ps->pos;
        ps_advance(ps);
        if (ps_at(ps, STOK_IDENT)) {
            ps_advance(ps);
            SharpTokKind ahead = ps_peek(ps).kind;
            ps->pos = save;
            if (ahead == STOK_LBRACE || ahead == STOK_SEMI) {
                AstNode *sd = parse_struct_def(ps);
                eat_attribute_specifiers(ps, NULL);
                /* C: union body may be followed by variable declarators:
                 *   union Tag { ... } var = init;
                 *   union Tag t, *p;
                 * After the union body, check for declarators.
                 * Same BUG-004 guard as struct — skip if next IDENT
                 * is a typedef name followed by another IDENT. */
                if (!next_ident_starts_new_decl(ps) &&
                    (ps_at(ps, STOK_IDENT) || ps_at(ps, STOK_STAR) ||
                    ps_at(ps, STOK_LPAREN))) {
                    astvec_push(&ps->pending_decls, sd);
                    AstNode *ty = ast_node_new(AST_TYPE_NAME, sd->loc);
                    ty->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
                    DeclSpecs vds = {0};
                    vds.base_ty = ty;
                    vds.loc = sd->loc;
                    AstNode *vd = parse_init_declarator_list(ps, &vds, /*stmt_wrap=*/false);
                    return vd ? vd : sd;
                }
                ps_match(ps, STOK_SEMI);
                return sd;
            }
        } else {
            ps->pos = save;
            AstNode *sd = parse_struct_def(ps);
            eat_attribute_specifiers(ps, NULL);
            /* C: anonymous union body may be followed by variable declarators:
             *   union { Uint64 u64; double d; } inf = { 0x7ff...ULL };
             * After the union body, check for declarators.
             * Same BUG-004 guard as anonymous struct. */
            if (!next_ident_starts_new_decl(ps) &&
                (ps_at(ps, STOK_IDENT) || ps_at(ps, STOK_STAR) ||
                ps_at(ps, STOK_LPAREN))) {
                astvec_push(&ps->pending_decls, sd);
                AstNode *ty = ast_node_new(AST_TYPE_NAME, sd->loc);
                ty->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
                DeclSpecs vds = {0};
                vds.base_ty = ty;
                vds.loc = sd->loc;
                AstNode *vd = parse_init_declarator_list(ps, &vds, /*stmt_wrap=*/false);
                return vd ? vd : sd;
            }
            ps_match(ps, STOK_SEMI);
            return sd;
        }
    }

    /* `struct Tag { ... }` / `class Tag { ... }` at top level */
    if (t.kind == STOK_STRUCT || t.kind == STOK_CLASS) {
        size_t save = ps->pos;
        ps_advance(ps);
        if (ps_at(ps, STOK_IDENT)) {
            ps_advance(ps);
            if (ps_at(ps, STOK_LT)) {
                int depth = 0;
                while (!ps_at(ps, STOK_EOF)) {
                    if (ps_at(ps, STOK_LT)) depth++;
                    else if (ps_at(ps, STOK_GT)) {
                        depth--;
                        if (depth == 0) { ps_advance(ps); break; }
                    } else if (ps_at(ps, STOK_GTGT)) {
                        depth -= 2;
                        if (depth <= 0) { ps_advance(ps); break; }
                    }
                    ps_advance(ps);
                }
            }
            SharpTokKind ahead = ps_peek(ps).kind;
            ps->pos = save;
            if (ahead == STOK_LBRACE || ahead == STOK_SEMI) {
                AstNode *sd = parse_struct_def(ps);
                eat_attribute_specifiers(ps, NULL);
                /* C: struct body may be followed by variable declarators:
                 *   struct { int a; } s = {1};
                 *   struct Tag t, *p;
                 * After the struct body, check for declarators.
                 * BUG-004 fix: skip if the next IDENT is a typedef name
                 * followed by another IDENT — that pattern is the start of
                 * a NEW top-level declaration, not a declarator for this struct.
                 * E.g. `struct P { } P make() {}` must NOT treat `P` as a
                 * variable name; `P make` is a new function declaration.
                 * Similarly, `VecC operator+(VecC a, struct VecS b)` after
                 * a class/struct body must NOT treat `VecC` as a declarator;
                 * `TypedefName operator` is the start of a free-function
                 * operator definition returning a class type. */
                if (!next_ident_starts_new_decl(ps) &&
                    (ps_at(ps, STOK_IDENT) || ps_at(ps, STOK_STAR) ||
                    ps_at(ps, STOK_LPAREN))) {
                    /* Push the struct def to pending_decls so it gets
                     * emitted to the file's decl list. */
                    astvec_push(&ps->pending_decls, sd);
                    /* Build a TYPE_NAME referring to the struct we just parsed. */
                    AstNode *ty = ast_node_new(AST_TYPE_NAME, sd->loc);
                    ty->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
                    /* Parse variable declarators using the struct type as base. */
                    DeclSpecs vds = {0};
                    vds.base_ty = ty;
                    vds.loc = sd->loc;
                    AstNode *vd = parse_init_declarator_list(ps, &vds, /*stmt_wrap=*/false);
                    /* Return the var decl as the primary result so the
                     * caller sees a proper declaration. */
                    return vd ? vd : sd;
                }
                ps_match(ps, STOK_SEMI);
                return sd;
            }
        } else {
            ps->pos = save;
            AstNode *sd = parse_struct_def(ps);
            eat_attribute_specifiers(ps, NULL);
            /* C: anonymous struct body may be followed by variable declarators:
             *   struct { int a; int b; int c; } s = {1, 2, 3};
             * After the struct body, check for declarators.
             * BUG-004 fix: same guard as tagged struct — skip if next IDENT
             * is a typedef name followed by another IDENT (new declaration). */
            if (!next_ident_starts_new_decl(ps) &&
                (ps_at(ps, STOK_IDENT) || ps_at(ps, STOK_STAR) ||
                ps_at(ps, STOK_LPAREN))) {
                /* Push the struct def to pending_decls so it gets
                 * emitted to the file's decl list. */
                astvec_push(&ps->pending_decls, sd);
                /* Build a TYPE_NAME referring to the struct we just parsed. */
                AstNode *ty = ast_node_new(AST_TYPE_NAME, sd->loc);
                ty->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
                /* Parse variable declarators using the struct type as base. */
                DeclSpecs vds = {0};
                vds.base_ty = ty;
                vds.loc = sd->loc;
                AstNode *vd = parse_init_declarator_list(ps, &vds, /*stmt_wrap=*/false);
                /* Return the var decl as the primary result so the
                 * caller sees a proper declaration. */
                return vd ? vd : sd;
            }
            ps_match(ps, STOK_SEMI);
            return sd;
        }
        /* Fall through: `struct Tag *p;` etc. is handled by the general
         * declarator path because parse_decl_specifiers absorbs `struct Tag`. */
    }

    /* `extern struct Vec<int>;` — Sharp-specific explicit instantiation.
     * Tells the compiler to emit the monomorphised struct for the named
     * generic type without declaring a variable.  Parsed into a clean
     * AST_EXTERN_INST node (no sentinel string hacks). */
    if (t.kind == STOK_EXTERN && ps_peek2(ps).kind == STOK_STRUCT) {
        size_t save = ps->pos;
        ps_advance(ps);  /* consume extern */
        ps_advance(ps);  /* consume struct */
        if (ps_at(ps, STOK_IDENT)) {
            AstNode *ty = parse_type(ps);
            if (ps_match(ps, STOK_SEMI)) {
                AstNode *ei = ast_node_new(AST_EXTERN_INST, t.loc);
                ei->u.extern_inst.type = ty;
                return ei;
            }
            ast_node_free(ty);
        }
        ps->pos = save;
    }

    /* `extern swap<int>;` — function explicit instantiation (AST_EXTERN_INST). */
    if (t.kind == STOK_EXTERN && ps_peek2(ps).kind == STOK_IDENT &&
        ps_peek3(ps).kind == STOK_LT) {
        size_t save = ps->pos;
        ps_advance(ps);
        size_t sv2 = ps->pos;
        ps_advance(ps);
        ps->pos = sv2;
        AstNode *ty = parse_type(ps);
        if (ps_match(ps, STOK_SEMI)) {
            AstNode *ei = ast_node_new(AST_EXTERN_INST, t.loc);
            ei->u.extern_inst.type = ty;
            return ei;
        }
        ast_node_free(ty);
        ps->pos = save;
    }

    if (!is_type_start(ps)) {
        /* Extension method with generic return type: `Stack<T> Stack.new()`.
         * is_type_start returns false for `Stack` followed by `<`, but it
         * is still a valid extension method pattern.  Try pre-scanning for
         * the `IDENT . IDENT(` pattern to catch this case early. */
        if (ps_peek(ps).kind == STOK_IDENT && ps_peek2(ps).kind == STOK_LT) {
            goto try_ext_method;
        }

        /* C++/Rust style generic prefix: <T, U> RetType name(args) { ... }
         * This allows T to be used in the return type, e.g. "<T> T Vec__last(...)".
         * The generic params are registered as typedef-names so parse_decl_specifiers
         * can recognise T as a type. */
        if (ps_at(ps, STOK_LT)) {
            size_t save_pos = ps->pos;
            size_t save_diag_len = ps->diags ? ps->diags->len : 0;
            int save_pending = ps->pending_close;
            AstVec tmp_params = {0};
            parse_generic_params(ps, &tmp_params);

            bool is_gfunc_prefix = false;
            bool is_ext_prefix = false;
            if (tmp_params.len > 0 && !ps_at(ps, STOK_EOF)) {
                /* Must be followed by a type then IDENT + '('.
                 * The type may be a generic type like `vec<T>`,
                 * so we can't just peek 2 tokens ahead.  Instead
                 * do a speculative parse: register generic params
                 * as typedefs, parse one DeclSpecs, and check if
                 * the next tokens are IDENT + '('. */
                for (size_t gi = 0; gi < tmp_params.len; gi++) {
                    AstNode *gp = tmp_params.data[gi];
                    if (gp && gp->kind == AST_GENERIC_PARAM && gp->u.generic_param.name)
                        td_add(&ps->typedefs, gp->u.generic_param.name);
                }
                if (is_type_start(ps)) {
                    size_t save2 = ps->pos;
                    size_t save_diag2 = ps->diags ? ps->diags->len : 0;
                    DeclSpecs probe = parse_decl_specifiers(ps);
                    if (!probe.empty && probe.base_ty &&
                        ps_at(ps, STOK_IDENT)) {
                        if (ps_peek2(ps).kind == STOK_LPAREN) {
                            is_gfunc_prefix = true;
                        } else if (ps_peek2(ps).kind == STOK_LT) {
                            /* Check extension method: <T> Ret Class<T>.method(...)
                             * Peek past IDENT<T,...> to find '.' */
                            size_t p = ps->pos + 1; /* skip IDENT */
                            p++; /* skip '<' */
                            int gd = 1;
                            while (p < ps->ntoks && gd > 0) {
                                SharpTokKind pk = ps->toks[p].kind;
                                if (pk == STOK_LT) gd++;
                                else if (pk == STOK_GT) { gd--; if (gd == 0) { p++; break; } }
                                else if (pk == STOK_GTGT) { gd -= 2; if (gd <= 0) { p++; break; } }
                                else if (pk == STOK_EOF) break;
                                p++;
                            }
                            if (p < ps->ntoks && ps->toks[p].kind == STOK_DOT)
                                is_ext_prefix = true; /* extension method */
                        }
                    }
                    ps->pos = save2;
                    rollback_diags(ps, save_diag2);
                }
                /* Clean up the typedefs we just added for probing */
                for (size_t gi = 0; gi < tmp_params.len; gi++) {
                    AstNode *gp = tmp_params.data[gi];
                    if (gp && gp->kind == AST_GENERIC_PARAM && gp->u.generic_param.name)
                        td_remove(&ps->typedefs, gp->u.generic_param.name);
                }
            }

            if (is_gfunc_prefix || is_ext_prefix) {
                /* Register generic params so return type can use them */
                for (size_t gi = 0; gi < tmp_params.len; gi++) {
                    AstNode *gp = tmp_params.data[gi];
                    if (gp && gp->kind == AST_GENERIC_PARAM && gp->u.generic_param.name)
                        td_add(&ps->typedefs, gp->u.generic_param.name);
                }

                if (is_ext_prefix) {
                    /* Extension method with <T> prefix: <K,V> Map<K,V> Map<K,V>.make(...)
                     * try_ext_method handles the rest; generic params already registered
                     * as typedefs above. Free tmp_params since finish_func re-parses them. */
                    astvec_free(&tmp_params);
                    goto try_ext_method;
                }

                DeclSpecs ds = parse_decl_specifiers(ps);
                if (!ds.empty && ds.base_ty && ps_at(ps, STOK_IDENT)) {
                    SharpTok name_tok = ps_advance(ps);
                    AstNode *fn = finish_func(ps, ds.base_ty, name_tok, false);
                    fn->u.func_def.storage   = ds.storage;
                    fn->u.func_def.is_inline = ds.is_inline;
                    fn->u.func_def.is_noreturn = ds.is_noreturn;
                    fn->u.func_def.is_constexpr = ds.is_constexpr;
                    fn->u.func_def.is_thread_local = ds.is_thread_local;
                    fn->u.func_def.gcc_attrs = ds.gcc_attrs;
                    ds.gcc_attrs = NULL;
                    /* Copy generic params to the function node */
                    astvec_free(&fn->u.func_def.generic_params);
                    fn->u.func_def.generic_params = tmp_params;
                    return fn;
                }

                /* Rollback */
                ps->pos = save_pos;
                ps->pending_close = save_pending;
                rollback_diags(ps, save_diag_len);
                for (size_t gi = 0; gi < tmp_params.len; gi++) {
                    AstNode *gp = tmp_params.data[gi];
                    if (gp && gp->kind == AST_GENERIC_PARAM && gp->u.generic_param.name)
                        td_remove(&ps->typedefs, gp->u.generic_param.name);
                }
                astvec_free(&tmp_params);
            } else {
                ps->pos = save_pos;
                ps->pending_close = save_pending;
                rollback_diags(ps, save_diag_len);
                astvec_free(&tmp_params);
            }
        }

        ps_error(ps, t.loc, "unexpected token '%.*s' at top level",
                 (int)t.len, t.text);
        ps_advance(ps);
        return NULL;
    }

    /* ── Extension method detection ────────────────────────────────────
     * Extension methods: RetType ClassName.methodName(params) { body }
     *                    RetType ClassName.operator+(params) { body }
     * parse_decl_specifiers would consume the class name as a 2nd type
     * specifier (because struct names are auto-typedef'd).  We pre-scan
     * for the IDENT . IDENT pattern to detect and handle this case. */
    try_ext_method:
    {
        size_t save_pos;
        size_t save_diag;
        int depth;
        const char *cname;
        size_t cname_len;
        size_t cname_tok;  /* token index of the class name */
        bool is_ext;
        bool is_ext_op;

        save_pos   = ps->pos;
        save_diag  = ps->diags ? ps->diags->len : 0;
        depth      = 0;
        cname      = NULL;
        cname_len  = 0;
        cname_tok  = 0;
        is_ext     = false;
        is_ext_op  = false;

        /* Scan forward at depth 0 to find IDENT . IDENT( or IDENT . operator+( */
        for (size_t i = save_pos; i < ps->ntoks; i++) {
            SharpTokKind k = ps->toks[i].kind;
            if (k == STOK_LT || k == STOK_LPAREN || k == STOK_LBRACKET) depth++;
            else if (k == STOK_GT || k == STOK_RPAREN || k == STOK_RBRACKET) {
                if (depth > 0) depth--;
            } else if (k == STOK_GTGT && depth >= 2) {
                depth -= 2; /* >> closes two generic levels */
            }
            if (depth == 0 && k == STOK_DOT) {
                if (i+1 < ps->ntoks &&
                    (ps->toks[i+1].kind == STOK_IDENT || ps->toks[i+1].kind == STOK_OPERATOR)) {
                    /* Check: after method name/operator, expect '(' */
                    size_t j = i + 2;
                    if (ps->toks[i+1].kind == STOK_OPERATOR && j < ps->ntoks) {
                        if (ps->toks[j].kind == STOK_LBRACKET) {
                            j++; /* operator[] — skip ']' too */
                            if (j < ps->ntoks && ps->toks[j].kind == STOK_RBRACKET) j++;
                        } else {
                            j++; /* operator+, operator-, etc. — skip the operator token */
                        }
                    }
                    if (j < ps->ntoks && ps->toks[j].kind == STOK_LPAREN) {
                        /* Find class name: it's the IDENT before the '.'.
                         * For generic classes like `Vec<T>.push`, the token
                         * before '.' is '>', so trace back through generic
                         * args to find the class name IDENT. */
                        size_t ci = i - 1;
                        if (ps->toks[ci].kind == STOK_GT ||
                            ps->toks[ci].kind == STOK_GTGT) {
                            /* Trace back past generic args: IDENT < ... > */
                            int gdepth = (ps->toks[ci].kind == STOK_GTGT) ? 2 : 1;
                            while (ci > 0 && gdepth > 0) {
                                ci--;
                                SharpTokKind gk = ps->toks[ci].kind;
                                if (gk == STOK_GT) gdepth++;
                                else if (gk == STOK_GTGT) gdepth += 2;
                                else if (gk == STOK_LT) gdepth--;
                            }
                            /* Skip past the '<' to get the IDENT */
                            if (gdepth <= 0 && ci > 0) ci--;
                        }
                        if (ci < ps->ntoks && ps->toks[ci].kind == STOK_IDENT) {
                            cname = ps->toks[ci].text;
                            cname_len = ps->toks[ci].len;
                            cname_tok = ci;
                            is_ext = true;
                            is_ext_op = (ps->toks[i+1].kind == STOK_OPERATOR);
                        }
                        break;
                    }
                }
            }
            if (depth == 0 && (k == STOK_SEMI || k == STOK_LBRACE || k == STOK_EOF))
                break;
        }

        if (is_ext && cname_len < 256) {
            /* Build temporary NUL-terminated copy of class name */
            char tmp_name[256];
            memcpy(tmp_name, cname, cname_len);
            tmp_name[cname_len] = '\0';

            /* Check if first token == class name (return type same as class name).
             * In this case we must NOT remove it from the typedef set. */
            SharpTok first_tok = ps->toks[save_pos];
            bool rtype_eq_cname = (first_tok.kind == STOK_IDENT &&
                                    cname_len == first_tok.len &&
                                    memcmp(cname, first_tok.text, cname_len) == 0);

            DeclSpecs ds;

            if (rtype_eq_cname) {
                /* Return type == class name. parse_decl_specifiers consumes
                 * the return type (which IS the class name), so it also
                 * consumes the first occurrence.  We then expect another
                 * IDENT (the class name), then '.', then method.
                 * Temporarily remove class name from typedefs so the
                 * second occurrence is NOT consumed as a type specifier. */
                bool was_td2 = td_has(&ps->typedefs, tmp_name);
                if (was_td2) td_remove(&ps->typedefs, tmp_name);
                ds = parse_decl_specifiers(ps);
                if (was_td2) td_add(&ps->typedefs, tmp_name);
            } else {
                /* Temporarily remove class name from typedefs so
                 * parse_decl_specifiers doesn't consume it as a type. */
                bool was_td = td_has(&ps->typedefs, tmp_name);
                if (was_td) td_remove(&ps->typedefs, tmp_name);
                ds = parse_decl_specifiers(ps);
                if (was_td) td_add(&ps->typedefs, tmp_name);
            }
            /* Handle pointer '*' after return type, e.g. `T* Vec<T>.begin(this)`.
             * Wrap the return type in AST_TYPE_PTR nodes so sema/cg see T*. */
            while (ps_at(ps, STOK_STAR)) {
                ps_advance(ps);
                AstNode *ptr = ast_node_new(AST_TYPE_PTR, ds.base_ty->loc);
                ptr->u.type_ptr.base = ds.base_ty;
                ds.base_ty = ptr;
            }

            if (!ds.empty && ds.base_ty) {
        parse_ext_body:
                if (ps_at(ps, STOK_IDENT) &&
                    ps_peek(ps).len == cname_len &&
                    memcmp(ps_peek(ps).text, cname, cname_len) == 0) {
                    ps_advance(ps); /* consume class name */
                }

                /* Skip generic args (return-type or class generics) */
                if (ps_at(ps, STOK_LT)) {
                    int gdepth = 0;
                    while (!ps_at(ps, STOK_EOF)) {
                        if (ps_at(ps, STOK_LT)) gdepth++;
                        else if (ps_at(ps, STOK_GT)) {
                            if (gdepth <= 1) { ps_advance(ps); break; }
                            gdepth--;
                        } else if (ps_at(ps, STOK_GTGT)) {
                            if (gdepth <= 2) { ps_advance(ps); break; }
                            gdepth -= 2;
                        } else if (ps_at(ps, STOK_COMMA) && gdepth == 0) break;
                        ps_advance(ps);
                    }
                }

                /* Now we might find the class name (if generic args were
                 * from the return type, the class name is after). */
                if (ps_at(ps, STOK_IDENT) &&
                    ps_peek(ps).len == cname_len &&
                    memcmp(ps_peek(ps).text, cname, cname_len) == 0) {
                    ps_advance(ps); /* consume class name */
                    /* Might have class generics too */
                    if (ps_at(ps, STOK_LT)) {
                        int gdepth = 0;
                        while (!ps_at(ps, STOK_EOF)) {
                            if (ps_at(ps, STOK_LT)) gdepth++;
                            else if (ps_at(ps, STOK_GT)) {
                                if (gdepth <= 1) { ps_advance(ps); break; }
                                gdepth--;
                            } else if (ps_at(ps, STOK_GTGT)) {
                                if (gdepth <= 2) { ps_advance(ps); break; }
                                gdepth -= 2;
                            } else if (ps_at(ps, STOK_COMMA) && gdepth == 0) break;
                            ps_advance(ps);
                        }
                    }
                }

                ps_expect(ps, STOK_DOT, "'.' in extension method");

                AstNode *fn;
                if (is_ext_op) {
                    /* Extension operator: ClassName.operator+(params) { body } */
                    ps_advance(ps); /* eat 'operator' */
                    SharpTok op_tok = ps_advance(ps);
                    if (op_tok.kind == STOK_LBRACKET) ps_match(ps, STOK_RBRACKET);
                    fn = finish_func(ps, ds.base_ty, op_tok, /*is_operator=*/true);
                    char buf[64];
                    if (op_tok.kind == STOK_LBRACKET)
                        snprintf(buf, sizeof buf, "operator[]");
                    else
                        snprintf(buf, sizeof buf, "operator%.*s", (int)op_tok.len, op_tok.text);
                    free(fn->u.func_def.name);
                    fn->u.func_def.name        = cpp_xstrndup(buf, strlen(buf));
                    fn->u.func_def.is_operator  = true;
                } else {
                    SharpTok mn_tok = ps_expect(ps, STOK_IDENT, "method name");
                    fn = finish_func(ps, ds.base_ty, mn_tok, /*is_operator=*/false);
                }

                /* Common extension method setup */
                fn->u.func_def.struct_name  = cpp_xstrndup(tmp_name, cname_len);
                fn->u.func_def.storage      = ds.storage;
                fn->u.func_def.is_inline    = ds.is_inline;
                fn->u.func_def.is_noreturn  = ds.is_noreturn;
                fn->u.func_def.is_constexpr = ds.is_constexpr;
                fn->u.func_def.is_thread_local = ds.is_thread_local;
                fn->u.func_def.gcc_attrs    = ds.gcc_attrs;
                ds.gcc_attrs = NULL;

                /* Detect has_receiver: first param is 'this' */
                fn->u.func_def.has_receiver = false;
                if (fn->u.func_def.params.len > 0) {
                    AstNode *p0 = fn->u.func_def.params.data[0];
                    if (p0 && p0->kind == AST_PARAM_DECL && p0->u.param_decl.name &&
                        strcmp(p0->u.param_decl.name, "this") == 0)
                        fn->u.func_def.has_receiver = true;
                }

                return fn;
            }

            /* Fallback: parse_decl_specifiers returned empty because
             * the return type uses generic params not in the typedef
             * set (e.g. `K* HashMap<K,V>.get(this)`).
             * Consume the return type manually up to the class name. */
            if (cname_tok > save_pos) {
                /* Restore position and rebuild return type */
                ps->pos = save_pos;
                rollback_diags(ps, save_diag);

                AstNode *ret_ty = NULL;
                while (ps->pos < cname_tok && !ps_at(ps, STOK_EOF)) {
                    if (ps_at(ps, STOK_IDENT) && !ret_ty) {
                        SharpTok rt = ps_advance(ps);
                        ret_ty = ast_node_new(AST_TYPE_NAME, rt.loc);
                        ret_ty->u.type_name.name = cpp_xstrndup(rt.text, rt.len);
                    } else if (ps_at(ps, STOK_STAR)) {
                        AstNode *ptr = ast_node_new(AST_TYPE_PTR, ps_advance(ps).loc);
                        ptr->u.type_ptr.base = ret_ty;
                        ret_ty = ptr;
                    } else {
                        ps_advance(ps);
                    }
                }
                if (ret_ty) {
                    memset(&ds, 0, sizeof ds);
                    ds.base_ty = ret_ty;
                    goto parse_ext_body;
                }
            }

            /* Restore on failure */
            ps->pos = save_pos;
            rollback_diags(ps, save_diag);
        }
    }

    size_t ds_save_pos = ps->pos;
    size_t ds_save_diag = ps->diags ? ps->diags->len : 0;
    DeclSpecs ds = parse_decl_specifiers(ps);
    if (ds.empty || !ds.base_ty) {
        ps_error(ps, t.loc, "missing declaration specifier");
        free(ds.gcc_attrs); /* avoid leak on error path */
        ps_sync(ps);
        return NULL;
    }

    /* ── Free-function operator: `RetType operator+(args) { body }` ─── */
    if (ps_at(ps, STOK_OPERATOR)) {
        ps_advance(ps);
        SharpTok op_tok = ps_advance(ps);
        if (op_tok.kind == STOK_LBRACKET) ps_match(ps, STOK_RBRACKET);
        AstNode *fn = finish_func(ps, ds.base_ty, op_tok, /*is_operator=*/true);
        char buf[64];
        snprintf(buf, sizeof buf, "operator%.*s", (int)op_tok.len, op_tok.text);
        free(fn->u.func_def.name);
        fn->u.func_def.name      = cpp_xstrndup(buf, strlen(buf));
        fn->u.func_def.storage   = ds.storage;
        fn->u.func_def.is_inline = ds.is_inline;
        fn->u.func_def.is_noreturn = ds.is_noreturn; /* */
        fn->u.func_def.is_constexpr = ds.is_constexpr;
        fn->u.func_def.is_thread_local = ds.is_thread_local; /* */
        fn->u.func_def.gcc_attrs = ds.gcc_attrs; /* */
        ds.gcc_attrs = NULL;
        return fn;
    }

    /* ── Sharp generic function: `RetType name<T,U>(args) { body }` ──
     * Detect the Sharp-specific `<...>` between name and parameter list.
     * If present we route to finish_func, which knows how to consume the
     * generic-params section.  Plain C functions never have this shape.
     *
     * Also check for extension method: `RetType<T> ClassName<T>.method(...)`.
     * The '<...>' followed by '.' indicates an extension method, not a
     * generic function. */
    if (ps_at(ps, STOK_IDENT) && ps_peek2(ps).kind == STOK_LT &&
        looks_like_sharp_generic_params(ps, ps->pos + 1)) {
        /* Peek past IDENT<T,...> to check for '.' (extension method marker).
         * `Map<K, V> Map<K, V>.make(...)` has '.' after generic params. */
        size_t peek = ps->pos + 1; /* skip IDENT */
        peek++;                     /* skip '<' */
        int gdepth = 1;
        while (peek < ps->ntoks && gdepth > 0) {
            SharpTokKind pk = ps->toks[peek].kind;
            if (pk == STOK_LT) gdepth++;
            else if (pk == STOK_GT) { gdepth--; if (gdepth == 0) { peek++; break; } }
            else if (pk == STOK_GTGT) { gdepth -= 2; if (gdepth <= 0) { peek++; break; } }
            else if (pk == STOK_EOF) break;
            peek++;
        }
        if (peek < ps->ntoks && ps->toks[peek].kind == STOK_DOT) {
            /* Extension method — restore position and retry via try_ext_method.
             * We must restore to before parse_decl_specifiers consumed the
             * return type.  The try_ext_method block re-parses everything. */
            ast_node_free(ds.base_ty);
            free(ds.gcc_attrs);
            memset(&ds, 0, sizeof ds);
            ps->pos = ds_save_pos;
            rollback_diags(ps, ds_save_diag);
            goto try_ext_method;
        }

        SharpTok name_tok = ps_advance(ps);
        AstNode *fn = finish_func(ps, ds.base_ty, name_tok, /*is_operator=*/false);
        fn->u.func_def.storage   = ds.storage;
        fn->u.func_def.is_inline = ds.is_inline;
        fn->u.func_def.is_noreturn = ds.is_noreturn; /* */
        fn->u.func_def.is_constexpr = ds.is_constexpr;
        fn->u.func_def.is_thread_local = ds.is_thread_local; /* */
        fn->u.func_def.gcc_attrs = ds.gcc_attrs; /* */
        ds.gcc_attrs = NULL;
        return fn;
    }

    /* ── Standard ISO-C declarator path. ───────────────────────────── */
    char    *name      = NULL;
    CppLoc   first_loc = ps_peek(ps).loc;
    AstNode *full_ty   = parse_declarator(ps, ds.base_ty, &name);
    /* `int x __attribute__((unused));` and
     * `void foo() __attribute__((noreturn));` — capture trailing attrs
     * so they can be forwarded to the func/var AST node.
     * Leading attrs are already in ds.gcc_attrs from parse_decl_specifiers. */
    char *top_trailing_attrs = NULL;
    eat_attribute_specifiers(ps, &top_trailing_attrs);
    /* Merge leading (from ds.gcc_attrs) + trailing into a single heap string. */
    /* C8: trailing attrs go into trailing_gcc_attrs only (not merged into
     * gcc_attrs). For var_decl and other uses, trailing attrs are separate. */
    char *top_trailing_attrs_saved = top_trailing_attrs;  /* take ownership */
    top_trailing_attrs = NULL;
    /* gcc_attrs = leading only (from ds.gcc_attrs) */
    char *top_gcc_attrs = ds.gcc_attrs;
    ds.gcc_attrs = NULL;

    /* Function definition or prototype: outermost shape is FUNC and the
     * next token is either `{` (definition), `const` (Sharp const-method
     * suffix — only meaningful inside struct, but tolerate at top level),
     * or `;` (prototype).  A `=` here would be a function-typed variable
     * with an initializer, which we treat as a variable declaration.
     *
     * SC_TYPEDEF must NOT take this branch.  ISO C allows the
     * K&R-classic form `typedef void Fn(int);` (no parentheses around
     * the name) which produces a *function-type* alias.  Without the
     * SC_TYPEDEF guard we mis-classify it as a function prototype and
     * never register `Fn` as a typedef-name; the next use as
     * `Fn *p = ...` then fails with "unknown type 'Fn'".  stb's
     * `typedef void stbi_write_func(void *, void *, int);` hits this. */
    /* C-superset K&R: detect `int add(a, b) int a; int b; { ... }` where the
     * next token after the declarator is a type specifier (start of old-style
     * parameter type declarations), not `{` / `;` / `const`. */
    bool is_kr_func = false;
    if (full_ty && full_ty->kind == AST_TYPE_FUNC && name &&
        ds.storage != SC_TYPEDEF &&
        !ps_at(ps, STOK_LBRACE) && !ps_at(ps, STOK_SEMI) &&
        !ps_at(ps, STOK_CONST) && !ps_at(ps, STOK_EOF)) {
        AstNode *fn_ty = full_ty;
        if (is_kr_param_list(&fn_ty->u.type_func.params)) {
            consume_kr_declarations(ps, &fn_ty->u.type_func.params);
            is_kr_func = true;
        }
    }

    if (full_ty && full_ty->kind == AST_TYPE_FUNC && name &&
        ds.storage != SC_TYPEDEF &&
        (ps_at(ps, STOK_LBRACE) || ps_at(ps, STOK_SEMI) || ps_at(ps, STOK_CONST) ||
         ps_at(ps, STOK_COMMA) ||   /* ← pc00121: int f(), g(), a; — comma group */
         is_kr_func)) {
        AstNode *fn = build_func_def_from_decl(full_ty, name, first_loc);
        if (fn && is_kr_func) fn->u.func_def.is_kr_style = true;
        fn->u.func_def.storage   = ds.storage;
        fn->u.func_def.is_inline = ds.is_inline;
        fn->u.func_def.is_noreturn = ds.is_noreturn; /* */
        fn->u.func_def.is_thread_local = ds.is_thread_local; /* */
        fn->u.func_def.is_constexpr = ds.is_constexpr; /* C23 */
        fn->u.func_def.gcc_attrs = top_gcc_attrs;
        fn->u.func_def.fmt |= (ds.fmt & FMTF_ATTRS_LEADING);
        fn->u.func_def.inline_kw      = ds.inline_kw;
        fn->u.func_def.fmt |= (ds.fmt & (FMTF_INLINE_LEADING));
        fn->u.func_def.fmt |= (ds.fmt & FMTF_INLINE_AFTER_ATTRS);
        fn->u.func_def.fmt |= (ds.fmt & FMTF_CONST_LEADING);
        fn->u.func_def.trailing_gcc_attrs = top_trailing_attrs_saved; /* C8 */
        top_trailing_attrs_saved = NULL;
        fn->u.func_def.name_paren = ps->last_name_paren; /* C8 */
        ps->last_name_paren = false;
        if (ps_match(ps, STOK_CONST)) fn->u.func_def.is_const_method = true;
        if (ps_at(ps, STOK_LBRACE)) {
            fn->u.func_def.body = parse_block(ps);
            return fn;
        } else if (ps_at(ps, STOK_COMMA)) {
            /* pc00121: `int f(int a), g(int a), a;`
             * First declarator is a function prototype; more declarators follow.
             * Parse each comma-separated declarator:
             *   - function type → push as AST_FUNC_DEF (body=NULL prototype)
             *   - variable type → push as AST_VAR_DECL
             * Return fn as the primary; continuations go to pending_decls. */
            while (ps_match(ps, STOK_COMMA)) {
                AstNode *bc = ast_clone_type(ds.base_ty);
                char    *nc = NULL;
                CppLoc   nl = ps_peek(ps).loc;
                AstNode *nt = parse_declarator(ps, bc, &nc);
                char *nattr = NULL;
                eat_attribute_specifiers(ps, &nattr);
                if (!nc) { nc = cpp_xstrdup("?"); }
                if (nt && nt->kind == AST_TYPE_FUNC) {
                    /* Another function prototype in the group */
                    AstNode *fn2 = build_func_def_from_decl(nt, nc, nl);
                    fn2->u.func_def.storage = ds.storage;
                    free(nattr);
                    astvec_push(&ps->pending_decls, fn2);
                } else {
                    /* Variable declarator in the group */
                    AstNode *vd = ast_node_new(AST_VAR_DECL, nl);
                    vd->u.var_decl.type    = nt;
                    vd->u.var_decl.name    = nc;
                    vd->u.var_decl.storage = ds.storage;
                    vd->u.var_decl.gcc_attrs = nattr;
                    if (ps_match(ps, STOK_EQ))
                        vd->u.var_decl.init = parse_expr_prec(ps, 2);
                    astvec_push(&ps->pending_decls, vd);
                }
            }
            ps_expect(ps, STOK_SEMI, "after function prototype list");
            return fn;
        } else {
            ps_expect(ps, STOK_SEMI, "after function prototype");
        }
        return fn;
    }

    /* Variable / typedef declaration.  Build the first node, then loop
     * over comma-separated additional declarators. */
    if (!name) {
        ps_error(ps, first_loc, "declaration is missing a name");
        name = cpp_xstrdup("?");
    }

    AstNode *first = NULL;

    if (ds.storage == SC_TYPEDEF) {
        /* detect transparent_union attribute on typedef
         * (e.g. `typedef union {...} Name __attribute__((transparent_union))`)
         * and mark the union's AST node so ty_from_ast returns the first
         * member's type instead of the union type. */
        const char *_ta = top_gcc_attrs ? top_gcc_attrs :
                          (ps->last_decl_attr ? ps->last_decl_attr : NULL);
        if (_ta && strstr(_ta, "transparent_union")) {
            AstNode *target = full_ty;
            while (target && (target->kind == AST_TYPE_CONST ||
                              target->kind == AST_TYPE_PTR ||
                              target->kind == AST_TYPE_VOLATILE))
                target = target->u.type_const.base;
            if (target && target->kind == AST_STRUCT_DEF && target->u.struct_def.is_union)
                target->u.struct_def.is_transparent_union = true;
            if (full_ty && full_ty->kind == AST_TYPE_NAME)
                mark_transparent_union_by_name(ps, full_ty->u.type_name.name);
        }
        free(top_gcc_attrs); /* typedefs don't propagate gcc_attrs to cg_func */
        /* capture declarator-suffix attribute for the typedef node. */
        char *td_attr = ps->last_decl_attr;
        ps->last_decl_attr = NULL;
        AstNode *td = ast_node_new(AST_TYPEDEF_DECL, first_loc);
        td->u.typedef_decl.alias     = name;
        td->u.typedef_decl.target    = full_ty;
        td->u.typedef_decl.gcc_attrs = td_attr;  /* C8: trailing attribute */
        if (full_ty && full_ty->kind == AST_ENUM_DEF) {
            full_ty->u.enum_def.inline_typedef = true;
            td->u.typedef_decl.target_unowned = true;
        }
        /* C8-fix: enum_body_ref path (inline enum body now stored separately) */
        if (ds.enum_body_ref && ds.enum_body_ref->kind == AST_ENUM_DEF) {
            ast_node_free(td->u.typedef_decl.target);
            td->u.typedef_decl.target = ds.enum_body_ref;
            td->u.typedef_decl.target_unowned = true;
            ds.enum_body_ref->u.enum_def.inline_typedef = true;
        }
        td_add(&ps->typedefs, name);
        td_add(&ps->typedef_aliases, name);
        if (ps_at(ps, STOK_EQ)) {
            ps_error(ps, ps_peek(ps).loc,
                "typedef declarations cannot have an initialiser");
            ps_advance(ps);
            ast_node_free(parse_expr_prec(ps, 2));
        }
        td->u.typedef_decl.has_extension = has_extension;
        first = td;
    } else {
        AstNode *vd = ast_node_new(AST_VAR_DECL, first_loc);
        vd->u.var_decl.type       = full_ty;
        vd->u.var_decl.name       = name;
        vd->u.var_decl.storage    = ds.storage;
        /* propagate _Thread_local to the AST node. */
        vd->u.var_decl.is_thread_local = ds.is_thread_local;
        /* C23: propagate constexpr to the AST node. */
        vd->u.var_decl.is_constexpr    = ds.is_constexpr;
        /* propagate _Alignas to the AST node. */
        vd->u.var_decl.alignas_text    = ds.alignas_text;
        ds.alignas_text = NULL;
        /* GCC attrs.
         * top_gcc_attrs = leading attrs (from decl-specifiers, before type/name)
         * ps->last_decl_attr = trailing attrs (after declarator name) */
        char *trailing_decl_attr = ps->last_decl_attr;
        ps->last_decl_attr = NULL;
        eat_attribute_specifiers(ps, &trailing_decl_attr);
        vd->u.var_decl.gcc_attrs          = top_gcc_attrs;
        vd->u.var_decl.gcc_attrs_trailing  = trailing_decl_attr;
        vd->u.var_decl.fmt |= (ds.fmt & FMTF_ATTRS_LEADING); /* p43 */
        (void)top_trailing_attrs_saved;  /* var trailing attrs via ps->last_decl_attr */
        if (ps_match(ps, STOK_EQ)) {
            vd->u.var_decl.init = ps_at(ps, STOK_LBRACE)
                                ? parse_init_list(ps)
                                : parse_expr_prec(ps, 2);
        } else if (full_ty && full_ty->kind == AST_TYPE_AUTO) {
            ps_error(ps, first_loc, "'auto %s' requires an initialiser", name);
        }
        vd->u.var_decl.has_extension = has_extension;
        first = vd;
    }

    /* Additional declarators after `,`.  Each gets its own clone of the
     * base type AST so that ast_node_free doesn't double-free. */
    while (ps_match(ps, STOK_COMMA)) {
        AstNode *base_copy = ast_clone_type(ds.base_ty);
        char    *next_name = NULL;
        CppLoc   next_loc  = ps_peek(ps).loc;
        AstNode *next_ty   = parse_declarator(ps, base_copy, &next_name);
        char *next_trailing = NULL;
        eat_attribute_specifiers(ps, &next_trailing); /* capture */
        if (!next_name) {
            ps_error(ps, next_loc, "declaration is missing a name");
            next_name = cpp_xstrdup("?");
        }
        AstNode *out = NULL;
        if (ds.storage == SC_TYPEDEF) {
            free(next_trailing); /* typedefs don't propagate gcc_attrs */
            AstNode *td = ast_node_new(AST_TYPEDEF_DECL, next_loc);
            td->u.typedef_decl.alias  = next_name;
            td->u.typedef_decl.target = next_ty;
            if (next_ty && next_ty->kind == AST_ENUM_DEF) {
                next_ty->u.enum_def.inline_typedef = true;
                td->u.typedef_decl.target_unowned = true;
            }
            td_add(&ps->typedefs, next_name);
            td_add(&ps->typedef_aliases, next_name);
            out = td;
        } else {
            AstNode *vd = ast_node_new(AST_VAR_DECL, next_loc);
            vd->u.var_decl.type       = next_ty;
            vd->u.var_decl.name       = next_name;
            vd->u.var_decl.storage    = ds.storage;
            /* propagate _Thread_local. */
            vd->u.var_decl.is_thread_local = ds.is_thread_local;
            /* C23: propagate constexpr to extra comma-separated decls. */
            vd->u.var_decl.is_constexpr    = ds.is_constexpr;
            /* _Alignas on extra comma-separated decls (rare but valid). */
            /* (ds->alignas_text already consumed by first decl; extra decls don't
             * re-read it — C11 allows _Alignas on each declarator separately.) */
            /* trailing attrs only (no leading for extra decls). */
            vd->u.var_decl.gcc_attrs  = next_trailing;
            next_trailing = NULL;
            vd->u.var_decl.is_comma_cont = true; /* Mark for faithful re-emission of comma group */
            if (ps_match(ps, STOK_EQ)) {
                vd->u.var_decl.init = ps_at(ps, STOK_LBRACE)
                                    ? parse_init_list(ps)
                                    : parse_expr_prec(ps, 2);
            } else if (next_ty && next_ty->kind == AST_TYPE_AUTO) {
                ps_error(ps, next_loc, "'auto %s' requires an initialiser", next_name);
            }
            out = vd;
        }
        astvec_push(&ps->pending_decls, out);
    }

    ps_expect(ps, STOK_SEMI, "after variable declaration");
    return first;
}

/* =========================================================================
 * Expression parsing — Pratt / precedence climbing
 * ====================================================================== */

static int binop_prec(SharpTokKind k) {
    switch (k) {
    case STOK_COMMA:                              return 1;
    case STOK_EQ:    case STOK_PLUSEQ:
    case STOK_MINUSEQ: case STOK_STAREQ:
    case STOK_SLASHEQ: case STOK_PERCENTEQ:
    case STOK_AMPEQ:   case STOK_PIPEEQ:
    case STOK_CARETEQ: case STOK_LTLTEQ:
    case STOK_GTGTEQ:                             return 2;
    case STOK_QUESTION:                           return 3;  /* ternary */
    case STOK_PIPEPIPE:                           return 4;
    case STOK_AMPAMP:                             return 5;
    case STOK_PIPE:                               return 6;
    case STOK_CARET:                              return 7;
    case STOK_AMP:                                return 8;
    case STOK_EQEQ:    case STOK_BANGEQ:          return 9;
    case STOK_LT:      case STOK_GT:
    case STOK_LTEQ:    case STOK_GTEQ:            return 10;
    case STOK_LTLT:    case STOK_GTGT:            return 11;
    case STOK_PLUS:    case STOK_MINUS:           return 12;
    case STOK_STAR:    case STOK_SLASH:
    case STOK_PERCENT:                            return 13;
    default:                                      return 0;
    }
}

static bool is_right_assoc(SharpTokKind k) {
    switch (k) {
    case STOK_EQ:      case STOK_PLUSEQ:   case STOK_MINUSEQ:
    case STOK_STAREQ:  case STOK_SLASHEQ:  case STOK_PERCENTEQ:
    case STOK_AMPEQ:   case STOK_PIPEEQ:   case STOK_CARETEQ:
    case STOK_LTLTEQ:  case STOK_GTGTEQ:   case STOK_QUESTION:
        return true;
    default: return false;
    }
}

/* =========================================================================
 * Expression parsing (C11 subset + Sharp extensions)
 * ====================================================================== */

/* Parse argument list for call/method: (e1, e2, ...) */
static void parse_call_args(PS *ps, AstVec *args) {
    ps_expect(ps, STOK_LPAREN, "call '('");
    while (!ps_at(ps, STOK_RPAREN) && !ps_at(ps, STOK_EOF)) {
        astvec_push(args, parse_expr_prec(ps, 2));
        if (!ps_match(ps, STOK_COMMA)) break;
    }
    ps_expect(ps, STOK_RPAREN, "call ')'");
}

static AstNode *parse_primary(PS *ps);
static AstNode *parse_expr_prec(PS *ps, int min_prec);

static AstNode *parse_postfix(PS *ps, AstNode *lhs) {
    for (;;) {
        SharpTok t = ps_peek(ps);
        /* ++ / -- */
        if (t.kind == STOK_PLUSPLUS || t.kind == STOK_MINUSMINUS) {
            ps_advance(ps);
            AstNode *n = ast_node_new(AST_UNARY, t.loc);
            n->u.unary.op      = t.kind;
            n->u.unary.operand = lhs;
            n->u.unary.postfix = true;
            lhs = n;
            continue;
        }
        /* . and -> */
        if (t.kind == STOK_DOT || t.kind == STOK_ARROW) {
            ps_advance(ps);
            bool arrow = (t.kind == STOK_ARROW);
            SharpTok field_tok = ps_expect(ps, STOK_IDENT, "field/method name");
            if (ps_at(ps, STOK_LPAREN)) {
                /* method call */
                AstNode *mc = ast_node_new(AST_METHOD_CALL, t.loc);
                mc->u.method_call.recv   = lhs;
                mc->u.method_call.arrow  = arrow;
                mc->u.method_call.method = cpp_xstrndup(field_tok.text, field_tok.len);
                parse_call_args(ps, &mc->u.method_call.args);
                lhs = mc;
            } else {
                AstNode *fa = ast_node_new(AST_FIELD_ACCESS, t.loc);
                fa->u.field_access.recv  = lhs;
                fa->u.field_access.arrow = arrow;
                fa->u.field_access.field = cpp_xstrndup(field_tok.text, field_tok.len);
                lhs = fa;
            }
            continue;
        }
        /* [] */
        if (t.kind == STOK_LBRACKET) {
            ps_advance(ps);
            AstNode *idx = ast_node_new(AST_INDEX, t.loc);
            idx->u.index_.base  = lhs;
            idx->u.index_.index = parse_expr(ps);
            ps_expect(ps, STOK_RBRACKET, "']'");
            lhs = idx;
            continue;
        }
        /* ( — call */
        if (t.kind == STOK_LPAREN) {
            /* GCC's `__builtin_offsetof(TYPE, MEMBER)` takes a type-name
             * as its first argument, not an expression.  The macro
             * `offsetof` from <stddef.h> usually expands to this form
             * (or an equivalent address-of-zero trick).  Sharp's
             * generic expression parser would choke on `struct X`
             * appearing where it expects an expression, so we
             * intercept the call here and skip its body up to the
             * matching `)` while preserving balanced-paren tracking.
             * The result is constant-folded to a 0-valued size — the
             * generated C output retains the original __builtin_offsetof
             * call so the C compiler computes the actual offset. */
            if (lhs->kind == AST_IDENT &&
                (strcmp(lhs->u.ident.name, "__builtin_offsetof") == 0 ||
                 strcmp(lhs->u.ident.name, "offsetof") == 0)) {
                /* GCC's `__builtin_offsetof(TYPE, MEMBER)` takes a type-name
                 * as its first argument.  We cannot parse `TYPE` as an
                 * expression, so we skip the balanced-paren payload and
                 * reconstruct the full token text as an opaque string that
                 * cg emits verbatim.  This preserves the actual offsetof
                 * value at C compile time (the C compiler evaluates it),
                 * which is critical for runtime-correct struct allocation
                 * (e.g. `sizelstring(l) = offsetof(TString,contents) + l+1`).
                 *
                 * The generated IDENT node's name holds the complete
                 * `__builtin_offsetof(...)` call text so cg can emit it
                 * without modification. */
                char buf[512];
                int blen = snprintf(buf, sizeof buf, "%s(", lhs->u.ident.name);
                ps_advance(ps);
                blen += collect_balanced_call_body(ps, buf + blen, (int)sizeof buf - blen);
                ast_node_free(lhs);
                lhs = ast_node_new(AST_IDENT, t.loc);
                lhs->u.ident.name = cpp_xstrndup(buf, blen);
                continue;
            }
            /* `va_arg(ap, TYPE)` and `__builtin_va_arg(ap, TYPE)` —
             * the type-name second argument cannot be parsed as an
             * expression.  We collect all tokens from '(' to the
             * matching ')' and store the full call text as an
             * AST_IDENT so cg can emit it verbatim.  This preserves
             * the actual va_arg semantics at C-compile time (the C
             * compiler evaluates it), which is critical for functions
             * like lua_gc that dispatch on va_arg results. */
            if (lhs->kind == AST_IDENT &&
                (strcmp(lhs->u.ident.name, "va_arg") == 0 ||
                 strcmp(lhs->u.ident.name, "__builtin_va_arg") == 0)) {
                char buf2[512];
                int blen2 = snprintf(buf2, sizeof buf2, "__builtin_va_arg(");
                ps_advance(ps);
                blen2 += collect_balanced_call_body(ps, buf2 + blen2, (int)sizeof buf2 - blen2);
                ast_node_free(lhs);
                lhs = ast_node_new(AST_IDENT, t.loc);
                lhs->u.ident.name = cpp_xstrndup(buf2, blen2);
                continue;
            }
            /* __builtin_convertvector(expr, TYPE) and
             * __builtin_shufflevector(vec, vec, indices...) take type-name
             * or vector arguments that cannot be parsed as regular
             * expressions.  Capture the entire call text verbatim so cg
             * can emit it unchanged — the C compiler evaluates these. */
            if (lhs->kind == AST_IDENT &&
                (strcmp(lhs->u.ident.name, "__builtin_convertvector") == 0 ||
                 strcmp(lhs->u.ident.name, "__builtin_shufflevector") == 0)) {
                char buf3[1024];
                int blen3 = snprintf(buf3, sizeof buf3, "%s(", lhs->u.ident.name);
                ps_advance(ps);
                blen3 += collect_balanced_call_body(ps, buf3 + blen3, (int)sizeof buf3 - blen3);
                ast_node_free(lhs);
                lhs = ast_node_new(AST_IDENT, t.loc);
                lhs->u.ident.name = cpp_xstrndup(buf3, blen3);
                continue;
            }
            AstNode *c = ast_node_new(AST_CALL, t.loc);
            c->u.call.callee = lhs;
            parse_call_args(ps, &c->u.call.args);
            lhs = c;
            continue;
        }
        break;
    }
    return lhs;
}

/* =========================================================================
 * S4: parse_init_list — `{ a, b, c }` or `{ .x=1, [3]=2 }` or nested.
 *
 * The opening `{` has NOT been consumed yet — the caller checks the
 * lookahead and only invokes us when at `{`.  We consume the matching
 * `}` ourselves.  Trailing commas are allowed (C99).
 *
 * Each element is one of:
 *   - bare expression (positional):       `42`, `foo()`, `{nested}`
 *   - field designator + value:           `.field = 42`
 *   - array designator + value:           `[2] = 42`
 *
 * Multi-designator chains (`.a.b[3] = v`) are not supported in S4 — the
 * scope of probes targeted only uses single designators.
 * ====================================================================== */
static AstNode *parse_init_list(PS *ps) {
    SharpTok lb = ps_expect(ps, STOK_LBRACE, "init list '{'");
    AstNode *il = ast_node_new(AST_INIT_LIST, lb.loc);

    while (!ps_at(ps, STOK_RBRACE) && !ps_at(ps, STOK_EOF)) {
        AstNode *item = NULL;

        if (ps_at(ps, STOK_DOT) && ps_peek2(ps).kind == STOK_IDENT) {
            /* `.field[.field2[...]] = value`  field/nested designator chain.
             * C99 §6.7.9 allows `.a.b`, `.a.b[3]`, etc. as a designator.
             * We collect the full path into a single string so cg emits it
             * verbatim: `.a.j = 5` → field_name = "a.j" → cg: `.a.j = 5`. */
            CppLoc dloc = ps_advance(ps).loc;  /* consume leading '.' */
            char chain[256]; int clen = 0;
            SharpTok nm = ps_advance(ps);      /* first IDENT */
            if (nm.len < (size_t)(sizeof chain - clen - 1)) {
                memcpy(chain + clen, nm.text, nm.len); clen += nm.len;
            }
            /* Continue as long as more .field or [idx] components follow
             * and we haven't yet reached '='. */
            while (!ps_at(ps, STOK_EQ) && !ps_at(ps, STOK_RBRACE) &&
                   !ps_at(ps, STOK_EOF)) {
                if (ps_at(ps, STOK_DOT) && ps_peek2(ps).kind == STOK_IDENT) {
                    ps_advance(ps);  /* '.' */
                    SharpTok nm2 = ps_advance(ps);
                    if (clen + 1 + (int)nm2.len < (int)sizeof chain - 1) {
                        chain[clen++] = '.';
                        memcpy(chain + clen, nm2.text, nm2.len); clen += nm2.len;
                    }
                } else if (ps_at(ps, STOK_LBRACKET)) {
                    /* collect [...] verbatim */
                    ps_advance(ps);  /* '[' */
                    chain[clen++] = '[';
                    while (!ps_at(ps, STOK_RBRACKET) && !ps_at(ps, STOK_EOF)) {
                        SharpTok xt = ps_advance(ps);
                        if (clen + (int)xt.len < (int)sizeof chain - 2) {
                            memcpy(chain + clen, xt.text, xt.len); clen += xt.len;
                        }
                    }
                    if (ps_at(ps, STOK_RBRACKET)) ps_advance(ps);
                    chain[clen++] = ']';
                } else break;
            }
            chain[clen] = '\0';
            ps_expect(ps, STOK_EQ, "designator '='");
            AstNode *val = ps_at(ps, STOK_LBRACE)
                         ? parse_init_list(ps)
                         : parse_expr_prec(ps, 2);
            AstNode *d = ast_node_new(AST_DESIGNATED_INIT, dloc);
            d->u.designated_init.designator_kind = 0;  /* field / chain */
            d->u.designated_init.field_name = cpp_xstrndup(chain, clen);
            d->u.designated_init.value      = val;
            item = d;
        } else if (ps_at(ps, STOK_LBRACKET)) {
            /* `[idx] = value` / `[lo ... hi] = value`  array designator */
            CppLoc dloc = ps_advance(ps).loc;       /* '[' */
            AstNode *ix = parse_expr_prec(ps, 2);
            AstNode *ix_end = NULL;
            if (ps_at(ps, STOK_ELLIPSIS)) {
                ps_advance(ps);  /* eat '...' */
                ix_end = parse_expr_prec(ps, 2);
            }
            ps_expect(ps, STOK_RBRACKET, "array designator ']'");
            ps_expect(ps, STOK_EQ,       "designator '='");
            AstNode *val = ps_at(ps, STOK_LBRACE)
                         ? parse_init_list(ps)
                         : parse_expr_prec(ps, 2);
            AstNode *d = ast_node_new(AST_DESIGNATED_INIT, dloc);
            d->u.designated_init.designator_kind = 1;  /* array */
            d->u.designated_init.index_expr = ix;
            d->u.designated_init.index_end  = ix_end;
            d->u.designated_init.value      = val;
            item = d;
        } else if (ps_at(ps, STOK_LBRACE)) {
            /* nested init list (no designator) */
            item = parse_init_list(ps);
        } else {
            /* positional value */
            item = parse_expr_prec(ps, 2);
        }

        astvec_push(&il->u.init_list.items, item);

        if (!ps_match(ps, STOK_COMMA)) break;
        /* C99 allows trailing comma immediately before `}`. */
        if (ps_at(ps, STOK_RBRACE)) {
            il->u.init_list.trailing_comma = true;
            break;
        }
    }

    ps_expect(ps, STOK_RBRACE, "init list '}'");
    return il;
}

/* =========================================================================
 * typeof inner-expression mini-parser
 *
 * Takes an array of SharpTok (already consumed from the main stream
 * while building the typeof verbatim string) and constructs a lightweight
 * expression AST.  The AST is only used by ty_from_ast to infer the
 * concrete C type — it does NOT go through sema or cg.
 *
 * Handles: integer/float/char/string literals, identifiers, parentheses,
 * unary operators (-, +, !, ~), and binary operators with precedence
 * climbing.  Returns NULL on any unrecognised pattern; the caller falls
 * back to the opaque typeof struct.
 * ====================================================================== */

static AstNode *typeof_parse_primary(SharpTok *tokens, int ntoks, int *pos) {
    if (*pos >= ntoks) return NULL;
    SharpTok t = tokens[*pos];
    AstNode *n = NULL;

    switch (t.kind) {
    case STOK_INT_LIT:
        n = ast_node_new(AST_INT_LIT, t.loc);
        n->u.int_lit.val = t.u.ival;
        n->u.int_lit.is_unsigned = t.is_unsigned;
        break;
    case STOK_FLOAT_LIT:
        n = ast_node_new(AST_FLOAT_LIT, t.loc);
        n->u.float_lit.val = t.u.fval;
        break;
    case STOK_CHAR_LIT:
        n = ast_node_new(AST_CHAR_LIT, t.loc);
        n->u.char_lit.val = t.u.ival;
        break;
    case STOK_STRING_LIT:
        n = ast_node_new(AST_STRING_LIT, t.loc);
        n->u.string_lit.text = cpp_xstrndup(t.text, t.len);
        n->u.string_lit.len  = t.len;
        break;
    case STOK_IDENT:
        n = ast_node_new(AST_IDENT, t.loc);
        n->u.ident.name = cpp_xstrndup(t.text, t.len);
        break;
    case STOK_VOID: case STOK_INT: case STOK_CHAR:
    case STOK_LONG: case STOK_SHORT: case STOK_FLOAT:
    case STOK_DOUBLE: case STOK_SIGNED: case STOK_UNSIGNED:
    case STOK__BOOL: case STOK_STRUCT: case STOK_UNION: case STOK_CLASS:
    case STOK_ENUM: case STOK_CONST:
        n = ast_node_new(AST_TYPE_NAME, t.loc);
        n->u.type_name.name = cpp_xstrndup(t.text, t.len);
        break;
    case STOK_LPAREN: {
        (*pos)++;
        n = typeof_parse_expr(tokens, ntoks, pos);
        if (!n || *pos >= ntoks) return NULL;
        if (tokens[*pos].kind != STOK_RPAREN) return NULL;
        (*pos)++;
        if ((n->kind == AST_IDENT || n->kind == AST_TYPE_NAME) && *pos < ntoks) {
            SharpTokKind nk = tokens[*pos].kind;
            if (nk == STOK_INT_LIT || nk == STOK_FLOAT_LIT ||
                nk == STOK_CHAR_LIT || nk == STOK_STRING_LIT ||
                nk == STOK_IDENT || nk == STOK_LPAREN ||
                nk == STOK_MINUS || nk == STOK_PLUS ||
                nk == STOK_BANG || nk == STOK_TILDE ||
                nk == STOK_VOID || nk == STOK_INT ||
                nk == STOK_CHAR || nk == STOK_LONG ||
                nk == STOK_SHORT || nk == STOK_FLOAT ||
                nk == STOK_DOUBLE || nk == STOK_SIGNED ||
                nk == STOK_UNSIGNED || nk == STOK__BOOL ||
                nk == STOK_STRUCT || nk == STOK_UNION || nk == STOK_CLASS ||
                nk == STOK_ENUM || nk == STOK_CONST) {
                AstNode *cast = ast_node_new(AST_CAST, t.loc);
                cast->u.cast.type = n;
                cast->u.cast.operand = typeof_parse_unary(tokens, ntoks, pos);
                if (!cast->u.cast.operand) { ast_node_free(cast); return NULL; }
                n = cast;
            } else {
                AstNode *pn = ast_node_new(AST_PAREN, t.loc);
                pn->u.paren.inner = n;
                n = pn;
            }
        } else {
            AstNode *pn = ast_node_new(AST_PAREN, t.loc);
            pn->u.paren.inner = n;
            n = pn;
        }
        break;
    }
    default:
        return NULL;
    }
    if (t.kind != STOK_LPAREN) (*pos)++;
    return n;
}

static AstNode *typeof_parse_unary(SharpTok *tokens, int ntoks, int *pos) {
    if (*pos >= ntoks) return NULL;
    SharpTok t = tokens[*pos];

    bool unary = false;
    SharpTokKind op_kind = STOK_EOF;
    switch (t.kind) {
    case STOK_MINUS: op_kind = STOK_MINUS; unary = true; break;
    case STOK_PLUS:  op_kind = STOK_PLUS;  unary = true; break;
    case STOK_BANG:  op_kind = STOK_BANG;  unary = true; break;
    case STOK_TILDE: op_kind = STOK_TILDE; unary = true; break;
    case STOK_STAR:  op_kind = STOK_STAR;  unary = true; break;
    case STOK_AMP:   op_kind = STOK_AMP;   unary = true; break;
    default: break;
    }
    if (unary) {
        (*pos)++;
        AstNode *operand = typeof_parse_unary(tokens, ntoks, pos);
        if (!operand) return NULL;
        AstNode *u = ast_node_new(AST_UNARY, t.loc);
        u->u.unary.op = op_kind;
        u->u.unary.operand = operand;
        return u;
    }
    return typeof_parse_primary(tokens, ntoks, pos);
}

static AstNode *typeof_parse_expr_prec(SharpTok *tokens, int ntoks, int *pos, int min_prec) {
    AstNode *lhs = typeof_parse_unary(tokens, ntoks, pos);
    if (!lhs) return NULL;

    while (*pos < ntoks) {
        SharpTok op = tokens[*pos];
        int prec = binop_prec(op.kind);
        if (prec == 0) prec = -1;
        if (prec < min_prec) break;
        (*pos)++;
        AstNode *rhs = typeof_parse_expr_prec(tokens, ntoks, pos, prec + 1);
        if (!rhs) return NULL;
        AstNode *bin = ast_node_new(AST_BINOP, op.loc);
        bin->u.binop.op = op.kind;
        bin->u.binop.lhs = lhs;
        bin->u.binop.rhs = rhs;
        lhs = bin;
    }
    return lhs;
}

static AstNode *typeof_parse_expr(SharpTok *tokens, int ntoks, int *pos) {
    return typeof_parse_expr_prec(tokens, ntoks, pos, 1);
}


static AstNode *parse_primary(PS *ps) {
    SharpTok t = ps_advance(ps);

    switch (t.kind) {

    /* -- Literals -- */
    case STOK_INT_LIT: {
        AstNode *n = ast_node_new(AST_INT_LIT, t.loc);
        n->u.int_lit.val         = t.u.ival;
        n->u.int_lit.is_unsigned = t.is_unsigned;
        n->u.int_lit.is_long     = t.is_long;
        n->u.int_lit.is_longlong = t.is_longlong;
        /* C3: preserve original token text for faithful C re-emission */
        n->u.int_lit.orig_text = (t.text && t.len > 0)
            ? cpp_xstrndup(t.text, t.len) : NULL;
        return parse_postfix(ps, n);
    }
    case STOK_FLOAT_LIT: {
        AstNode *n = ast_node_new(AST_FLOAT_LIT, t.loc);
        n->u.float_lit.val = t.u.fval;
        /* C3: preserve original token text (e.g. "1.5f", "2.0L") */
        n->u.float_lit.orig_text = (t.text && t.len > 0)
            ? cpp_xstrndup(t.text, t.len) : NULL;
        return parse_postfix(ps, n);
    }
    case STOK_STRING_LIT: {
        /* Collect all adjacent string literals (ISO C phase 6 concatenation).
         * We do NOT merge the string contents — instead we store the original
         * tokens space-joined so cg can emit them verbatim, producing output
         * that is token-identical to gcc -E (which also doesn't merge). */
        AstNode *n = ast_node_new(AST_STRING_LIT, t.loc);
        if (!ps_at(ps, STOK_STRING_LIT)) {
            /* Single string literal — simple case */
            n->u.string_lit.text = t.text;
            n->u.string_lit.len  = t.len;
        } else {
            /* Multiple adjacent strings — join with spaces, keep all quotes */
            size_t total = t.len;
            size_t count = 0;
            while (ps->pos + count < ps->ntoks &&
                   ps->toks[ps->pos + count].kind == STOK_STRING_LIT) {
                total += 1 + ps->toks[ps->pos + count].len; /* ' ' + token */
                count++;
            }
            char *buf = malloc(total + 1);
            if (!buf) abort();
            memcpy(buf, t.text, t.len);
            size_t off = t.len;
            for (size_t ci = 0; ci < count; ci++) {
                buf[off++] = ' ';
                memcpy(buf + off, ps->toks[ps->pos + ci].text,
                       ps->toks[ps->pos + ci].len);
                off += ps->toks[ps->pos + ci].len;
            }
            buf[off] = '\0';
            ps->pos += count;
            n->u.string_lit.text = buf;
            n->u.string_lit.len  = off;
        }
        return parse_postfix(ps, n);
    }
    case STOK_CHAR_LIT: {
        AstNode *n = ast_node_new(AST_CHAR_LIT, t.loc);
        n->u.char_lit.val = t.u.ival;
        /* C3: preserve original token text (e.g. "'n'", "'\\n'") */
        n->u.char_lit.orig_text = (t.text && t.len > 0)
            ? cpp_xstrndup(t.text, t.len) : NULL;
        return parse_postfix(ps, n);
    }
    /* null was removed as a Sharp keyword — users write NULL (C macro) */

    /* -- GCC extension keyword — consumes __extension__ in
     *    expression context, then parses the real expression.
     *    Used extensively by glibc assert.h / stdatomic.h
     *    to wrap statement-expressions.  The keyword is a
     *    no-op that suppresses warnings — dropping it from the
     *    AST is semantically correct in all cases. */
    case STOK__EXTENSION__:
        return parse_primary(ps);

    /* -- Grouping: (expr) or (Type)expr -- */
    case STOK_LPAREN: {
        /* GCC statement-expression ({ stmts; expr; }).
         * Detected by the lead token being `{` immediately after `(`.
         * `{` cannot start a type, so this check is unambiguous and must
         * precede the is_type_start heuristic.  The value is the last
         * expression-statement's value (sema determines the type). */
        if (ps_at(ps, STOK_LBRACE)) {
            AstNode *block = parse_block(ps);
            ps_expect(ps, STOK_RPAREN, "closing ')' of statement-expression");
            AstNode *n = ast_node_new(AST_STMT_EXPR, t.loc);
            n->u.stmt_expr.block = block;
            return parse_postfix(ps, n);
        }
        /* heuristic: if inner is a type, this is either a cast or a
         * compound literal (S4) — a `(Type){ init-list }` expression. */
        size_t save = ps->pos;
        /* GCC allows `(__attribute__(...) type)` casts.  Consume any
         * leading attributes BEFORE setting the save point, so that a
         * failed cast rollback does not re-expose the attributes to the
         * expression parser (which cannot handle them). */
        bool had_leading_attr = false;
        while (ps_at(ps, STOK_ATTRIBUTE)) {
            eat_attribute_specifiers(ps, NULL);
            had_leading_attr = true;
        }
        if (had_leading_attr) save = ps->pos;  /* update save past attrs */
        if (is_type_start(ps)) {
            AstNode *ty = parse_type(ps);
            if (ps_at(ps, STOK_RPAREN)) {
                ps_advance(ps);
                /* S4: compound literal — `(Type){...}` produces an
                 * unnamed object of the named type initialised by the
                 * brace-list.  Only matched when `{` immediately follows
                 * the closing `)`; otherwise this is a normal cast. */
                if (ps_at(ps, STOK_LBRACE)) {
                    AstNode *cl = ast_node_new(AST_COMPOUND_LIT, t.loc);
                    cl->u.compound_lit.type = ty;
                    cl->u.compound_lit.init = parse_init_list(ps);
                    return parse_postfix(ps, cl);
                }
                /* cast */
                AstNode *c = ast_node_new(AST_CAST, t.loc);
                c->u.cast.type    = ty;
                c->u.cast.operand = parse_expr_prec(ps, 14); /* unary prec */
                return parse_postfix(ps, c);
            }
            /* not a cast — rollback */
            ast_node_free(ty);
            ps->pos = save;
        }
        AstNode *inner = parse_expr(ps);
        ps_expect(ps, STOK_RPAREN, "closing ')'");
        /* C8: preserve user-written parens as AST_PAREN so cg emits them
         * verbatim — needed for token-identical round-trip. */
        AstNode *pn = ast_node_new(AST_PAREN, t.loc);
        pn->u.paren.inner = inner;
        return parse_postfix(ps, pn);
    }

    /* -- sizeof -- */
    /* __typeof__(expr) / __typeof__(type) in expression context.
     * Used as arguments to __builtin_types_compatible_p, etc.
     * Eat the balanced (...) payload and return a dummy 0 literal.
     * The enclosing __builtin_* call is handled by sema as returning int. */
    case STOK_TYPEOF: {
        if (ps_at(ps, STOK_LPAREN)) {
            ps_advance(ps);  /* eat '(' */
            int _td = 0;
            while (!ps_at(ps, STOK_EOF)) {
                SharpTokKind _tk = ps_peek(ps).kind;
                if (_tk == STOK_LPAREN) { _td++; ps_advance(ps); }
                else if (_tk == STOK_RPAREN) {
                    ps_advance(ps);
                    if (_td == 0) break;
                    _td--;
                } else { ps_advance(ps); }
            }
        }
        AstNode *_z = ast_node_new(AST_INT_LIT, t.loc);
        _z->u.int_lit.val = 0;
        return _z;
    }

    case STOK_SIZEOF: {
        AstNode *n = ast_node_new(AST_SIZEOF, t.loc);
        /* `sizeof expr` (without parentheses) is valid C when
         * the operand is an expression (ISO 6.5.3.4).  Parentheses are
         * only required for type operands.  Brotli's platform.h uses
         * `sizeof t` where `t` is a local variable. */
        if (!ps_at(ps, STOK_LPAREN)) {
            /* No '(' — must be `sizeof expr` (unary precedence, so parse
             * at the cast/unary level to avoid swallowing too much). */
            n->u.sizeof_.operand = parse_expr_prec(ps, 14); /* unary prec */
            n->u.sizeof_.is_type = false;
            return n;
        }
        ps_advance(ps);  /* consume '(' */
        if (is_type_start(ps)) {
            AstNode *ty = parse_type(ps);
            /* `sizeof(T[expr])` — VLA/array-type operand.
             * After parsing the base type, consume any `[size]` suffixes so
             * `sizeof(char[(c)?1:-1])` parses as `sizeof(char[-1])`.
             * `parse_array_suffix` handles nested `[N1][N2]` too. */
            if (ps_at(ps, STOK_LBRACKET))
                ty = parse_array_suffix(ps, ty);
            n->u.sizeof_.operand = ty;
            n->u.sizeof_.is_type = true;
        } else {
            n->u.sizeof_.operand = parse_expr(ps);
            n->u.sizeof_.is_type = false;
            n->u.sizeof_.expr_has_parens = true;  /* C8: had (expr) form */
        }
        ps_expect(ps, STOK_RPAREN, "sizeof ')'");
        return n;
    }

    /* `_Alignof(T)` / `__alignof(T)` / `__alignof__(T)`.
     * Returns the alignment requirement of T as a size_t constant.
     * We parse the type operand and emit `_Alignof(T)` in cg (standard C11
     * form accepted by all downstream compilers). */
    case STOK__ALIGNOF: {
        AstNode *n = ast_node_new(AST_SIZEOF, t.loc);  /* reuse sizeof node */
        ps_expect(ps, STOK_LPAREN, "_Alignof '('");
        if (is_type_start(ps)) {
            AstNode *ty = parse_type(ps);
            if (ps_at(ps, STOK_LBRACKET))
                ty = parse_array_suffix(ps, ty);
            n->u.sizeof_.operand = ty;
            n->u.sizeof_.is_type = true;
        } else {
            n->u.sizeof_.operand = parse_expr(ps);
            n->u.sizeof_.is_type = false;
        }
        n->u.sizeof_.is_alignof = true;  /* signal cg to emit _Alignof */
        /* C8: preserve original spelling: __alignof__, __alignof, or _Alignof */
        n->u.sizeof_.alignof_spelling = cpp_xstrndup(t.text, t.len);
        ps_expect(ps, STOK_RPAREN, "_Alignof ')'");
        return n;
    }

    /* `_Generic(expr, T1: val1, ..., default: valD)`.
     * C11 generic selection — returns the value whose type matches. */
    case STOK__GENERIC: {
        AstNode *n = ast_node_new(AST_GENERIC_EXPR, t.loc);
        ps_expect(ps, STOK_LPAREN, "_Generic '('");
        n->u.generic_expr.controlling = parse_expr_prec(ps, 2);
        ps_expect(ps, STOK_COMMA, "',' after controlling expression in _Generic");
        while (!ps_at(ps, STOK_RPAREN) && !ps_at(ps, STOK_EOF)) {
            AstNode *assoc = ast_node_new(AST_GENERIC_ASSOC, ps_peek(ps).loc);
            if (ps_at(ps, STOK_DEFAULT)) {
                ps_advance(ps);
                assoc->u.generic_assoc.is_default = true;
            } else {
                assoc->u.generic_assoc.type_name = parse_type(ps);
            }
            ps_expect(ps, STOK_COLON, "':' in _Generic association");
            assoc->u.generic_assoc.value = parse_expr_prec(ps, 2);
            astvec_push(&n->u.generic_expr.associations, assoc);
            if (!ps_match(ps, STOK_COMMA)) break;
        }
        ps_expect(ps, STOK_RPAREN, "_Generic ')'");
        return parse_postfix(ps, n);
    }

    /* -- @intrinsic -- */
    case STOK_AT_INTRINSIC: {
        AstNode *n = ast_node_new(AST_AT_INTRINSIC, t.loc);
        /* text is "@name"; skip the '@' */
        size_t skip = (t.len > 0 && t.text[0] == '@') ? 1 : 0;
        n->u.at_intrinsic.name = cpp_xstrndup(t.text + skip, t.len - skip);

        if (ps_at(ps, STOK_LPAREN)) {
            /* @has_operator(T, op) requires special parsing for the
             * second argument: `op` is a raw operator token (+, ==,
             * [], etc.) which parse_expr cannot handle as a standalone
             * expression.  Detect the @has_operator name and parse its
             * two args separately; all other @intrinsics use the normal
             * parse_call_args path. */
            const char *iname = n->u.at_intrinsic.name;
            bool is_has_op     = strcmp(iname, "has_operator") == 0;
            bool is_has_method = strcmp(iname, "has_method") == 0;
            bool is_has_field  = strcmp(iname, "has_field")  == 0;
            bool is_type_name  = strcmp(iname, "type_name")  == 0;
            bool is_is_arith   = strcmp(iname, "is_arithmetic") == 0;
            bool is_is_ptr     = strcmp(iname, "is_pointer") == 0;
            /* v0.13: the 5 new intrinsics all take a TYPE as the first
             * arg (which may be a built-in keyword like `int`).  We use
             * parse_type for arg[0]; has_method/has_field additionally
             * expect an IDENT for arg[1]. */
            bool needs_type_arg = is_has_method || is_has_field ||
                                  is_type_name || is_is_arith || is_is_ptr;
            if (is_has_op) {
                ps_advance(ps);  /* eat '(' */
                /* Arg 0: type name — parse as normal expression (IDENT). */
                astvec_push(&n->u.at_intrinsic.args, parse_expr_prec(ps, 2));
                ps_match(ps, STOK_COMMA);
                /* Arg 1: operator token — consume it directly and wrap
                 * in an AST_IDENT carrying the operator text.  This
                 * handles: +  -  *  /  %  ==  !=  <  >  <=  >=
                 *           &  |  ^  <<  >>  []  =  (and combos)
                 * We collect up to two tokens for two-char operators
                 * (== != <= >= << >> []); a single token for the rest. */
                SharpTok op1 = ps_advance(ps);
                CppLoc op_loc = op1.loc;
                char op_buf[8] = {0};
                size_t op_len = op1.len < 4 ? op1.len : 3;
                memcpy(op_buf, op1.text, op_len);
                /* For `[]`, the lexer gives us two separate tokens. */
                if (op1.kind == STOK_LBRACKET && ps_at(ps, STOK_RBRACKET)) {
                    ps_advance(ps);
                    op_buf[op_len++] = ']';
                }
                AstNode *op_node = ast_node_new(AST_IDENT, op_loc);
                op_node->u.ident.name = cpp_xstrndup(op_buf, op_len);
                astvec_push(&n->u.at_intrinsic.args, op_node);
                ps_expect(ps, STOK_RPAREN, "@has_operator ')'");
            } else if (needs_type_arg) {
                ps_advance(ps);  /* eat '(' */
                /* Arg 0: type (may be a C keyword like `int`). */
                AstNode *ty_arg = parse_type(ps);
                astvec_push(&n->u.at_intrinsic.args, ty_arg);
                /* Arg 1: identifier (only for has_method / has_field). */
                if (is_has_method || is_has_field) {
                    ps_match(ps, STOK_COMMA);
                    SharpTok nm = ps_expect(ps, STOK_IDENT,
                        is_has_method ? "method name" : "field name");
                    AstNode *nm_node = ast_node_new(AST_IDENT, nm.loc);
                    nm_node->u.ident.name = cpp_xstrndup(nm.text, nm.len);
                    astvec_push(&n->u.at_intrinsic.args, nm_node);
                }
                ps_expect(ps, STOK_RPAREN, "@<intrinsic> ')'");
            } else {
                parse_call_args(ps, &n->u.at_intrinsic.args);
            }
        }
        return n;
    }

    /* -- Identifier (or Type-qualified call / struct literal) -- */
    case STOK_IDENT: {
        AstNode *id = ast_node_new(AST_IDENT, t.loc);
        id->u.ident.name = cpp_xstrndup(t.text, t.len);

        /* struct literal: TypeName { field: val, ... }
         * Detected when IDENT is followed by '{' */
        if (ps_at(ps, STOK_LBRACE)) {
            /* peek ahead: is it a struct literal?
             * Heuristic: IDENT '{' IDENT ':' → struct literal */
            SharpTok nxt  = ps_peek(ps);    /* '{' */
            (void)nxt;
            /* We do a simple lookahead-2 check */
            size_t save2 = ps->pos;
            ps_advance(ps); /* consume '{' */
            /* struct literal: starts with .field = ... (C-style designated init) */
            if (ps_at(ps, STOK_DOT) || ps_at(ps, STOK_RBRACE)) {
                if (ps_at(ps, STOK_DOT)) {
                    ps_advance(ps); /* '.' */
                    if (ps_at(ps, STOK_IDENT) && ps_peek2(ps).kind == STOK_EQ) {
                        /* yes, struct literal */
                        ps->pos = save2; /* back to '{' */
                        ps_advance(ps); /* consume '{' */
                        AstNode *sl = ast_node_new(AST_STRUCT_LIT, t.loc);
                        AstNode *ty = ast_node_new(AST_TYPE_NAME, t.loc);
                        ty->u.type_name.name = cpp_xstrndup(t.text, t.len);
                        sl->u.struct_lit.type = ty;
                        while (!ps_at(ps, STOK_RBRACE) && !ps_at(ps, STOK_EOF)) {
                            ps_expect(ps, STOK_DOT, "designator '.'");
                            SharpTok fn_tok = ps_expect(ps, STOK_IDENT, "field name");
                            AstNode *fn_node = ast_node_new(AST_IDENT, fn_tok.loc);
                            fn_node->u.ident.name = cpp_xstrndup(fn_tok.text, fn_tok.len);
                            astvec_push(&sl->u.struct_lit.field_names, fn_node);
                            ps_expect(ps, STOK_EQ, "'=' after field name");
                            astvec_push(&sl->u.struct_lit.field_vals, parse_expr_prec(ps, 2));
                            if (!ps_match(ps, STOK_COMMA)) break;
                        }
                        ps_expect(ps, STOK_RBRACE, "struct literal '}'");
                        ast_node_free(id);
                        return parse_postfix(ps, sl);
                    }
                }
                /* empty braces or not a struct literal — rollback */
                ps->pos = save2;
            } else {
                ps->pos = save2;
            }
        }

        /* ── IDENT '<' — generic expression disambiguation ─────────────
         *
         * parse_generic_args uses safe speculation: if the '<' is not
         * followed by a valid generic argument list it backtracks without
         * consuming any tokens.  So we always try — no pre-scan gate needed.
         *
         * Three sub-cases after a successful type-arg parse:
         *   (a) IDENT<args>(call_args)           → AST_GENERIC_CALL
         *   (b) IDENT<args>.method / ->method     → carrier then postfix
         *   (c) IDENT<args>{ .field=val, ... }    → AST_STRUCT_LIT      */
        if (ps_at(ps, STOK_LT)) {
            size_t save2    = ps->pos;
            size_t save_diag = ps->diags ? ps->diags->len : 0;
            AstVec type_args = {0};

            if (parse_generic_args(ps, &type_args)) {

                /* (c) Generic struct literal ───────────────────────── */
                if (ps_at(ps, STOK_LBRACE)) {
                    ps_advance(ps); /* consume '{' */
                    AstNode *sl     = ast_node_new(AST_STRUCT_LIT, id->loc);
                    AstNode *gen_ty = ast_node_new(AST_TYPE_GENERIC, id->loc);
                    gen_ty->u.type_generic.name = cpp_xstrdup(id->u.ident.name);
                    gen_ty->u.type_generic.args = type_args;
                    sl->u.struct_lit.type = gen_ty;
                    while (!ps_at(ps, STOK_RBRACE) && !ps_at(ps, STOK_EOF)) {
                        ps_expect(ps, STOK_DOT, "designator '.'");
                        SharpTok fn_tok = ps_expect(ps, STOK_IDENT, "field name");
                        AstNode *fn_node = ast_node_new(AST_IDENT, fn_tok.loc);
                        fn_node->u.ident.name =
                            cpp_xstrndup(fn_tok.text, fn_tok.len);
                        astvec_push(&sl->u.struct_lit.field_names, fn_node);
                        ps_expect(ps, STOK_EQ, "'=' after field name");
                        astvec_push(&sl->u.struct_lit.field_vals,
                                    parse_expr_prec(ps, 2));
                        if (!ps_match(ps, STOK_COMMA)) break;
                    }
                    ps_expect(ps, STOK_RBRACE, "struct literal '}'");
                    ast_node_free(id);
                    return parse_postfix(ps, sl);
                }

                /* (a) Generic function call ─────────────────────────
                 * IDENT<args>(call_args)  →  AST_GENERIC_CALL
                 * Replaces the old AST_CAST{TYPE_GENERIC,NULL} hack.
                 * The completed call node is passed to parse_postfix so
                 * chained operations like swap<int>(&a,&b)[0] work.    */
                if (ps_at(ps, STOK_LPAREN)) {
                    ps_advance(ps); /* consume '(' */
                    AstNode *gc = ast_node_new(AST_GENERIC_CALL, id->loc);
                    gc->u.generic_call.name =
                        cpp_xstrdup(id->u.ident.name);
                    gc->u.generic_call.type_args = type_args;
                    while (!ps_at(ps, STOK_RPAREN) && !ps_at(ps, STOK_EOF)) {
                        astvec_push(&gc->u.generic_call.call_args,
                                    parse_expr_prec(ps, 2));
                        if (!ps_match(ps, STOK_COMMA)) break;
                    }
                    ps_expect(ps, STOK_RPAREN, "')' in generic function call");
                    ast_node_free(id);
                    return parse_postfix(ps, gc);
                }

                /* (b) Generic type used as method receiver ──────────
                 * IDENT<args>.method(…)  or  IDENT<args>->method(…)
                 * Use the AST_CAST{TYPE_GENERIC,NULL} carrier so that
                 * parse_postfix builds AST_METHOD_CALL on top of it,
                 * and cg_expr's method-call path can read the generic
                 * type from carrier->u.cast.type.                      */
                if (ps_at(ps, STOK_DOT) || ps_at(ps, STOK_ARROW)) {
                    AstNode *gen = ast_node_new(AST_TYPE_GENERIC, id->loc);
                    gen->u.type_generic.name = cpp_xstrdup(id->u.ident.name);
                    gen->u.type_generic.args = type_args;
                    AstNode *carrier = ast_node_new(AST_CAST, id->loc);
                    carrier->u.cast.type    = gen;
                    carrier->u.cast.operand = NULL; /* signals "type-expr, not cast" */
                    ast_node_free(id);
                    return parse_postfix(ps, carrier);
                }

                /* Unexpected suffix — roll back type-arg parse */
                for (size_t _i = 0; _i < type_args.len; _i++)
                    ast_node_free(type_args.data[_i]);
                astvec_free(&type_args);
            }
            /* parse_generic_args failed — roll back */
            ps->pos = save2;
            rollback_diags(ps, save_diag);
        }

        return parse_postfix(ps, id);
    }

    /* -- this -- */
    case STOK_THIS: {
        AstNode *n = ast_node_new(AST_IDENT, t.loc);
        n->u.ident.name = cpp_xstrndup("this", 4);
        return parse_postfix(ps, n);
    }

    /* -- Address-of / deref (treated as unary, handled in parse_expr_prec) -- */
    default:
        ps->pos--;  /* un-advance */
        ps_error(ps, t.loc, "unexpected token '%.*s' in expression",
                 (int)t.len, t.text);
        {
            AstNode *err = ast_node_new(AST_IDENT, t.loc);
            err->u.ident.name = cpp_xstrndup("?", 1);
            ps_advance(ps);
            return err;
        }
    }
}

static AstNode *parse_expr_prec(PS *ps, int min_prec) {
    SharpTok t = ps_peek(ps);
    AstNode *lhs;

    /* Phase S5: GCC labels-as-values — `&&label`.
     * `&&` only appears here (expression start) when it is the
     * address-of-label operator; the binary `&&` is consumed in the
     * binop loop after an lhs is built.  We peek for an IDENT
     * follow-up to be safe; anything else is a parse error. */
    if (t.kind == STOK_AMPAMP) {
        SharpTok t2 = ps_peek2(ps);
        if (t2.kind == STOK_IDENT) {
            ps_advance(ps); /* consume `&&` */
            SharpTok lbl = ps_advance(ps); /* consume label name */
            AstNode *n = ast_node_new(AST_ADDR_OF_LABEL, t.loc);
            n->u.addr_of_label.label = cpp_xstrndup(lbl.text, lbl.len);
            lhs = n;
            goto have_lhs;
        }
    }

    /* Prefix unary operators */
    if (t.kind == STOK_MINUS || t.kind == STOK_BANG ||
        t.kind == STOK_TILDE || t.kind == STOK_PLUS  ||
        t.kind == STOK_AMP   || t.kind == STOK_STAR  ||
        t.kind == STOK_PLUSPLUS || t.kind == STOK_MINUSMINUS) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_UNARY, t.loc);
        n->u.unary.op      = t.kind;
        n->u.unary.operand = parse_expr_prec(ps, 14);  /* unary binds tight */
        n->u.unary.postfix = false;
        lhs = parse_postfix(ps, n);
    } else {
        lhs = parse_primary(ps);
    }
have_lhs:;

    for (;;) {
        SharpTok op = ps_peek(ps);
        int prec = binop_prec(op.kind);
        if (prec < min_prec || prec == 0) break;

        ps_advance(ps);

        /* ternary */
        if (op.kind == STOK_QUESTION) {
            AstNode *then_ = parse_expr(ps);
            ps_expect(ps, STOK_COLON, "':' in ternary");
            AstNode *else_ = parse_expr_prec(ps, prec);
            AstNode *ter = ast_node_new(AST_TERNARY, op.loc);
            ter->u.ternary.cond  = lhs;
            ter->u.ternary.then_ = then_;
            ter->u.ternary.else_ = else_;
            lhs = ter;
            continue;
        }

        int next_prec = is_right_assoc(op.kind) ? prec : prec + 1;
        AstNode *rhs = parse_expr_prec(ps, next_prec);

        /* comma operator → AST_COMMA (not AST_BINOP) */
        if (op.kind == STOK_COMMA) {
            AstNode *c = ast_node_new(AST_COMMA, op.loc);
            c->u.comma.lhs = lhs;
            c->u.comma.rhs = rhs;
            lhs = c;
        } else {
            AstNode *bin = ast_node_new(AST_BINOP, op.loc);
            bin->u.binop.op  = op.kind;
            bin->u.binop.lhs = lhs;
            bin->u.binop.rhs = rhs;
            lhs = bin;
        }
    }
    return lhs;
}

static AstNode *parse_expr(PS *ps) {
    return parse_expr_prec(ps, 1);  /* start at lowest precedence (comma) */
}

/* =========================================================================
 * Statement parsing
 * ====================================================================== */

static AstNode *parse_block(PS *ps) {
    SharpTok t = ps_peek(ps);
    ps_expect(ps, STOK_LBRACE, "block '{'");
    AstNode *blk = ast_node_new(AST_BLOCK, t.loc);
    while (!ps_at(ps, STOK_RBRACE) && !ps_at(ps, STOK_EOF)) {
        AstNode *s = parse_stmt(ps);
        /* fix: drain pending_decls in two passes so that inline
         * anonymous type definitions (AST_STRUCT_DEF / AST_ENUM_DEF) are
         * emitted BEFORE the variable that uses them, while additional
         * declarators from `int x=1, y=2;` (which ARE var_decl siblings)
         * are emitted AFTER the primary declarator `s`.
         *
         * Example of the ordering problem without this fix:
         *   const union { U32 u; BYTE c[4]; } one = { 1 };
         * would emit:
         *   const __anon_union_19 one = {1};   ← type not yet defined!
         *   typedef union __anon_union_19 ...;
         *   union __anon_union_19 { ... };
         * With the fix:
         *   typedef union __anon_union_19 ...;
         *   union __anon_union_19 { ... };     ← defined first ✓
         *   const __anon_union_19 one = {1};
         *
         * Pass 1: struct/union/enum definitions — must precede their use */
        size_t npd = ps->pending_decls.len;
        for (size_t i = 0; i < npd; i++) {
            AstNode *pd = ps->pending_decls.data[i];
            if (pd && (pd->kind == AST_STRUCT_DEF || pd->kind == AST_ENUM_DEF)) {
                astvec_push(&blk->u.block.stmts, pd);
                if (pd->u.struct_def.name) {
                    td_add(&ps->typedefs, pd->u.struct_def.name);
                    td_add(&ps->tag_names, pd->u.struct_def.name);
                }
            }
        }
        /* Main statement */
        if (s) astvec_push(&blk->u.block.stmts, s);
        /* Pass 2: additional declarators (`int x=1, y=2;` siblings) */
        for (size_t i = 0; i < npd; i++) {
            AstNode *pd = ps->pending_decls.data[i];
            if (pd && pd->kind != AST_STRUCT_DEF && pd->kind != AST_ENUM_DEF)
                astvec_push(&blk->u.block.stmts, pd);
        }
        ps->pending_decls.len = 0;
    }
    ps_expect(ps, STOK_RBRACE, "block '}'");
    return blk;
}

/* =========================================================================
 * Statement and block parsing
 * ====================================================================== */

static AstNode *parse_stmt(PS *ps) {
    SharpTok t = ps_peek(ps);

    /* bare ';' — null statement. Return an AST_EXPR_STMT with NULL expr
     * so that cg.c emits the semicolon faithfully (needed for token-level
     * roundtrip when a macro expands to a block `{ ... }` and is called as
     * a statement: `MACRO();` → `{ ... };` — the trailing `;` must be kept). */
    if (t.kind == STOK_SEMI) {
        ps_advance(ps);
        AstNode *ns = ast_node_new(AST_EXPR_STMT, t.loc);
        ns->u.expr_stmt.expr = NULL;
        return ns;
    }

    /* C8: bare `__attribute__((...));` as a statement.  GCC's
     * fallthrough marker (`__attribute__((fallthrough));`) and other
     * statement-level attributes appear unbound.  In C mode emit them
     * verbatim (AST_GCC_VERBATIM); in Sharp mode silently discard. */
    if (t.kind == STOK_ATTRIBUTE) {
        size_t start_pos = ps->pos;
        eat_attribute_specifiers(ps, NULL);
        size_t end_pos = ps->pos;
        if (ps_at(ps, STOK_SEMI)) {
            ps_advance(ps);
            char *text = build_verbatim(ps, start_pos, end_pos);
            AstNode *vn = ast_node_new(AST_GCC_VERBATIM, t.loc);
            vn->u.gcc_verbatim.text    = text;
            vn->u.gcc_verbatim.is_stmt = true;
            return vn;
        }
        /* attributes followed by something else are part of a decl;
         * fall through to the declaration path below. */
    }

    /*  C7: `__asm__(...)` as a standalone statement.
     * Preserve as AST_GCC_VERBATIM for C mode passthrough. */
    if (t.kind == STOK_ASM) {
        size_t start_pos = ps->pos;
        ps_advance(ps);  /* eat 'asm' / '__asm__' */
        while (ps_at(ps, STOK_VOLATILE) || ps_at(ps, STOK_INLINE) ||
               ps_at(ps, STOK_GOTO)) {
            ps_advance(ps);
        }
        eat_attribute_specifiers(ps, NULL);
        if (ps_match(ps, STOK_LPAREN)) {
            skip_balanced_parens(ps);
        }
        size_t end_pos = ps->pos;
        ps_match(ps, STOK_SEMI);
        char *text = build_verbatim(ps, start_pos, end_pos);
        AstNode *vn = ast_node_new(AST_GCC_VERBATIM, t.loc);
        vn->u.gcc_verbatim.text    = text;
        vn->u.gcc_verbatim.is_stmt = true;
        return vn;
    }

    /*  C7: `_Static_assert(cond, "msg");` as a statement.
     * Preserve as AST_GCC_VERBATIM for C mode passthrough. */
    if (t.kind == STOK__STATIC_ASSERT) {
        size_t start_pos = ps->pos;
        ps_advance(ps);
        if (ps_match(ps, STOK_LPAREN)) {
            skip_balanced_parens(ps);
        }
        size_t end_pos = ps->pos;
        ps_match(ps, STOK_SEMI);
        char *text = build_verbatim(ps, start_pos, end_pos);
        AstNode *vn = ast_node_new(AST_GCC_VERBATIM, t.loc);
        vn->u.gcc_verbatim.text    = text;
        vn->u.gcc_verbatim.is_stmt = true;
        return vn;
    }
    /* block */
    if (t.kind == STOK_LBRACE) return parse_block(ps);

    /* if */
    if (t.kind == STOK_IF) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_IF, t.loc);
        ps_expect(ps, STOK_LPAREN, "if '('");
        n->u.if_.cond = parse_expr(ps);
        ps_expect(ps, STOK_RPAREN, "if ')'");
        n->u.if_.then_ = parse_stmt(ps);
        if (ps_at(ps, STOK_ELSE)) {
            ps_advance(ps);
            n->u.if_.else_ = parse_stmt(ps);
        }
        return n;
    }

    /* while */
    if (t.kind == STOK_WHILE) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_WHILE, t.loc);
        ps_expect(ps, STOK_LPAREN, "while '('");
        n->u.while_.cond = parse_expr(ps);
        ps_expect(ps, STOK_RPAREN, "while ')'");
        n->u.while_.body = parse_stmt(ps);
        return n;
    }

    /* do-while */
    if (t.kind == STOK_DO) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_DO_WHILE, t.loc);
        n->u.do_while.body = parse_stmt(ps);
        ps_expect(ps, STOK_WHILE, "do-while 'while'");
        ps_expect(ps, STOK_LPAREN, "do-while '('");
        n->u.do_while.cond = parse_expr(ps);
        ps_expect(ps, STOK_RPAREN, "do-while ')'");
        ps_expect(ps, STOK_SEMI,   "do-while ';'");
        return n;
    }

    /* S2: switch (cond) body  ---  body is normally a brace-block whose
     *     statements interleave 'case' / 'default' labels with regular
     *     statements.  We accept any statement form for the body (even a
     *     bare statement) to match the C grammar permissively. */
    if (t.kind == STOK_SWITCH) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_SWITCH, t.loc);
        ps_expect(ps, STOK_LPAREN, "switch '('");
        n->u.switch_.cond = parse_expr(ps);
        ps_expect(ps, STOK_RPAREN, "switch ')'");
        AstNode *body = parse_stmt(ps);
        /* Duff's device / switch-without-braces: when the body is a case
         * or default label (not wrapped in {}), the statements that follow
         * — including cascade labels (case 0: case 1: stmt;) and mixed
         * case/body sequences — belong inside the switch.  We collect them
         * into a synthetic block so codegen wraps them correctly. */
        if (body && (body->kind == AST_CASE || body->kind == AST_DEFAULT)) {
            AstNode *block = ast_node_new(AST_BLOCK, body->loc);
            astvec_push(&block->u.block.stmts, body);
            int had_body = 0;
            while (!ps_at(ps, STOK_EOF)) {
                SharpTok peek = ps_peek(ps);
                if (had_body && peek.kind != STOK_CASE && peek.kind != STOK_DEFAULT)
                    break;
                AstNode *stmt = parse_stmt(ps);
                if (!stmt) break;
                astvec_push(&block->u.block.stmts, stmt);
                had_body = (stmt->kind != AST_CASE && stmt->kind != AST_DEFAULT);
        }
        body = block;
    }
        n->u.switch_.body = body;
        return n;
    }

    /* S2: case  CONST_EXPR  ':'   ---  the label is a "statement" in
     *     parse-tree terms but produces no work itself; the next
     *     statement after the colon is unrelated and parsed by the
     *     surrounding block loop.  GCC-style range-cases (`case A...B:`)
     *     are not supported. */
    if (t.kind == STOK_CASE) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_CASE, t.loc);
        /* Use prec-2 to skip the comma operator; case values are constant
         * expressions in C (we don't evaluate, cc does). */
        n->u.case_.value = parse_expr_prec(ps, 2);
        ps_expect(ps, STOK_COLON, "case ':'");
        return n;
    }

    if (t.kind == STOK_DEFAULT) {
        ps_advance(ps);
        ps_expect(ps, STOK_COLON, "default ':'");
        return ast_node_new(AST_DEFAULT, t.loc);
    }

    /* for */
    if (t.kind == STOK_FOR) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_FOR, t.loc);
        ps_expect(ps, STOK_LPAREN, "for '('");
        /* init */
        if (!ps_at(ps, STOK_SEMI)) {
            if (is_type_start(ps)) {
                CppLoc init_loc = ps_peek(ps).loc;
                AstNode *init = parse_stmt(ps);  /* decl stmt */
                /* Multi-variable form `for (int i=0, j=1; …)`: the
                 * first declarator is returned as `init`; the rest land
                 * in ps->pending_decls.  Drain them into a compound-init
                 * AST_BLOCK so the codegen can emit them as a C99
                 * comma-declaration (e.g. `int i=0, j=1`). */
                if (ps->pending_decls.len > 0) {
                    AstNode *ci = ast_node_new(AST_BLOCK, init_loc);
                    astvec_push(&ci->u.block.stmts, init);
                    for (size_t i = 0; i < ps->pending_decls.len; i++)
                        astvec_push(&ci->u.block.stmts,
                                     ps->pending_decls.data[i]);
                    ps->pending_decls.len = 0;
                    n->u.for_.init = ci;
                } else {
                    n->u.for_.init = init;
                }
            } else {
                AstNode *es = ast_node_new(AST_EXPR_STMT, ps_peek(ps).loc);
                es->u.expr_stmt.expr = parse_expr(ps);
                n->u.for_.init = es;
                ps_expect(ps, STOK_SEMI, "for init ';'");
            }
        } else {
            ps_advance(ps); /* skip empty ';' */
        }
        /* cond */
        if (!ps_at(ps, STOK_SEMI)) n->u.for_.cond = parse_expr(ps);
        ps_expect(ps, STOK_SEMI, "for cond ';'");
        /* post */
        if (!ps_at(ps, STOK_RPAREN)) n->u.for_.post = parse_expr(ps);
        ps_expect(ps, STOK_RPAREN, "for ')'");
        n->u.for_.body = parse_stmt(ps);
        return n;
    }

    /* return */
    if (t.kind == STOK_RETURN) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_RETURN, t.loc);
        if (!ps_at(ps, STOK_SEMI)) n->u.return_.value = parse_expr(ps);
        ps_expect(ps, STOK_SEMI, "after return");
        return n;
    }

    /* break / continue */
    if (t.kind == STOK_BREAK) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_BREAK, t.loc);
        ps_expect(ps, STOK_SEMI, "after break");
        return n;
    }
    if (t.kind == STOK_CONTINUE) {
        ps_advance(ps);
        AstNode *n = ast_node_new(AST_CONTINUE, t.loc);
        ps_expect(ps, STOK_SEMI, "after continue");
        return n;
    }

    /* goto */
    if (t.kind == STOK_GOTO) {
        ps_advance(ps);
        /* Phase S5: GCC computed goto — `goto *expr;` jumps to a label
         * whose address was taken with `&&label`.  Used by Lua's lvm.c
         * dispatch loop. */
        if (ps_at(ps, STOK_STAR)) {
            ps_advance(ps); /* consume '*' */
            AstNode *n = ast_node_new(AST_COMPUTED_GOTO, t.loc);
            n->u.computed_goto.target = parse_expr(ps);
            ps_expect(ps, STOK_SEMI, "after computed goto");
            return n;
        }
        SharpTok lbl = ps_expect(ps, STOK_IDENT, "goto label");
        AstNode *n   = ast_node_new(AST_GOTO, t.loc);
        n->u.goto_.label = cpp_xstrndup(lbl.text, lbl.len);
        ps_expect(ps, STOK_SEMI, "after goto");
        return n;
    }

    /* label: IDENT ':' */
    if (t.kind == STOK_IDENT && ps_peek2(ps).kind == STOK_COLON) {
        ps_advance(ps);  /* IDENT */
        ps_advance(ps);  /* ':' */
        AstNode *n = ast_node_new(AST_LABEL, t.loc);
        n->u.label_.label = cpp_xstrndup(t.text, t.len);
        return n;
    }

    /* defer */
    if (t.kind == STOK_DEFER) {
        ps_advance(ps);
        if (ps->in_defer) {
            ps_error(ps, t.loc, "nested 'defer' is not allowed");
        }
        AstNode *n = ast_node_new(AST_DEFER, t.loc);
        bool was = ps->in_defer;
        ps->in_defer = true;
        if (ps_at(ps, STOK_LBRACE)) {
            n->u.defer_.body = parse_block(ps);
        } else {
            AstNode *es = ast_node_new(AST_EXPR_STMT, ps_peek(ps).loc);
            es->u.expr_stmt.expr = parse_expr(ps);
            ps_expect(ps, STOK_SEMI, "after defer expr");
            n->u.defer_.body = es;
        }
        ps->in_defer = was;
        return n;
    }

    /* declaration — supports `int x = 3, y = 7;`, `int arr[3];`,
     * `static int g = 5;`, `const char *s = "abc";`, etc.  We use the
     * full DeclSpecs path so storage-class specifiers and type
     * qualifiers in block scope round-trip to the generated C. */
    if (is_type_start(ps)) {
        DeclSpecs ds = parse_decl_specifiers(ps);
        if (ds.empty || !ds.base_ty) {
            ps_error(ps, t.loc, "missing type specifier in declaration");
            ps_sync(ps);
            return NULL;
        }
        /* BUG-005 fix: local function prototype `int f(char *);`.
         * parse_init_declarator_list creates AST_VAR_DECL for everything,
         * but a function-type declarator followed by `;` (no body) is a
         * forward declaration — build AST_FUNC_DEF with no body instead. */
        {
            char *_fname = NULL;
            size_t _save = ps->pos;
            /* Clone base_ty so parse_declarator can consume it without
             * invalidating ds.base_ty for the fallback path. */
            AstNode *_base_clone = ast_clone_type(ds.base_ty);
            AstNode *_fty = parse_declarator(ps, _base_clone, &_fname);
            if (_fty && _fty->kind == AST_TYPE_FUNC && _fname &&
                ds.storage != SC_TYPEDEF &&
                ps_at(ps, STOK_SEMI)) {
                /* It's a local function prototype */
                AstNode *fn = build_func_def_from_decl(_fty, _fname, t.loc);
                fn->u.func_def.storage   = ds.storage;
                fn->u.func_def.is_inline = ds.is_inline;
                fn->u.func_def.is_constexpr = ds.is_constexpr; /* C23 */
                fn->u.func_def.gcc_attrs = ds.gcc_attrs;
                ds.gcc_attrs = NULL;
                ps_advance(ps);  /* consume ';' */
                AstNode *wrap = ast_node_new(AST_DECL_STMT, t.loc);
                wrap->u.decl_stmt.decl = fn;
                free(ds.gcc_attrs);  /* already NULL, safe */
                return wrap;
            }
            /* Not a function prototype — rewind and fall through to
             * parse_init_declarator_list which re-parses from the save. */
            ast_node_free(_fty);   /* frees _base_clone too (it's owned) */
            free(_fname);
            ps->pos = _save;
        }
        return parse_init_declarator_list(ps, &ds, /*stmt_wrap=*/true);
    }

    /* expression statement */
    {
        AstNode *es = ast_node_new(AST_EXPR_STMT, t.loc);
        es->u.expr_stmt.expr = parse_expr(ps);
        if (!ps_at(ps, STOK_SEMI)) {
            ps_error(ps, ps_peek(ps).loc, "expected ';' after expression");
            ps_sync(ps);
            return es;
        }
        ps_advance(ps);
        return es;
    }
}

/* =========================================================================
 * Entry point
 * ====================================================================== */

/* Append a unique #include string to file->u.file.user_includes. */
void file_add_include(AstNode *file, const char *inc_str) {
    /* Dedup */
    for (size_t i = 0; i < file->u.file.nuser_includes; i++)
        if (strcmp(file->u.file.user_includes[i], inc_str) == 0) return;
    if (file->u.file.nuser_includes >= file->u.file.user_includes_cap) {
        size_t nc = file->u.file.user_includes_cap
                  ? file->u.file.user_includes_cap * 2 : 8;
        file->u.file.user_includes =
            realloc(file->u.file.user_includes, nc * sizeof(char *));
        if (!file->u.file.user_includes) abort();
        file->u.file.user_includes_cap = nc;
    }
    file->u.file.user_includes[file->u.file.nuser_includes++] =
        cpp_xstrdup(inc_str);
}

static AstNode *parse_file_impl(const SharpTok *tokens, size_t ntokens,
                                const char *filename, FeDiagArr *diags,
                                const char **extra_typedefs) {
    PS ps = { tokens, ntokens, 0, filename, diags,
              /*in_defer=*/false, /*pending_close=*/0,
              /*pending_decls=*/{0}, /*typedefs=*/{0}, /*tag_names=*/{0},
              /*typedef_aliases=*/{0}, /*generic_names=*/{0},
              /*anon_struct_counter=*/0, /*struct_body_depth=*/0,
              /*last_decl_attr=*/NULL,
              /*last_params_unspecified=*/false, /*last_name_paren=*/false };

    /* Phase G: build generic_names before parsing any declarations. */
    prescan_generic_defs(&ps);

    /* Pre-populate typedef set if extra names were provided */
    if (extra_typedefs) {
        for (size_t i = 0; extra_typedefs[i]; i++)
            td_add(&ps.typedefs, extra_typedefs[i]);
    }

    AstNode *file = ast_node_new(AST_FILE, ps_peek(&ps).loc);
    file->u.file.path = cpp_xstrndup(filename, strlen(filename));

    while (!ps_at(&ps, STOK_EOF)) {
        AstNode *decl = parse_top_decl(&ps);
        /* Inner anonymous struct/union/enum bodies parsed during the
         * decl-specifier phase of `decl` were queued to pending_decls
         * by tspec_try_consume / parse_type.  Drain them BEFORE pushing
         * `decl` so the inner type's full definition precedes its
         * first use.  Without this ordering an outer struct body that
         * embeds an anonymous member by value (e.g. `union U { struct
         * { int x, y; } pt; };`) would reference the synthetic tag
         * before its `{...}` block, and the C compiler would reject
         * the field as having incomplete type.
         *
         * Multi-declarator drains (`int x = 1, y = 2;`) are unaffected
         * — those pending entries are siblings of `decl`, so coming
         * either before or after is semantically equivalent. */
        /* Drain pending_decls: struct/enum definitions first (must precede
         * their use), then the primary declarator, then continuation vars. */
        for (size_t i = 0; i < ps.pending_decls.len; i++) {
            AstNode *pd = ps.pending_decls.data[i];
            if (pd && (pd->kind == AST_STRUCT_DEF || pd->kind == AST_ENUM_DEF)) {
                astvec_push(&file->u.file.decls, pd);
                if (pd->u.struct_def.name) {
                    td_add(&ps.typedefs, pd->u.struct_def.name);
                    td_add(&ps.tag_names, pd->u.struct_def.name);
                }
            }
        }
        if (decl) {
            astvec_push(&file->u.file.decls, decl);
            if (decl->kind == AST_STRUCT_DEF && decl->u.struct_def.name) {
                td_add(&ps.typedefs, decl->u.struct_def.name);
                td_add(&ps.tag_names, decl->u.struct_def.name);
            }
        }
        /* Continuation declarators (is_comma_cont siblings) go after primary */
        for (size_t i = 0; i < ps.pending_decls.len; i++) {
            AstNode *pd = ps.pending_decls.data[i];
            if (pd && pd->kind != AST_STRUCT_DEF && pd->kind != AST_ENUM_DEF)
                astvec_push(&file->u.file.decls, pd);
        }
        ps.pending_decls.len = 0;
    }
    astvec_free(&ps.pending_decls);
    td_free(&ps.typedefs);
    td_free(&ps.tag_names);
    td_free(&ps.typedef_aliases);
    td_free(&ps.generic_names);   /* Phase G */
    return file;
}

AstNode *parse_file(const SharpTok *tokens, size_t ntokens,
                    const char *filename, FeDiagArr *diags) {
    return parse_file_impl(tokens, ntokens, filename, diags, NULL);
}

AstNode *parse_file_with_typedefs(const SharpTok *tokens, size_t ntokens,
                                   const char *filename, FeDiagArr *diags,
                                   const char **extra_typedefs) {
    return parse_file_impl(tokens, ntokens, filename, diags, extra_typedefs);
}

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
 * parse_top_decl and parse_var_decl_list).  We do not pop on block
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

typedef struct {
    const SharpTok *toks;
    size_t          ntoks;
    size_t          pos;           /* index of current token               */
    const char     *filename;
    FeDiagArr      *diags;
    bool            in_defer;      /* true while inside a defer body       */
    int             pending_close; /* leftover '>' from a split '>>'       */
    /* Multi-variable declarations: parse_var_decl_list pushes the *first*
     * decl as the function's "return value" and queues the rest here.
     * parse_file / parse_block / for-init drain this before re-entering
     * the recogniser.  This lets multi-decls work without changing every
     * call site's return type. */
    AstVec          pending_decls;
    /* Typedef-name set, populated as we parse typedef declarations.
     * Used by dd_paren_is_subdeclarator to disambiguate `( IDENT )`. */
    TdSet           typedefs;
    /* Counter for anonymous struct/union synthetic names.  Used when a
     * `typedef struct { ... } Name;` form is encountered: the inner
     * struct has no source-level tag, so we synthesise one from this
     * counter and emit it as a normal AST_STRUCT_DEF.  The C compiler
     * sees a tagged struct in the generated output, semantically
     * identical to the original anonymous form. */
    unsigned        anon_struct_counter;
} PS;

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

/* Sync past the next ';' or '}' for error recovery. */
static void ps_sync(PS *ps) {
    while (!ps_at(ps, STOK_EOF)) {
        SharpTokKind k = ps_peek(ps).kind;
        if (k == STOK_SEMI) { ps_advance(ps); return; }
        if (k == STOK_RBRACE) return;   /* don't consume the '}' */
        ps_advance(ps);
    }
}

/* Make a node at the location of the current token. */
static AstNode *ps_node(PS *ps, AstKind k) {
    return ast_node_new(k, ps_peek(ps).loc);
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
    case STOK_RESTRICT:
    /* GCC `__attribute__((…))` may legally lead a declaration:
     *   `__attribute__((unused)) static int counter;`
     * Treat it as a decl-specifier opener so parse_top_decl /
     * parse_stmt route into the declarator path. */
    case STOK_ATTRIBUTE:
        return true;
    case STOK_CONST: case STOK_STRUCT: case STOK_UNION: case STOK_ENUM:
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
        if (k2 == STOK_RPAREN) {
            SharpTok t = ps_peek(ps);
            return td_has_n(&ps->typedefs, t.text, t.len);
        }
        /* Phase R3: IDENT '(' — only a type-start when the IDENT is a
         * typedef-name and the parenthesis opens a parenthesised
         * declarator (e.g. `sqlite3_syscall_ptr (*x)(args);` — typedef
         * as the return type of a function-pointer field).  Without
         * the typedef-set probe we'd consume calls like `f(x)` as
         * declarations.  sqlite3.c hits this dozens of times. */
        if (k2 == STOK_LPAREN) {
            SharpTok t = ps_peek(ps);
            return td_has_n(&ps->typedefs, t.text, t.len);
        }
        /* Phase R3: IDENT followed by a type-qualifier (const, volatile,
         * restrict) — this is the postfix-qualifier form `Mt const *p`,
         * equivalent to `const Mt *p` per ISO C99 §6.7.3.  Same gate
         * as IDENT '(' / IDENT ')': only a type-start when IDENT is a
         * known typedef-name.  sqlite3.c has many `T const *` fields
         * and casts (e.g. `sqlite3_io_methods const *pMethod`). */
        if (k2 == STOK_CONST || k2 == STOK_VOLATILE ||
            k2 == STOK_RESTRICT) {
            SharpTok t = ps_peek(ps);
            return td_has_n(&ps->typedefs, t.text, t.len);
        }
        if (k2 == STOK_LT) {
            SharpTokKind k3 = ps_peek3(ps).kind;
            switch (k3) {
            case STOK_CONST: case STOK_STRUCT:
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
/* Phase R3: needed by parse_type's abstract function-pointer
 * declarator handling (defined later in the file). */
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
static AstNode *parse_var_decl_list(PS *ps, AstNode *base_ty, bool stmt_wrap);
static AstNode *parse_array_suffix(PS *ps, AstNode *inner_ty);
static AstNode *parse_init_list(PS *ps);

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
 * Sharp's design (per S1 PHASE_S1_SUMMARY) treats GCC attributes as
 * ignorable decoration: they affect codegen on the C compiler side,
 * never sema or transpilation.  We accept them at every grammar
 * position they may appear at and silently discard.  asm-rename is
 * similarly discarded — sharp's transpiled output goes through cc
 * which will re-derive the real linker name from the source headers.
 *
 * Attribute syntax: `__attribute__` `(` `(` BALANCED `)` `)`
 * Asm-name syntax:  `__asm__` `(` BALANCED `)`
 * Both are eaten in any order, repeatedly, at the call site.
 *
 * Returns true if at least one specifier was eaten. */
static bool eat_attribute_specifiers(PS *ps) {
    bool ate_any = false;
    for (;;) {
        if (ps_at(ps, STOK_ATTRIBUTE)) {
            ps_advance(ps);  /* eat __attribute__ */
            /* Expect double-open-paren.  If malformed, advance through what
             * we have and return — the surrounding parser will diagnose any
             * follow-on error. */
            if (!ps_match(ps, STOK_LPAREN)) { ate_any = true; continue; }
            if (!ps_match(ps, STOK_LPAREN)) { ate_any = true; continue; }
            /* Skip tokens until balanced double-close-paren. */
            int depth = 0;
            while (!ps_at(ps, STOK_EOF)) {
                SharpTokKind k = ps_peek(ps).kind;
                if (k == STOK_LPAREN) {
                    depth++;
                    ps_advance(ps);
                } else if (k == STOK_RPAREN) {
                    if (depth == 0) {
                        ps_advance(ps);  /* inner ')' */
                        ps_match(ps, STOK_RPAREN);  /* outer ')' */
                        break;
                    }
                    depth--;
                    ps_advance(ps);
                } else {
                    ps_advance(ps);
                }
            }
            ate_any = true;
            continue;
        }
        if (ps_at(ps, STOK_ASM)) {
            /* `__asm__(<balanced>)` — symbol-rename declarator suffix. */
            ps_advance(ps);
            if (!ps_match(ps, STOK_LPAREN)) { ate_any = true; continue; }
            int depth = 0;
            while (!ps_at(ps, STOK_EOF)) {
                SharpTokKind k = ps_peek(ps).kind;
                if (k == STOK_LPAREN) {
                    depth++;
                    ps_advance(ps);
                } else if (k == STOK_RPAREN) {
                    if (depth == 0) { ps_advance(ps); break; }
                    depth--;
                    ps_advance(ps);
                } else {
                    ps_advance(ps);
                }
            }
            ate_any = true;
            continue;
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
        bool ok_start = (k == STOK_CONST  || k == STOK_STRUCT ||
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
    for (size_t i = 0; i < args->len; i++) ast_node_free(args->data[i]);
    astvec_free(args);
    *args = (AstVec){0};
    /* Roll back any diagnostics emitted during the failed trial parse. */
    if (ps->diags && ps->diags->len > save_diag_len) {
        for (size_t i = save_diag_len; i < ps->diags->len; i++)
            free(ps->diags->data[i].msg);
        ps->diags->len = save_diag_len;
    }
    return false;
}

static AstNode *parse_type(PS *ps) {
    SharpTok t = ps_peek(ps);
    AstNode *base = NULL;

    /* const T — const wraps only the base type; pointer suffixes apply to
     * the const-qualified result.  Examples:
     *   const char*   → PTR(CONST(char))    — pointer to const char
     *   char* const   → CONST(PTR(char))    — const pointer to char
     * We parse: CONST( base-type ), then handle '*' and 'const' suffixes. */
    if (t.kind == STOK_CONST) {
        ps_advance(ps);
        AstNode *inner = parse_type_unqual(ps);
        AstNode *cn = ast_node_new(AST_TYPE_CONST, t.loc);
        cn->u.type_const.base = inner;
        base = cn;
        goto apply_suffix;
    }

    /* volatile T — symmetric with const.  Sharp transpiles volatile
     * straight through to the generated C; semantically it has no effect
     * on Sharp's interned Type* but is preserved for correctness. */
    if (t.kind == STOK_VOLATILE) {
        ps_advance(ps);
        AstNode *inner = parse_type_unqual(ps);
        AstNode *vn = ast_node_new(AST_TYPE_VOLATILE, t.loc);
        vn->u.type_volatile.base = inner;
        base = vn;
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
    if (t.kind == STOK_STRUCT || t.kind == STOK_UNION) {
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
        base = n;
        goto try_generic;
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
        if (t.kind != STOK_IDENT) {
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
    /* try to attach <T1, T2, ...> */
    if (ps_at(ps, STOK_LT)) {
        AstVec args = {0};
        if (parse_generic_args(ps, &args)) {
            AstNode *gen = ast_node_new(AST_TYPE_GENERIC, base->loc);
            gen->u.type_generic.name = cpp_xstrdup(base->u.type_name.name);
            gen->u.type_generic.args = args;
            ast_node_free(base);
            base = gen;
        }
    }

apply_suffix:
    /* Phase R3: postfix qualifiers on the base type — `T const`, `T
     * volatile` are equivalent to `const T` / `volatile T` per ISO C99
     * §6.7.3.  parse_type used to accept only the prefix form, so
     * casts like `(unsigned char const *)p` failed (sqlite3.c uses
     * this idiom in its UTF-8 helpers).  Eat any trailing const /
     * volatile here, then proceed to the pointer / array suffix loop. */
    for (;;) {
        if (ps_at(ps, STOK_CONST)) {
            CppLoc cl = ps_advance(ps).loc;
            AstNode *cn = ast_node_new(AST_TYPE_CONST, cl);
            cn->u.type_const.base = base;
            base = cn;
        } else if (ps_at(ps, STOK_VOLATILE)) {
            CppLoc vl = ps_advance(ps).loc;
            AstNode *vn = ast_node_new(AST_TYPE_VOLATILE, vl);
            vn->u.type_volatile.base = base;
            base = vn;
        } else if (ps_at(ps, STOK_RESTRICT)) {
            ps_advance(ps);  /* discard */
        } else {
            break;
        }
    }
    /* pointer suffix: T* or T**  with optional pointer-qualifiers after */
    while (ps_at(ps, STOK_STAR)) {
        ps_advance(ps);
        AstNode *ptr = ast_node_new(AST_TYPE_PTR, base->loc);
        ptr->u.type_ptr.base = base;
        base = ptr;
        /* Pointer-side qualifiers: T * const, T * volatile, T * restrict */
        for (;;) {
            if (ps_at(ps, STOK_CONST)) {
                ps_advance(ps);
                AstNode *cn = ast_node_new(AST_TYPE_CONST, base->loc);
                cn->u.type_const.base = base;
                base = cn;
            } else if (ps_at(ps, STOK_VOLATILE)) {
                ps_advance(ps);
                AstNode *vn = ast_node_new(AST_TYPE_VOLATILE, base->loc);
                vn->u.type_volatile.base = base;
                base = vn;
            } else if (ps_at(ps, STOK_RESTRICT)) {
                ps_advance(ps);  /* discard */
            } else {
                break;
            }
        }
    }
    /* Phase R3: abstract function-pointer declarator in a type-name
     * position — `RetT (*)(args)` or `RetT (* const)(args)`.  This is
     * the cast form used to call a function-pointer field through a
     * void-ish base, e.g. sqlite3.c's syscall-table dispatch:
     *
     *   ((int (*)(int, uid_t, gid_t)) aSyscall[20].pCurrent)(fd, uid, gid)
     *
     * After parsing the return type and any prefix `*`s, we may see
     * `(`.  Look ahead for the `(` `*+` (qualifiers)? `)` `(` shape;
     * if it matches, consume the inner `*`s, the closing `)`, the
     * function parameter list, and wrap the resulting TYPE_FUNC
     * in `nstars` TYPE_PTR layers.
     *
     * We detect by peeking — `(` followed by `*` is unambiguous in
     * this grammar position (no expression context: parse_type only
     * reaches this point when the caller already committed to a
     * type-expression). */
    if (ps_at(ps, STOK_LPAREN) && ps_peek2(ps).kind == STOK_STAR) {
        CppLoc lp = ps_advance(ps).loc;  /* eat '(' */
        int nstars = 0;
        while (ps_at(ps, STOK_STAR)) {
            ps_advance(ps);
            nstars++;
            /* Eat any pointer-side qualifiers — these decorate the
             * pointer itself; cg layers them on the outermost wrap. */
            while (ps_at(ps, STOK_CONST) || ps_at(ps, STOK_VOLATILE) ||
                   ps_at(ps, STOK_RESTRICT)) {
                ps_advance(ps);  /* discard for abstract-decl shape */
            }
        }
        ps_expect(ps, STOK_RPAREN, "abstract declarator ')'");
        /* Function suffix: `(args)` for the function the pointer
         * points to.  Without parameters, `()` denotes a function
         * taking unspecified args; `(void)` an explicit-empty list. */
        if (ps_at(ps, STOK_LPAREN)) {
            ps_advance(ps);
            AstNode *fn = ast_node_new(AST_TYPE_FUNC, lp);
            fn->u.type_func.ret = base;
            parse_param_list_inner(ps, &fn->u.type_func.params);
            ps_expect(ps, STOK_RPAREN, "function-declarator ')'");
            base = fn;
        }
        /* Wrap with `nstars` pointer layers.  Common form is exactly
         * one star (`(*)`), giving pointer-to-function. */
        for (int i = 0; i < nstars; i++) {
            AstNode *p = ast_node_new(AST_TYPE_PTR, lp);
            p->u.type_ptr.base = base;
            base = p;
        }
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
    } else if (t.kind == STOK_IDENT || t.kind == STOK_INT || t.kind == STOK_CHAR ||
               t.kind == STOK_LONG  || t.kind == STOK_SHORT || t.kind == STOK_FLOAT ||
               t.kind == STOK_DOUBLE|| t.kind == STOK_SIGNED || t.kind == STOK_UNSIGNED ||
               t.kind == STOK__BOOL) {
        ps_advance(ps);
        char namebuf[64];
        size_t off = 0;
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
        }
        base = ast_node_new(AST_TYPE_NAME, t.loc);
        base->u.type_name.name = cpp_xstrndup(namebuf, off);
    } else {
        ps_error(ps, t.loc, "expected base type, got '%.*s'", (int)t.len, t.text);
        base = ast_node_new(AST_TYPE_NAME, t.loc);
        base->u.type_name.name = cpp_xstrndup("?", 1);
        return base;
    }
    if (ps_at(ps, STOK_LT)) {
        AstVec args = {0};
        if (parse_generic_args(ps, &args)) {
            AstNode *gen = ast_node_new(AST_TYPE_GENERIC, base->loc);
            gen->u.type_generic.name = cpp_xstrdup(base->u.type_name.name);
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
    bool         is_inline;      /* C99 function-specifier                  */
    bool         is_const;       /* type qualifier on the base specifier    */
    bool         is_volatile;
    /* Phase R2: C11 _Thread_local — orthogonal to storage class (can
     * combine with static or extern per ISO 6.7.1).  Recorded as its
     * own flag rather than a SC_THREAD_LOCAL because the existing
     * StorageClass enum is single-valued.  stb_image: `static
     * _Thread_local const char *xx;`. */
    bool         is_thread_local;
    AstNode     *base_ty;        /* raw type-specifier AST                  */
    CppLoc       loc;            /* location of the first specifier token   */
    bool         empty;          /* true iff no decl-spec tokens consumed   */
} DeclSpecs;

/* Type-specifier accumulator.  Combines repeated tokens into a single
 * canonical type name, e.g. "unsigned" + "long" + "long" -> "unsigned long
 * long".  All counts default to zero. */
typedef struct {
    int      saw_void, saw_bool, saw_char, saw_short, saw_int;
    int      saw_long_count;            /* 0, 1, or 2 (`long long`)         */
    int      saw_float, saw_double;
    int      saw_signed, saw_unsigned;
    AstNode *user_ty;                   /* struct tag / generic / typedef   */
} TSpec;

static bool tspec_has_anything(const TSpec *t) {
    return t->saw_void || t->saw_bool || t->saw_char || t->saw_short ||
           t->saw_int  || t->saw_long_count > 0 ||
           t->saw_float|| t->saw_double || t->saw_signed || t->saw_unsigned ||
           t->user_ty;
}

/* Resolve TSpec into a single AstNode type tree.  The resulting node is
 * always one of: AST_TYPE_NAME (for primitives), AST_TYPE_VOID, or the
 * user_ty node passed through unchanged. */
static AstNode *tspec_resolve(PS *ps, TSpec *ts, CppLoc loc) {
    if (ts->user_ty) {
        /* User-defined type (struct tag / IDENT / generic) — primitive
         * tokens cannot legally combine with it.  We don't enforce that
         * here; the C compiler will reject illegal mixes. */
        AstNode *out = ts->user_ty;
        ts->user_ty = NULL;
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
    else if (ts->saw_int && !ts->saw_signed && !ts->saw_unsigned) PUSH("int");

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
    return n;
    #undef PUSH
}

/* Try to extend `ts` with the current token.  Returns true and advances
 * `ps` on success; returns false and leaves `ps` unchanged otherwise. */
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
    case STOK_SIGNED:  ts->saw_signed++;   ps_advance(ps); return true;
    case STOK_UNSIGNED:ts->saw_unsigned++; ps_advance(ps); return true;

    case STOK_STRUCT:
    case STOK_UNION:
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
        if ((kw_kind == STOK_STRUCT || kw_kind == STOK_UNION) &&
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
         * The struct body must be parsed inline; we route to
         * parse_struct_def, queue the resulting AST_STRUCT_DEF to
         * pending_decls so it surfaces at file scope, and resolve the
         * type-specifier as a TYPE_NAME referring to the tag. */
        if ((kw_kind == STOK_STRUCT || kw_kind == STOK_UNION) &&
            ps_peek2(ps).kind == STOK_IDENT) {
            SharpTokKind k3 = ps_peek3(ps).kind;
            if (k3 == STOK_LBRACE) {
                AstNode *sd = parse_struct_def(ps);
                ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
                ts->user_ty->u.type_name.name = cpp_xstrdup(sd->u.struct_def.name);
                astvec_push(&ps->pending_decls, sd);
                return true;
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
            /* Sharp's Type* layer collapses enums to int, so the
             * decl-spec resolves as `int`.  cg pass-1 emits the enum
             * body separately from pending_decls. */
            ts->saw_int++;
            return true;
        }

        ps_advance(ps);
        if (!ps_at(ps, STOK_IDENT)) {
            ps_error(ps, ps_peek(ps).loc,
                "expected %s tag name",
                kw_kind == STOK_STRUCT ? "struct" :
                kw_kind == STOK_UNION  ? "union"  : "enum");
            ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
            ts->user_ty->u.type_name.name = cpp_xstrdup("?");
            return true;
        }
        SharpTok nm = ps_advance(ps);
        ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
        ts->user_ty->u.type_name.name = cpp_xstrndup(nm.text, nm.len);
        /* optional generic args after a struct tag (Sharp generic types) */
        if (kw_kind == STOK_STRUCT && ps_at(ps, STOK_LT)) {
            AstVec args = {0};
            if (parse_generic_args(ps, &args)) {
                AstNode *gen = ast_node_new(AST_TYPE_GENERIC, t.loc);
                gen->u.type_generic.name = cpp_xstrdup(ts->user_ty->u.type_name.name);
                gen->u.type_generic.args = args;
                ast_node_free(ts->user_ty);
                ts->user_ty = gen;
            }
        }
        return true;
    }

    case STOK_IDENT: {
        /* User-named type (IDENT possibly followed by `<...>`).  Only
         * accepted when it's the FIRST type-specifier seen — otherwise
         * it must be the start of a declarator (the variable name). */
        if (tspec_has_anything(ts)) return false;
        /* Must look-ahead to confirm this IDENT is a type, not a
         * variable name in a context where the type is implicit-int.
         * Implicit-int doesn't appear in modern code, so we trust
         * is_type_start()'s caller-level decision: by the time we get
         * here, we've already classified this position as a declaration. */
        ps_advance(ps);
        ts->user_ty = ast_node_new(AST_TYPE_NAME, t.loc);
        ts->user_ty->u.type_name.name = cpp_xstrndup(t.text, t.len);
        if (ps_at(ps, STOK_LT)) {
            AstVec args = {0};
            if (parse_generic_args(ps, &args)) {
                AstNode *gen = ast_node_new(AST_TYPE_GENERIC, t.loc);
                gen->u.type_generic.name = cpp_xstrdup(ts->user_ty->u.type_name.name);
                gen->u.type_generic.args = args;
                ast_node_free(ts->user_ty);
                ts->user_ty = gen;
            }
        }
        return true;
    }

    default:
        return false;
    }
}

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
            ps_advance(ps);
            continue;
        }

        /* Phase R2: C11 _Thread_local storage-class.  Orthogonal to the
         * SC_* enum because per ISO 6.7.1 it may combine with `static`
         * or `extern`.  `static _Thread_local const char *p;` (from
         * stb_image) was failing with "missing type specifier". */
        if (t.kind == STOK__THREAD_LOCAL) {
            ds.is_thread_local = true;
            ds.empty           = false;
            ps_advance(ps);
            continue;
        }

        /* Type qualifiers.  `restrict` is a no-op for our type model — we
         * accept it and discard.  `const` and `volatile` are recorded; the
         * caller (declarator loop) wraps the type tree with TYPE_CONST /
         * TYPE_VOLATILE at the end. */
        if (t.kind == STOK_CONST)    { ds.is_const = true;    ds.empty = false; ps_advance(ps); continue; }
        if (t.kind == STOK_VOLATILE) { ds.is_volatile = true; ds.empty = false; ps_advance(ps); continue; }
        if (t.kind == STOK_RESTRICT) { ds.empty = false; ps_advance(ps); continue; }

        /* GCC `__attribute__((…))` — accepted in any decl-specifier
         * position (before, between, or after the type spec).  Eaten
         * silently; does not toggle ds.empty since attributes never
         * stand alone as a declaration. */
        if (t.kind == STOK_ATTRIBUTE) {
            eat_attribute_specifiers(ps);
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
        if (tspec_try_consume(ps, &ts)) {
            ds.empty = false;
            continue;
        }

        /* Anything else ends the decl-specifier sequence. */
        break;
    }

    if (ds.empty) {
        return ds;
    }

    AstNode *base = tspec_resolve(ps, &ts, ds.loc);
    if (ds.is_const) {
        AstNode *c = ast_node_new(AST_TYPE_CONST, ds.loc);
        c->u.type_const.base = base;
        base = c;
    }
    if (ds.is_volatile) {
        AstNode *v = ast_node_new(AST_TYPE_VOLATILE, ds.loc);
        v->u.type_volatile.base = base;
        base = v;
    }
    ds.base_ty = base;
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

/* Forward decl — used by direct-declarator's function-suffix path. */
static void parse_param_list_inner(PS *ps, AstVec *params);

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
    while (ps_at(ps, STOK_STAR)) {
        CppLoc star_loc = ps_advance(ps).loc;  /* eat '*' */
        AstNode *ptr = ast_node_new(AST_TYPE_PTR, star_loc);
        ptr->u.type_ptr.base = base;
        base = ptr;
        /* qualifiers attached to the pointer itself: T * const, T * volatile */
        for (;;) {
            if (ps_at(ps, STOK_CONST)) {
                CppLoc q = ps_advance(ps).loc;
                AstNode *c = ast_node_new(AST_TYPE_CONST, q);
                c->u.type_const.base = base;
                base = c;
            } else if (ps_at(ps, STOK_VOLATILE)) {
                CppLoc q = ps_advance(ps).loc;
                AstNode *v = ast_node_new(AST_TYPE_VOLATILE, q);
                v->u.type_volatile.base = base;
                base = v;
            } else if (ps_at(ps, STOK_RESTRICT)) {
                ps_advance(ps);  /* discard */
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
        inner_root  = parse_declarator(ps, placeholder, out_name);
        ps_expect(ps, STOK_RPAREN, "closing ')' of grouped declarator");
    } else if (ps_at(ps, STOK_IDENT)) {
        SharpTok nm = ps_advance(ps);
        *out_name = cpp_xstrndup(nm.text, nm.len);
    } else {
        /* abstract declarator: no name, no inner */
        *out_name = NULL;
    }

    /* Stage 2: collect array / function suffixes, wrapping `base`.
     * `inner` always tracks the slot where the next deeper element type
     * lives, so consecutive suffixes nest in left-to-right order. */
    AstNode  *tree  = base;
    AstNode **inner = &tree;
    for (;;) {
        if (ps_at(ps, STOK_LBRACKET)) {
            CppLoc lb = ps_advance(ps).loc;
            AstNode *sz = NULL;
            if (!ps_at(ps, STOK_RBRACKET)) {
                sz = parse_expr_prec(ps, 2);
            }
            ps_expect(ps, STOK_RBRACKET, "array suffix ']'");
            AstNode *arr = ast_node_new(AST_TYPE_ARRAY, lb);
            arr->u.type_array.base = *inner;
            arr->u.type_array.size = sz;
            *inner = arr;
            inner  = &arr->u.type_array.base;
        } else if (ps_at(ps, STOK_LPAREN)) {
            CppLoc lp = ps_advance(ps).loc;
            AstNode *fn = ast_node_new(AST_TYPE_FUNC, lp);
            fn->u.type_func.ret = *inner;
            parse_param_list_inner(ps, &fn->u.type_func.params);
            ps_expect(ps, STOK_RPAREN, "function-declarator ')'");
            *inner = fn;
            inner  = &fn->u.type_func.ret;
        } else {
            break;
        }
    }

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
     * treat it the same as (void) for transpilation). */
    if (ps_at(ps, STOK_RPAREN)) return;

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

        AstNode *pd = ast_node_new(AST_PARAM_DECL, ds.loc);
        pd->u.param_decl.type = pty;
        pd->u.param_decl.name = pname;  /* may be NULL for prototype */
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


/* =========================================================================
 * Function definition body (after name + generic params + param list)
 * ====================================================================== */
static AstNode *finish_func(PS *ps, AstNode *ret_type,
                             SharpTok name_tok, bool is_operator) {
    AstNode *fn = ast_node_new(AST_FUNC_DEF, name_tok.loc);
    fn->u.func_def.name        = cpp_xstrndup(name_tok.text, name_tok.len);
    fn->u.func_def.ret_type    = ret_type;
    fn->u.func_def.is_operator = is_operator;
    fn->u.func_def.is_static   = false;

    /* optional <T, U> */
    parse_generic_params(ps, &fn->u.func_def.generic_params);

    /* (params) */
    parse_param_list(ps, &fn->u.func_def.params);

    /* optional 'const' suffix (const method) */
    if (ps_match(ps, STOK_CONST)) fn->u.func_def.is_const_method = true;

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
        if (ps_match(ps, STOK_EQ)) {
            /* prec-2 to skip the comma operator: `A = 1, B = 2`. */
            en->u.enumerator.value = parse_expr_prec(ps, 2);
        }
        astvec_push(&ed->u.enum_def.items, en);
        if (!ps_match(ps, STOK_COMMA)) break;
        /* C allows a trailing comma (C99); accept it gracefully. */
        if (ps_at(ps, STOK_RBRACE)) break;
    }
    ps_expect(ps, STOK_RBRACE, "enum body '}'");
    /* Phase R2: do NOT consume the trailing `;` here.  The various
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
    if (is_union)  ps_advance(ps);
    else           ps_expect(ps, STOK_STRUCT, "struct keyword");

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

    parse_generic_params(ps, &sd->u.struct_def.generic_params);

    /* optional forward decl (just 'struct Name;') */
    if (ps_match(ps, STOK_SEMI)) return sd;

    ps_expect(ps, STOK_LBRACE, "struct body '{'");

    bool saw_method = false;
    while (!ps_at(ps, STOK_RBRACE) && !ps_at(ps, STOK_EOF)) {
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
        /* Flatten qualifiers onto base_ty for operator / method return type */
        AstNode *base_ty = fds.base_ty;
        /* Wrap const/volatile qualifiers for the field — same as the
         * top-level declarator path. */
        AstNode *ty_for_method = base_ty;  /* operators/methods use raw */

        /* RetType operator+(…) — operator definition inside struct */
        if (ps_at(ps, STOK_OPERATOR)) {
            ps_advance(ps);
            SharpTok op_tok = ps_advance(ps);  /* '+', '-', '[]', etc. */
            if (op_tok.kind == STOK_LBRACKET) ps_match(ps, STOK_RBRACKET);
            saw_method = true;
            char buf[64];
            if (op_tok.kind == STOK_LBRACKET)
                snprintf(buf, sizeof buf, "operator[]");
            else
                snprintf(buf, sizeof buf, "operator%.*s", (int)op_tok.len, op_tok.text);
            AstNode *fn = finish_func(ps, ty_for_method, op_tok, /*is_operator=*/true);
            free(fn->u.func_def.name);
            fn->u.func_def.name = cpp_xstrndup(buf, strlen(buf));
            astvec_push(&sd->u.struct_def.methods, fn);
            continue;
        }

        /* Field path: parse a declarator to handle the simple
         *   `int x;`             (IDENT after the type)
         * as well as the function-pointer-as-field idiom used by libc:
         *   `void (*_function)(__sigval_t);`
         * The declarator returns the wrapped type and the field name.
         *
         * Critically: parse_declarator is given `base_ty` (the raw base
         * from parse_decl_specifiers, WITHOUT pointer/array suffixes).
         * parse_declarator itself applies `*` and `[]` from the source.
         * The comma-list loop clones `base_ty` — NOT the full declarator
         * result — so each declarator gets an independent copy of the
         * base to wrap. This correctly handles `struct CI *prev, *next`
         * where the base is `CI` and each declarator contributes its
         * own `*`.  Previously parse_type was used which applied `*`
         * early; cloning the already-pointer-wrapped type then adding
         * another `*` in parse_declarator produced `CI**` for `next`.
         *
         * Methods (Sharp extension): `RetType name(params) { body }` —
         * detected by an `(` immediately after the IDENT *without* a
         * preceding `*` or `(` in the declarator chain.  We probe by
         * peeking: if the first declarator token is plain IDENT and the
         * one after is `(`, route to the method path; otherwise use
         * the full declarator parser. */
        bool is_method =
            ps_at(ps, STOK_IDENT) &&
            ps_peek2(ps).kind == STOK_LPAREN;
        SharpTok name_tok = {0};
        AstNode *field_ty = base_ty;
        char    *field_name = NULL;

        if (is_method) {
            /* Existing method-detection path: peek IDENT + '(' decides
             * a method definition.  Keep base_ty as the return type;
             * the IDENT becomes the method name. */
            name_tok = ps_advance(ps);
            field_name = cpp_xstrndup(name_tok.text, name_tok.len);
        } else {
            /* General declarator: handles `*name`, `(*name)(args)`,
             * `name[N]`, `name[N][M]`, and combinations.  We pass
             * base_ty; parse_declarator wraps it appropriately.
             * The name comes back through *out. */
            field_ty = parse_declarator(ps, base_ty, &field_name);
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
                    /* Phase R3: ISO C99 anonymous bit-field — `int :32;`
                     * or `int :0;` for explicit alignment / padding.
                     * Per 6.7.2.1¶12 the field has no name and the
                     * width determines its storage occupancy.  Emit
                     * with empty name; cg_decl writes only the type
                     * when name is empty, and the bit_width arm adds
                     * the `: N` suffix.  sqlite3.c uses this for
                     * reserved struct padding. */
                    field_name = cpp_xstrdup("");
                } else {
                    ps_error(ps, ps_peek(ps).loc,
                        "field declaration is missing a name");
                    field_name = cpp_xstrdup("?");
                }
            }
            name_tok.loc = ps_peek(ps).loc;
        }

        /* S2: field array suffix(es) — `int arr[4]; char buf[16][32];`
         * Already consumed by parse_declarator above for the field
         * path; only the method path may have trailing arrays after
         * the IDENT.  Method path has no array, so this is a no-op
         * unless we're in the legacy method shape. */
        if (is_method && ps_at(ps, STOK_LBRACKET)) {
            field_ty = parse_array_suffix(ps, field_ty);
        }

        /* S5: GCC `__attribute__((aligned(...))) ` etc. after a field
         * declarator.  This is the form gcc's <stddef.h> uses for
         * max_align_t.  Eat silently — alignment is the C compiler's
         * concern, not ours. */
        eat_attribute_specifiers(ps);

        /* S4: bit-field width — `unsigned int a : 3;`.  The width is a
         * constant expression; we accept any expr-prec-2 (skips comma)
         * and let the C compiler validate constraints (must be a constant
         * integer, must not exceed the underlying type's width, etc.). */
        AstNode *bw = NULL;
        if (!is_method && ps_at(ps, STOK_COLON)) {
            ps_advance(ps);
            bw = parse_expr_prec(ps, 2);
        }

        if (!is_method && (ps_at(ps, STOK_SEMI) || ps_at(ps, STOK_COMMA))) {
            /* field declaration; supports comma list `int x, y, z;` */
            if (saw_method) {
                ps_error(ps, name_tok.loc,
                    "field '%s' declared after method — fields must come first",
                    field_name);
            }
            AstNode *fd = ast_node_new(AST_FIELD_DECL, name_tok.loc);
            fd->u.field_decl.type      = field_ty;
            fd->u.field_decl.name      = field_name;
            fd->u.field_decl.bit_width = bw;
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
                eat_attribute_specifiers(ps);
                if (!next_name) {
                    ps_error(ps, next_loc, "field declaration is missing a name");
                    next_name = cpp_xstrdup("?");
                }
                AstNode *fd2 = ast_node_new(AST_FIELD_DECL, next_loc);
                fd2->u.field_decl.type      = next_ty;
                fd2->u.field_decl.name      = next_name;
                fd2->u.field_decl.bit_width = NULL;
                astvec_push(&sd->u.struct_def.fields, fd2);
            }
            ps_expect(ps, STOK_SEMI, "after field declaration");
        } else if (is_method) {
            /* method / associated function */
            saw_method = true;
            AstNode *fn = finish_func(ps, base_ty, name_tok, false);
            free(field_name);  /* finish_func sets its own name */
            astvec_push(&sd->u.struct_def.methods, fn);
        } else {
            ps_error(ps, ps_peek(ps).loc,
                "expected ';' after field declaration");
            ps_advance(ps);
            ast_node_free(field_ty);
            free(field_name);
        }
        continue;
    }
    ps_expect(ps, STOK_RBRACE, "struct closing '}'");
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
    /* Collect sizes in source order: a[3][4] gives sizes = [3, 4] */
    AstNode *sizes[16];
    CppLoc   locs[16];
    size_t   nsizes = 0;
    while (ps_at(ps, STOK_LBRACKET)) {
        if (nsizes >= 16) {
            ps_error(ps, ps_peek(ps).loc, "too many array dimensions (max 16)");
            break;
        }
        SharpTok lb = ps_advance(ps);  /* '[' */
        AstNode *sz = NULL;
        if (!ps_at(ps, STOK_RBRACKET)) {
            /* Parse at precedence 2 — disallow comma operator inside [...]. */
            sz = parse_expr_prec(ps, 2);
        }
        ps_expect(ps, STOK_RBRACKET, "array suffix ']'");
        sizes[nsizes] = sz;
        locs[nsizes]  = lb.loc;
        nsizes++;
    }
    /* Apply suffixes left-to-right, outermost first.
     * After loop, `t` represents the innermost element type.
     * For a[3][4]: result should be array(3, array(4, T)).
     * Wrap in *reverse* order so that the leftmost suffix ends up outermost. */
    AstNode *t = inner_ty;
    for (size_t i = nsizes; i > 0; i--) {
        AstNode *arr = ast_node_new(AST_TYPE_ARRAY, locs[i-1]);
        arr->u.type_array.base = t;
        arr->u.type_array.size = sizes[i-1];
        t = arr;
    }
    return t;
}

/* =========================================================================
 * S1: parse_init_declarator_list
 *
 * Replaces the old type-prefix `parse_var_decl_list`.  The caller has
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
         * `int x __attribute__((unused)) = 0;`).  Eaten silently. */
        eat_attribute_specifiers(ps);

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
                ast_node_free(vty);
                ps_advance(ps);   /* eat `;` */
                return NULL;
            }
            ps_error(ps, vloc, "declaration is missing a name");
            vname = cpp_xstrdup("?");
        }

        if (ds->storage == SC_TYPEDEF) {
            /* `typedef T NAME;` — produce AST_TYPEDEF_DECL.  Any pointer
             * / array / function suffix has already been folded into vty
             * by parse_declarator, so the alias resolves to the full
             * declared type. */
            AstNode *td = ast_node_new(AST_TYPEDEF_DECL, vloc);
            td->u.typedef_decl.alias  = vname;
            td->u.typedef_decl.target = vty;
            td_add(&ps->typedefs, vname);
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
        vd->u.var_decl.type    = vty;
        vd->u.var_decl.name    = vname;
        vd->u.var_decl.storage = ds->storage;
        /* Phase R2: propagate _Thread_local to the AST node. */
        vd->u.var_decl.is_thread_local = ds->is_thread_local;

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

/* Back-compat wrapper.  Some call sites still hand us a pre-parsed base
 * type with no DeclSpecs context; treat them as having no storage class. */
static AstNode *parse_var_decl_list(PS *ps, AstNode *base_ty, bool stmt_wrap) {
    DeclSpecs ds = {0};
    ds.base_ty = base_ty;
    ds.loc     = base_ty ? base_ty->loc : ps_peek(ps).loc;
    return parse_init_declarator_list(ps, &ds, stmt_wrap);
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
    /* Detach so freeing func_ty doesn't free the now-owned children. */
    func_ty->u.type_func.ret    = NULL;
    func_ty->u.type_func.params = (AstVec){0};
    ast_node_free(func_ty);
    return fn;
}

/* =========================================================================
 * Top-level declaration  (S1 — DeclSpecs / declarator model)
 * ====================================================================== */
static AstNode *parse_top_decl(PS *ps) {
    SharpTok t = ps_peek(ps);

    /* import "path.sp"; */
    if (t.kind == STOK_IMPORT) {
        ps_advance(ps);
        SharpTok path_tok = ps_peek(ps);
        AstNode *imp = ast_node_new(AST_IMPORT, t.loc);
        if (path_tok.kind == STOK_STRING_LIT) {
            ps_advance(ps);
            size_t len = path_tok.len >= 2 ? path_tok.len - 2 : 0;
            imp->u.import.path = cpp_xstrndup(path_tok.text + 1, len);
        } else {
            ps_error(ps, path_tok.loc,
                "import expects a quoted path, e.g. import \"vec.sp\"");
            imp->u.import.path = cpp_xstrndup("?", 1);
            ps_sync(ps);
            return imp;
        }
        ps_expect(ps, STOK_SEMI, "after import path");
        return imp;
    }

    /* S2: enum at top level.  Two shapes:
     *   enum [Tag] { ... };           — definition  (parse_enum_def)
     *   enum Tag x; / enum Tag *p;    — type usage  (general declarator)
     * The body presence ('{' after the optional tag) decides. */
    if (t.kind == STOK_ENUM) {
        SharpTokKind k2 = ps_peek2(ps).kind;
        SharpTokKind k3 = ps->pos + 2 < ps->ntoks ? ps->toks[ps->pos+2].kind : STOK_EOF;
        if (k2 == STOK_LBRACE || (k2 == STOK_IDENT && k3 == STOK_LBRACE)) {
            AstNode *ed = parse_enum_def(ps);
            /* Phase R2: top-level `enum {...};` requires a trailing
             * semicolon.  parse_enum_def no longer eats it (so stmt /
             * field paths can own it correctly); the top-level path
             * must consume it itself. */
            ps_expect(ps, STOK_SEMI, "after top-level enum definition");
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
                ps_match(ps, STOK_SEMI);
                return sd;
            }
        } else {
            ps->pos = save;
            AstNode *sd = parse_struct_def(ps);
            ps_match(ps, STOK_SEMI);
            return sd;
        }
    }

    /* `struct Tag { ... }` or `struct Tag;` at top level — parse_struct_def
     * handles this.  We distinguish from a top-level `struct Tag *p;` by
     * looking past any optional generic params to either '{' or ';'. */
    if (t.kind == STOK_STRUCT) {
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
                /* Consume the trailing ';' that source-level top-level
                 * struct definitions carry — `struct X { ... };`.
                 * parse_struct_def itself no longer eats SEMI (it's
                 * shared with the inline-typedef and field-type paths
                 * where SEMI ownership belongs to the enclosing
                 * declaration). */
                ps_match(ps, STOK_SEMI);
                return sd;
            }
        } else {
            ps->pos = save;
            AstNode *sd = parse_struct_def(ps);
            ps_match(ps, STOK_SEMI);
            return sd;
        }
        /* Fall through: `struct Tag *p;` etc. is handled by the general
         * declarator path because parse_decl_specifiers absorbs `struct Tag`. */
    }

    /* `extern struct Vec<int>;` — Sharp-specific explicit instantiation
     * marker (no declarator, no body).  Distinguished from `extern int x;`
     * by the bare-tag-then-semicolon shape. */
    if (t.kind == STOK_EXTERN && ps_peek2(ps).kind == STOK_STRUCT) {
        size_t save = ps->pos;
        ps_advance(ps);  /* extern */
        ps_advance(ps);  /* struct */
        if (ps_at(ps, STOK_IDENT)) {
            AstNode *ty = parse_type(ps);
            if (ps_match(ps, STOK_SEMI)) {
                AstNode *vd = ast_node_new(AST_VAR_DECL, t.loc);
                vd->u.var_decl.type    = ty;
                vd->u.var_decl.name    = cpp_xstrndup("<extern-struct>", 15);
                vd->u.var_decl.storage = SC_EXTERN;
                return vd;
            }
            ast_node_free(ty);
        }
        ps->pos = save;
    }

    if (!is_type_start(ps)) {
        ps_error(ps, t.loc, "unexpected token '%.*s' at top level",
                 (int)t.len, t.text);
        ps_advance(ps);
        return NULL;
    }

    DeclSpecs ds = parse_decl_specifiers(ps);
    if (ds.empty || !ds.base_ty) {
        ps_error(ps, t.loc, "missing declaration specifier");
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
        fn->u.func_def.name = cpp_xstrndup(buf, strlen(buf));
        fn->u.func_def.storage   = ds.storage;
        fn->u.func_def.is_static = (ds.storage == SC_STATIC);
        fn->u.func_def.is_inline = ds.is_inline;
        fn->u.func_def.is_thread_local = ds.is_thread_local; /* Phase R2 */
        return fn;
    }

    /* ── Sharp generic function: `RetType name<T,U>(args) { body }` ──
     * Detect the Sharp-specific `<...>` between name and parameter list.
     * If present we route to finish_func, which knows how to consume the
     * generic-params section.  Plain C functions never have this shape. */
    if (ps_at(ps, STOK_IDENT) && ps_peek2(ps).kind == STOK_LT &&
        looks_like_sharp_generic_params(ps, ps->pos + 1)) {
        SharpTok name_tok = ps_advance(ps);
        AstNode *fn = finish_func(ps, ds.base_ty, name_tok, /*is_operator=*/false);
        fn->u.func_def.storage   = ds.storage;
        fn->u.func_def.is_static = (ds.storage == SC_STATIC);
        fn->u.func_def.is_inline = ds.is_inline;
        fn->u.func_def.is_thread_local = ds.is_thread_local; /* Phase R2 */
        return fn;
    }

    /* ── Standard ISO-C declarator path. ───────────────────────────── */
    char    *name      = NULL;
    CppLoc   first_loc = ps_peek(ps).loc;
    AstNode *full_ty   = parse_declarator(ps, ds.base_ty, &name);
    /* `int x __attribute__((unused));`, `void foo() __attribute__((noreturn));` —
     * GCC attributes after the declarator (and before `;`/`{`/`,`/`=`).
     * Eat them silently. */
    eat_attribute_specifiers(ps);

    /* Function definition or prototype: outermost shape is FUNC and the
     * next token is either `{` (definition), `const` (Sharp const-method
     * suffix — only meaningful inside struct, but tolerate at top level),
     * or `;` (prototype).  A `=` here would be a function-typed variable
     * with an initializer, which we treat as a variable declaration.
     *
     * Phase R2: SC_TYPEDEF must NOT take this branch.  ISO C allows the
     * K&R-classic form `typedef void Fn(int);` (no parentheses around
     * the name) which produces a *function-type* alias.  Without the
     * SC_TYPEDEF guard we mis-classify it as a function prototype and
     * never register `Fn` as a typedef-name; the next use as
     * `Fn *p = ...` then fails with "unknown type 'Fn'".  stb's
     * `typedef void stbi_write_func(void *, void *, int);` hits this. */
    if (full_ty && full_ty->kind == AST_TYPE_FUNC && name &&
        ds.storage != SC_TYPEDEF &&
        (ps_at(ps, STOK_LBRACE) || ps_at(ps, STOK_SEMI) || ps_at(ps, STOK_CONST))) {
        AstNode *fn = build_func_def_from_decl(full_ty, name, first_loc);
        fn->u.func_def.storage   = ds.storage;
        fn->u.func_def.is_static = (ds.storage == SC_STATIC);
        fn->u.func_def.is_inline = ds.is_inline;
        fn->u.func_def.is_thread_local = ds.is_thread_local; /* Phase R2 */
        if (ps_match(ps, STOK_CONST)) fn->u.func_def.is_const_method = true;
        if (ps_at(ps, STOK_LBRACE)) {
            fn->u.func_def.body = parse_block(ps);
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
        AstNode *td = ast_node_new(AST_TYPEDEF_DECL, first_loc);
        td->u.typedef_decl.alias  = name;
        td->u.typedef_decl.target = full_ty;
        td_add(&ps->typedefs, name);
        if (ps_at(ps, STOK_EQ)) {
            ps_error(ps, ps_peek(ps).loc,
                "typedef declarations cannot have an initialiser");
            ps_advance(ps);
            ast_node_free(parse_expr_prec(ps, 2));
        }
        first = td;
    } else {
        AstNode *vd = ast_node_new(AST_VAR_DECL, first_loc);
        vd->u.var_decl.type    = full_ty;
        vd->u.var_decl.name    = name;
        vd->u.var_decl.storage = ds.storage;
        /* Phase R2: propagate _Thread_local to the AST node. */
        vd->u.var_decl.is_thread_local = ds.is_thread_local;
        if (ps_match(ps, STOK_EQ)) {
            vd->u.var_decl.init = ps_at(ps, STOK_LBRACE)
                                ? parse_init_list(ps)
                                : parse_expr_prec(ps, 2);
        } else if (full_ty && full_ty->kind == AST_TYPE_AUTO) {
            ps_error(ps, first_loc, "'auto %s' requires an initialiser", name);
        }
        first = vd;
    }

    /* Additional declarators after `,`.  Each gets its own clone of the
     * base type AST so that ast_node_free doesn't double-free. */
    while (ps_match(ps, STOK_COMMA)) {
        AstNode *base_copy = ast_clone_type(ds.base_ty);
        char    *next_name = NULL;
        CppLoc   next_loc  = ps_peek(ps).loc;
        AstNode *next_ty   = parse_declarator(ps, base_copy, &next_name);
        eat_attribute_specifiers(ps);
        if (!next_name) {
            ps_error(ps, next_loc, "declaration is missing a name");
            next_name = cpp_xstrdup("?");
        }
        AstNode *out = NULL;
        if (ds.storage == SC_TYPEDEF) {
            AstNode *td = ast_node_new(AST_TYPEDEF_DECL, next_loc);
            td->u.typedef_decl.alias  = next_name;
            td->u.typedef_decl.target = next_ty;
            td_add(&ps->typedefs, next_name);
            out = td;
        } else {
            AstNode *vd = ast_node_new(AST_VAR_DECL, next_loc);
            vd->u.var_decl.type    = next_ty;
            vd->u.var_decl.name    = next_name;
            vd->u.var_decl.storage = ds.storage;
            /* Phase R2: propagate _Thread_local. */
            vd->u.var_decl.is_thread_local = ds.is_thread_local;
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
                strcmp(lhs->u.ident.name, "__builtin_offsetof") == 0) {
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
                /* Collect all tokens from '(' up to and including matching ')'. */
                char buf[512];
                int blen = 0;
                blen += snprintf(buf + blen, sizeof buf - blen, "__builtin_offsetof(");
                ps_advance(ps);  /* eat '(' */
                int depth = 0;
                while (!ps_at(ps, STOK_EOF)) {
                    SharpTok tk = ps_peek(ps);
                    if (tk.kind == STOK_LPAREN) {
                        depth++;
                        if (blen + (int)tk.len + 1 < (int)sizeof buf) {
                            memcpy(buf + blen, tk.text, tk.len);
                            blen += tk.len;
                        }
                        ps_advance(ps);
                    } else if (tk.kind == STOK_RPAREN) {
                        if (depth == 0) {
                            blen += snprintf(buf + blen, sizeof buf - blen, ")");
                            ps_advance(ps);
                            break;
                        }
                        depth--;
                        if (blen + (int)tk.len + 1 < (int)sizeof buf) {
                            memcpy(buf + blen, tk.text, tk.len);
                            blen += tk.len;
                        }
                        ps_advance(ps);
                    } else {
                        /* Add spacing heuristic: add a space before most tokens. */
                        if (blen > 0 && blen + (int)tk.len + 2 < (int)sizeof buf) {
                            /* Only add space if last char wasn't '(' or ','. */
                            char last = buf[blen-1];
                            if (last != '(' && last != ',')
                                buf[blen++] = ' ';
                            memcpy(buf + blen, tk.text, tk.len);
                            blen += tk.len;
                        }
                        ps_advance(ps);
                    }
                }
                buf[blen < (int)sizeof buf ? blen : (int)sizeof buf - 1] = '\0';
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
                const char *va_name = lhs->u.ident.name;
                char buf2[512];
                int blen2 = 0;
                blen2 += snprintf(buf2 + blen2, sizeof buf2 - blen2,
                                  "__builtin_va_arg(");
                ps_advance(ps);  /* eat '(' */
                int depth2 = 0;
                while (!ps_at(ps, STOK_EOF)) {
                    SharpTok tk = ps_peek(ps);
                    if (tk.kind == STOK_LPAREN) {
                        depth2++;
                        if (blen2 + (int)tk.len + 1 < (int)sizeof buf2) {
                            memcpy(buf2 + blen2, tk.text, tk.len); blen2 += tk.len;
                        }
                        ps_advance(ps);
                    } else if (tk.kind == STOK_RPAREN) {
                        if (depth2 == 0) {
                            blen2 += snprintf(buf2 + blen2, sizeof buf2 - blen2, ")");
                            ps_advance(ps); break;
                        }
                        depth2--;
                        if (blen2 + (int)tk.len + 1 < (int)sizeof buf2) {
                            memcpy(buf2 + blen2, tk.text, tk.len); blen2 += tk.len;
                        }
                        ps_advance(ps);
                    } else {
                        if (blen2 > 0 && blen2 + (int)tk.len + 2 < (int)sizeof buf2) {
                            char last2 = buf2[blen2-1];
                            if (last2 != '(' && last2 != ',')
                                buf2[blen2++] = ' ';
                            memcpy(buf2 + blen2, tk.text, tk.len); blen2 += tk.len;
                        }
                        ps_advance(ps);
                    }
                }
                buf2[blen2 < (int)sizeof buf2 ? blen2 : (int)sizeof buf2 - 1] = '\0';
                (void)va_name;
                ast_node_free(lhs);
                lhs = ast_node_new(AST_IDENT, t.loc);
                lhs->u.ident.name = cpp_xstrndup(buf2, blen2);
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
            /* `.field = value`  field designator */
            CppLoc dloc = ps_advance(ps).loc;       /* '.' */
            SharpTok nm = ps_advance(ps);           /* IDENT */
            ps_expect(ps, STOK_EQ, "designator '='");
            AstNode *val = ps_at(ps, STOK_LBRACE)
                         ? parse_init_list(ps)
                         : parse_expr_prec(ps, 2);
            AstNode *d = ast_node_new(AST_DESIGNATED_INIT, dloc);
            d->u.designated_init.designator_kind = 0;  /* field */
            d->u.designated_init.field_name = cpp_xstrndup(nm.text, nm.len);
            d->u.designated_init.value      = val;
            item = d;
        } else if (ps_at(ps, STOK_LBRACKET)) {
            /* `[idx] = value`  array designator */
            CppLoc dloc = ps_advance(ps).loc;       /* '[' */
            AstNode *ix = parse_expr_prec(ps, 2);
            ps_expect(ps, STOK_RBRACKET, "array designator ']'");
            ps_expect(ps, STOK_EQ,       "designator '='");
            AstNode *val = ps_at(ps, STOK_LBRACE)
                         ? parse_init_list(ps)
                         : parse_expr_prec(ps, 2);
            AstNode *d = ast_node_new(AST_DESIGNATED_INIT, dloc);
            d->u.designated_init.designator_kind = 1;  /* array */
            d->u.designated_init.index_expr = ix;
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
        if (ps_at(ps, STOK_RBRACE)) break;
    }

    ps_expect(ps, STOK_RBRACE, "init list '}'");
    return il;
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
        return parse_postfix(ps, n);
    }
    case STOK_FLOAT_LIT: {
        AstNode *n = ast_node_new(AST_FLOAT_LIT, t.loc);
        n->u.float_lit.val = t.u.fval;
        return parse_postfix(ps, n);
    }
    case STOK_STRING_LIT: {
        AstNode *n = ast_node_new(AST_STRING_LIT, t.loc);
        n->u.string_lit.text = t.text;
        n->u.string_lit.len  = t.len;
        return parse_postfix(ps, n);
    }
    case STOK_CHAR_LIT: {
        AstNode *n = ast_node_new(AST_CHAR_LIT, t.loc);
        n->u.char_lit.val = t.u.ival;
        return parse_postfix(ps, n);
    }
    case STOK_NULL: {
        AstNode *n = ast_node_new(AST_NULL_LIT, t.loc);
        return parse_postfix(ps, n);
    }

    /* -- Grouping: (expr) or (Type)expr -- */
    case STOK_LPAREN: {
        /* Phase R4: GCC statement-expression ({ stmts; expr; }).
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
        return parse_postfix(ps, inner);
    }

    /* -- sizeof -- */
    case STOK_SIZEOF: {
        AstNode *n = ast_node_new(AST_SIZEOF, t.loc);
        ps_expect(ps, STOK_LPAREN, "sizeof '('");
        if (is_type_start(ps)) {
            n->u.sizeof_.operand = parse_type(ps);
            n->u.sizeof_.is_type = true;
        } else {
            n->u.sizeof_.operand = parse_expr(ps);
            n->u.sizeof_.is_type = false;
        }
        ps_expect(ps, STOK_RPAREN, "sizeof ')'");
        return n;
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
            bool is_has_op = (strncmp(n->u.at_intrinsic.name,
                                      "has_operator", 12) == 0 &&
                              n->u.at_intrinsic.name[12] == '\0');
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
            if (ps_at(ps, STOK_IDENT) || ps_at(ps, STOK_RBRACE)) {
                size_t save3 = ps->pos;
                if (ps_at(ps, STOK_IDENT)) {
                    ps_advance(ps); /* field name */
                    if (ps_at(ps, STOK_COLON)) {
                        /* yes, struct literal */
                        ps->pos = save2; /* back to '{' */
                        ps_advance(ps); /* consume '{' */
                        AstNode *sl = ast_node_new(AST_STRUCT_LIT, t.loc);
                        AstNode *ty = ast_node_new(AST_TYPE_NAME, t.loc);
                        ty->u.type_name.name = cpp_xstrndup(t.text, t.len);
                        sl->u.struct_lit.type = ty;
                        while (!ps_at(ps, STOK_RBRACE) && !ps_at(ps, STOK_EOF)) {
                            SharpTok fn_tok = ps_expect(ps, STOK_IDENT, "field name");
                            AstNode *fn_node = ast_node_new(AST_IDENT, fn_tok.loc);
                            fn_node->u.ident.name = cpp_xstrndup(fn_tok.text, fn_tok.len);
                            astvec_push(&sl->u.struct_lit.field_names, fn_node);
                            ps_expect(ps, STOK_COLON, "':' after field name");
                            astvec_push(&sl->u.struct_lit.field_vals, parse_expr_prec(ps, 2));
                            if (!ps_match(ps, STOK_COMMA)) break;
                        }
                        ps_expect(ps, STOK_RBRACE, "struct literal '}'");
                        ast_node_free(id);
                        return parse_postfix(ps, sl);
                    }
                    ps->pos = save3;
                }
                /* empty braces or not a struct literal — treat as empty block? */
                /* rollback */
                ps->pos = save2;
            } else {
                ps->pos = save2;
            }
        }

        /* Generic type use as expression: Vec<int>.new() or Vec<int>.field */
        if (ps_at(ps, STOK_LT)) {
            size_t save2 = ps->pos;
            AstVec args = {0};
            if (parse_generic_args(ps, &args) &&
                (ps_at(ps, STOK_DOT) || ps_at(ps, STOK_ARROW) ||
                 ps_at(ps, STOK_LPAREN))) {
                /* Build FIELD_ACCESS(TYPE_GENERIC, method).call */
                AstNode *gen = ast_node_new(AST_TYPE_GENERIC, id->loc);
                gen->u.type_generic.name = cpp_xstrdup(id->u.ident.name);
                gen->u.type_generic.args = args;
                /* Wrap in an IDENT-like node for postfix parsing */
                AstNode *cast_node = ast_node_new(AST_CAST, id->loc);
                cast_node->u.cast.type    = gen;
                cast_node->u.cast.operand = NULL;
                ast_node_free(id);
                return parse_postfix(ps, cast_node);
            }
            /* rollback */
            ps->pos = save2;
            for (size_t i = 0; i < args.len; i++) ast_node_free(args.data[i]);
            astvec_free(&args);
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
        lhs = n;
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
        AstNode *bin = ast_node_new(AST_BINOP, op.loc);
        bin->u.binop.op  = op.kind;
        bin->u.binop.lhs = lhs;
        bin->u.binop.rhs = rhs;
        lhs = bin;
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
        /* Phase R5 fix: drain pending_decls in two passes so that inline
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
            if (pd && (pd->kind == AST_STRUCT_DEF || pd->kind == AST_ENUM_DEF))
                astvec_push(&blk->u.block.stmts, pd);
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

static AstNode *parse_stmt(PS *ps) {
    SharpTok t = ps_peek(ps);

    /* bare ';' */
    if (t.kind == STOK_SEMI) { ps_advance(ps); return NULL; }

    /* Phase R3: bare `__attribute__((...));` as a statement.  GCC's
     * fallthrough marker (`__attribute__((fallthrough));` in switch
     * case fall-through positions, sqlite uses it heavily) and other
     * statement-attached attributes appear unbound.  Eat the attribute
     * spec and the trailing `;`, return NULL (block loop skips). */
    if (t.kind == STOK_ATTRIBUTE) {
        eat_attribute_specifiers(ps);
        if (ps_at(ps, STOK_SEMI)) {
            ps_advance(ps);
            return NULL;
        }
        /* attributes followed by something else are part of a decl;
         * fall through to the declaration path below. */
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
        n->u.switch_.body = parse_stmt(ps);
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
                n->u.for_.init = init;
                /* parse_stmt already consumed ';' for the init-decl.
                 * Multi-variable form `for (int i=0, j=1; …)` is not
                 * supported — only the first declarator becomes for-init,
                 * and the rest would otherwise leak into the block. */
                if (ps->pending_decls.len > 0) {
                    ps_error(ps, init_loc,
                        "multiple declarators in 'for' init not supported");
                    for (size_t i = 0; i < ps->pending_decls.len; i++)
                        ast_node_free(ps->pending_decls.data[i]);
                    ps->pending_decls.len = 0;
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

AstNode *parse_file(const SharpTok *tokens, size_t ntokens,
                    const char *filename, FeDiagArr *diags) {
    PS ps = { tokens, ntokens, 0, filename, diags,
              /*in_defer=*/false, /*pending_close=*/0,
              /*pending_decls=*/{0}, /*typedefs=*/{0},
              /*anon_struct_counter=*/0 };
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
        for (size_t i = 0; i < ps.pending_decls.len; i++) {
            astvec_push(&file->u.file.decls, ps.pending_decls.data[i]);
        }
        ps.pending_decls.len = 0;
        if (decl) astvec_push(&file->u.file.decls, decl);
    }
    astvec_free(&ps.pending_decls);
    td_free(&ps.typedefs);
    return file;
}

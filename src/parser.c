/* parser.c - recursive-descent parser for Sharp (phase 1).
 *
 * Grammar sketch:
 *   program   = top_decl*
 *   top_decl  = struct_decl | func_decl
 *   struct    = 'struct' IDENT '{' field_decl* '}'
 *   field     = type IDENT ';'
 *   func      = type IDENT '(' params? ')' block
 *   stmt      = block | if | while | for | return | break | continue
 *             | vardecl | expr_stmt
 *   vardecl   = type IDENT ('=' expr)? ';'
 *   expr      = assign
 *   assign    = logic_or ( ('=' | '+=' | ...) assign )?
 *   logic_or  = logic_and ('||' logic_and)*
 *   ...
 *   postfix   = primary ( '.' IDENT | '(' args? ')' )*
 */
#include "sharp.h"

typedef struct {
    Lexer*   lex;
    Arena**  arena;
} P;

/* ---------- helpers ---------- */
static Node* mk(P* p, NodeKind k, int line) {
    Node* n = ARENA_NEW(p->arena, Node);
    n->kind = k;
    n->line = line;
    return n;
}

static void errtok(P* p, const Tok* t, const char* what) {
    error_at(t->line, "%s (got %s '%.*s')", what, tok_name(t->kind),
             t->len, t->start ? t->start : "");
}

static Tok expect(P* p, TokKind k, const char* what) {
    Tok t = lex_peek(p->lex);
    if (t.kind != k) {
        errtok(p, &t, what);
        /* return the token we have; do not advance on mismatch to avoid cascade */
        return t;
    }
    return lex_next(p->lex);
}

static bool accept(P* p, TokKind k) {
    if (lex_peek(p->lex).kind == k) { lex_next(p->lex); return true; }
    return false;
}

/* dynamic append to an arena-backed array of Node* */
typedef struct { Node** data; int len; int cap; } NodeVec;
static void nv_push(NodeVec* v, Node* n) {
    if (v->len == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        Node** nd = (Node**)malloc((size_t)v->cap * sizeof(Node*));
        if (v->len) memcpy(nd, v->data, (size_t)v->len * sizeof(Node*));
        free(v->data);
        v->data = nd;
    }
    v->data[v->len++] = n;
}
static Node** nv_freeze(P* p, NodeVec* v, int* out_len) {
    *out_len = v->len;
    if (v->len == 0) { free(v->data); return NULL; }
    Node** dst = ARENA_NEW_ARR(p->arena, Node*, v->len);
    memcpy(dst, v->data, (size_t)v->len * sizeof(Node*));
    free(v->data);
    return dst;
}

/* ---------- types ---------- */
static bool is_prim_tk(TokKind k) {
    return (k >= TK_VOID && k <= TK_USIZE) ||
           k == TK_INT_TYPE || k == TK_FLOAT_TYPE || k == TK_DOUBLE_TYPE;
}

static TypeKind prim_from_tk(TokKind k) {
    if (k == TK_INT_TYPE)   return TY_I32;
    if (k == TK_FLOAT_TYPE) return TY_F32;
    if (k == TK_DOUBLE_TYPE) return TY_F64;
    return (TypeKind)(TY_VOID + (k - TK_VOID));
}

static bool is_c_type_mod(TokKind k) {
    return k == TK_SHORT || k == TK_LONG || k == TK_UNSIGNED || k == TK_SIGNED ||
           k == TK_CHAR ||
           k == TK_INT_TYPE || k == TK_FLOAT_TYPE || k == TK_DOUBLE_TYPE ||
           k == TK___INT64;
}

/* Parse a C-style type specification.
 * Supported forms:
 *   void, bool, char, i8..u64, f32, f64, isize, usize  (Sharp style)
 *   int, short, long, long long, float, double          (C style)
 *   unsigned int, unsigned long, unsigned long long     (with modifiers)
 *   signed char, unsigned char                          (with signedness)
 */
static TypeKind parse_c_type(P* p) {
    /* Collect modifiers and base type from the token stream.
     * C allows: unsigned long long int, long long, signed char, etc.
     * We scan ahead without consuming, then consume in a second pass. */
    bool has_unsigned = false;
    bool has_signed   = false;
    bool has_short    = false;
    bool has_long     = false;
    bool has_long_long = false;
    bool has_int      = false;
    bool has_float    = false;
    bool has_double   = false;
    bool has_char     = false;

    /* Lookahead phase: scan up to 4 tokens without consuming */
    int n = 0;
    for (int i = 0; i < 5; i++) {
        Tok t = lex_peek_n(p->lex, i);
        if (t.kind == TK_EOF) break;
        if (t.kind == TK_IDENT) break; /* identifier ends the type spec */

        bool consumed = true;
        switch (t.kind) {
            case TK_UNSIGNED: has_unsigned = true; break;
            case TK_SIGNED:   has_signed   = true; break;
            case TK_SHORT:    has_short    = true; break;
            case TK_LONG: {
                Tok next = lex_peek_n(p->lex, i+1);
                if (next.kind == TK_LONG) {
                    has_long_long = true;
                    n = i + 2; /* consume both longs */
                    consumed = false; /* prevent post-switch overwrite of n */
                    i = 4; /* for-loop i++ makes it 5, terminating the loop */
                } else {
                    has_long = true;
                }
                break;
            }
            case TK_CHAR:       has_char    = true; break;
            case TK_INT_TYPE:   has_int   = true; break;
            case TK_FLOAT_TYPE: has_float = true; break;
            case TK_DOUBLE_TYPE:has_double = true; break;
            case TK___INT64:    /* standalone __int64 → 64-bit signed */
                                return TY_I64;
            default: consumed = false; break;
        }
        if (consumed) n = i + 1; /* count consumed tokens */
    }

    /* Consume all recognized tokens */
    for (int i = 0; i < n; i++) lex_next(p->lex);

    /* Resolve to a TypeKind */
    if (has_char)     return has_unsigned ? TY_U8 : TY_I8;
    if (has_short)    return has_unsigned ? TY_U16 : TY_I16;
    if (has_long_long) return has_unsigned ? TY_U64 : TY_I64;
    if (has_long)     return has_unsigned ? TY_U64 : TY_I64;
    if (has_double)   return TY_F64;
    if (has_float)    return TY_F32;
    if (has_int)      return has_unsigned ? TY_U32 : TY_I32;
    /* Bare unsigned / signed without base type → unsigned int / signed int */
    if (has_unsigned) return TY_U32;
    if (has_signed)   return TY_I32;

    return TY_VOID; /* fallback */
}

/* Parse a C-style type specification and return the raw C spelling.
 * This preserves the original C type keywords for pass-through. */
static const char* parse_c_type_raw(P* p) {
    StrBuf sb; sb_init(&sb);
    bool first = true;
    for (int i = 0; i < 5; i++) {
        Tok t = lex_peek(p->lex);
        if (t.kind == TK_UNSIGNED || t.kind == TK_SIGNED || t.kind == TK_SHORT ||
            t.kind == TK_LONG || t.kind == TK_CHAR || t.kind == TK_INT_TYPE ||
            t.kind == TK_FLOAT_TYPE || t.kind == TK_DOUBLE_TYPE || t.kind == TK___INT64) {
            if (!first) sb_putc(&sb, ' ');
            first = false;
            sb_putn(&sb, t.start, t.len);
            lex_next(p->lex);
            if (t.kind == TK_LONG && lex_peek(p->lex).kind == TK_LONG) {
                sb_putc(&sb, ' ');
                sb_putn(&sb, lex_peek(p->lex).start, lex_peek(p->lex).len);
                lex_next(p->lex);
            }
        } else {
            break;
        }
    }
    const char* s = arena_strndup(p->arena, sb.data, (int)sb.len);
    sb_free(&sb);
    return s;
}

/* Helper: consume `__declspec(...)` prefix if present.
 * Returns the inner content string (e.g. "noreturn", "dllimport"), or NULL.
 * The returned pointer points into the lexer's source buffer. */
static const char* parse_declspec(P* p) {
    if (!accept(p, TK___DECLSPEC)) return NULL;
    expect(p, TK_LPAREN, "expected '(' after __declspec");
    const char* start = lex_peek(p->lex).start;
    int depth = 1;
    while (depth > 0) {
        Tok t = lex_peek(p->lex);
        if (t.kind == TK_EOF) break;
        if (t.kind == TK_LPAREN) depth++;
        else if (t.kind == TK_RPAREN) depth--;
        if (depth == 0) {
            /* The inner content is from `start` to `t.start` (exclusive of ')'). */
            size_t len = (size_t)(t.start - start);
            lex_next(p->lex); /* consume the ')' */
            return len > 0 ? arena_strndup(p->arena, start, (int)len) : "";
        }
        lex_next(p->lex);
    }
    lex_next(p->lex); /* consume the ')' if we fell through */
    return "";
}

/* Helper: consume `__declspec(...)` if present, discard content.
 * Legacy wrapper for use in type-prefix positions where we don't care. */
static bool skip_declspec(P* p) {
    return parse_declspec(p) != NULL;
}

/* Helper: consume `__attribute__((...))` suffix if present.
 * Handles nested parentheses correctly. Returns true if an attribute
 * was consumed, false otherwise. */
static bool skip_attribute(P* p) {
    if (!accept(p, TK___ATTRIBUTE__)) return false;
    /* Must be followed by `(( ... ))` */
    expect(p, TK_LPAREN, "expected '(' after __attribute__");
    expect(p, TK_LPAREN, "expected '((' after __attribute__");
    /* Skip tokens until we find matching `))` */
    int depth = 2;
    while (depth > 0) {
        Tok t = lex_peek(p->lex);
        if (t.kind == TK_EOF) {
            errtok(p, &t, "unterminated __attribute__");
            return true;
        }
        if (t.kind == TK_LPAREN) depth++;
        else if (t.kind == TK_RPAREN) depth--;
        lex_next(p->lex);
    }
    return true;
}

static bool is_type_start(TokKind k) {
    return is_prim_tk(k) || k == TK_IDENT ||
           k == TK_SHORT || k == TK_LONG || k == TK_UNSIGNED || k == TK_SIGNED || k == TK_CHAR ||
           k == TK_INT_TYPE || k == TK_FLOAT_TYPE || k == TK_DOUBLE_TYPE ||
           k == TK___INT64 ||
           k == TK_CONST || k == TK_EXTERN ||
           k == TK___INLINE__ || k == TK___INLINE ||
           k == TK___DECLSPEC || k == TK___ATTRIBUTE__ ||
           k == TK___CDECL || k == TK___STDCALL || k == TK___FASTCALL || k == TK___UNALIGNED ||
           k == TK_STRUCT || k == TK_UNION;
}

/* Parse an optional calling-convention keyword.
 * Returns the canonical string ("__cdecl", "__stdcall", "__fastcall",
 * "__unaligned") or NULL if the next token is not a calling convention. */
static const char* parse_calling_conv(P* p) {
    Tok t = lex_peek(p->lex);
    switch (t.kind) {
        case TK___CDECL:     lex_next(p->lex); return "__cdecl";
        case TK___STDCALL:   lex_next(p->lex); return "__stdcall";
        case TK___FASTCALL:  lex_next(p->lex); return "__fastcall";
        case TK___UNALIGNED: lex_next(p->lex); return "__unaligned";
        default: return NULL;
    }
}

static Type* parse_type(P* p) {
    /* Leading `const` — applies to the innermost (base) type. */
    bool leading_const = false;
    if (accept(p, TK_CONST)) leading_const = true;

    /* Skip C storage-class / inline / __declspec / calling-conv / attribute modifiers
     * that are not type keywords. */
    for (;;) {
        TokKind k = lex_peek(p->lex).kind;
        if (k == TK___INLINE__ || k == TK___INLINE || k == TK_EXTERN ||
            k == TK___RESTRICT || k == TK___RESTRICT__ || k == TK_RESTRICT) {
            lex_next(p->lex);
        } else if (k == TK___DECLSPEC) {
            skip_declspec(p);
        } else if (k == TK___CDECL || k == TK___STDCALL || k == TK___FASTCALL || k == TK___UNALIGNED) {
            lex_next(p->lex);
        } else if (k == TK___ATTRIBUTE__) {
            skip_attribute(p);
        } else {
            break;
        }
    }

    Type* base;
    Tok t = lex_peek(p->lex);
    if (t.kind == TK___INT64) {
        lex_next(p->lex);
        base = type_prim(p->arena, TY_I64);
    } else if (t.kind == TK_IDENT && lex_ident_is(t, "wchar_t")) {
        lex_next(p->lex);
        base = type_named(p->arena, "wchar_t");
    } else if (is_c_type_mod(t.kind)) {
        /* C-style type with modifiers: unsigned int, long long, etc.
         * Pass through raw spelling for TDD-1.4 compatibility. */
        const char* raw = parse_c_type_raw(p);
        base = type_named(p->arena, raw);
    } else if (is_prim_tk(t.kind)) {
        lex_next(p->lex);
        base = type_prim(p->arena, prim_from_tk(t.kind));
    } else if (t.kind == TK_SHORT) {
        lex_next(p->lex);
        base = type_prim(p->arena, TY_I16);
    } else if (t.kind == TK_LONG) {
        lex_next(p->lex);
        if (lex_peek(p->lex).kind == TK_LONG) {
            lex_next(p->lex);
        }
        base = type_prim(p->arena, TY_I64);
    } else if (t.kind == TK_UNSIGNED) {
        lex_next(p->lex);
        if (lex_peek(p->lex).kind == TK_SHORT) {
            lex_next(p->lex);
            base = type_prim(p->arena, TY_U16);
        } else if (lex_peek(p->lex).kind == TK_LONG) {
            lex_next(p->lex);
            if (lex_peek(p->lex).kind == TK_LONG) {
                lex_next(p->lex);
            }
            base = type_prim(p->arena, TY_U64);
        } else if (lex_peek(p->lex).kind == TK_INT_TYPE) {
            lex_next(p->lex);
            base = type_prim(p->arena, TY_U32);
        } else {
            base = type_prim(p->arena, TY_U32);
        }
    } else if (t.kind == TK_IDENT) {
        lex_next(p->lex);
        char* nm = arena_strndup(p->arena, t.start, t.len);
        /* Phase 4: optional generic instantiation `Name<Type, Type, ...>`.
         * Because we are firmly in a type context here, the `<` is always
         * read as the start of a targ list — no ambiguity with `<` as a
         * comparison operator at this position. */
        if (accept(p, TK_LT)) {
            Type** targs = NULL;
            int    ntargs = 0;
            /* Simple dynamic array of Type*, copied into the arena once
             * we know the final count. */
            Type** buf = NULL; int cap = 0;
            for (;;) {
                Type* ta = parse_type(p);
                if (ntargs == cap) {
                    cap = cap ? cap * 2 : 4;
                    Type** nd = (Type**)malloc((size_t)cap * sizeof(Type*));
                    if (ntargs) memcpy(nd, buf, (size_t)ntargs * sizeof(Type*));
                    free(buf);
                    buf = nd;
                }
                buf[ntargs++] = ta;
                if (!accept(p, TK_COMMA)) break;
            }
            expect(p, TK_GT, "expected '>' to close type arguments");
            if (ntargs > 0) {
                targs = ARENA_NEW_ARR(p->arena, Type*, ntargs);
                memcpy(targs, buf, (size_t)ntargs * sizeof(Type*));
            }
            free(buf);
            base = type_named_generic(p->arena, nm, targs, ntargs);
        } else {
            base = type_named(p->arena, nm);
        }
    } else if (t.kind == TK_STRUCT) {
        /* In C headers, `struct Tag` is used as a type.
         * Parse it as a named type "struct Tag". */
        lex_next(p->lex); /* consume 'struct' */
        Tok tag = lex_next(p->lex);
        char nm[256];
        int n = snprintf(nm, sizeof(nm), "struct %.*s", tag.len, tag.start);
        base = type_named(p->arena, arena_strndup(p->arena, nm, n));
    } else if (t.kind == TK_UNION) {
        /* Same for `union Tag`. */
        lex_next(p->lex); /* consume 'union' */
        Tok tag = lex_next(p->lex);
        char nm[256];
        int n = snprintf(nm, sizeof(nm), "union %.*s", tag.len, tag.start);
        base = type_named(p->arena, arena_strndup(p->arena, nm, n));
    } else {
        errtok(p, &t, "expected type");
        return type_prim(p->arena, TY_VOID);
    }

    /* Apply leading const to the base type. */
    if (leading_const) base = type_const(p->arena, base);

    /* pointer suffix: 'T*' [const] [restrict] ('*' ... repeated)
     * Each `*` creates a new pointer type; an optional trailing `const`
     * or `restrict` applies to that pointer itself (e.g. `u8* const`). */
    while (accept(p, TK_STAR)) {
        base = type_ptr(p->arena, base);
        if (accept(p, TK_CONST)) {
            base = type_const(p->arena, base);
        }
        if (accept(p, TK___RESTRICT) || accept(p, TK___RESTRICT__) || accept(p, TK_RESTRICT)) {
            /* restrict qualifier on pointer — consumed and dropped
             * for now since Sharp's type system doesn't model restrict. */
        }
    }

    return base;
}

/* ===================================================================== *
 *   Phase C: C-style declarator system
 * ===================================================================== */

/* Parse declaration specifiers: type + storage-class + qualifiers.
 * Returns the base type. Consumes tokens like `const`, `extern`, `__thread`,
 * `__extension__`, `__declspec(...)`, calling conventions, etc.
 *
 * This is the FIRST HALF of a C declaration. The SECOND HALF is the
 * declarator (name + pointer/array/function suffixes). */
static Type* parse_decl_specifiers(P* p) {
    /* Storage-class and qualifier flags (not used for type construction,
     * but consumed to allow C headers to parse). */
    bool saw_extern = false;
    bool saw_static = false;
    bool saw_thread = false;
    bool saw_extension = false;

    /* Skip storage-class specifiers, __extension__, __declspec, calling conv, attributes */
    for (;;) {
        TokKind k = lex_peek(p->lex).kind;
        if (k == TK_EXTERN) { saw_extern = true; lex_next(p->lex); }
        else if (k == TK_STATIC) { saw_static = true; lex_next(p->lex); }
        else if (k == TK___INLINE__ || k == TK___INLINE) { lex_next(p->lex); }
        else if (k == TK___RESTRICT || k == TK___RESTRICT__ || k == TK_RESTRICT) { lex_next(p->lex); }
        else if (k == TK___DECLSPEC) { skip_declspec(p); }
        else if (k == TK___CDECL || k == TK___STDCALL || k == TK___FASTCALL || k == TK___UNALIGNED) {
            lex_next(p->lex);
        }
        /* __attribute__ can appear before return type in C headers:
         *   extern __attribute__((...)) int foo(...); */
        else if (k == TK___ATTRIBUTE__) { skip_attribute(p); }
        /* __extension__ keyword (GCC) — suppress warnings */
        else if (k == TK_IDENT && lex_ident_is(lex_peek(p->lex), "__extension__")) {
            saw_extension = true; lex_next(p->lex);
        }
        /* __thread / __declspec(thread) — thread-local storage */
        else if (k == TK_IDENT && lex_ident_is(lex_peek(p->lex), "__thread")) {
            saw_thread = true; lex_next(p->lex);
        }
        else { break; }
    }

    /* Now parse the actual type — delegate to existing parse_type() logic */
    return parse_type(p);
}

/* Forward declarations for recursive declarator parsing */
static Node* parse_expr(P* p);
static Type* parse_declarator_internal(P* p, Type* base, const char** out_name, bool abstract);

/* Parse a declarator: the part of a declaration that includes the name
 * and pointer/array/function suffixes.
 *
 *   declarator = pointer? direct_declarator
 *   direct_declarator = IDENT
 *                     | '(' declarator ')'
 *                     | direct_declarator '[' expr? ']'
 *                     | direct_declarator '(' param_list? ')'
 *   pointer = '*' type_qualifier* pointer?
 *
 * Returns the fully-constructed type (e.g., `int (*)(int)` for a function
 * pointer) and sets *out_name to the declared identifier (NULL for abstract).
 *
 * If `abstract` is true, the IDENT is optional (for casts, sizeof, etc.).
 */
static Type* parse_declarator_internal(P* p, Type* base, const char** out_name, bool abstract) {
    *out_name = NULL;

    /* ----- pointer prefix ----- */
    while (accept(p, TK_STAR)) {
        base = type_ptr(p->arena, base);
        /* Optional const/volatile/restrict after each '*' */
        for (;;) {
            TokKind k = lex_peek(p->lex).kind;
            if (k == TK_CONST) {
                lex_next(p->lex);
                base = type_const(p->arena, base);
            } else if (k == TK___RESTRICT || k == TK___RESTRICT__ || k == TK_RESTRICT) {
                lex_next(p->lex); /* discard for now */
            } else if (k == TK_IDENT && lex_ident_is(lex_peek(p->lex), "__restrict")) {
                lex_next(p->lex); /* discard */
            } else { break; }
        }
    }

    /* Skip __declspec that appears between type and declarator name.
     * MSVC allows: `char* __declspec(nothrow) _base;` */
    if (lex_peek(p->lex).kind == TK___DECLSPEC) {
        skip_declspec(p);
    }

    /* ----- direct declarator ----- */
    Tok t = lex_peek(p->lex);

    if (t.kind == TK_LPAREN) {
        /* Could be: ( declarator )  — parenthesized declarator
         * Or:     ( abstract_declarator )  — for function pointer params */
        LexerState saved = lex_save(p->lex);
        lex_next(p->lex); /* consume '(' */

        /* Try to parse an inner declarator. If it fails and we're abstract,
         * it might be a parameter list. */
        int saved_err = g_error_count;
        g_silent = true;
        const char* inner_name = NULL;
        Type* inner_type = parse_declarator_internal(p, base, &inner_name, abstract);
        g_silent = false;

        if (g_error_count == saved_err && lex_peek(p->lex).kind == TK_RPAREN) {
            /* Success: ( declarator ) */
            lex_next(p->lex); /* consume ')' */
            base = inner_type;
            if (inner_name) *out_name = inner_name;
        } else {
            /* Not a parenthesized declarator — restore and treat as function params */
            lex_restore(p->lex, saved);
            g_error_count = saved_err;
            /* Don't consume '(', let the suffix parser handle it */
        }
    } else if (t.kind == TK_IDENT && !abstract) {
        /* Named declarator */
        lex_next(p->lex);
        *out_name = arena_strndup(p->arena, t.start, t.len);
    } else if (t.kind == TK_IDENT && abstract) {
        /* In abstract declarator, an identifier could be a type name */
        /* Check if it looks like a type (already handled by base) */
        /* For pure abstract, we don't consume identifiers here */
    }

    /* Skip __attribute__ that appears after the declarator name.
     * C allows: `int x __attribute__((aligned(16)));` */
    skip_attribute(p);

    /* ----- suffixes: [array] and (function params) ----- */
    for (;;) {
        Tok s = lex_peek(p->lex);
        if (s.kind == TK_LBRACKET) {
            /* Array suffix: name[N] or name[] */
            lex_next(p->lex); /* consume '[' */
            if (accept(p, TK_RBRACKET)) {
                /* Unsized array — becomes pointer */
                base = type_ptr(p->arena, base);
            } else {
                /* Parse dimension expression, but for type construction
                 * we just make it a pointer (C arrays decay anyway). */
                Node* dim = parse_expr(p);
                (void)dim;
                expect(p, TK_RBRACKET, "expected ']' after array dimension");
                base = type_ptr(p->arena, base);
            }
        } else if (s.kind == TK_LPAREN) {
            /* Function parameter list */
            lex_next(p->lex); /* consume '(' */
            Type** params = NULL;
            int nparams = 0;
            int pcap = 0;
            bool variadic = false;

            if (lex_peek(p->lex).kind != TK_RPAREN) {
                for (;;) {
                    if (accept(p, TK_ELLIPSIS)) {
                        variadic = true;
                        if (!accept(p, TK_COMMA)) break;
                        continue;
                    }
                    /* Parse parameter type (abstract declarator allowed) */
                    Type* pty = parse_decl_specifiers(p);
                    const char* pname;
                    pty = parse_declarator_internal(p, pty, &pname, true);
                    if (pcap == nparams) {
                        pcap = pcap ? pcap * 2 : 4;
                        Type** nd = (Type**)malloc((size_t)pcap * sizeof(Type*));
                        if (nparams) memcpy(nd, params, (size_t)nparams * sizeof(Type*));
                        free(params);
                        params = nd;
                    }
                    params[nparams++] = pty;
                    if (!accept(p, TK_COMMA)) break;
                }
            }
            expect(p, TK_RPAREN, "expected ')' after parameter list");

            /* Construct function type.
             * If base is a pointer (from `*name` inside parentheses),
             * this means we have a function pointer: `ret (*name)(params)`.
             * The pointer's base is the return type, and we need to
             * wrap the function type in a pointer.
             * If base is not a pointer, it's the return type directly. */
            if (base && base->kind == TY_PTR) {
                Type* ret_type = base->base;
                bool ptr_is_const = base->is_const;
                base = type_func(p->arena, ret_type, params, nparams, variadic);
                base = type_ptr(p->arena, base);
                if (ptr_is_const) base = type_const(p->arena, base);
            } else {
                base = type_func(p->arena, base, params, nparams, variadic);
            }
            free(params);
        } else {
            break;
        }
    }

    return base;
}

/* Parse a full declarator (with name). */
static Type* parse_declarator(P* p, Type* base, const char** out_name) {
    return parse_declarator_internal(p, base, out_name, false);
}

/* Parse an abstract declarator (type only, no name). */
static Type* parse_abstract_declarator(P* p, Type* base) {
    const char* name;
    return parse_declarator_internal(p, base, &name, true);
}

/* Parse an optional `<T, U, ...>` type-parameter list after a struct/impl
 * name. Returns the number consumed and fills `*out_names` with an
 * arena-owned array. Emits no output when there are none. */
static int parse_type_params(P* p, const char*** out_names) {
    *out_names = NULL;
    if (!accept(p, TK_LT)) return 0;
    const char** buf = NULL; int n = 0, cap = 0;
    for (;;) {
        Tok id = expect(p, TK_IDENT, "expected type parameter name");
        if (n == cap) {
            cap = cap ? cap * 2 : 4;
            const char** nd = (const char**)malloc((size_t)cap * sizeof(char*));
            if (n) memcpy(nd, buf, (size_t)n * sizeof(char*));
            free(buf);
            buf = nd;
        }
        buf[n++] = arena_strndup(p->arena, id.start, id.len);
        if (!accept(p, TK_COMMA)) break;
    }
    expect(p, TK_GT, "expected '>' to close type parameters");
    if (n > 0) {
        const char** dst = ARENA_NEW_ARR(p->arena, const char*, n);
        memcpy(dst, buf, (size_t)n * sizeof(char*));
        *out_names = dst;
    }
    free(buf);
    return n;
}

/* ---------- forward decls ---------- */
static Node* parse_stmt(P* p);
static Node* parse_block(P* p);
static Node* parse_expr(P* p);
static Node* parse_assign(P* p);
static Node* parse_logic_or(P* p);
static Node* parse_logic_and(P* p);
static Node* parse_equality(P* p);
static Node* parse_compare(P* p);
static Node* parse_bor(P* p);
static Node* parse_bxor(P* p);
static Node* parse_band(P* p);
static Node* parse_shift(P* p);
static Node* parse_add(P* p);
static Node* parse_mul(P* p);
static Node* parse_unary(P* p);
static Node* parse_postfix(P* p);
static Node* parse_primary(P* p);
static Node* parse_init_list(P* p);

/* ---------- expression parsing ---------- */
static Node* parse_expr(P* p) { return parse_assign(p); }

static OpKind assign_op_for(TokKind k) {
    switch (k) {
        case TK_ASSIGN:    return OP_ASSIGN;
        case TK_PLUSEQ:    return OP_ADDEQ;
        case TK_MINUSEQ:   return OP_SUBEQ;
        case TK_STAREQ:    return OP_MULEQ;
        case TK_SLASHEQ:   return OP_DIVEQ;
        case TK_PERCENTEQ: return OP_MODEQ;
        default:           return OP_ASSIGN;
    }
}

static Node* parse_assign(P* p) {
    Node* lhs = parse_logic_or(p);
    Tok t = lex_peek(p->lex);
    switch (t.kind) {
        case TK_ASSIGN: case TK_PLUSEQ: case TK_MINUSEQ:
        case TK_STAREQ: case TK_SLASHEQ: case TK_PERCENTEQ: {
            lex_next(p->lex);
            Node* rhs = parse_assign(p);
            Node* n = mk(p, ND_ASSIGN, t.line);
            n->op  = assign_op_for(t.kind);
            n->lhs = lhs;
            n->rhs = rhs;
            return n;
        }
        default: return lhs;
    }
}

static Node* bin(P* p, OpKind op, Node* l, Node* r, int line) {
    Node* n = mk(p, ND_BINOP, line);
    n->op = op; n->lhs = l; n->rhs = r;
    return n;
}

static Node* parse_logic_or(P* p) {
    Node* l = parse_logic_and(p);
    while (lex_peek(p->lex).kind == TK_OR_OR) {
        Tok t = lex_next(p->lex);
        Node* r = parse_logic_and(p);
        l = bin(p, OP_OR, l, r, t.line);
    }
    return l;
}
static Node* parse_logic_and(P* p) {
    Node* l = parse_equality(p);
    while (lex_peek(p->lex).kind == TK_AND_AND) {
        Tok t = lex_next(p->lex);
        Node* r = parse_equality(p);
        l = bin(p, OP_AND, l, r, t.line);
    }
    return l;
}
static Node* parse_equality(P* p) {
    Node* l = parse_compare(p);
    for (;;) {
        Tok t = lex_peek(p->lex);
        OpKind op;
        if      (t.kind == TK_EQ)  op = OP_EQ;
        else if (t.kind == TK_NEQ) op = OP_NEQ;
        else break;
        lex_next(p->lex);
        Node* r = parse_compare(p);
        l = bin(p, op, l, r, t.line);
    }
    return l;
}
static Node* parse_compare(P* p) {
    Node* l = parse_bor(p);
    for (;;) {
        Tok t = lex_peek(p->lex);
        OpKind op;
        if      (t.kind == TK_LT) op = OP_LT;
        else if (t.kind == TK_GT) op = OP_GT;
        else if (t.kind == TK_LE) op = OP_LE;
        else if (t.kind == TK_GE) op = OP_GE;
        else break;
        lex_next(p->lex);
        Node* r = parse_bor(p);
        l = bin(p, op, l, r, t.line);
    }
    return l;
}
static Node* parse_bor(P* p) {
    Node* l = parse_bxor(p);
    while (lex_peek(p->lex).kind == TK_PIPE) {
        Tok t = lex_next(p->lex);
        Node* r = parse_bxor(p);
        l = bin(p, OP_BOR, l, r, t.line);
    }
    return l;
}
static Node* parse_bxor(P* p) {
    Node* l = parse_band(p);
    while (lex_peek(p->lex).kind == TK_CARET) {
        Tok t = lex_next(p->lex);
        Node* r = parse_band(p);
        l = bin(p, OP_BXOR, l, r, t.line);
    }
    return l;
}
static Node* parse_band(P* p) {
    Node* l = parse_shift(p);
    while (lex_peek(p->lex).kind == TK_AMP) {
        Tok t = lex_next(p->lex);
        Node* r = parse_shift(p);
        l = bin(p, OP_BAND, l, r, t.line);
    }
    return l;
}
static Node* parse_shift(P* p) {
    Node* l = parse_add(p);
    for (;;) {
        Tok t = lex_peek(p->lex);
        OpKind op;
        if      (t.kind == TK_SHL) op = OP_SHL;
        else if (t.kind == TK_SHR) op = OP_SHR;
        else break;
        lex_next(p->lex);
        Node* r = parse_add(p);
        l = bin(p, op, l, r, t.line);
    }
    return l;
}
static Node* parse_add(P* p) {
    Node* l = parse_mul(p);
    for (;;) {
        Tok t = lex_peek(p->lex);
        OpKind op;
        if      (t.kind == TK_PLUS)  op = OP_ADD;
        else if (t.kind == TK_MINUS) op = OP_SUB;
        else break;
        lex_next(p->lex);
        Node* r = parse_mul(p);
        l = bin(p, op, l, r, t.line);
    }
    return l;
}
static Node* parse_mul(P* p) {
    Node* l = parse_unary(p);
    for (;;) {
        Tok t = lex_peek(p->lex);
        OpKind op;
        if      (t.kind == TK_STAR)    op = OP_MUL;
        else if (t.kind == TK_SLASH)   op = OP_DIV;
        else if (t.kind == TK_PERCENT) op = OP_MOD;
        else break;
        lex_next(p->lex);
        Node* r = parse_unary(p);
        l = bin(p, op, l, r, t.line);
    }
    return l;
}
static Node* parse_unary(P* p) {
    Tok t = lex_peek(p->lex);
    OpKind op; bool is_unary = true;
    switch (t.kind) {
        case TK_NOT:   op = OP_NOT;   break;
        case TK_MINUS: op = OP_NEG;   break;
        case TK_PLUS:  op = OP_POS;   break;
        case TK_TILDE: op = OP_BNOT;  break;
        case TK_AMP:   op = OP_ADDR;  break;  /* &e : address-of */
        case TK_STAR:  op = OP_DEREF; break;  /* *e : dereference */
        default: is_unary = false; op = OP_POS; break;
    }
    if (is_unary) {
        lex_next(p->lex);
        Node* rhs = parse_unary(p);
        Node* n = mk(p, ND_UNOP, t.line);
        n->op  = op;
        n->rhs = rhs;
        return n;
    }
    /* sizeof — can take a type or an expression as argument */
    if (t.kind == TK_SIZEOF) {
        lex_next(p->lex); /* consume 'sizeof' */
        expect(p, TK_LPAREN, "expected '(' after sizeof");

        /* Try to parse as a type first. If successful and followed by ')',
         * it's sizeof(Type). Otherwise it's sizeof(expr). */
        LexerState saved = lex_save(p->lex);
        int saved_err = g_error_count;
        g_silent = true;
        Type* sz_ty = parse_type(p);
        g_silent = false;
        bool is_type = (sz_ty && g_error_count == saved_err &&
                        lex_peek(p->lex).kind == TK_RPAREN);

        if (is_type) {
            lex_restore(p->lex, saved);
            g_error_count = saved_err;
            /* Re-parse the type properly */
            Type* ty = parse_type(p);
            expect(p, TK_RPAREN, "expected ')' after sizeof type");
            Node* n = mk(p, ND_SIZEOF, t.line);
            n->declared_type = ty;
            return n;
        } else {
            /* Not a type — parse as expression */
            lex_restore(p->lex, saved);
            g_error_count = saved_err;
            Node* expr = parse_expr(p);
            expect(p, TK_RPAREN, "expected ')' after sizeof expr");
            Node* n = mk(p, ND_SIZEOF, t.line);
            n->rhs = expr;
            return n;
        }
    }
    return parse_postfix(p);
}

static Node* parse_postfix(P* p) {
    Node* e = parse_primary(p);
    for (;;) {
        Tok t = lex_peek(p->lex);
        /* Both `.field` and `->field` produce an ND_MEMBER node; cgen picks
         * `.` vs `->` at emission time based on the receiver's resolved
         * type. Accepting `->` here is a convenience for code that deals
         * in pointer-typed fields (e.g. `self->log->sum` inside a dtor). */
        if (t.kind == TK_DOT || t.kind == TK_ARROW) {
            lex_next(p->lex);
            Tok id = expect(p, TK_IDENT, "expected field name after '.' or '->'");
            Node* n = mk(p, ND_MEMBER, t.line);
            n->lhs  = e;
            n->name = arena_strndup(p->arena, id.start, id.len);
            e = n;
        } else if (t.kind == TK_LPAREN) {
            lex_next(p->lex);
            NodeVec args = {0};
            if (lex_peek(p->lex).kind != TK_RPAREN) {
                nv_push(&args, parse_expr(p));
                while (accept(p, TK_COMMA)) nv_push(&args, parse_expr(p));
            }
            expect(p, TK_RPAREN, "expected ')'");
            Node* n = mk(p, ND_CALL, t.line);
            n->callee = e;
            n->args   = nv_freeze(p, &args, &n->nargs);
            e = n;
        } else if (t.kind == TK_LBRACKET) {
            /* Raw-pointer subscript `a[i]`. Compiled as plain C `a[i]`. */
            lex_next(p->lex);
            Node* idx = parse_expr(p);
            expect(p, TK_RBRACKET, "expected ']' after index");
            Node* n = mk(p, ND_INDEX, t.line);
            n->lhs = e;
            n->rhs = idx;
            e = n;
        } else if (t.kind == TK_PLUSPLUS || t.kind == TK_MINUSMINUS) {
            lex_next(p->lex);
            Node* n = mk(p, ND_UNOP, t.line);
            n->op  = (t.kind == TK_PLUSPLUS) ? OP_POSTINC : OP_POSTDEC;
            n->rhs = e;
            e = n;
        } else {
            break;
        }
    }
    return e;
}

static Node* parse_struct_lit(P* p, const char* type_name, int line) {
    /* '{' already peeked, consume it */
    expect(p, TK_LBRACE, "expected '{'");
    Node* lit = mk(p, ND_STRUCT_LIT, line);
    lit->name = type_name;
    NodeVec fis = {0};

    if (lex_peek(p->lex).kind != TK_RBRACE) {
        for (;;) {
            int fl = lex_peek(p->lex).line;
            Node* fi = mk(p, ND_FIELD_INIT, fl);

            /* Lookahead: if we see IDENT followed by ':', it's a named init.
             * If we see IDENT followed by ',' or '}', it's shorthand
             * (treated as named init: `x` -> { .x = x }).
             * Otherwise fall through to a positional expression. */
            Tok t0 = lex_peek(p->lex);
            Tok t1 = lex_peek2(p->lex);
            if (t0.kind == TK_IDENT &&
                (t1.kind == TK_COLON || t1.kind == TK_COMMA || t1.kind == TK_RBRACE)) {
                Tok id = lex_next(p->lex);
                fi->name = arena_strndup(p->arena, id.start, id.len);
                if (accept(p, TK_COLON)) {
                    fi->rhs = parse_expr(p);
                } else {
                    /* shorthand: field name reused as value expression */
                    Node* ref_e = mk(p, ND_IDENT, id.line);
                    ref_e->name = fi->name;
                    fi->rhs = ref_e;
                }
            } else {
                /* positional */
                fi->name = NULL;
                fi->rhs  = parse_expr(p);
            }

            nv_push(&fis, fi);
            if (!accept(p, TK_COMMA)) break;
            if (lex_peek(p->lex).kind == TK_RBRACE) break;   /* trailing comma */
        }
    }
    expect(p, TK_RBRACE, "expected '}' to close struct literal");
    lit->children = nv_freeze(p, &fis, &lit->nchildren);
    return lit;
}

static Node* parse_primary(P* p) {
    Tok t = lex_peek(p->lex);
    switch (t.kind) {
        case TK_INT: {
            lex_next(p->lex);
            Node* n = mk(p, ND_INT, t.line);
            n->ival = t.ival;
            return n;
        }
        case TK_FLOAT: {
            lex_next(p->lex);
            Node* n = mk(p, ND_FLOAT, t.line);
            n->fval = t.fval;
            return n;
        }
        case TK_STRING: {
            lex_next(p->lex);
            Node* n = mk(p, ND_STR, t.line);
            n->sval = arena_strndup(p->arena, t.start, t.len);
            n->slen = t.len;
            return n;
        }
        case TK_CHAR_LIT: {
            lex_next(p->lex);
            Node* n = mk(p, ND_CHAR, t.line);
            n->sval = arena_strndup(p->arena, t.start, t.len);
            n->slen = t.len;
            return n;
        }
        case TK_TRUE:  lex_next(p->lex); { Node* n = mk(p, ND_BOOL, t.line); n->bval = true;  return n; }
        case TK_FALSE: lex_next(p->lex); { Node* n = mk(p, ND_BOOL, t.line); n->bval = false; return n; }
        case TK_NULL:  lex_next(p->lex); { Node* n = mk(p, ND_NULL, t.line); return n; }
        case TK_IDENT: {
            /* Phase 4 speculation: if the IDENT is followed by `<`, try to
             * parse it as a generic instantiation (Name<T, ...>). The result
             * is only kept when the closing `>` is followed by `.` (static
             * method call) or `{` (struct literal) — the two places where a
             * generic instantiation can syntactically surface inside an
             * expression.
             *
             * All errors inside the speculative parse are silenced; only a
             * full successful parse with the right continuation is accepted.
             * On any mismatch we lex_restore and fall back to plain ident. */
            if (lex_peek2(p->lex).kind == TK_LT) {
                LexerState saved = lex_save(p->lex);
                int saved_err    = g_error_count;
                g_silent         = true;
                Type* probe      = parse_type(p);
                g_silent         = false;
                Tok after        = lex_peek(p->lex);
                bool accept_it   = (probe && probe->kind == TY_NAMED &&
                                    probe->ntargs > 0 &&
                                    g_error_count == saved_err &&
                                    (after.kind == TK_DOT || after.kind == TK_LBRACE));
                if (!accept_it) {
                    lex_restore(p->lex, saved);
                    g_error_count = saved_err;
                    goto plain_ident;
                }
                if (after.kind == TK_LBRACE) {
                    Node* lit = parse_struct_lit(p, probe->name, t.line);
                    lit->type = probe;
                    return lit;
                }
                /* `.` case: synthesise a type-carrying ND_IDENT node. */
                Node* n = mk(p, ND_IDENT, t.line);
                n->name = probe->name;
                n->type = probe;
                return n;
            }

        plain_ident:
            /* Struct-literal lookahead: IDENT '{' -> struct literal */
            if (lex_peek2(p->lex).kind == TK_LBRACE) {
                Tok id = lex_next(p->lex);
                char* nm = arena_strndup(p->arena, id.start, id.len);
                return parse_struct_lit(p, nm, id.line);
            }
            lex_next(p->lex);
            Node* n = mk(p, ND_IDENT, t.line);
            n->name = arena_strndup(p->arena, t.start, t.len);
            return n;
        }
        case TK_LPAREN: {
            /* Three possibilities:
             * 1. (Type){ init_list } — C99 compound literal
             * 2. (Type)expr          — C-style type cast
             * 3. (expr)              — grouped expression
             * Try parsing a type after `(`; if successful and next token is
             * `)`, it's a cast. If next token is `{`, it's a compound literal.
             * Otherwise fall back to grouped expression. */
            LexerState saved = lex_save(p->lex);
            lex_next(p->lex); /* consume '(' */
            int saved_err    = g_error_count;
            g_silent         = true;
            Type* cast_ty    = parse_type(p);
            g_silent         = false;
            bool type_ok     = (cast_ty && g_error_count == saved_err);
            TokKind next     = lex_peek(p->lex).kind;

            /* Compound literal: (Type){ init_list }
             * After parsing the type, we expect ')' then '{'.
             * We need to peek two tokens ahead. */
            bool is_compound = false;
            if (type_ok && next == TK_RPAREN) {
                Tok after_rparen = lex_peek2(p->lex);
                is_compound = (after_rparen.kind == TK_LBRACE);
            }

            if (is_compound) {
                /* Compound literal: (Type){ init_list } */
                lex_restore(p->lex, saved);
                g_error_count = saved_err;
                lex_next(p->lex); /* consume '(' */
                Type* ty = parse_type(p);
                expect(p, TK_RPAREN, "expected ')' after compound literal type");
                Node* init = parse_init_list(p);
                Node* n = mk(p, ND_COMPOUND_LIT, t.line);
                n->declared_type = ty;
                n->rhs = init;
                return n;
            }

            bool is_cast = (type_ok && next == TK_RPAREN);
            if (!is_cast) {
                /* Not a cast — restore lexer to before '(' and parse as grouped expression */
                lex_restore(p->lex, saved);
                g_error_count = saved_err;
                lex_next(p->lex); /* re-consume '(' */
                Node* e = parse_expr(p);
                expect(p, TK_RPAREN, "expected ')'");
                return e;
            }
            /* It's a cast: consume the type + `)`, then parse operand */
            lex_restore(p->lex, saved);
            g_error_count = saved_err;
            lex_next(p->lex); /* consume '(' */
            Type* ty = parse_type(p);
            expect(p, TK_RPAREN, "expected ')' after cast type");
            /* Parse the cast operand — can be another parenthesized expr */
            Node* operand = parse_unary(p);
            Node* n = mk(p, ND_CAST, t.line);
            n->declared_type = ty;
            n->rhs = operand;
            return n;
        }
        case TK_PRINT:
        case TK_PRINTLN: {
            /* print(expr) / println(expr) built-in I/O.
             * Parsed here so they work as expressions (return void). */
            NodeKind pk = (t.kind == TK_PRINT) ? ND_PRINT : ND_PRINTLN;
            lex_next(p->lex);
            expect(p, TK_LPAREN, "expected '(' after print/println");
            Node* val = parse_expr(p);
            expect(p, TK_RPAREN, "expected ')'");
            Node* n = mk(p, pk, t.line);
            n->rhs = val;
            return n;
        }
        default:
            errtok(p, &t, "expected expression");
            lex_next(p->lex); /* skip to recover */
            return mk(p, ND_INT, t.line);
    }
    /* unreachable; suppresses -Wreturn-type */
    return mk(p, ND_INT, 0);
}

/* ---------- statements ---------- */

/* A statement starts with a type if:
 *  - first token is a primitive-type keyword, OR
 *  - first token is IDENT and second token is IDENT or '*'.
 * (For phase 1 that's enough; generics etc. belong to later phases.)
 */
/* Heuristically determine whether the next tokens begin a variable
 * declaration. Returns true for:
 *   prim_type IDENT/STAR
 *   IDENT IDENT          (struct-type variable)
 *   IDENT *              (pointer variable)
 *   IDENT < ... > IDENT  (generic-instantiation variable, phase 4)
 *
 * For the last case we do a silent speculative parse so the generic `<...>`
 * is parsed properly without exposing the 2-token lookahead limit. */
static bool looks_like_decl(P* p) {
    Tok t0 = lex_peek(p->lex);
    Tok t1 = lex_peek2(p->lex);
    if (is_type_start(t0.kind)) {
        if (t1.kind == TK_IDENT || t1.kind == TK_STAR || t1.kind == TK_LT) return true;
        /* Phase C: function pointer variable like `int (*fp)(int)`.
         * Only valid when t0 is a primitive type keyword (int, i32, etc.).
         * If t0 is TK_IDENT, it's almost certainly a function call. */
        if (t1.kind == TK_LPAREN) {
            if (t0.kind == TK_IDENT) return false;
            LexerState saved = lex_save(p->lex);
            int saved_err    = g_error_count;
            g_silent         = true;
            Type* base = parse_decl_specifiers(p);
            const char* name = NULL;
            Type* ty = parse_declarator(p, base, &name);
            g_silent         = false;
            bool ok = (g_error_count == saved_err && ty && name);
            /* Additional check: after the declarator, we expect ';' or '=' or ',' */
            if (ok) {
                TokKind k = lex_peek(p->lex).kind;
                ok = (k == TK_SEMI || k == TK_ASSIGN || k == TK_COMMA);
            }
            lex_restore(p->lex, saved);
            g_error_count    = saved_err;
            return ok;
        }
        /* Compound C type: "unsigned long", "long long", etc.
         * Speculatively parse the type and verify an identifier follows. */
        if (is_type_start(t1.kind) && t1.kind != TK_IDENT) {
            LexerState saved = lex_save(p->lex);
            int saved_err    = g_error_count;
            g_silent         = true;
            Type* ty         = parse_type(p);
            g_silent         = false;
            bool ok = (g_error_count == saved_err && ty &&
                       lex_peek(p->lex).kind == TK_IDENT);
            lex_restore(p->lex, saved);
            g_error_count    = saved_err;
            return ok;
        }
        return false;
    }
    if (t0.kind == TK_IDENT) {
        if (t1.kind == TK_IDENT) return true;
        if (t1.kind == TK_STAR)  return true;
        /* Phase 4: `Name<T, ...> varname` — speculatively parse the type
         * and check that a variable name follows. All errors are silenced
         * and the lexer is fully restored on both paths. */
        if (t1.kind == TK_LT) {
            LexerState saved = lex_save(p->lex);
            int saved_err    = g_error_count;
            g_silent         = true;
            Type* ty         = parse_type(p);
            g_silent         = false;
            bool ok = (g_error_count == saved_err && ty &&
                       ty->kind == TY_NAMED && ty->ntargs > 0 &&
                       lex_peek(p->lex).kind == TK_IDENT);
            lex_restore(p->lex, saved);
            g_error_count    = saved_err;
            return ok;
        }
        /* Phase C: function pointer variable like `int (*fp)(int)` —
         * NOT handled here. TK_IDENT followed by '(' is almost always a
         * function call, not a declaration. Function pointer declarations
         * with primitive type starts (e.g., `int (*fp)(int)`) are handled
         * in the `is_type_start(t0.kind)` branch above. */
        if (t1.kind == TK_LPAREN) {
            return false;
        }
    }
    return false;
}

/* Parse a C initializer list: { expr, .field = expr, [index] = expr, ... }
 * Supports designated initializers (.field = expr) and nested init lists. */
static Node* parse_init_list(P* p) {
    int line = lex_peek(p->lex).line;
    expect(p, TK_LBRACE, "expected '{' in initializer list");
    Node* list = mk(p, ND_INIT_LIST, line);
    NodeVec items = {0};
    while (lex_peek(p->lex).kind != TK_RBRACE && lex_peek(p->lex).kind != TK_EOF) {
        int iline = lex_peek(p->lex).line;
        /* Designated initializer: .field = expr */
        if (lex_peek(p->lex).kind == TK_DOT) {
            lex_next(p->lex); /* consume '.' */
            Tok field = expect(p, TK_IDENT, "expected field name after '.'");
            const char* fname = arena_strndup(p->arena, field.start, field.len);
            expect(p, TK_ASSIGN, "expected '=' after designated field name");
            Node* val = parse_expr(p);
            Node* d = mk(p, ND_DESIG_INIT, iline);
            d->name = fname;
            d->rhs = val;
            nv_push(&items, d);
        }
        /* Array designated initializer: [index] = expr */
        else if (lex_peek(p->lex).kind == TK_LBRACKET) {
            lex_next(p->lex); /* consume '[' */
            Node* idx = parse_expr(p);
            expect(p, TK_RBRACKET, "expected ']'");
            expect(p, TK_ASSIGN, "expected '=' after array designator");
            Node* val = parse_expr(p);
            Node* d = mk(p, ND_DESIG_INIT, iline);
            d->name = NULL; /* NULL name means array designator */
            d->lhs = idx;
            d->rhs = val;
            nv_push(&items, d);
        }
        /* Nested initializer list */
        else if (lex_peek(p->lex).kind == TK_LBRACE) {
            Node* nested = parse_init_list(p);
            nv_push(&items, nested);
        }
        /* Regular expression */
        else {
            Node* val = parse_expr(p);
            nv_push(&items, val);
        }
        if (!accept(p, TK_COMMA)) break;
        /* Allow trailing comma before '}' */
        if (lex_peek(p->lex).kind == TK_RBRACE) break;
    }
    expect(p, TK_RBRACE, "expected '}' in initializer list");
    list->children = nv_freeze(p, &items, &list->nchildren);
    return list;
}

static Node* parse_vardecl(P* p) {
    int line = lex_peek(p->lex).line;
    const char* cc = parse_calling_conv(p);

    /* Use declarator system for all declarations.
     * This handles function pointers, arrays, and simple types uniformly. */
    Type* base = parse_decl_specifiers(p);
    NodeVec decls = {0};
    for (;;) {
        const char* vname = NULL;
        Type* ty = parse_declarator(p, base, &vname);
        if (!vname) {
            Tok err_t = lex_peek(p->lex);
            errtok(p, &err_t, "expected variable name");
            break;
        }
        Node* init = NULL;
        if (accept(p, TK_ASSIGN)) {
            if (lex_peek(p->lex).kind == TK_LBRACE)
                init = parse_init_list(p);
            else
                init = parse_expr(p);
        }
        Node* n = mk(p, ND_VARDECL, line);
        n->declared_type = ty;
        n->name = vname;
        n->rhs = init;
        n->cc = cc;
        nv_push(&decls, n);
        if (!accept(p, TK_COMMA)) break;
    }
    expect(p, TK_SEMI, "expected ';' after variable declaration");
    if (decls.len == 1) return decls.data[0];
    Node* block = mk(p, ND_BLOCK, line);
    block->children = nv_freeze(p, &decls, &block->nchildren);
    return block;
}

/* Parse a local variable declaration with full C declarator support.
 * Handles `__extension__`, `__thread`, function pointers, etc. */
static Node* parse_vardecl_c(P* p) {
    int line = lex_peek(p->lex).line;

    /* Parse declaration specifiers (includes type + storage class) */
    Type* base_ty = parse_decl_specifiers(p);

    /* Parse comma-separated declarators */
    NodeVec decls = {0};
    for (;;) {
        const char* vname = NULL;
        Type* ty = parse_declarator(p, base_ty, &vname);

        if (!vname) {
            Tok t = lex_peek(p->lex);
            errtok(p, &t, "expected variable name");
            return mk(p, ND_BLOCK, line);
        }

        Node* init = NULL;
        if (accept(p, TK_ASSIGN)) {
            if (lex_peek(p->lex).kind == TK_LBRACE)
                init = parse_init_list(p);
            else
                init = parse_expr(p);
        }

        Node* n = mk(p, ND_VARDECL, line);
        n->declared_type = ty;
        n->name = vname;
        n->rhs  = init;
        nv_push(&decls, n);

        if (!accept(p, TK_COMMA)) break;
    }
    expect(p, TK_SEMI, "expected ';' after variable declaration");

    if (decls.len == 1) return decls.data[0];
    Node* block = mk(p, ND_BLOCK, line);
    block->children = nv_freeze(p, &decls, &block->nchildren);
    return block;
}

static Node* parse_if(P* p) {
    Tok t = lex_next(p->lex); /* 'if' */
    expect(p, TK_LPAREN, "expected '(' after 'if'");
    Node* cond = parse_expr(p);
    expect(p, TK_RPAREN, "expected ')' after if-condition");
    Node* thn = parse_stmt(p);
    Node* els = NULL;
    if (accept(p, TK_ELSE)) els = parse_stmt(p);
    Node* n = mk(p, ND_IF, t.line);
    n->cond = cond; n->then_b = thn; n->else_b = els;
    return n;
}

static Node* parse_while(P* p) {
    Tok t = lex_next(p->lex); /* 'while' */
    expect(p, TK_LPAREN, "expected '(' after 'while'");
    Node* cond = parse_expr(p);
    expect(p, TK_RPAREN, "expected ')' after while-condition");
    Node* body = parse_stmt(p);
    Node* n = mk(p, ND_WHILE, t.line);
    n->cond = cond; n->body = body;
    return n;
}

static Node* parse_for(P* p) {
    Tok t = lex_next(p->lex); /* 'for' */
    expect(p, TK_LPAREN, "expected '(' after 'for'");

    Node* init = NULL;
    if (!accept(p, TK_SEMI)) {
        if (looks_like_decl(p)) init = parse_vardecl(p);
        else if (lex_peek(p->lex).kind == TK_IDENT) {
            Tok ft = lex_peek(p->lex);
            size_t fl = (size_t)ft.len;
            bool is_storage_kw = ((fl == 13 && strncmp(ft.start, "__extension__", 13) == 0) ||
                                  (fl == 8 && strncmp(ft.start, "__thread", 8) == 0));
            if (is_storage_kw) init = parse_vardecl_c(p);
            else {
                Node* e = parse_expr(p);
                expect(p, TK_SEMI, "expected ';' after for-init");
                Node* es = mk(p, ND_EXPR_STMT, e->line);
                es->rhs = e;
                init = es;
            }
        }
        else {
            Node* e = parse_expr(p);
            expect(p, TK_SEMI, "expected ';' after for-init");
            Node* es = mk(p, ND_EXPR_STMT, e->line);
            es->rhs = e;
            init = es;
        }
    }
    Node* cond = NULL;
    if (!accept(p, TK_SEMI)) {
        cond = parse_expr(p);
        expect(p, TK_SEMI, "expected ';' after for-condition");
    }
    Node* upd = NULL;
    if (lex_peek(p->lex).kind != TK_RPAREN) upd = parse_expr(p);
    expect(p, TK_RPAREN, "expected ')'");
    Node* body = parse_stmt(p);

    Node* n = mk(p, ND_FOR, t.line);
    n->init = init; n->cond = cond; n->update = upd; n->body = body;
    return n;
}

static Node* parse_return(P* p) {
    Tok t = lex_next(p->lex); /* 'return' */
    Node* n = mk(p, ND_RETURN, t.line);
    if (lex_peek(p->lex).kind != TK_SEMI) n->rhs = parse_expr(p);
    expect(p, TK_SEMI, "expected ';' after return");
    return n;
}

static Node* parse_block(P* p) {
    Tok lb = expect(p, TK_LBRACE, "expected '{'");
    Node* blk = mk(p, ND_BLOCK, lb.line);
    NodeVec v = {0};
    while (lex_peek(p->lex).kind != TK_RBRACE && lex_peek(p->lex).kind != TK_EOF)
        nv_push(&v, parse_stmt(p));
    expect(p, TK_RBRACE, "expected '}'");
    blk->children = nv_freeze(p, &v, &blk->nchildren);
    return blk;
}

static Node* parse_stmt(P* p) {
    Tok t = lex_peek(p->lex);
    switch (t.kind) {
        case TK_LBRACE:   return parse_block(p);
        case TK_IF:       return parse_if(p);
        case TK_WHILE:    return parse_while(p);
        case TK_FOR:      return parse_for(p);
        case TK_RETURN:   return parse_return(p);
        case TK_BREAK: {
            lex_next(p->lex);
            expect(p, TK_SEMI, "expected ';' after 'break'");
            return mk(p, ND_BREAK, t.line);
        }
        case TK_CONTINUE: {
            lex_next(p->lex);
            expect(p, TK_SEMI, "expected ';' after 'continue'");
            return mk(p, ND_CONTINUE, t.line);
        }
        case TK_SEMI: {   /* empty stmt */
            lex_next(p->lex);
            return mk(p, ND_BLOCK, t.line);
        }
        case TK_ASM: {
            /* __asm__ ... ;  pass-through */
            int line = t.line;
            const char* start = t.start;
            lex_next(p->lex); /* consume '__asm__' */
            if (lex_peek(p->lex).kind == TK_IDENT && lex_ident_is(lex_peek(p->lex), "volatile")) {
                lex_next(p->lex);
            }
            /* Skip everything until matching ');' or ';' */
            if (lex_peek(p->lex).kind == TK_LPAREN) {
                int depth = 0;
                while (lex_peek(p->lex).kind != TK_EOF) {
                    Tok tok = lex_peek(p->lex);
                    if (tok.kind == TK_LPAREN) depth++;
                    else if (tok.kind == TK_RPAREN) {
                        depth--;
                        lex_next(p->lex);
                        if (depth <= 0) break;
                        continue;
                    }
                    lex_next(p->lex);
                }
            }
            expect(p, TK_SEMI, "expected ';' after __asm__");
            Node* n = mk(p, ND_ASM, line);
            const char* end = lex_peek(p->lex).start;
            size_t len = (size_t)(end - start);
            n->raw_text = arena_strndup(p->arena, start, (int)len);
            return n;
        }
        default:
            /* SEH: __try { ... } __except(expr) { ... } __finally { ... } */
            if (t.kind == TK_IDENT) {
                const char* id_str = t.start;
                size_t id_len = (size_t)t.len;
                bool is_seh_kw = ((id_len == 5 && strncmp(id_str, "__try", 5) == 0) ||
                                  (id_len == 8 && strncmp(id_str, "__except", 8) == 0) ||
                                  (id_len == 9 && strncmp(id_str, "__finally", 9) == 0));
                if (is_seh_kw) {
                    int line = t.line;
                    const char* start = t.start;
                    lex_next(p->lex);
                    /* skip optional (...) */
                    if (lex_peek(p->lex).kind == TK_LPAREN) {
                        int depth = 1;
                        lex_next(p->lex);
                        while (depth > 0 && lex_peek(p->lex).kind != TK_EOF) {
                            if (lex_peek(p->lex).kind == TK_LPAREN) depth++;
                            else if (lex_peek(p->lex).kind == TK_RPAREN) depth--;
                            lex_next(p->lex);
                        }
                    }
                    /* skip {...} */
                    if (lex_peek(p->lex).kind == TK_LBRACE) {
                        int depth = 1;
                        lex_next(p->lex);
                        while (depth > 0 && lex_peek(p->lex).kind != TK_EOF) {
                            if (lex_peek(p->lex).kind == TK_LBRACE) depth++;
                            else if (lex_peek(p->lex).kind == TK_RBRACE) depth--;
                            lex_next(p->lex);
                        }
                    }
                    /* Skip continuations: __except(expr) { ... } or __finally { ... } */
                    while (lex_peek(p->lex).kind != TK_EOF) {
                        Tok nxt = lex_peek(p->lex);
                        size_t nl = (size_t)nxt.len;
                        const char* ns = nxt.start;
                        bool is_except = (nl == 8 && strncmp(ns, "__except", 8) == 0);
                        bool is_finally = (nl == 9 && strncmp(ns, "__finally", 9) == 0);
                        if (!is_except && !is_finally) break;
                        lex_next(p->lex);
                        /* skip optional (...) */
                        if (lex_peek(p->lex).kind == TK_LPAREN) {
                            int depth = 1;
                            lex_next(p->lex);
                            while (depth > 0 && lex_peek(p->lex).kind != TK_EOF) {
                                if (lex_peek(p->lex).kind == TK_LPAREN) depth++;
                                else if (lex_peek(p->lex).kind == TK_RPAREN) depth--;
                                lex_next(p->lex);
                            }
                        }
                        /* skip {...} */
                        if (lex_peek(p->lex).kind == TK_LBRACE) {
                            int depth = 1;
                            lex_next(p->lex);
                            while (depth > 0 && lex_peek(p->lex).kind != TK_EOF) {
                                if (lex_peek(p->lex).kind == TK_LBRACE) depth++;
                                else if (lex_peek(p->lex).kind == TK_RBRACE) depth--;
                                lex_next(p->lex);
                            }
                        }
                    }
                    const char* end = lex_peek(p->lex).start;
                    if (end <= start) end = start + 1;
                    Node* n = mk(p, ND_ASM, line);
                    n->raw_text = arena_strndup(p->arena, start, (int)(end - start));
                    return n;
                }
            }
            if (looks_like_decl(p)) return parse_vardecl(p);
            /* Check for C-style declaration with storage-class keywords
             * like __extension__, __thread that looks_like_decl doesn't catch */
            if (t.kind == TK_IDENT) {
                const char* id_str = t.start;
                size_t id_len = (size_t)t.len;
                bool is_storage_kw = ((id_len == 13 && strncmp(id_str, "__extension__", 13) == 0) ||
                                      (id_len == 8 && strncmp(id_str, "__thread", 8) == 0));
                if (is_storage_kw) return parse_vardecl_c(p);
            }
            {
                Node* e = parse_expr(p);
                expect(p, TK_SEMI, "expected ';' after expression");
                Node* n = mk(p, ND_EXPR_STMT, t.line);
                n->rhs = e;
                return n;
            }
    }
}

/* ---------- top-level declarations ---------- */

/* Helper: check if a type is a struct or union named type */
static bool is_struct_or_union_type(Type* ty) {
    if (!ty) return false;
    if (ty->kind == TY_PTR) return is_struct_or_union_type(ty->base);
    /* Check if it's a const-wrapped type */
    if (ty->is_const && ty->base) return is_struct_or_union_type(ty->base);
    if (ty->kind != TY_NAMED || !ty->name) return false;
    return strncmp(ty->name, "struct ", 7) == 0 || strncmp(ty->name, "union ", 6) == 0;
}

static Node* parse_struct(P* p) {
    Tok t = lex_next(p->lex); /* 'struct' */
    Tok id = expect(p, TK_IDENT, "expected struct name");
    /* Phase 4: optional `<T, U>` generic parameter list. */
    const char** tps = NULL;
    int ntps = parse_type_params(p, &tps);

    /* Forward declaration: `struct Name;` — no body */
    if (lex_peek(p->lex).kind == TK_SEMI) {
        lex_next(p->lex); /* consume ';' */
        Node* s = mk(p, ND_STRUCT_FWD, t.line);
        s->name = arena_strndup(p->arena, id.start, id.len);
        return s;
    }

    expect(p, TK_LBRACE, "expected '{' in struct");
    NodeVec fields = {0};
    while (lex_peek(p->lex).kind != TK_RBRACE && lex_peek(p->lex).kind != TK_EOF) {
        int line = lex_peek(p->lex).line;

        /* Skip __declspec on field */
        const char* field_declspec = NULL;
        if (lex_peek(p->lex).kind == TK___DECLSPEC) {
            field_declspec = parse_declspec(p);
        }

        /* Check for nested struct/union definition:
         *   struct Inner { ... } field_name;
         *   union  { ... } field_name;   (anonymous)
         *   struct { ... };              (anonymous, no name) */
        if (lex_peek(p->lex).kind == TK_STRUCT || lex_peek(p->lex).kind == TK_UNION) {
            bool is_union = (lex_peek(p->lex).kind == TK_UNION);
            lex_next(p->lex); /* consume 'struct'/'union' */

            /* Check if next token is '{' (anonymous) or IDENT followed by '{' (named nested) */
            bool has_name = false;
            const char* nested_name = NULL;
            if (lex_peek(p->lex).kind == TK_IDENT) {
                Tok maybe_name = lex_peek(p->lex);
                Tok after = lex_peek2(p->lex);
                if (after.kind == TK_LBRACE || after.kind == TK_IDENT || after.kind == TK_STAR) {
                    has_name = true;
                    lex_next(p->lex);
                    nested_name = arena_strndup(p->arena, maybe_name.start, maybe_name.len);
                }
            }

            if (lex_peek(p->lex).kind == TK_LBRACE) {
                /* Parse the nested struct/union body */
                lex_next(p->lex); /* consume '{' */
                NodeVec nested_fields = {0};
                while (lex_peek(p->lex).kind != TK_RBRACE && lex_peek(p->lex).kind != TK_EOF) {
                    /* Recursively parse fields (simplified: type + name + ;) */
                    int fline = lex_peek(p->lex).line;
                    const char* fd = NULL;
                    if (lex_peek(p->lex).kind == TK___DECLSPEC) {
                        fd = parse_declspec(p);
                    }
                    Type* fbase = parse_decl_specifiers(p);
                    const char* fn = NULL;
                    Type* fty = parse_declarator(p, fbase, &fn);
                    int bw = 0;
                    if (accept(p, TK_COLON)) {
                        Tok w = expect(p, TK_INT, "expected bitfield width");
                        bw = (int)w.ival;
                        fty = type_bitfield(p->arena, fty, bw);
                    }
                    expect(p, TK_SEMI, "expected ';' after field");
                    Node* nf = mk(p, ND_FIELD, fline);
                    nf->declared_type = fty;
                    nf->name = fn ? fn : "";
                    nf->bit_width = bw;
                    nf->declspec = fd;
                    nv_push(&nested_fields, nf);
                }
                expect(p, TK_RBRACE, "expected '}' to close nested struct/union");

                /* Build the nested struct/union type name */
                char su_name[256];
                if (has_name) {
                    snprintf(su_name, sizeof(su_name), "%s %s", is_union ? "union" : "struct", nested_name);
                } else {
                    static int anon_counter = 0;
                    snprintf(su_name, sizeof(su_name), "__anon_%s_%d", is_union ? "union" : "struct", anon_counter++);
                }
                Type* su_type = type_named(p->arena, arena_strndup(p->arena, su_name, (int)strlen(su_name)));

                /* Check for field name after the closing brace */
                const char* fname = NULL;
                if (lex_peek(p->lex).kind == TK_IDENT) {
                    Tok ft = lex_next(p->lex);
                    fname = arena_strndup(p->arena, ft.start, ft.len);
                }

                expect(p, TK_SEMI, "expected ';' after field");

                if (!fname) {
                    /* Anonymous struct/union field */
                    Node* f = mk(p, is_union ? ND_ANON_UNION : ND_ANON_STRUCT, line);
                    f->declared_type = su_type;
                    f->name = "";
                    f->declspec = field_declspec;
                    /* Store nested fields for code generation */
                    f->fields = nv_freeze(p, &nested_fields, &f->nfields);
                    nv_push(&fields, f);
                } else {
                    /* Named field of nested struct/union type */
                    Node* f = mk(p, ND_FIELD, line);
                    f->declared_type = su_type;
                    f->name = fname;
                    f->declspec = field_declspec;
                    f->fields = nv_freeze(p, &nested_fields, &f->nfields);
                    nv_push(&fields, f);
                }
                continue;
            } else {
                /* Not a nested definition — put back 'struct'/'union' and fall through */
                /* We already consumed 'struct'/'union' and possibly the name.
                 * Reconstruct the type using parse_type's logic. */
                if (has_name) {
                    /* We have 'struct Name' but no '{' — it's a type reference */
                    char su_name[256];
                    snprintf(su_name, sizeof(su_name), "%s %s", is_union ? "union" : "struct", nested_name);
                    Type* su_type = type_named(p->arena, arena_strndup(p->arena, su_name, (int)strlen(su_name)));

                    /* Parse pointer suffixes and declarator */
                    while (accept(p, TK_STAR)) {
                        su_type = type_ptr(p->arena, su_type);
                        if (accept(p, TK_CONST)) su_type = type_const(p->arena, su_type);
                    }

                    const char* fname = NULL;
                    Type* fty = parse_declarator(p, su_type, &fname);
                    int bw = 0;
                    if (accept(p, TK_COLON)) {
                        Tok w = expect(p, TK_INT, "expected bitfield width");
                        bw = (int)w.ival;
                        fty = type_bitfield(p->arena, fty, bw);
                    }
                    expect(p, TK_SEMI, "expected ';' after field");
                    Node* f = mk(p, ND_FIELD, line);
                    f->declared_type = fty;
                    f->name = fname ? fname : "";
                    f->bit_width = bw;
                    f->declspec = field_declspec;
                    nv_push(&fields, f);
                    continue;
                }
                /* No name and no brace — error recovery */
            }
        }

        /* Parse field using declarator system */
        Type* base = parse_decl_specifiers(p);
        const char* fname = NULL;
        Type* fty = parse_declarator(p, base, &fname);

        /* Check for bitfield */
        int bit_width = 0;
        if (accept(p, TK_COLON)) {
            Tok w = expect(p, TK_INT, "expected bitfield width");
            bit_width = (int)w.ival;
            fty = type_bitfield(p->arena, fty, bit_width);
        }

        expect(p, TK_SEMI, "expected ';' after field");

        /* Determine node kind */
        Node* f;
        if (!fname) {
            /* Anonymous struct/union field */
            if (is_struct_or_union_type(fty)) {
                bool is_union = (fty->name && strncmp(fty->name, "union ", 6) == 0);
                f = mk(p, is_union ? ND_ANON_UNION : ND_ANON_STRUCT, line);
            } else {
                /* Anonymous non-struct field — error, but create ND_FIELD for recovery */
                f = mk(p, ND_FIELD, line);
                f->name = "";
            }
        } else {
            f = mk(p, ND_FIELD, line);
            f->name = fname;
        }
        f->declared_type = fty;
        f->bit_width = bit_width;
        f->declspec = field_declspec;
        nv_push(&fields, f);
    }
    expect(p, TK_RBRACE, "expected '}' to close struct");
    accept(p, TK_SEMI); /* optional ';' after struct body */

    Node* s = mk(p, ND_STRUCT_DECL, t.line);
    s->name = arena_strndup(p->arena, id.start, id.len);
    s->fields = nv_freeze(p, &fields, &s->nfields);
    s->type_params  = tps;
    s->ntype_params = ntps;
    return s;
}

/* Parse a union declaration: `union Name { ... };` or `union Name;` (forward).
 * Unlike structs, unions in Sharp are opaque C pass-through types. */
static Node* parse_union(P* p) {
    Tok t = lex_next(p->lex); /* 'union' */
    Tok id = expect(p, TK_IDENT, "expected union name");

    /* Forward declaration: `union Name;` */
    if (lex_peek(p->lex).kind == TK_SEMI) {
        lex_next(p->lex); /* consume ';' */
        Node* u = mk(p, ND_UNION_FWD, t.line);
        u->name = arena_strndup(p->arena, id.start, id.len);
        return u;
    }

    expect(p, TK_LBRACE, "expected '{' in union");
    NodeVec fields = {0};
    while (lex_peek(p->lex).kind != TK_RBRACE && lex_peek(p->lex).kind != TK_EOF) {
        int line = lex_peek(p->lex).line;

        /* Skip __declspec on field */
        const char* field_declspec = NULL;
        if (lex_peek(p->lex).kind == TK___DECLSPEC) {
            field_declspec = parse_declspec(p);
        }

        /* Parse field using declarator system */
        Type* base = parse_decl_specifiers(p);
        const char* fname = NULL;
        Type* fty = parse_declarator(p, base, &fname);

        /* Check for bitfield */
        int bit_width = 0;
        if (accept(p, TK_COLON)) {
            Tok w = expect(p, TK_INT, "expected bitfield width");
            bit_width = (int)w.ival;
            fty = type_bitfield(p->arena, fty, bit_width);
        }

        expect(p, TK_SEMI, "expected ';' after field");

        Node* f = mk(p, ND_FIELD, line);
        f->declared_type = fty;
        f->name = fname ? fname : "";
        f->bit_width = bit_width;
        f->declspec = field_declspec;
        nv_push(&fields, f);
    }
    expect(p, TK_RBRACE, "expected '}' to close union");
    expect(p, TK_SEMI, "expected ';' after union");

    Node* u = mk(p, ND_UNION_DECL, t.line);
    u->name = arena_strndup(p->arena, id.start, id.len);
    u->fields = nv_freeze(p, &fields, &u->nfields);
    return u;
}

/* Map an operator-token kind to its canonical method name.
 * Returns NULL if the token cannot follow the 'operator' keyword. */
static const char* operator_method_name(TokKind k) {
    switch (k) {
        case TK_PLUS:    return "op_add";
        case TK_MINUS:   return "op_sub";   /* param count disambiguates unary -- see sema */
        case TK_STAR:    return "op_mul";
        case TK_SLASH:   return "op_div";
        case TK_PERCENT: return "op_mod";
        case TK_EQ:      return "op_eq";
        case TK_NEQ:     return "op_neq";
        case TK_LT:      return "op_lt";
        case TK_GT:      return "op_gt";
        case TK_LE:      return "op_le";
        case TK_GE:      return "op_ge";
        case TK_NOT:     return "op_not";   /* unary only */
        case TK_TILDE:   return "op_bnot";  /* unary only */
        default:         return NULL;
    }
}

/* Parse the name portion of a method:
 *   - `IDENT`            -> arena-owned copy of the identifier
 *   - `operator <OP>`    -> the canonical op_* name (only inside impl blocks)
 * Always advances past the tokens it consumes, even on error (to help recovery). */
static const char* parse_method_name(P* p, bool allow_operator) {
    Tok t = lex_peek(p->lex);
    if (t.kind == TK_OPERATOR) {
        if (!allow_operator) {
            errtok(p, &t, "'operator' is only allowed inside impl blocks");
        }
        lex_next(p->lex);
        Tok op = lex_peek(p->lex);
        const char* nm = operator_method_name(op.kind);
        if (!nm) {
            errtok(p, &op, "expected an overloadable operator after 'operator'");
            return "op_unknown";
        }
        lex_next(p->lex);
        return nm;
    }
    Tok id = expect(p, TK_IDENT, "expected method name");
    return arena_strndup(p->arena, id.start, id.len);
}

/* Parse a function or method declaration.
 *
 * parent:      NULL for free functions; struct name for methods inside `impl`.
 * is_static:   already-consumed `static` modifier (only meaningful when parent!=NULL).
 *
 * Implicit self handling (only when parent != NULL && !is_static):
 *   - If the first declared param is literally named `self`, use it verbatim
 *     (its type kind determines value vs. ref self).
 *   - Otherwise prepend an implicit value-self param `parent self`.
 */
static Node* parse_func_common(P* p, const char* parent, bool is_static) {
    int line = lex_peek(p->lex).line;
    const char* cc1 = parse_calling_conv(p);
    Type* ret = parse_type(p);
    const char* cc2 = parse_calling_conv(p);
    const char* cc = cc1 ? cc1 : cc2;
    const char* name = parse_method_name(p, parent != NULL);
    expect(p, TK_LPAREN, "expected '('");

    NodeVec params = {0};
    if (lex_peek(p->lex).kind != TK_RPAREN) {
        for (;;) {
            int pline = lex_peek(p->lex).line;

            /* Skip __declspec on parameter */
            if (lex_peek(p->lex).kind == TK___DECLSPEC) {
                skip_declspec(p);
            }

            /* Use declarator system for parameter parsing */
            Type* base = parse_decl_specifiers(p);
            const char* pname = NULL;
            Type* pty = parse_declarator(p, base, &pname);

            /* If no name parsed (e.g., abstract declarator in function pointer),
             * generate a default name. */
            if (!pname) pname = "p";

            Node* pr = mk(p, ND_PARAM, pline);
            pr->declared_type = pty;
            pr->name = pname;
            nv_push(&params, pr);
            if (!accept(p, TK_COMMA)) break;
        }
    }
    expect(p, TK_RPAREN, "expected ')'");

    /* Function body: C-style block `{ stmts }` */
    Node* body = parse_block(p);

    /* Determine self_kind and possibly prepend an implicit self param. */
    int self_kind = SELF_NONE;
    if (parent && !is_static) {
        int n = params.len;
        Node* first = n > 0 ? params.data[0] : NULL;
        bool first_is_self = first && strcmp(first->name, "self") == 0;
        if (first_is_self) {
            Type* ft = first->declared_type;
            /* `T* self` → ref-self (pointer to parent struct).
             * Anything else (value type) → value-self. */
            self_kind = (ft && ft->kind == TY_PTR) ? SELF_REF : SELF_VALUE;
        } else {
            /* prepend implicit `parent self` */
            Node* pr = mk(p, ND_PARAM, line);
            pr->declared_type = type_named(p->arena, parent);
            pr->name = "self";
            /* shift right */
            NodeVec shifted = {0};
            nv_push(&shifted, pr);
            for (int i = 0; i < params.len; i++) nv_push(&shifted, params.data[i]);
            free(params.data);
            params = shifted;
            self_kind = SELF_VALUE;
        }
    }

    Node* f = mk(p, ND_FUNC_DECL, line);
    f->ret_type    = ret;
    f->name        = name;
    f->params      = nv_freeze(p, &params, &f->nparams);
    f->body        = body;
    f->parent_type = parent;
    f->self_kind   = self_kind;
    f->cc          = cc;
    return f;
}

static Node* parse_func(P* p) { return parse_func_common(p, NULL, false); }

/* Parse a destructor: `~TypeName () [block | '=>' expr ';']`
 *
 * Internal representation: a ND_FUNC_DECL whose name is the sentinel string
 * "~" (not a legal identifier, so user methods cannot collide), with an
 * implicit `ref Parent self` parameter and `void` return type. Cgen then
 * mangles the C output name to `Parent___drop`. */
static Node* parse_dtor(P* p, const char* parent) {
    Tok tilde = lex_next(p->lex); /* '~' */
    Tok id = expect(p, TK_IDENT, "expected struct name after '~'");

    /* Verify the destructor name matches the impl type. */
    size_t parent_len = strlen(parent);
    if ((size_t)id.len != parent_len || memcmp(id.start, parent, parent_len) != 0) {
        error_at(id.line,
                 "destructor name '~%.*s' does not match impl type '%s'",
                 id.len, id.start, parent);
    }

    expect(p, TK_LPAREN, "expected '(' after destructor name");
    if (lex_peek(p->lex).kind != TK_RPAREN) {
        Tok bad = lex_peek(p->lex);
        errtok(p, &bad, "destructor takes no parameters");
        /* consume until ')' for recovery */
        while (lex_peek(p->lex).kind != TK_RPAREN &&
               lex_peek(p->lex).kind != TK_EOF)
            lex_next(p->lex);
    }
    expect(p, TK_RPAREN, "expected ')' in destructor");

    Node* body = parse_block(p);

    /* Synthesize implicit `Parent* self` — destructor always receives
     * a pointer to the object being destroyed. */
    Node* self_param = mk(p, ND_PARAM, tilde.line);
    self_param->declared_type = type_ptr(p->arena, type_named(p->arena, parent));
    self_param->name = "self";

    Node* f = mk(p, ND_FUNC_DECL, tilde.line);
    f->ret_type    = type_prim(p->arena, TY_VOID);
    f->name        = "~";                 /* sentinel: not a valid identifier */
    f->params      = ARENA_NEW_ARR(p->arena, Node*, 1);
    f->params[0]   = self_param;
    f->nparams     = 1;
    f->body        = body;
    f->parent_type = parent;
    f->self_kind   = SELF_REF;
    return f;
}

static Node* parse_impl(P* p) {
    Tok t = lex_next(p->lex); /* 'impl' */
    Tok id = expect(p, TK_IDENT, "expected type name after 'impl'");
    char* parent = arena_strndup(p->arena, id.start, id.len);
    /* Phase 4: `impl Name<T, U>` introduces the same type parameters as the
     * struct declaration did. The grammar accepts them verbatim; they flow
     * into every method body as in-scope type names during monomorphisation. */
    const char** tps = NULL;
    int ntps = parse_type_params(p, &tps);
    expect(p, TK_LBRACE, "expected '{' after impl type");

    NodeVec methods = {0};
    while (lex_peek(p->lex).kind != TK_RBRACE && lex_peek(p->lex).kind != TK_EOF) {
        /* Destructor: '~TypeName () ...'. No 'static' allowed. */
        if (lex_peek(p->lex).kind == TK_TILDE) {
            Node* d = parse_dtor(p, parent);
            nv_push(&methods, d);
            continue;
        }

        bool is_static = accept(p, TK_STATIC);
        /* A method declaration must begin with a type (primitive keyword
         * or ident). The return-type parser handles both. */
        Tok look = lex_peek(p->lex);
        if (!is_type_start(look.kind)) {
            errtok(p, &look, "expected method declaration");
            lex_next(p->lex);
            continue;
        }
        Node* m = parse_func_common(p, parent, is_static);
        nv_push(&methods, m);
    }
    expect(p, TK_RBRACE, "expected '}' to close impl");

    Node* impl = mk(p, ND_IMPL, t.line);
    impl->name         = parent;
    impl->children     = nv_freeze(p, &methods, &impl->nchildren);
    impl->type_params  = tps;
    impl->ntype_params = ntps;
    return impl;
}

Node* parse_program(Lexer* lx, Arena** arena) {
    P pp = { .lex = lx, .arena = arena };
    P* p = &pp;

    Node* prog = mk(p, ND_PROGRAM, 1);
    NodeVec decls = {0};
    while (lex_peek(p->lex).kind != TK_EOF) {
        Tok t = lex_peek(p->lex);
        if (t.kind == TK_STRUCT) {
            nv_push(&decls, parse_struct(p));
        } else if (t.kind == TK_UNION) {
            nv_push(&decls, parse_union(p));
        } else if (t.kind == TK_IMPL) {
            nv_push(&decls, parse_impl(p));
        } else if (t.kind == TK_TYPEDEF) {
            lex_next(p->lex); /* consume 'typedef' */
            int line = t.line;

            /* C-style: `typedef struct { ... } Name;` or `typedef union { ... } Name;` */
            if (lex_peek(p->lex).kind == TK_STRUCT || lex_peek(p->lex).kind == TK_UNION) {
                bool is_union = (lex_peek(p->lex).kind == TK_UNION);
                Tok t0 = lex_next(p->lex); /* consume 'struct'/'union' */

                /* Anonymous: `typedef struct { ... } Name;` — generate a name */
                if (lex_peek(p->lex).kind == TK_LBRACE) {
                    /* Generate an internal name */
                    static int anon_counter = 0;
                    char anon_name[64];
                    snprintf(anon_name, sizeof(anon_name), "__anon_%s_%d", is_union ? "union" : "struct", anon_counter++);

                    expect(p, TK_LBRACE, is_union ? "expected '{' in union" : "expected '{' in struct");
                    NodeVec fields = {0};
                    while (lex_peek(p->lex).kind != TK_RBRACE && lex_peek(p->lex).kind != TK_EOF) {
                        int fline = lex_peek(p->lex).line;
                        Type* ty = parse_type(p);
                        Tok fid = expect(p, TK_IDENT, "expected field name");
                        expect(p, TK_SEMI, "expected ';' after field");
                        Node* f = mk(p, ND_FIELD, fline);
                        f->declared_type = ty;
                        f->name = arena_strndup(p->arena, fid.start, fid.len);
                        nv_push(&fields, f);
                    }
                    expect(p, TK_RBRACE, is_union ? "expected '}' to close union" : "expected '}' to close struct");
                    /* No ';' here — typedef alias follows */

                    const char* aname = arena_strndup(p->arena, anon_name, strlen(anon_name));
                    Node* su = mk(p, is_union ? ND_UNION_DECL : ND_STRUCT_DECL, t0.line);
                    su->name = aname;
                    su->fields = nv_freeze(p, &fields, &su->nfields);

                    Tok alias = expect(p, TK_IDENT, "expected typedef name after struct/union");
                    expect(p, TK_SEMI, "expected ';' after typedef");

                    Node* nd = mk(p, ND_TYPEDEF_DECL, line);
                    nd->declared_type = type_named(p->arena, aname);
                    nd->name = arena_strndup(p->arena, alias.start, alias.len);
                    nv_push(&decls, nd);
                    nv_push(&decls, su);
                } else {
                    /* Named struct: `typedef struct Tag Name;` or `typedef struct Tag *Name;`
                     * We already consumed 'struct', now parse the tag name.
                     * Use declarator system to handle pointer typedefs like:
                     *   typedef struct threadlocaleinfostruct *pthreadlocinfo; */
                    Tok tag = expect(p, TK_IDENT, "expected struct tag name");
                    char nm[256];
                    int n = snprintf(nm, sizeof(nm), "struct %.*s", tag.len, tag.start);
                    Type* base = type_named(p->arena, arena_strndup(p->arena, nm, n));

                    /* Parse comma-separated declarators: `typedef struct Tag A, *B;` */
                    for (;;) {
                        const char* tname = NULL;
                        Type* ty = parse_declarator(p, base, &tname);
                        
                        if (!tname) {
                            Tok err_t = lex_peek(p->lex);
                            errtok(p, &err_t, "expected typedef name");
                            break;
                        }

                        Node* nd = mk(p, ND_TYPEDEF_DECL, line);
                        nd->declared_type = ty;
                        nd->name = tname;
                        nv_push(&decls, nd);

                        if (!accept(p, TK_COMMA)) break;
                    }

                    expect(p, TK_SEMI, "expected ';' after typedef");
                }
            } else {
                /* Use declarator system for typedef parsing */
                Type* base = parse_decl_specifiers(p);

                /* Parse comma-separated declarators: `typedef T A, *B, **C;` */
                for (;;) {
                    const char* tname = NULL;
                    Type* ty = parse_declarator(p, base, &tname);

                    if (!tname) {
                        Tok err_t = lex_peek(p->lex);
                        errtok(p, &err_t, "expected typedef name");
                        break;
                    }

                    /* Array dimensions: `typedef T Name[N];` */
                    Node* dim = NULL;
                    if (accept(p, TK_LBRACKET)) {
                        if (!accept(p, TK_RBRACKET)) {
                            dim = parse_expr(p);
                            expect(p, TK_RBRACKET, "expected ']'");
                        }
                    }

                    Node* nd = mk(p, ND_TYPEDEF_DECL, line);
                    nd->declared_type = ty;
                    nd->name = tname;
                    nd->lhs = dim;
                    nv_push(&decls, nd);

                    if (!accept(p, TK_COMMA)) break;
                }

                /* Optional trailing __attribute__((...)) */
                skip_attribute(p);
                expect(p, TK_SEMI, "expected ';' after typedef");
            }
        } else if (t.kind == TK_STATIC_ASSERT) {
            /* _Static_assert(constant_expr, "message"); — pass-through to C */
            int line = t.line;
            const char* start = t.start;
            lex_next(p->lex); /* consume '_Static_assert' */
            expect(p, TK_LPAREN, "expected '(' after _Static_assert");
            /* Skip everything until matching `);` — expression may contain parentheses */
            int depth = 1;
            while (depth > 0) {
                Tok tok = lex_peek(p->lex);
                if (tok.kind == TK_EOF) {
                    errtok(p, &tok, "unterminated _Static_assert");
                    break;
                }
                if (tok.kind == TK_LPAREN) depth++;
                else if (tok.kind == TK_RPAREN) depth--;
                lex_next(p->lex);
            }
            expect(p, TK_SEMI, "expected ';' after _Static_assert");
            Node* sa = mk(p, ND_STATIC_ASSERT, line);
            /* Capture original text from start to current position */
            const char* end = lex_peek(p->lex).start;
            size_t len = (size_t)(end - start);
            sa->raw_text = arena_strndup(p->arena, start, (int)len);
            nv_push(&decls, sa);
        } else if (t.kind == TK_EXTERN) {
            /* Check for extern linkage block: `extern "C++" { ... }` */
            LexerState saved = lex_save(p->lex);
            lex_next(p->lex); /* consume 'extern' */
            if (lex_peek(p->lex).kind == TK_STRING && lex_peek2(p->lex).kind == TK_LBRACE) {
                /* extern "linkage" { ... } block — skip entirely */
                lex_next(p->lex); /* consume string literal */
                expect(p, TK_LBRACE, "expected '{' after extern linkage");
                int depth = 1;
                while (depth > 0 && lex_peek(p->lex).kind != TK_EOF) {
                    if (lex_peek(p->lex).kind == TK_LBRACE) depth++;
                    else if (lex_peek(p->lex).kind == TK_RBRACE) depth--;
                    lex_next(p->lex);
                }
            } else {
                /* Not a linkage block — restore and let is_type_start handle it */
                lex_restore(p->lex, saved);
                goto type_start_path;
            }
        } else if (is_type_start(t.kind)) {
type_start_path:
            /* Peek ahead to distinguish between:
             * - `Type name(params);` — C function declaration
             * - `Type name(params) { ... }` — function definition
             * - `Type name => expr;` — Sharp expression function
             * - `Type name = expr;` — global variable with init
             * - `Type name;` — global variable without init
             * - `Type name[N];` — global array */
            LexerState saved = lex_save(p->lex);
            int saved_err = g_error_count;
            g_silent = true;

            /* Skip leading modifiers that can appear before the actual type. */
            const char* declspec = NULL;
            bool saw_extern_top = false;
            /* Skip TK_EXTERN and TK___INLINE__ prefixes */
            for (;;) {
                TokKind k = lex_peek(p->lex).kind;
                if (k == TK_EXTERN) { saw_extern_top = true; lex_next(p->lex); }
                else if (k == TK___INLINE__ || k == TK___INLINE) {
                    lex_next(p->lex);
                } else if (k == TK___DECLSPEC) {
                    declspec = parse_declspec(p);
                } else if (k == TK___CDECL || k == TK___STDCALL || k == TK___FASTCALL || k == TK___UNALIGNED) {
                    lex_next(p->lex);
                } else if (k == TK___ATTRIBUTE__) {
                    skip_attribute(p);
                } else {
                    break;
                }
            }

            const char* cc = parse_calling_conv(p);
            Type* ret = parse_type(p);
            const char* cc2 = parse_calling_conv(p);
            if (!cc) cc = cc2;

            /* Save state AFTER type parsing but BEFORE declarator.
             * This is needed for the complex declarator path: `Type (*name)[];` */
            LexerState after_type = lex_save(p->lex);

            const char* fname = NULL;
            if (lex_peek(p->lex).kind == TK_IDENT) {
                fname = parse_method_name(p, false);
            } else if (lex_peek(p->lex).kind == TK_LPAREN) {
                /* Complex declarator: `(*name)` - will be handled below */
            }
            bool has_paren = (lex_peek(p->lex).kind == TK_LPAREN);

            g_silent = false;

            if (has_paren && fname) {
                /* Parse the parameter list */
                lex_next(p->lex); /* consume '(' */
                NodeVec params = {0};
                bool is_variadic = false;
                int param_idx = 0;
                if (lex_peek(p->lex).kind != TK_RPAREN) {
                    for (;;) {
                        /* Variadic parameter: `...` */
                        if (accept(p, TK_ELLIPSIS)) {
                            is_variadic = true;
                            if (!accept(p, TK_COMMA)) break;
                            continue;
                        }
                        int pline = lex_peek(p->lex).line;

                        /* Use declarator system for parameter parsing */
                        Type* pbase = parse_decl_specifiers(p);
                        const char* pname = NULL;
                        Type* pty = parse_declarator(p, pbase, &pname);

                        /* Handle `void` parameter: `func(void)` means no parameters */
                        if (pbase->kind == TY_VOID && !pname) {
                            if (accept(p, TK_COMMA)) continue;
                            break;
                        }

                        /* Generate unique parameter name if missing */
                        if (!pname) {
                            char buf[32];
                            snprintf(buf, sizeof(buf), "p%d", param_idx);
                            pname = arena_strndup(p->arena, buf, strlen(buf));
                        }

                        Node* pr = mk(p, ND_PARAM, pline);
                        pr->declared_type = pty;
                        pr->name = pname;
                        nv_push(&params, pr);
                        param_idx++;
                        if (!accept(p, TK_COMMA)) break;
                    }
                }
                bool has_rparen = accept(p, TK_RPAREN);
                g_silent = false;

                if (has_rparen) {
                    /* Optional trailing __attribute__((...)) */
                    skip_attribute(p);
                    if (lex_peek(p->lex).kind == TK_SEMI) {
                        /* C function declaration: `int foo(int x);` */
                        lex_next(p->lex); /* consume ';' */
                        Node* ext = mk(p, ND_EXTERN_DECL, t.line);
                        ext->ret_type = ret;
                        ext->name = fname;
                        ext->params = nv_freeze(p, &params, &ext->nparams);
                        ext->is_variadic = is_variadic;
                        ext->cc = cc;
                        ext->declspec = declspec;
                        nv_push(&decls, ext);
                        g_error_count = saved_err;
                        continue;
                    } else if (lex_peek(p->lex).kind == TK_LBRACE ||
                           lex_peek(p->lex).kind == TK_ARROW) {
                        /* Sharp function definition — restore and parse normally */
                        lex_restore(p->lex, saved);
                        g_error_count = saved_err;
                        free(params.data); /* speculative parse discarded */
                        nv_push(&decls, parse_func(p));
                        continue;
                    }
                    /* Neither ';' nor '{'/'→' — speculative parse failed,
                     * restore and fall through to variable parsing */
                    free(params.data);
                    lex_restore(p->lex, saved);
                    g_error_count = saved_err;
                } else {
                    /* Missing ')' — speculative parse failed */
                    free(params.data);
                    lex_restore(p->lex, saved);
                    g_error_count = saved_err;
                }
            } else if (fname) {
                /* Not a function — could be global variable or array.
                 * Use already-parsed type (ret) and name (fname) from lookahead. */
                g_silent = false;

                const char* vname = fname;
                int line = t.line;

                if (accept(p, TK_LBRACKET)) {
                    /* Global array: `Type name[N];` or `Type name[];` */
                    Node* dim = NULL;
                    if (accept(p, TK_RBRACKET)) {
                        /* Unsized extern array — treat as pointer (C array decay) */
                        ret = type_ptr(p->arena, ret);
                    } else {
                        /* Sized array — parse dimension */
                        dim = parse_expr(p);
                        expect(p, TK_RBRACKET, "expected ']'");
                    }
                    expect(p, TK_SEMI, "expected ';' after array declaration");
                    Node* nd = mk(p, saw_extern_top ? ND_EXTERN_VAR : ND_VARDECL, line);
                    nd->declared_type = ret;
                    nd->name = vname;
                    nd->lhs = dim;
                    nd->declspec = declspec;
                    nv_push(&decls, nd);
                } else if (accept(p, TK_ASSIGN)) {
                    /* Global variable with init: `Type name = expr;` or `Type name = { ... };` */
                    Node* val = NULL;
                    if (lex_peek(p->lex).kind == TK_LBRACE)
                        val = parse_init_list(p);
                    else
                        val = parse_expr(p);
                    expect(p, TK_SEMI, "expected ';' after variable declaration");
                    Node* nd = mk(p, ND_VARDECL, line);
                    nd->declared_type = ret;
                    nd->name = vname;
                    nd->rhs = val;
                    nd->declspec = declspec;
                    nv_push(&decls, nd);
                } else {
                    /* Global variable without init: `Type name;`
                     * If __declspec(dllimport) or `extern` was seen, treat as extern var.
                     * __declspec(thread) is a storage modifier, not extern. */
                    bool is_extern_decl = saw_extern_top ||
                        (declspec && strcmp(declspec, "dllimport") == 0);
                    /* Skip trailing __attribute__ */
                    skip_attribute(p);
                    expect(p, TK_SEMI, "expected ';' after variable declaration");
                    Node* nd = mk(p, is_extern_decl ? ND_EXTERN_VAR : ND_VARDECL, line);
                    nd->declared_type = ret;
                    nd->name = vname;
                    nd->declspec = declspec;
                    nv_push(&decls, nd);
                }
                continue;
            } else if (!fname && has_paren) {
                /* Complex declarator: `Type (*name)[];` or `Type (*name)(params);`
                 * e.g., `extern FILE (*_imp___iob)[];`
                 * This is a pointer-to-array or pointer-to-function variable. */

                /* Restore to after-type position so we can re-parse the declarator */
                lex_restore(p->lex, after_type);

                /* Check if next is `(*` */
                bool is_complex_declarator = false;
                if (lex_peek(p->lex).kind == TK_LPAREN) {
                    LexerState peek_save = lex_save(p->lex);
                    lex_next(p->lex); /* consume '(' */
                    if (lex_peek(p->lex).kind == TK_STAR) {
                        is_complex_declarator = true;
                    }
                    lex_restore(p->lex, peek_save);
                }

                if (is_complex_declarator) {

                    /* Consume `(*` */
                    lex_next(p->lex); /* '(' */
                    lex_next(p->lex); /* '*' */

                    /* Get the name */
                    Tok name_tok = lex_peek(p->lex);
                    const char* vname = NULL;
                    if (name_tok.kind == TK_IDENT) {
                        /* Copy the identifier name into the arena */
                        char* buf = (char*)arena_alloc(p->arena, name_tok.len + 1);
                        memcpy(buf, name_tok.start, name_tok.len);
                        buf[name_tok.len] = '\0';
                        vname = buf;
                        lex_next(p->lex);
                    }

                    /* Consume `)` */
                    expect(p, TK_RPAREN, "expected ')' in complex declarator");

                    int line = t.line;

                    /* Check what follows: `[]` or `()` */
                    if (accept(p, TK_LBRACKET)) {
                        /* Pointer to array: `Type (*name)[];` or `Type (*name)[N];` */
                        Node* dim = NULL;
                        if (!accept(p, TK_RBRACKET)) {
                            dim = parse_expr(p);
                            expect(p, TK_RBRACKET, "expected ']'");
                        }
                        /* The type is pointer to array, but for C code gen we treat
                         * it as a pointer to the base type (array decay) */
                        Type* pty = type_ptr(p->arena, ret);
                        expect(p, TK_SEMI, "expected ';' after complex declarator");
                        Node* nd = mk(p, ND_EXTERN_VAR, line);
                        nd->declared_type = pty;
                        nd->name = vname ? vname : "";
                        nd->lhs = dim;
                        nd->declspec = declspec;
                        nv_push(&decls, nd);
                    } else if (accept(p, TK_LPAREN)) {
                        /* Pointer to function: `Type (*name)(params);` */
                        NodeVec params = {0};
                        bool is_variadic = false;
                        if (lex_peek(p->lex).kind != TK_RPAREN) {
                            for (;;) {
                                if (accept(p, TK_ELLIPSIS)) {
                                    is_variadic = true;
                                    if (!accept(p, TK_COMMA)) break;
                                    continue;
                                }
                                int pline = lex_peek(p->lex).line;
                                Type* pbase = parse_decl_specifiers(p);
                                const char* pname = NULL;
                                Type* pty = parse_declarator(p, pbase, &pname);
                                if (!pname) pname = "p";
                                Node* pr = mk(p, ND_PARAM, pline);
                                pr->declared_type = pty;
                                pr->name = pname;
                                nv_push(&params, pr);
                                if (!accept(p, TK_COMMA)) break;
                            }
                        }
                        expect(p, TK_RPAREN, "expected ')' in function pointer declarator");
                        /* Create function pointer type */
                        Type* func_ty = type_func(p->arena, ret, NULL, 0, false);
                        Type* pty = type_ptr(p->arena, func_ty);
                        expect(p, TK_SEMI, "expected ';' after function pointer declarator");
                        Node* nd = mk(p, ND_EXTERN_DECL, line);
                        nd->ret_type = pty;
                        nd->name = vname ? vname : "";
                        nd->params = nv_freeze(p, &params, &nd->nparams);
                        nd->is_variadic = is_variadic;
                        nd->cc = cc;
                        nd->declspec = declspec;
                        nv_push(&decls, nd);
                    } else {
                        expect(p, TK_SEMI, "expected ';' after complex declarator");
                        /* Just a pointer variable: `Type (*name);` = `Type *name;` */
                        Type* pty = type_ptr(p->arena, ret);
                        Node* nd = mk(p, saw_extern_top ? ND_EXTERN_VAR : ND_VARDECL, line);
                        nd->declared_type = pty;
                        nd->name = vname ? vname : "";
                        nd->declspec = declspec;
                        nv_push(&decls, nd);
                    }
                    continue;
                }
            }

            /* Fallback: not a recognizable declaration — restore and try parse_func */
            lex_restore(p->lex, saved);
            g_error_count = saved_err;
            g_silent = false;
            nv_push(&decls, parse_func(p));
        } else {
            errtok(p, &t, "expected top-level declaration");
            lex_next(p->lex); /* skip to recover */
        }
    }
    prog->children = nv_freeze(p, &decls, &prog->nchildren);
    return prog;
}

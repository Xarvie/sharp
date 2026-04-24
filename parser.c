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

    /* Lookahead phase: scan up to 4 tokens without consuming */
    int n = 0;
    for (int i = 0; i < 4; i++) {
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
                    i = 3; /* for-loop i++ makes it 4, terminating the loop */
                } else {
                    has_long = true;
                }
                break;
            }
            case TK_INT_TYPE:   has_int   = true; break;
            case TK_FLOAT_TYPE: has_float = true; break;
            case TK_DOUBLE_TYPE:has_double = true; break;
            case TK___INT64:    /* standalone __int64 → 64-bit signed */
                                return TY_I64;
            case TK_WCHAR_T:    /* wchar_t → named type (platform-dependent size) */
                                lex_next(p->lex);
                                return TY_NAMED;  /* will be handled by caller */
            default: consumed = false; break;
        }
        if (consumed) n = i + 1; /* count consumed tokens */
    }

    /* Consume all recognized tokens */
    for (int i = 0; i < n; i++) lex_next(p->lex);

    /* Resolve to a TypeKind */
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

static bool is_type_start(TokKind k) {
    return is_prim_tk(k) || k == TK_IDENT ||
           k == TK_SHORT || k == TK_LONG || k == TK_UNSIGNED || k == TK_SIGNED ||
           k == TK_INT_TYPE || k == TK_FLOAT_TYPE || k == TK_DOUBLE_TYPE ||
           k == TK___INT64 || k == TK_WCHAR_T ||
           k == TK_CONST;
}

static Type* parse_type(P* p) {
    /* Leading `const` — applies to the innermost (base) type. */
    bool leading_const = false;
    if (accept(p, TK_CONST)) leading_const = true;

    Type* base;
    Tok t = lex_peek(p->lex);
    if (t.kind == TK___INT64) {
        lex_next(p->lex);
        base = type_prim(p->arena, TY_I64);
    } else if (t.kind == TK_WCHAR_T) {
        lex_next(p->lex);
        base = type_named(p->arena, "wchar_t");
    } else if (is_c_type_mod(t.kind)) {
        /* C-style type with modifiers: unsigned int, long long, etc. */
        base = type_prim(p->arena, parse_c_type(p));
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

    /* pointer suffix: 'T*' [const] ('*' [const] repeated)
     * Each `*` creates a new pointer type; an optional trailing `const`
     * applies to that pointer itself (e.g. `u8* const`). */
    while (accept(p, TK_STAR)) {
        base = type_ptr(p->arena, base);
        if (accept(p, TK_CONST)) {
            base = type_const(p->arena, base);
        }
    }

    return base;
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
            lex_next(p->lex);
            Node* e = parse_expr(p);
            expect(p, TK_RPAREN, "expected ')'");
            return e;
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
        return t1.kind == TK_IDENT || t1.kind == TK_STAR || is_type_start(t1.kind);
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
    }
    return false;
}

static Node* parse_vardecl(P* p) {
    int line = lex_peek(p->lex).line;
    Type* ty = parse_type(p);
    Tok id = expect(p, TK_IDENT, "expected variable name");
    Node* init = NULL;
    if (accept(p, TK_ASSIGN)) init = parse_expr(p);
    expect(p, TK_SEMI, "expected ';' after variable declaration");

    Node* n = mk(p, ND_VARDECL, line);
    n->declared_type = ty;
    n->name = arena_strndup(p->arena, id.start, id.len);
    n->rhs  = init;
    return n;
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
        default:
            if (looks_like_decl(p)) return parse_vardecl(p);
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

static Node* parse_struct(P* p) {
    Tok t = lex_next(p->lex); /* 'struct' */
    Tok id = expect(p, TK_IDENT, "expected struct name");
    /* Phase 4: optional `<T, U>` generic parameter list. */
    const char** tps = NULL;
    int ntps = parse_type_params(p, &tps);
    expect(p, TK_LBRACE, "expected '{' in struct");
    NodeVec fields = {0};
    while (lex_peek(p->lex).kind != TK_RBRACE && lex_peek(p->lex).kind != TK_EOF) {
        int line = lex_peek(p->lex).line;
        Type* ty = parse_type(p);
        Tok fid = expect(p, TK_IDENT, "expected field name");
        expect(p, TK_SEMI, "expected ';' after field");
        Node* f = mk(p, ND_FIELD, line);
        f->declared_type = ty;
        f->name = arena_strndup(p->arena, fid.start, fid.len);
        nv_push(&fields, f);
    }
    expect(p, TK_RBRACE, "expected '}' to close struct");

    Node* s = mk(p, ND_STRUCT_DECL, t.line);
    s->name = arena_strndup(p->arena, id.start, id.len);
    s->fields = nv_freeze(p, &fields, &s->nfields);
    s->type_params  = tps;
    s->ntype_params = ntps;
    return s;
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
    Type* ret = parse_type(p);
    const char* name = parse_method_name(p, parent != NULL);
    expect(p, TK_LPAREN, "expected '('");

    NodeVec params = {0};
    if (lex_peek(p->lex).kind != TK_RPAREN) {
        for (;;) {
            int pline = lex_peek(p->lex).line;
            Type* pty = parse_type(p);
            Tok pid = expect(p, TK_IDENT, "expected parameter name");
            Node* pr = mk(p, ND_PARAM, pline);
            pr->declared_type = pty;
            pr->name = arena_strndup(p->arena, pid.start, pid.len);
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
        } else if (t.kind == TK_IMPL) {
            nv_push(&decls, parse_impl(p));
        } else if (t.kind == TK_EXTERN) {
            /* Phase 7: `extern RetType name(params);` — C-linkage function declaration.
             *            `extern Type name;` or `extern Type name[];` — C-linkage variable declaration.
             * After 'extern' + type, check if next token is '(' (function) or identifier (variable). */
            lex_next(p->lex);  /* consume 'extern' */
            int line = lex_peek(p->lex).line;
            Type* ret = parse_type(p);
            const char* ename = parse_method_name(p, false);
            if (lex_peek(p->lex).kind == TK_LPAREN) {
                /* Extern function declaration */
                lex_next(p->lex); /* consume '(' */
                NodeVec params = {0};
                if (lex_peek(p->lex).kind != TK_RPAREN) {
                    for (;;) {
                        int pline = lex_peek(p->lex).line;
                        Type* pty = parse_type(p);
                        const char* pname = "p";
                        if (lex_peek(p->lex).kind == TK_IDENT) {
                            Tok pid = lex_next(p->lex);
                            pname = arena_strndup(p->arena, pid.start, pid.len);
                        }
                        Node* pr = mk(p, ND_PARAM, pline);
                        pr->declared_type = pty;
                        pr->name = pname;
                        nv_push(&params, pr);
                        if (!accept(p, TK_COMMA)) break;
                    }
                }
                expect(p, TK_RPAREN, "expected ')'");
                expect(p, TK_SEMI, "expected ';' after extern declaration");
                Node* ext = mk(p, ND_EXTERN_DECL, line);
                ext->ret_type = ret;
                ext->name     = ename;
                ext->params   = nv_freeze(p, &params, &ext->nparams);
                nv_push(&decls, ext);
            } else {
                /* Extern variable declaration: `extern Type name;` or `extern Type name[];` */
                bool is_array = false;
                if (accept(p, TK_LBRACKET)) {
                    is_array = true;
                    expect(p, TK_RBRACKET, "expected ']' in extern array");
                }
                expect(p, TK_SEMI, "expected ';' after extern variable");
                Node* ext = mk(p, ND_EXTERN_VAR, line);
                ext->name = ename;
                ext->declared_type = is_array ? type_ptr(p->arena, ret) : ret;
                nv_push(&decls, ext);
            }
        } else if (t.kind == TK_TYPEDEF) {
            lex_next(p->lex); /* consume 'typedef' */
            Type* base = parse_type(p);
            Tok name_tok = expect(p, TK_IDENT, "expected typedef name");
            const char* tname = arena_strndup(p->arena, name_tok.start, name_tok.len);
            expect(p, TK_SEMI, "expected ';' after typedef");
            Node* nd = mk(p, ND_TYPEDEF_DECL, t.line);
            nd->declared_type = base;
            nd->name = tname;
            nv_push(&decls, nd);
        } else if (is_type_start(t.kind)) {
            /* Peek ahead to distinguish between:
             * - `Type name(params);` — C function declaration (extern, no body)
             * - `Type name(params) { ... }` — Sharp function definition
             * We do speculative parsing: parse the signature, then check
             * if the next token is `;` (declaration) or `{` (definition). */
            LexerState saved = lex_save(p->lex);
            int saved_err = g_error_count;
            g_silent = true;

            Type* ret = parse_type(p);
            const char* fname = parse_method_name(p, false);
            bool has_paren = (lex_peek(p->lex).kind == TK_LPAREN);

            g_silent = false;

            if (has_paren && fname) {
                /* Parse the parameter list */
                lex_next(p->lex); /* consume '(' */
                int param_line = lex_peek(p->lex).line;
                NodeVec params = {0};
                if (lex_peek(p->lex).kind != TK_RPAREN) {
                    for (;;) {
                        int pline = lex_peek(p->lex).line;
                        Type* pty = parse_type(p);
                        /* Parameter name is optional in C declarations. */
                        const char* pname = "p";
                        if (lex_peek(p->lex).kind == TK_IDENT) {
                            Tok pid = lex_next(p->lex);
                            pname = arena_strndup(p->arena, pid.start, pid.len);
                        }
                        Node* pr = mk(p, ND_PARAM, pline);
                        pr->declared_type = pty;
                        pr->name = pname;
                        nv_push(&params, pr);
                        if (!accept(p, TK_COMMA)) break;
                    }
                }
                bool has_rparen = accept(p, TK_RPAREN);
                g_silent = false;

                if (has_rparen) {
                    if (lex_peek(p->lex).kind == TK_SEMI) {
                        /* C function declaration: `int foo(int x);` */
                        lex_next(p->lex); /* consume ';' */
                        Node* ext = mk(p, ND_EXTERN_DECL, t.line);
                        ext->ret_type = ret;
                        ext->name = fname;
                        ext->params = nv_freeze(p, &params, &ext->nparams);
                        nv_push(&decls, ext);
                        g_error_count = saved_err;
                        continue;
                    } else if (lex_peek(p->lex).kind == TK_LBRACE ||
                               lex_peek(p->lex).kind == TK_EQ) {
                        /* Sharp function definition — restore and parse normally */
                        lex_restore(p->lex, saved);
                        g_error_count = saved_err;
                        nv_push(&decls, parse_func(p));
                        continue;
                    }
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

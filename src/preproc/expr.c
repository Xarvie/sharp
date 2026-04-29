/*
 * expr.c — Constant-expression evaluator for #if / #elif
 *
 * Implements the full C11 preprocessing constant-expression grammar:
 *
 *   expr      ::= ternary
 *   ternary   ::= logor ('?' expr ':' ternary)?
 *   logor     ::= logand ('||' logand)*
 *   logand    ::= bitor  ('&&' bitor)*
 *   bitor     ::= bitxor ('|'  bitxor)*
 *   bitxor    ::= bitand ('^'  bitand)*
 *   bitand    ::= equal  ('&'  equal)*
 *   equal     ::= relat  (('=='|'!=') relat)*
 *   relat     ::= shift  (('<'|'>'|'<='|'>=') shift)*
 *   shift     ::= add    (('<<'|'>>') add)*
 *   add       ::= mul    (('+'|'-') mul)*
 *   mul       ::= unary  (('*'|'/'|'%') unary)*
 *   unary     ::= ('+'|'-'|'~'|'!') unary | primary
 *   primary   ::= INT | CHAR | '(' expr ')' | 'defined' '(' NAME ')' | 'defined' NAME
 *
 * All arithmetic is performed in intmax_t / uintmax_t (§6.10.1 p4).
 * No floating-point is permitted in preprocessing constant expressions.
 */

#include "cpp_internal.h"
#include "expr.h"
#include "macro.h"

#include <inttypes.h>

typedef intmax_t  ival;
typedef uintmax_t uval;

/* =========================================================================
 * Token cursor over a TokList (skips whitespace automatically)
 * ====================================================================== */

typedef struct {
    TokNode      *cur;
    MacroTable   *mt;
    InternTable  *interns;
    CppDiagArr *diags;
    bool          had_error;
} EvalCtx;

static const PPTok *ec_peek(EvalCtx *ec) {
    TokNode *n = ec->cur;
    while (n && (n->tok.kind == CPPT_SPACE || n->tok.kind == CPPT_NEWLINE ||
                 n->tok.kind == CPPT_COMMENT))
        n = n->next;
    return n ? &n->tok : NULL;
}

static void ec_skip_ws(EvalCtx *ec) {
    while (ec->cur && (ec->cur->tok.kind == CPPT_SPACE ||
                       ec->cur->tok.kind == CPPT_NEWLINE ||
                       ec->cur->tok.kind == CPPT_COMMENT))
        ec->cur = ec->cur->next;
}

static const PPTok *ec_get(EvalCtx *ec) {
    ec_skip_ws(ec);
    if (!ec->cur) return NULL;
    const PPTok *t = &ec->cur->tok;
    ec->cur = ec->cur->next;
    return t;
}

static bool ec_accept(EvalCtx *ec, const char *sp) {
    ec_skip_ws(ec);
    if (!ec->cur) return false;
    if (strcmp(pptok_spell(&ec->cur->tok), sp) == 0) {
        ec->cur = ec->cur->next;
        return true;
    }
    return false;
}

static void ec_error(EvalCtx *ec, CppLoc loc, const char *msg) {
    CppDiag d = { CPP_DIAG_ERROR, loc, cpp_xstrdup(msg) };
    da_push(ec->diags, d);
    ec->had_error = true;
}

/* =========================================================================
 * Parse an integer constant (pp-number token)
 * ====================================================================== */

static ival parse_int_literal(const char *sp, bool *is_unsigned) {
    /* Handle 0x, 0b, 0 prefixes, and ULL/LL suffixes. */
    const char *p = sp;
    int base = 10;
    uval val = 0;
    *is_unsigned = false;

    if (*p == '0') {
        p++;
        if (*p == 'x' || *p == 'X') { base = 16; p++; }
        else if (*p == 'b' || *p == 'B') { base = 2; p++; }
        else base = 8;
    }

    for (; *p; p++) {
        char c = (char)tolower((unsigned char)*p);
        int digit;
        if      (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = 10 + (c - 'a');
        else break; /* suffix */
        if (digit >= base) break;
        val = val * (uval)base + (uval)digit;
    }
    /* Suffixes */
    while (*p == 'u' || *p == 'U') { *is_unsigned = true; p++; }
    while (*p == 'l' || *p == 'L') p++;
    while (*p == 'u' || *p == 'U') { *is_unsigned = true; p++; }

    return (ival)val;
}

/* =========================================================================
 * Parse a character constant (returns its numeric value)
 * ====================================================================== */

static ival parse_char_const(const char *sp) {
    /* Strip prefix (L, u, U, u8) and quotes. */
    while (*sp && *sp != '\'') sp++;
    if (*sp == '\'') sp++;
    if (*sp == '\\') {
        sp++;
        switch (*sp) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"':  return '"';
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'v':  return '\v';
        case 'x': {
            uval v = 0;
            sp++;
            while (isxdigit((unsigned char)*sp)) {
                v = v*16 + (uval)(isdigit((unsigned char)*sp)
                                  ? *sp-'0' : tolower((unsigned char)*sp)-'a'+10);
                sp++;
            }
            return (ival)v;
        }
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': {
            /* Octal escape: \ddd (up to 3 digits) */
            uval v = 0;
            int count = 0;
            while (count < 3 && *sp >= '0' && *sp <= '7') {
                v = v*8 + (uval)(*sp - '0');
                sp++; count++;
            }
            return (ival)v;
        }
        default: return *sp;
        }
    }
    return (ival)(unsigned char)*sp;
}

/* =========================================================================
 * Recursive descent evaluator
 * ====================================================================== */

static ival eval_expr(EvalCtx *ec);

static ival eval_primary(EvalCtx *ec) {
    ec_skip_ws(ec);
    const PPTok *t = ec_get(ec);
    if (!t) { ec->had_error = true; return 0; }

    /* Parenthesised expression */
    if (t->kind == CPPT_PUNCT && strcmp(pptok_spell(t), "(") == 0) {
        ival v = eval_expr(ec);
        if (!ec_accept(ec, ")")) {
            ec_error(ec, t->loc, "expected ')' in #if expression");
        }
        return v;
    }

    /* defined(NAME) or defined NAME */
    if (t->kind == CPPT_IDENT && strcmp(pptok_spell(t), "defined") == 0) {
        bool paren = ec_accept(ec, "(");
        ec_skip_ws(ec);
        const PPTok *name_tok = ec_get(ec);
        if (!name_tok || name_tok->kind != CPPT_IDENT) {
            ec_error(ec, t->loc, "expected macro name after 'defined'");
            if (paren) ec_accept(ec, ")");
            return 0;
        }
        const char *name = pptok_spell(name_tok);
        if (paren && !ec_accept(ec, ")"))
            ec_error(ec, t->loc, "expected ')' after defined(name)");
        return macro_lookup(ec->mt, name) ? 1 : 0;
    }

    /* Integer constant */
    if (t->kind == CPPT_PP_NUMBER) {
        bool uns = false;
        return parse_int_literal(pptok_spell(t), &uns);
    }

    /* Character constant */
    if (t->kind == CPPT_CHAR_CONST) {
        return parse_char_const(pptok_spell(t));
    }

    /* Unexpanded identifier — evaluates to 0 (§6.10.1 p4) */
    if (t->kind == CPPT_IDENT) {
        return 0;
    }

    char msg[128];
    snprintf(msg, sizeof msg, "unexpected token '%s' in #if expression",
             pptok_spell(t));
    ec_error(ec, t->loc, msg);
    return 0;
}

static ival eval_unary(EvalCtx *ec) {
    ec_skip_ws(ec);
    const PPTok *t = ec_peek(ec);
    if (!t) return eval_primary(ec);

    const char *sp = pptok_spell(t);
    if (t->kind == CPPT_PUNCT) {
        if (strcmp(sp, "+") == 0) { ec_get(ec); return  eval_unary(ec); }
        if (strcmp(sp, "-") == 0) { ec_get(ec); return -eval_unary(ec); }
        if (strcmp(sp, "~") == 0) { ec_get(ec); return ~eval_unary(ec); }
        if (strcmp(sp, "!") == 0) { ec_get(ec); return !eval_unary(ec); }
    }
    return eval_primary(ec);
}

static ival eval_mul(EvalCtx *ec) {
    ival v = eval_unary(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        if (strcmp(sp, "*") == 0) { ec_get(ec); v *= eval_unary(ec); }
        else if (strcmp(sp, "/") == 0) {
            ec_get(ec); ival r = eval_unary(ec);
            if (r == 0) { CppDiag d={CPP_DIAG_ERROR,t->loc,cpp_xstrdup("division by zero in #if")};
                          da_push(ec->diags,d); ec->had_error=true; break; }
            v /= r;
        }
        else if (strcmp(sp, "%") == 0) {
            ec_get(ec); ival r = eval_unary(ec);
            if (r == 0) { CppDiag d={CPP_DIAG_ERROR,t->loc,cpp_xstrdup("modulo by zero in #if")};
                          da_push(ec->diags,d); ec->had_error=true; break; }
            v %= r;
        }
        else break;
    }
    return v;
}

static ival eval_add(EvalCtx *ec) {
    ival v = eval_mul(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        if      (strcmp(sp, "+") == 0) { ec_get(ec); v += eval_mul(ec); }
        else if (strcmp(sp, "-") == 0) { ec_get(ec); v -= eval_mul(ec); }
        else break;
    }
    return v;
}

static ival eval_shift(EvalCtx *ec) {
    ival v = eval_add(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        if      (strcmp(sp, "<<") == 0) { ec_get(ec); v <<= eval_add(ec); }
        else if (strcmp(sp, ">>") == 0) { ec_get(ec); v >>= eval_add(ec); }
        else break;
    }
    return v;
}

static ival eval_relat(EvalCtx *ec) {
    ival v = eval_shift(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        if      (strcmp(sp, "<")  == 0) { ec_get(ec); v = v <  eval_shift(ec); }
        else if (strcmp(sp, ">")  == 0) { ec_get(ec); v = v >  eval_shift(ec); }
        else if (strcmp(sp, "<=") == 0) { ec_get(ec); v = v <= eval_shift(ec); }
        else if (strcmp(sp, ">=") == 0) { ec_get(ec); v = v >= eval_shift(ec); }
        else break;
    }
    return v;
}

static ival eval_equal(EvalCtx *ec) {
    ival v = eval_relat(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        if      (strcmp(sp, "==") == 0) { ec_get(ec); v = v == eval_relat(ec); }
        else if (strcmp(sp, "!=") == 0) { ec_get(ec); v = v != eval_relat(ec); }
        else break;
    }
    return v;
}

static ival eval_bitand(EvalCtx *ec) {
    ival v = eval_equal(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "&") == 0)
        { ec_get(ec); v &= eval_equal(ec); }
    return v;
}

static ival eval_bitxor(EvalCtx *ec) {
    ival v = eval_bitand(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "^") == 0)
        { ec_get(ec); v ^= eval_bitand(ec); }
    return v;
}

static ival eval_bitor(EvalCtx *ec) {
    ival v = eval_bitxor(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "|") == 0)
        { ec_get(ec); v |= eval_bitxor(ec); }
    return v;
}

/* Skip an expression without evaluating (for short-circuit).
 * Skips balanced parentheses, unary operators, identifiers, literals, etc.
 * Stops at comma, ')', or end of tokens. */
static void ec_skip_expr(EvalCtx *ec) {
    int depth = 0;
    int tern_depth = 0;
    while (ec->cur) {
        ec_skip_ws(ec);
        if (!ec->cur) break;
        const char *sp = pptok_spell(ec_peek(ec));
        if (strcmp(sp, "(") == 0) { depth++; ec_get(ec); }
        else if (strcmp(sp, ")") == 0) {
            if (depth == 0) break;
            depth--; ec_get(ec);
        }
        else if (strcmp(sp, ",") == 0 && depth == 0) break;
        else if (strcmp(sp, "?") == 0 && depth == 0) {
            tern_depth++; ec_get(ec);
        }
        else if (strcmp(sp, ":") == 0 && depth == 0) {
            if (tern_depth == 0) break;
            tern_depth--; ec_get(ec);
        }
        else if (strcmp(sp, "||") == 0 && depth == 0 && tern_depth == 0) break;
        else if (strcmp(sp, "&&") == 0 && depth == 0 && tern_depth == 0) break;
        else ec_get(ec);
    }
}

static ival eval_logand(EvalCtx *ec) {
    ival v = eval_bitor(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "&&") == 0)
        { ec_get(ec); if (v) { ival r = eval_bitor(ec); v = v && r; } else { ec_skip_expr(ec); } }
    return v;
}

static ival eval_logor(EvalCtx *ec) {
    ival v = eval_logand(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "||") == 0)
        { ec_get(ec); if (!v) { ival r = eval_logand(ec); v = v || r; } else { ec_skip_expr(ec); } }
    return v;
}

static ival eval_expr(EvalCtx *ec) {
    ival cond = eval_logor(ec);
    ec_skip_ws(ec);
    const PPTok *t = ec_peek(ec);
    if (t && t->kind == CPPT_PUNCT && strcmp(pptok_spell(t), "?") == 0) {
        ec_get(ec);
        /* Save position before then-branch so we can seek to colon if needed */
        TokNode *then_start = ec->cur;
        /* Evaluate both branches conditionally */
        if (cond) {
            ival then_val = eval_expr(ec);
            ec_accept(ec, ":");
            ec_skip_expr(ec);
            return then_val;
        } else {
            /* Skip the then-branch to get to ':' */
            ec_skip_expr(ec);
            ec_accept(ec, ":");
            return eval_expr(ec);
        }
    }
    /* Handle comma operator: return value after last comma.
     * This is needed for P99's comma-detection trick in #if expressions. */
    while (ec->cur && ec->cur->tok.kind == CPPT_PUNCT && 
           pptok_spell(&ec->cur->tok) && strcmp(pptok_spell(&ec->cur->tok), ",") == 0) {
        ec_get(ec);  /* consume comma */
        ec_skip_ws(ec);
        if (ec->cur && ec->cur->tok.kind != CPPT_NEWLINE && ec->cur->tok.kind != CPPT_EOF) {
            cond = eval_logor(ec);
            ec_skip_ws(ec);
        } else {
            break;
        }
    }
    fprintf(stderr, "[DEBUG eval_expr] returning cond=%lld\n", (long long)cond);
    return cond;
}

/* =========================================================================
 * Public entry point
 * ====================================================================== */

ival cpp_eval_if_expr(TokList *toks, MacroTable *mt,
                      InternTable *interns, CppDiagArr *diags,
                      bool *error_out) {
    EvalCtx ec = {
        .cur     = toks->head,
        .mt      = mt,
        .interns = interns,
        .diags   = diags,
        .had_error = false,
    };
    /* Check for empty expression (#if without expression) */
    if (!ec.cur || ec.cur->tok.kind == CPPT_NEWLINE) {
        ec_error(&ec, (CppLoc){0}, "#if with no expression");
        if (error_out) *error_out = true;
        return 0;
    }
    ival v = eval_expr(&ec);
    if (error_out) *error_out = ec.had_error;
    return v;
}

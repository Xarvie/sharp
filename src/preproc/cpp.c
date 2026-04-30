/*
 * cpp.c — Sharp/C Preprocessor — public API implementation
 *
 * This file implements the CppCtx lifecycle and the two public entry points
 * (cpp_run / cpp_run_buf).  The heavy lifting is in the sub-modules:
 *
 *   pptok.c      — phases 1-3 (trigraphs, line-splicing, tokenisation)
 *   macro.c      — macro table + expansion engine (§6.10.3)
 *   expr.c       — #if constant-expression evaluator (§6.10.1)
 *   directive.c  — directive dispatcher + include stack + cond stack
 *
 * Phase 5 (execution character-set conversion) and phase 6 (string-literal
 * concatenation) are performed here after the directive phase completes.
 */

#include "cpp_internal.h"
#include "cpp.h"
#include "pptok.h"
#include "macro.h"
#include "expr.h"
#include "directive.h"

#include <time.h>

/* =========================================================================
 * CppCtx lifecycle
 * ====================================================================== */

CppCtx *cpp_ctx_new(void) {
    CppCtx *ctx = cpp_xmalloc(sizeof *ctx);
    memset(ctx, 0, sizeof *ctx);
    ctx->emit_linemarkers  = true;
    ctx->keep_whitespace   = false;
    ctx->keep_comments     = false;
    ctx->max_include_depth = 200;
    return ctx;
}

void cpp_ctx_free(CppCtx *ctx) {
    if (!ctx) return;
    strarr_free_contents(&ctx->user_include_paths);
    strarr_free_contents(&ctx->sys_include_paths);
    strarr_free_contents(&ctx->cmdline_ops);
    strarr_free_contents(&ctx->cmdline_defines);
    strarr_free_contents(&ctx->cmdline_undefines);
    intern_free(&ctx->interns);
    free(ctx);
}

/* =========================================================================
 * Configuration
 * ====================================================================== */

void cpp_add_sys_include(CppCtx *ctx, const char *dir) {
    strarr_push(&ctx->sys_include_paths, cpp_xstrdup(dir));
}

void cpp_add_user_include(CppCtx *ctx, const char *dir) {
    strarr_push(&ctx->user_include_paths, cpp_xstrdup(dir));
}

void cpp_define(CppCtx *ctx, const char *name, const char *value) {
    StrBuf sb = {0};
    sb_push_cstr(&sb, "D:");
    sb_push_cstr(&sb, name);
    if (value) { sb_push_ch(&sb, '='); sb_push_cstr(&sb, value); }
    strarr_push(&ctx->cmdline_ops, sb_take(&sb));
}

void cpp_undefine(CppCtx *ctx, const char *name) {
    StrBuf sb = {0};
    sb_push_cstr(&sb, "U:");
    sb_push_cstr(&sb, name);
    strarr_push(&ctx->cmdline_ops, sb_take(&sb));
}

void cpp_keep_whitespace(CppCtx *ctx, bool keep)    { ctx->keep_whitespace   = keep; }
void cpp_emit_linemarkers(CppCtx *ctx, bool emit)   { ctx->emit_linemarkers  = emit; }
void cpp_keep_comments(CppCtx *ctx, bool keep)      { ctx->keep_comments     = keep; }
void cpp_set_max_include_depth(CppCtx *ctx, int d)  { ctx->max_include_depth = d;    }

void cpp_set_pragma_handler(CppCtx *ctx, CppPragmaHandler fn, void *ud) {
    ctx->pragma_handler  = fn;
    ctx->pragma_userdata = ud;
}

/* =========================================================================
 * Phase 6: Adjacent string-literal concatenation (§6.4.5p4 / §6.10.3.2)
 *
 * Runs over the output token array after the directive phase.  Consecutive
 * CPPT_STRING_LIT tokens (possibly separated by whitespace/comments) are
 * merged.  Wide / encoding prefixes: L + L → L, u8 + u8 → u8, otherwise the
 * implementation-defined rule is: non-plain beats plain, and mismatched
 * wide types are rejected.
 * ====================================================================== */

typedef struct {
    const char *prefix;  /* "" | "L" | "u" | "U" | "u8" */
    StrBuf      content; /* the inner characters (without quotes)          */
} StrFragment;

static const char *merge_encoding_prefix(const char *a, const char *b,
                                          CppDiagArr *diags, CppLoc loc) {
    if (strcmp(a, b) == 0) return a;
    if (strcmp(a, "") == 0) return b;
    if (strcmp(b, "") == 0) return a;
    CppDiag d = { CPP_DIAG_ERROR, loc,
                  cpp_xstrdup("incompatible string literal prefixes in concatenation") };
    da_push(diags, d);
    return a;
}

static void parse_str_tok(const char *sp, StrFragment *frag) {
    /* Extract prefix */
    frag->prefix = "";
    frag->content = (StrBuf){0};
    if (sp[0] == 'L') { frag->prefix = "L"; sp++; }
    else if (sp[0] == 'u' && sp[1] == '8') { frag->prefix = "u8"; sp += 2; }
    else if (sp[0] == 'u') { frag->prefix = "u"; sp++; }
    else if (sp[0] == 'U') { frag->prefix = "U"; sp++; }
    /* Skip opening quote */
    if (*sp == '"') sp++;
    /* Copy until closing quote (handling escapes) */
    while (*sp && *sp != '"') {
        if (*sp == '\\' && *(sp+1)) {
            sb_push_ch(&frag->content, *sp++);
            sb_push_ch(&frag->content, *sp++);
        } else {
            sb_push_ch(&frag->content, *sp++);
        }
    }
}

static CppTok *phase6_concat(CppTok *tokens, size_t n,
                              size_t *out_n, CppDiagArr *diags) {
    DA(CppTok) result = {0};
    for (size_t i = 0; i < n; ) {
        if (tokens[i].kind != CPPT_STRING_LIT) {
            da_push(&result, tokens[i]);
            i++;
            continue;
        }
        /* Collect a run of string literals (skip whitespace between them) */
        size_t j = i + 1;
        while (j < n && (tokens[j].kind == CPPT_STRING_LIT ||
                          tokens[j].kind == CPPT_SPACE)) j++;
        /* Did we find at least one more string? */
        bool has_second = false;
        for (size_t k = i+1; k < j; k++)
            if (tokens[k].kind == CPPT_STRING_LIT) { has_second = true; break; }

        if (!has_second) {
            da_push(&result, tokens[i++]);
            continue;
        }

        /* Merge all string fragments in [i, j) */
        StrFragment merged;
        merged.prefix  = "";
        merged.content = (StrBuf){0};

        for (size_t k = i; k < j; k++) {
            if (tokens[k].kind != CPPT_STRING_LIT) continue;
            StrFragment frag;
            char *sp_copy = cpp_xstrndup(tokens[k].text, tokens[k].len);
            parse_str_tok(sp_copy, &frag);
            free(sp_copy);
            merged.prefix = merge_encoding_prefix(merged.prefix, frag.prefix,
                                                   diags, tokens[k].loc);
            sb_push(&merged.content, frag.content.buf ? frag.content.buf : "",
                    frag.content.len);
            sb_free(&frag.content);
        }

        /* Build the merged token text */
        StrBuf full = {0};
        sb_push_cstr(&full, merged.prefix);
        sb_push_ch(&full, '"');
        if (merged.content.buf)
            sb_push(&full, merged.content.buf, merged.content.len);
        sb_push_ch(&full, '"');
        sb_free(&merged.content);

        CppTok merged_tok = tokens[i];
        /* The text pointer now points into a new heap buffer; we store it
         * by recording the string in the intern table (acceptable since
         * this happens once at the end).                                  */
        char *interned_text = sb_take(&full);
        merged_tok.text          = interned_text; /* heap-allocated */
        merged_tok.len           = strlen(interned_text);
        merged_tok.concat_done   = true;
        da_push(&result, merged_tok);
        i = j;
    }
    *out_n = result.len;
    return result.data;
}

/* =========================================================================
 * Build CppResult from CppState
 * ====================================================================== */


/* =========================================================================
 * Phase 6: Adjacent string-literal concatenation — text-level scanner
 *
 * Scans the preprocessed text for runs of adjacent string literals
 * (separated only by whitespace/newlines) and merges them.
 * ====================================================================== */

static void p6_skip_ws(const char **p, const char *end) {
    while (*p < end && (**p == ' ' || **p == '\t' || **p == '\n' || **p == '\r'))
        (*p)++;
}

static bool p6_read_prefix(const char **p, const char *end, char prefix[4]) {
    const char *s = *p; int pi = 0;
    if (s < end && (*s == 'L' || *s == 'u' || *s == 'U')) {
        prefix[pi++] = *s++;
        if (s < end && *s == '8' && prefix[0] == 'u') prefix[pi++] = *s++;
    }
    prefix[pi] = '\0';
    if (s < end && *s == '"') { *p = s; return true; }
    *p -= pi; prefix[0] = '\0'; return false;
}

static bool p6_read_body(const char **p, const char *end, StrBuf *body) {
    if (*p >= end || **p != '"') return false;
    (*p)++;
    while (*p < end && **p != '"') {
        /* String literals cannot contain unescaped newlines.
         * If we hit a newline without a preceding backslash, stop —
         * this means we likely opened on a closing quote.          */
        if (**p == '\n') {
            /* Back up: the '"' we consumed was a closing quote, not opening */
            (*p)--;
            body->len = 0;  /* discard partial body */
            return false;
        }
        if (**p == '\\' && *p + 1 < end) { sb_push_ch(body, '\\'); (*p)++; }
        sb_push_ch(body, **p); (*p)++;
    }
    if (*p < end) (*p)++;
    return true;
}

/* Skip whitespace in phase-6 adjacency scanning, but stop before any line
 * that begins with '#' (linemarker or #pragma).  Returns true if stopped
 * at a linemarker boundary; in that case *p is left pointing at the '\n'
 * that precedes the '#' line so the caller can back up cleanly.           */
static bool p6_skip_ws_no_lm(const char **p, const char *end) {
    const char *s = *p;
    while (s < end) {
        if (*s == ' ' || *s == '\t' || *s == '\r') { s++; continue; }
        if (*s == '\n') {
            const char *nl = s;
            s++;
            while (s < end && (*s == ' ' || *s == '\t')) s++;
            if (s < end && *s == '#') { *p = nl; return true; }
            /* Not a linemarker — stop here, don't skip the whole line */
            break;
        }
        /* Non-whitespace, non-newline character — stop here */
        break;
    }
    *p = s;
    return false;
}

static char *phase6_apply_text(const char *text, size_t len, size_t *out_len) {
    StrBuf out = {0};
    const char *p = text, *end = text + len;
    bool line_start = true;   /* true at text start and after every '\n' */

    while (p < end) {

        /* ── Linemarker fast-path ───────────────────────────────────────────
         * At the start of a line, if (after optional spaces/tabs) we see '#',
         * copy the ENTIRE line verbatim.  This prevents the filename string
         * inside "# 123 \"file.c\"" from ever entering string-merge logic.  */
        if (line_start) {
            line_start = false;
            const char *ls = p;
            while (ls < end && (*ls == ' ' || *ls == '\t')) ls++;
            if (ls < end && *ls == '#') {
                while (p < end && *p != '\n') sb_push_ch(&out, *p++);
                if (p < end) sb_push_ch(&out, *p++);   /* consume '\n' */
                line_start = true;
                continue;
            }
        }

        /* ── Non-string / non-char-literal character ────────────────────── */
        const char *tok_start = p;

        /* First check for character literal: '...'
         * Must be handled before string detection to avoid confusing a '"'
         * inside a char literal with a string opening.                      */
        if (*p == '\'') {
            sb_push_ch(&out, *p++);   /* opening ' */
            while (p < end && *p != '\'') {
                if (*p == '\\' && p + 1 < end) { sb_push_ch(&out, *p++); }
                sb_push_ch(&out, *p++);
            }
            if (p < end) sb_push_ch(&out, *p++);   /* closing ' */
            /* char literal done; don't set line_start, loop continues       */
            continue;
        }

        char prefix[4] = {0};
        bool is_str = p6_read_prefix(&p, end, prefix);
        if (!is_str || p >= end || *p != '"') {
            p = tok_start;
            if (*p == '\n') line_start = true;
            sb_push_ch(&out, *p++);
            continue;
        }

        /* ── String literal (possibly with encoding prefix) ─────────────── */
        StrBuf body = {0};
        p6_read_body(&p, end, &body);

        /* Collect adjacent string literals; stop at linemarkers.            */
        for (;;) {
            const char *ws = p;
            bool hit_lm = p6_skip_ws_no_lm(&p, end);
            if (hit_lm) { p = ws; break; }

            const char *peek = p; char np[4] = {0};
            if (!p6_read_prefix(&peek, end, np) || peek >= end || *peek != '"')
                { p = ws; break; }
            if (*prefix && *np && strcmp(prefix, np) != 0) { p = ws; break; }
            if (*np) { memcpy(prefix, np, 3); prefix[3] = '\0'; }
            p = peek;
            p6_read_body(&p, end, &body);
        }

        sb_push_cstr(&out, prefix);
        sb_push_ch(&out, '"');
        if (body.buf) sb_push(&out, body.buf, body.len);
        sb_push_ch(&out, '"');
        sb_free(&body);
        /* p now sits just after the last merged string.  The outer loop will
         * process the next character; if it's '\n' line_start is set there.  */
    }

    *out_len = out.len;
    return sb_take(&out);
}

static CppResult build_result(CppState *st, CppCtx *ctx, CppDiagArr *diags) {
    CppResult res = {0};

    /* Phase 6: apply adjacent string literal concatenation to a new buffer.
     * We read from the raw output text BEFORE transferring its ownership.  */
    const char *raw_text = cpp_state_text(st);
    size_t      raw_len  = cpp_state_text_len(st);

    size_t      ph6_len  = 0;
    char       *ph6_text = phase6_apply_text(raw_text, raw_len, &ph6_len);

    res.text     = ph6_text;
    res.text_len = ph6_len;

    /* Transfer ownership of the raw text buffer to CppResult so that
     * CppTok.text pointers (which index into it) remain valid after
     * cpp_state_free().  cpp_state_free() skips freeing a NULL buf.      */
    res._raw_text = cpp_state_take_text(st);

    /* Token array: copy without whitespace (phase 6 is reflected in text only). */
    size_t  ntok  = cpp_state_ntokens(st);
    CppTok *toks  = cpp_state_tokens(st);
    CppTok *ftoks = (CppTok *)cpp_xmalloc((ntok + 1) * sizeof(CppTok));
    size_t  flen  = 0;
    for (size_t i = 0; i < ntok; i++) {
        if (!ctx->keep_whitespace &&
            (toks[i].kind == CPPT_SPACE || toks[i].kind == CPPT_NEWLINE ||
             toks[i].kind == CPPT_COMMENT)) continue;
        ftoks[flen++] = toks[i];
    }
    res.tokens  = ftoks;
    res.ntokens = flen;

    /* Transfer diagnostics */
    res.diags  = diags->data;
    res.ndiags = diags->len;

    /* Set top-level error pointer if any error-level diag exists */
    for (size_t i = 0; i < res.ndiags; i++) {
        if (res.diags[i].level >= CPP_DIAG_ERROR) {
            res.error = res.diags[i].msg;
            break;
        }
    }

    return res;
}

/* =========================================================================
 * Public entry points
 * ====================================================================== */

CppResult cpp_run(CppCtx *ctx, const char *filename, CppLang lang) {
    CppDiagArr diags = {0};
    CppState *st = cpp_state_new(ctx, &diags);
    cpp_state_run_file(st, filename, lang);
    CppResult res = build_result(st, ctx, &diags);
    /* diags ownership transferred to res */
    cpp_state_free(st);
    return res;
}

CppResult cpp_run_buf(CppCtx *ctx, const char *buf, size_t len,
                       const char *filename, CppLang lang) {
    CppDiagArr diags = {0};
    CppState *st = cpp_state_new(ctx, &diags);
    cpp_state_run_buf(st, buf, len, filename, lang);
    CppResult res = build_result(st, ctx, &diags);
    cpp_state_free(st);
    return res;
}

/* =========================================================================
 * Result helpers
 * ====================================================================== */

void cpp_result_free(CppResult *res) {
    if (!res) return;
    /* Token texts with concat_done were separately heap-allocated by phase6_concat */
    for (size_t i = 0; i < res->ntokens; i++) {
        if (res->tokens[i].concat_done)
            free((char *)res->tokens[i].text);
    }
    free(res->tokens);
    free(res->text);
    free(res->_raw_text);   /* raw buffer that token.text pointers index into */
    for (size_t i = 0; i < res->ndiags; i++)
        free(res->diags[i].msg);
    free(res->diags);
    memset(res, 0, sizeof *res);
}

static const char *tok_kind_names[CPPT_COUNT] = {
    [CPPT_EOF]          = "EOF",
    [CPPT_SPACE]        = "whitespace",
    [CPPT_NEWLINE]      = "newline",
    [CPPT_COMMENT]      = "comment",
    [CPPT_HEADER_NAME]  = "header-name",
    [CPPT_IDENT]        = "identifier",
    [CPPT_PP_NUMBER]    = "pp-number",
    [CPPT_CHAR_CONST]   = "char-constant",
    [CPPT_STRING_LIT]   = "string-literal",
    [CPPT_PUNCT]        = "punctuator",
    [CPPT_OTHER]        = "other",
    [CPPT_PLACEMARKER]  = "placemarker",
};

const char *cpp_tok_kind_name(CppTokKind kind) {
    if (kind < 0 || kind >= CPPT_COUNT) return "??";
    return tok_kind_names[kind] ? tok_kind_names[kind] : "??";
}

void cpp_print_diags(const CppResult *res) {
    for (size_t i = 0; i < res->ndiags; i++) {
        const CppDiag *d = &res->diags[i];
        const char *lvl  = (d->level == CPP_DIAG_NOTE)    ? "note"    :
                           (d->level == CPP_DIAG_WARNING)  ? "warning" :
                           (d->level == CPP_DIAG_ERROR)    ? "error"   : "fatal";
        if (d->loc.file)
            fprintf(stderr, "%s:%d:%d: %s: %s\n",
                    d->loc.file, d->loc.line, d->loc.col, lvl, d->msg);
        else
            fprintf(stderr, "%s: %s\n", lvl, d->msg);
    }
}

/* =========================================================================
 * Bridge: macro_state_table — lets cpp.c get the MacroTable from CppState
 * without exposing CppState's internals in the header.
 * Defined here rather than directive.h to keep the coupling local.
 * ====================================================================== */


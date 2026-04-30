#define _POSIX_C_SOURCE 200809L
/*
 * directive.c — Preprocessing directive processor (§6.10)
 *
 * Handles (in execution order within a translation unit):
 *   #include  (§6.10.2)   — with include-guard optimisation
 *   #define   (§6.10.3)
 *   #undef    (§6.10.3.5)
 *   #line     (§6.10.4)
 *   #error    (§6.10.5)
 *   #warning  (GCC extension, accepted)
 *   #pragma   (§6.10.6)   — passed through / callback
 *   #if / #ifdef / #ifndef / #elif / #else / #endif  (§6.10.1)
 *
 * Sharp-specific extensions (enabled when lang == CPP_LANG_SHARP):
 *   #sharp_only … #end_sharp_only  — block emitted only by Sharp compiler
 *   #c_only     … #end_c_only      — block emitted only when compiling as C
 */

#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#define access _access
#ifndef R_OK
#define R_OK 04
#endif
#else
#include <unistd.h>
#endif
#include "cpp_internal.h"
#include "directive.h"
#include "macro.h"
#include "expr.h"
#include "pptok.h"

#include <time.h>

/* =========================================================================
 * Helpers
 * ====================================================================== */

static bool in_live_branch(const CppState *st) {
    for (int i = 0; i < st->cond_depth; i++)
        if (!st->cond_stack[i].in_true_branch)
            return false;
    return true;
}

static void emit_diag(CppState *st, CppDiagLevel lvl, CppLoc loc, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    CppDiag d = { lvl, loc, cpp_xstrdup(buf) };
    diag_push(st->diags, d);
    if (lvl == CPP_DIAG_FATAL) st->fatal = true;
}

/* Emit a linemarker: `# <line> "<file>"\n`
 * Also resets the newline counter so the next flush_pending_lm() can detect
 * whether a new marker is actually needed.                                  */
static void emit_linemarker(CppState *st, int line, const char *file) {
    if (!st->emit_linemarkers) return;
    /* Convert Windows backslashes to forward slashes in the file path.
     * C string literals interpret \ as escape sequences, so paths like
     * C:\Users become invalid (e.g. \U is an invalid hex escape). */
    char path_buf[1024];
    const char *out_file = file;
    {
        const char *s = file;
        char *d = path_buf;
        int need_convert = 0;
        while (*s && (size_t)(d - path_buf) < sizeof(path_buf) - 1) {
            if (*s == '\\') { *d++ = '/'; need_convert = 1; }
            else            { *d++ = *s; }
            s++;
        }
        *d = '\0';
        if (need_convert) out_file = path_buf;
    }
    sb_printf(&st->out_text, "# %d \"%s\"\n", line, out_file);
    st->last_lm_src_line = line;
    st->last_lm_src_file = file;  /* keep original for comparison */
    st->out_newlines     = 0;
}

/* Queue a linemarker to fire lazily before the next real content token.
 * Multiple consecutive directives will overwrite the same pending slot,
 * so only one marker is ever emitted for a run of directives.               */
static void queue_linemarker(CppState *st, int line, const char *file) {
    st->pending_lm      = true;
    st->pending_lm_line = line;
    st->pending_lm_file = file;
}

/* Fire the pending linemarker if one is queued AND if the expected output
 * line doesn't already match the pending source line.                       */
static void flush_pending_lm(CppState *st) {
    if (!st->pending_lm) return;
    st->pending_lm = false;
    if (!st->emit_linemarkers) return;
    /* expected source line = line declared in last marker + newlines since */
    int expected = st->last_lm_src_line + st->out_newlines;
    if (expected == st->pending_lm_line &&
        st->last_lm_src_file == st->pending_lm_file)
        return; /* already in sync — no marker needed */
    emit_linemarker(st, st->pending_lm_line, st->pending_lm_file);
}

/* Emit a token's text to the output text buffer.
 * Flushes any pending linemarker before the first non-whitespace token so
 * that runs of directives don't produce redundant markers.                  */
static void emit_tok_text(CppState *st, const PPTok *t) {
    /* Flush pending marker before any real (non-whitespace) content. */
    if (t->kind != CPPT_SPACE && t->kind != CPPT_COMMENT)
        flush_pending_lm(st);

    const char *sp = pptok_spell(t);
    if (t->has_leading_space) sb_push_ch(&st->out_text, ' ');
    sb_push(&st->out_text, sp, strlen(sp));

    /* Record public CppTok pointing into the output buffer. */
    size_t start = st->out_text.len - strlen(sp);
    CppTok pub = {
        .kind       = t->kind,
        .text       = st->out_text.buf + start,
        .len        = strlen(sp),
        .loc        = t->loc,
        .from_macro = t->from_macro,
    };
    da_push(&st->out_tokens, pub);
}

/* =========================================================================
 * Include-guard management
 * ====================================================================== */

static GuardEntry *guard_find(CppState *st, const char *filename) {
    uint32_t h = intern_hash(filename, strlen(filename)) & (GUARD_BUCKETS-1);
    for (GuardEntry *e = st->guards[h]; e; e = e->next)
        if (e->filename == filename) return e;
    return NULL;
}

static void guard_set(CppState *st, const char *filename, const char *macro) {
    uint32_t h = intern_hash(filename, strlen(filename)) & (GUARD_BUCKETS-1);
    GuardEntry *e = guard_find(st, filename);
    if (!e) {
        e = cpp_xmalloc(sizeof *e);
        e->filename = filename;
        e->next = st->guards[h];
        st->guards[h] = e;
    }
    e->macro = macro;
}

static bool guard_already_included(CppState *st, const char *filename) {
    GuardEntry *e = guard_find(st, filename);
    if (!e || !e->macro) return false;
    return macro_lookup(st->macros, e->macro) != NULL;
}

/* =========================================================================
 * Apply command-line -D / -U operations to the macro table.
 * Called AFTER install_builtins so that -U can suppress built-in macros and
 * -D can override them.  Processes ops in left-to-right order (POSIX cc
 * convention: later -D/-U wins).
 * ====================================================================== */

static void apply_cmdline_macros(CppState *st) {
    CppCtx      *ctx    = st->ctx;
    MacroTable  *mt     = st->macros;
    InternTable *interns = st->interns;
    CppLoc       cloc   = {intern_cstr(interns, "<command-line>"), 0, 0};
    for (size_t i = 0; i < ctx->cmdline_ops.len; i++) {
        const char *op = ctx->cmdline_ops.data[i];
        if (op[0] == 'U' && op[1] == ':') {
            macro_undef(mt, op + 2);
        } else if (op[0] == 'D' && op[1] == ':') {
            const char *body = op + 2;
            const char *eq   = strchr(body, '=');
            if (eq) {
                size_t nlen = (size_t)(eq - body);
                char  *name = cpp_xstrndup(body, nlen);
                macro_define_object(mt, interns, name, eq + 1, cloc);
                free(name);
            } else {
                macro_define_object(mt, interns, body, "1", cloc);
            }
        }
    }
}

/* =========================================================================
 * Built-in macros
 * ====================================================================== */

static void install_builtin_str(MacroTable *mt, InternTable *interns,
                                const char *name, const char *value) {
    CppLoc loc = {intern_cstr(interns, "<builtin>"), 0, 0};
    macro_define_object(mt, interns, name, value, loc);
}

static void install_builtin_num(MacroTable *mt, InternTable *interns,
                                const char *name, long long value) {
    char buf[64];
    snprintf(buf, sizeof buf, "%lld", value);
    CppLoc loc = {intern_cstr(interns, "<builtin>"), 0, 0};
    macro_define_object(mt, interns, name, buf, loc);
}

static void install_builtins(CppState *st, CppLang lang) {
    MacroTable  *mt = st->macros;
    InternTable *it = st->interns;

    /* Standard C defaults — overridden by apply_cmdline_macros() for targets
     * that need different values (freestanding → __STDC_HOSTED__=0, etc.). */
    install_builtin_num(mt, it, "__STDC__",         1);
    install_builtin_num(mt, it, "__STDC_HOSTED__",  1);
    install_builtin_num(mt, it, "__STDC_VERSION__", 201112L);

    /* __DATE__ / __TIME__ — dynamic at compile time, not target-dependent. */
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char date_buf[32], time_buf[32];
    strftime(date_buf, sizeof date_buf, "\"%b %e %Y\"", tm_now);
    strftime(time_buf, sizeof time_buf, "\"%H:%M:%S\"", tm_now);
    install_builtin_str(mt, it, "__DATE__", date_buf);
    install_builtin_str(mt, it, "__TIME__", time_buf);

    /* Compiler identity — not target-dependent. */
    install_builtin_num(mt, it, "__SHARP_CPP__",       1);
    install_builtin_num(mt, it, "__SHARP_CPP_MAJOR__", 1);
    install_builtin_num(mt, it, "__SHARP_CPP_MINOR__", 0);

    if (lang == CPP_LANG_SHARP) {
        install_builtin_num(mt, it, "__SHARP__", 1);
        install_builtin_str(mt, it, "__SHARP_VERSION__", "\"0.4\"");
    }

    /* NOTE: __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, _WIN32, _WIN64,
     * __linux__, __APPLE__, __STDC_HOSTED__, and all target/ABI macros are
     * installed by apply_cmdline_macros() which runs immediately after this
     * function (see cpp_state_run_file / cpp_state_run_buf).  Do NOT add
     * host-conditional (#ifdef _WIN32 etc.) macros here — they must come
     * from the target triple, not the build host.
     *
     * __FILE__, __LINE__, __COUNTER__, __TIMESTAMP__ are handled dynamically
     * in the token-emission path (process_buf) and must NOT appear here. */
}

/* =========================================================================
 * Forward declarations
 * ====================================================================== */

static void process_file(CppState *st, const char *filename, CppLang lang);
static void process_buf(CppState *st, CppReader *rd, CppLang lang);

/* =========================================================================
 * Collect tokens for a directive line (up to but not including CPPT_NEWLINE)
 * ====================================================================== */

static TokList collect_directive_line(CppReader *rd, bool keep_comments) {
    TokList line = {0};
    for (;;) {
        PPTok t = reader_next_tok(rd, keep_comments);
        if (t.kind == CPPT_EOF || t.kind == CPPT_NEWLINE) {
            pptok_free(&t);
            break;
        }
        tl_append(&line, t);
    }
    return line;
}

/* =========================================================================
 * File search for #include
 * ====================================================================== */

/* Find an include file by name.
 * `is_system`    — true for <...>, false for "..."
 * `current_file` — path of the file issuing the #include (for relative "...")
 * `skip_until`   — for #include_next: skip all search dirs up to and
 *                  including the one that contains `skip_until` (interned).
 *                  Pass NULL for normal #include behaviour.               */
static char *find_include_ex(CppState *st, const char *name, bool is_system,
                              const char *current_file, const char *skip_until) {
    char path[4096];
    bool skipping = (skip_until != NULL);

    /* Absolute path: try directly first (works on both POSIX and Windows) */
    if (name[0] == '/' || (name[0] && name[1] == ':')) {
        if (!skipping && access(name, R_OK) == 0)
            return cpp_xstrdup(name);
        /* absolute paths cannot match relative include_next logic */
        if (skipping) return NULL;
    }

    auto_skip:; /* label for the skip_until logic */
    (void)0;

    /* For "..." and "<...>" first search relative to the including file's
     * directory (skipped when skip_until is active).
     * This handles cases where system headers use <> for same-directory
     * includes. */
    if (current_file && !skipping) {
        const char *slash = strrchr(current_file, '/');
#ifdef _WIN32
        const char *bslash = strrchr(current_file, '\\');
        if (bslash && (!slash || bslash > slash)) slash = bslash;
#endif
        if (slash) {
            size_t dir_len = (size_t)(slash - current_file + 1);
            snprintf(path, sizeof path, "%.*s%s", (int)dir_len, current_file, name);
            if (access(path, R_OK) == 0) {
                const char *found = intern_cstr(st->interns, path);
                if (skip_until && strcmp(found, skip_until) == 0) {
                    skipping = false; /* found the origin; continue from next */
                    goto auto_skip;
                }
                return cpp_xstrdup(path);
            }
        }
        /* If no directory separator (e.g. virtual filename like "<test>"),
         * fall through to try the name directly relative to CWD.          */
    }

    /* User include paths */
    if (!is_system) {
        for (size_t i = 0; i < st->user_paths.len; i++) {
            snprintf(path, sizeof path, "%s/%s", st->user_paths.data[i], name);
            if (access(path, R_OK) == 0) {
                const char *found = intern_cstr(st->interns, path);
                if (skipping && skip_until && strcmp(found, skip_until) == 0)
                    { skipping = false; continue; }
                if (!skipping) return cpp_xstrdup(path);
            }
        }
    }

    /* System include paths */
    for (size_t i = 0; i < st->sys_paths.len; i++) {
        snprintf(path, sizeof path, "%s/%s", st->sys_paths.data[i], name);
        if (access(path, R_OK) == 0) {
            const char *found = intern_cstr(st->interns, path);
            if (skipping && skip_until && strcmp(found, skip_until) == 0)
                { skipping = false; continue; }
            if (!skipping) return cpp_xstrdup(path);
        }
    }
    return NULL;
}

/* Convenience wrapper for standard #include */
static char *find_include(CppState *st, const char *name, bool is_system,
                           const char *current_file) {
    return find_include_ex(st, name, is_system, current_file, NULL);
}

/* Check if a header file exists in the search paths (for __has_include).
 * Uses the same logic as find_include_ex but just checks existence. */
static bool cpp_has_include(CppState *st, const char *name, bool is_system) {
    /* For absolute paths, just check access() */
    if (name[0] == '/' || (name[0] && name[1] == ':'))
        return access(name, R_OK) == 0;

    char path[4096];

    /* For <...> search system/user paths */
    const StrArr *paths[] = { &st->user_paths, &st->sys_paths };
    for (int pi = 0; pi < 2; pi++) {
        const StrArr *arr = paths[pi];
        for (size_t i = 0; i < arr->len; i++) {
            snprintf(path, sizeof path, "%s/%s", arr->data[i], name);
            if (access(path, R_OK) == 0) return true;
#ifdef _WIN32
            snprintf(path, sizeof path, "%s\\%s", arr->data[i], name);
            if (access(path, R_OK) == 0) return true;
#endif
        }
    }

    /* For "..." try relative to current directory too */
    if (!is_system) {
        if (access(name, R_OK) == 0) return true;
    }

    return false;
}

/* =========================================================================
 * Directive handlers
 * ====================================================================== */

static void handle_include(CppState *st, TokList *line, CppLoc loc,
                            const char *current_file, CppLang lang,
                            const char *skip_until) {
    /* C99 6.10.2: If a #include directive already uses the <...> or "..."
     * syntax directly (not via a macro), the content between delimiters
     * must NOT be macro-expanded.  Only #include MACRO (where MACRO
     * expands to a header name) requires expansion.
     *
     * Strategy: check the first non-whitespace token BEFORE expansion.
     *   - If it's '<' or '"': parse header name from raw tokens (no expansion)
     *   - Otherwise: macro-expand the line, then parse the result */

    /* Find first non-whitespace token */
    TokNode *first = line->head;
    while (first && first->tok.kind == CPPT_SPACE) first = first->next;

    char name[4096];
    bool is_sys = false;
    bool used_expansion = false;

    if (first && first->tok.kind == CPPT_STRING_LIT) {
        /* "name.h" — raw, no expansion */
        const char *sp = pptok_spell(&first->tok);
        size_t len = strlen(sp);
        if (len >= 2 && sp[0] == '"' && sp[len-1] == '"')
            snprintf(name, sizeof name, "%.*s", (int)(len-2), sp+1);
        else snprintf(name, sizeof name, "%s", sp);
        is_sys = false;
    } else if (first && first->tok.kind == CPPT_PUNCT &&
               strcmp(pptok_spell(&first->tok), "<") == 0) {
        /* <name.h> — raw, no expansion */
        StrBuf sb = {0};
        TokNode *n = first->next;
        while (n && !(n->tok.kind == CPPT_PUNCT &&
                      strcmp(pptok_spell(&n->tok), ">") == 0)) {
            sb_push_cstr(&sb, pptok_spell(&n->tok));
            n = n->next;
        }
        snprintf(name, sizeof name, "%s", sb.buf ? sb.buf : "");
        sb_free(&sb);
        is_sys = true;
    } else {
        /* Not a direct header name — macro-expand and re-parse */
        TokList expanded = {0};
        macro_expand(line, st->macros, st->interns, st->diags, &expanded);
        used_expansion = true;

        TokNode *n = expanded.head;
        while (n && n->tok.kind == CPPT_SPACE) n = n->next;
        if (!n) {
            emit_diag(st, CPP_DIAG_ERROR, loc, "#include expects a filename");
            tl_free(&expanded);
            return;
        }

        const char *sp = pptok_spell(&n->tok);
        if (n->tok.kind == CPPT_STRING_LIT) {
            size_t len = strlen(sp);
            if (len >= 2 && sp[0] == '"' && sp[len-1] == '"')
                snprintf(name, sizeof name, "%.*s", (int)(len-2), sp+1);
            else snprintf(name, sizeof name, "%s", sp);
            is_sys = false;
        } else if (n->tok.kind == CPPT_PUNCT && strcmp(sp, "<") == 0) {
            StrBuf sb = {0};
            n = n->next;
            while (n && !(n->tok.kind == CPPT_PUNCT &&
                          strcmp(pptok_spell(&n->tok), ">") == 0)) {
                sb_push_cstr(&sb, pptok_spell(&n->tok));
                n = n->next;
            }
            snprintf(name, sizeof name, "%s", sb.buf ? sb.buf : "");
            sb_free(&sb);
            is_sys = true;
        } else {
            emit_diag(st, CPP_DIAG_ERROR, loc, "invalid #include argument");
            tl_free(&expanded);
            return;
        }
        tl_free(&expanded);
    }

    if (st->include_depth >= st->max_include_depth) {
        emit_diag(st, CPP_DIAG_FATAL, loc, "#include nesting too deep (max %d)",
                  st->max_include_depth);
        return;
    }

    const char *iname = intern_cstr(st->interns, name);

    /* Include-guard optimisation */
    if (guard_already_included(st, iname)) return;

    /* All #include are treated uniformly: find the file and preprocess it.
     * Whether the result causes downstream errors is the concern of the
     * lexer/parser, not the preprocessor. */
    char *found = find_include_ex(st, name, is_sys, current_file, skip_until);
    if (!found) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "file not found: %s", name);
        return;
    }

    const char *found_interned = intern_cstr(st->interns, found);
    free(found);

    /* Second guard check with the resolved absolute path.
     * The first check (above) uses the raw #include name; this one uses the
     * canonical path so that #pragma once guards stored under the absolute
     * path are honoured even when included via a relative name. */
    if (guard_already_included(st, found_interned)) return;

    st->include_depth++;
    /* process_file itself emits the # 1 "file" marker — don't duplicate it. */
    process_file(st, found_interned, lang);
    st->include_depth--;
    /* Resume linemarker for the including file (immediate, not pending). */
    emit_linemarker(st, loc.line + 1, current_file);
}

static void handle_define(CppState *st, TokList *line, CppLoc loc) {
    MacroDef *def = macro_parse_define(line, st->interns, st->diags);
    if (def) macro_define(st->macros, def, st->diags, loc);
}

static void handle_undef(CppState *st, TokList *line, CppLoc loc) {
    TokNode *n = line->head;
    while (n && n->tok.kind == CPPT_SPACE) n = n->next;
    if (!n || n->tok.kind != CPPT_IDENT) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "expected macro name after #undef");
        return;
    }
    macro_undef(st->macros, pptok_spell(&n->tok));
}

static void handle_if_common(CppState *st, bool condition, CppLoc loc) {
    if (st->cond_depth >= COND_STACK_MAX) {
        emit_diag(st, CPP_DIAG_FATAL, loc, "#if nesting too deep");
        return;
    }
    /* Capture parent liveness BEFORE pushing the new frame — in_live_branch()
     * iterates [0, cond_depth) so it must be called before cond_depth is
     * incremented, otherwise it reads the uninitialized frame we just allocated. */
    bool parent_live = in_live_branch(st);
    CondFrame *f = &st->cond_stack[st->cond_depth++];
    f->in_true_branch = condition && parent_live;
    f->ever_true      = condition;
    f->has_else       = false;
    f->open_loc       = loc;
}

/* Resolve all `defined(X)` and `defined X` occurrences in-place,
 * replacing them with pp-number "1" or "0", BEFORE macro expansion.
 * This prevents the argument from being expanded away (ISO C11 §6.10.1). */
static TokList resolve_defined(MacroTable *mt, const TokList *in) {
    TokList out = {0};
    for (TokNode *n = in->head; n; n = n->next) {
        /* Look for `defined` identifier */
        if (n->tok.kind == CPPT_IDENT &&
            strcmp(pptok_spell(&n->tok), "defined") == 0) {
            TokNode *nx = n->next;
            /* skip whitespace */
            while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
            bool have_paren = false;
            if (nx && nx->tok.kind == CPPT_PUNCT &&
                strcmp(pptok_spell(&nx->tok), "(") == 0) {
                have_paren = true;
                nx = nx->next;
                while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
            }
            if (nx && nx->tok.kind == CPPT_IDENT) {
                bool def = macro_lookup(mt, pptok_spell(&nx->tok)) != NULL;
                nx = nx->next;
                if (have_paren) {
                    while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
                    if (nx && nx->tok.kind == CPPT_PUNCT &&
                        strcmp(pptok_spell(&nx->tok), ")") == 0)
                        nx = nx->next;
                }
                /* emit 1 or 0 */
                PPTok r = {0};
                r.kind = CPPT_PP_NUMBER;
                r.loc  = n->tok.loc;
                sb_push_cstr(&r.spell, def ? "1" : "0");
                tl_append(&out, r);
                /* Append remaining tokens from nx onwards and return */
                for (TokNode *rest = nx; rest; rest = rest->next)
                    tl_append_copy(&out, &rest->tok);
                return out;
            }
            /* Not a proper defined() — emit `defined` as-is and continue */
            tl_append_copy(&out, &n->tok);
        } else {
            tl_append_copy(&out, &n->tok);
        }
    }
    return out;
}

/* Iteratively resolve ALL defined() occurrences (handles multiple per line). */
static TokList resolve_all_defined(MacroTable *mt, const TokList *in) {
    TokList cur = {0};
    for (TokNode *n = in->head; n; n = n->next)
        tl_append_copy(&cur, &n->tok);

    /* Repeat until no more `defined` tokens remain */
    for (int pass = 0; pass < 64; pass++) {
        bool found = false;
        for (TokNode *n = cur.head; n; n = n->next)
            if (n->tok.kind == CPPT_IDENT &&
                strcmp(pptok_spell(&n->tok), "defined") == 0)
                { found = true; break; }
        if (!found) break;
        TokList next = resolve_defined(mt, &cur);
        tl_free(&cur);
        cur = next;
    }
    return cur;
}

/* Resolve `__has_include("file")` / `__has_include(<file>)` in-place,
 * replacing with pp-number "1" or "0" (C23 §6.10.1). Must run BEFORE
 * macro expansion because the header name must not be expanded. */
static TokList resolve_has_include(CppState *st, const TokList *in) {
    TokList out = {0};
    for (TokNode *n = in->head; n; n = n->next) {
        if (n->tok.kind == CPPT_IDENT &&
            (strcmp(pptok_spell(&n->tok), "__has_include") == 0 ||
             strcmp(pptok_spell(&n->tok), "__has_include_next") == 0)) {
            bool is_next = (strcmp(pptok_spell(&n->tok), "__has_include_next") == 0);
            bool result = false;
            TokNode *nx = n->next;
            while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
            if (nx && nx->tok.kind == CPPT_PUNCT &&
                strcmp(pptok_spell(&nx->tok), "(") == 0) {
                nx = nx->next;
                while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
                if (nx && nx->tok.kind == CPPT_STRING_LIT) {
                    /* __has_include("file.h") */
                    const char *sp = pptok_spell(&nx->tok);
                    size_t len = strlen(sp);
                    if (len >= 2 && sp[0] == '"' && sp[len-1] == '"') {
                        char name_buf[4096];
                        size_t nlen = len - 2;
                        if (nlen < sizeof(name_buf)) {
                            memcpy(name_buf, sp + 1, nlen);
                            name_buf[nlen] = '\0';
                            result = cpp_has_include(st, name_buf, false);
                        }
                    }
                    nx = nx->next;
                } else if (nx && nx->tok.kind == CPPT_PUNCT &&
                           strcmp(pptok_spell(&nx->tok), "<") == 0) {
                    /* __has_include(<file.h>) */
                    StrBuf hdr = {0};
                    nx = nx->next;
                    while (nx && !(nx->tok.kind == CPPT_PUNCT &&
                                   strcmp(pptok_spell(&nx->tok), ">") == 0)) {
                        sb_push(&hdr, pptok_spell(&nx->tok),
                                strlen(pptok_spell(&nx->tok)));
                        nx = nx->next;
                    }
                    if (nx) nx = nx->next; /* skip '>' */
                    if (hdr.len > 0) {
                        result = cpp_has_include(st, hdr.buf, true);
                    }
                    sb_free(&hdr);
                }
                /* skip ')' */
                while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
                if (nx && nx->tok.kind == CPPT_PUNCT &&
                    strcmp(pptok_spell(&nx->tok), ")") == 0)
                    nx = nx->next;
            } else {
                /* Not __has_include(...) — leave as-is */
                tl_append_copy(&out, &n->tok);
                continue;
            }
            (void)is_next; /* __has_include_next needs extra logic, for now same */
            PPTok r = {0};
            r.kind = CPPT_PP_NUMBER;
            r.loc  = n->tok.loc;
            sb_push_cstr(&r.spell, result ? "1" : "0");
            tl_append(&out, r);
            /* Append remaining tokens */
            for (TokNode *rest = nx; rest; rest = rest->next)
                tl_append_copy(&out, &rest->tok);
            return out;
        } else {
            tl_append_copy(&out, &n->tok);
        }
    }
    return out;
}

static TokList resolve_all_has_include(CppState *st, const TokList *in) {
    TokList cur = {0};
    for (TokNode *n = in->head; n; n = n->next)
        tl_append_copy(&cur, &n->tok);
    for (int pass = 0; pass < 64; pass++) {
        bool found = false;
        for (TokNode *n = cur.head; n; n = n->next)
            if (n->tok.kind == CPPT_IDENT &&
                (strcmp(pptok_spell(&n->tok), "__has_include") == 0 ||
                 strcmp(pptok_spell(&n->tok), "__has_include_next") == 0))
                { found = true; break; }
        if (!found) break;
        TokList next = resolve_has_include(st, &cur);
        tl_free(&cur);
        cur = next;
    }
    return cur;
}

static void handle_if(CppState *st, TokList *line, CppLoc loc) {
    /* Step 1: resolve defined() BEFORE macro expansion (ISO C11 §6.10.1p4) */
    TokList preresolved = resolve_all_defined(st->macros, line);
    /* Step 1b: resolve __has_include(...) (C23 §6.10.1) */
    TokList hi_resolved = resolve_all_has_include(st, &preresolved);
    tl_free(&preresolved);
    /* Step 2: macro-expand the remaining tokens */
    TokList expanded = {0};
    macro_expand(&hi_resolved, st->macros, st->interns, st->diags, &expanded);
    tl_free(&hi_resolved);
    /* Step 3: evaluate the constant expression */
    bool err = false;
    intmax_t val = cpp_eval_if_expr(&expanded, st->macros, st->interns,
                                     st->diags, &err);
    tl_free(&expanded);
    handle_if_common(st, !err && val != 0, loc);
}

static void handle_ifdef(CppState *st, TokList *line, CppLoc loc, bool invert) {
    TokNode *n = line->head;
    while (n && n->tok.kind == CPPT_SPACE) n = n->next;
    if (!n || n->tok.kind == CPPT_NEWLINE) {
        emit_diag(st, CPP_DIAG_ERROR, loc,
                  invert ? "#ifndef with no macro name" : "#ifdef with no macro name");
        handle_if_common(st, false, loc);
        return;
    }
    if (n->tok.kind != CPPT_IDENT) {
        emit_diag(st, CPP_DIAG_ERROR, n->tok.loc,
                  invert ? "#ifndef requires a macro name" : "#ifdef requires a macro name");
        handle_if_common(st, false, loc);
        return;
    }
    bool defined = macro_lookup(st->macros, pptok_spell(&n->tok)) != NULL;
    handle_if_common(st, invert ? !defined : defined, loc);
}

static void handle_elif(CppState *st, TokList *line, CppLoc loc) {
    if (st->cond_depth == 0) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "#elif without #if");
        return;
    }
    CondFrame *f = &st->cond_stack[st->cond_depth - 1];
    if (f->has_else) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "#elif after #else");
        return;
    }
    if (f->ever_true) {
        f->in_true_branch = false;
        return;
    }

    /* Step 1: resolve defined() BEFORE macro expansion (ISO C11 §6.10.1p4) */
    TokList preresolved = resolve_all_defined(st->macros, line);
    /* Step 1b: resolve __has_include(...) (C23 §6.10.1) */
    TokList hi_resolved = resolve_all_has_include(st, &preresolved);
    tl_free(&preresolved);
    /* Step 2: macro-expand the remaining tokens */
    TokList expanded = {0};
    macro_expand(&hi_resolved, st->macros, st->interns, st->diags, &expanded);
    tl_free(&hi_resolved);
    /* Step 3: evaluate the constant expression */
    bool err = false;
    intmax_t val = cpp_eval_if_expr(&expanded, st->macros, st->interns,
                                     st->diags, &err);
    tl_free(&expanded);
    bool cond = !err && val != 0;

    /* Only activate if parent scopes are all live */
    bool parent_live = true;
    for (int i = 0; i < st->cond_depth - 1; i++)
        if (!st->cond_stack[i].in_true_branch) { parent_live = false; break; }
    f->in_true_branch = cond && parent_live;
    if (cond) f->ever_true = true;
}

static void handle_else(CppState *st, CppLoc loc) {
    if (st->cond_depth == 0) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "#else without #if");
        return;
    }
    CondFrame *f = &st->cond_stack[st->cond_depth - 1];
    if (f->has_else) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "duplicate #else");
        return;
    }
    f->has_else = true;
    bool parent_live = true;
    for (int i = 0; i < st->cond_depth - 1; i++)
        if (!st->cond_stack[i].in_true_branch) { parent_live = false; break; }
    f->in_true_branch = !f->ever_true && parent_live;
}

static void handle_endif(CppState *st, CppLoc loc) {
    if (st->cond_depth == 0) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "#endif without #if");
        return;
    }
    st->cond_depth--;
}

static void handle_line(CppState *st, CppReader *rd, TokList *line, CppLoc loc) {
    /* #line digit-sequence ["filename"] */
    TokList expanded = {0};
    macro_expand(line, st->macros, st->interns, st->diags, &expanded);
    TokNode *n = expanded.head;
    while (n && n->tok.kind == CPPT_SPACE) n = n->next;
    if (!n || n->tok.kind != CPPT_PP_NUMBER) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "#line requires a line number");
        tl_free(&expanded);
        return;
    }
    bool uns;
    long new_line = (long)parse_int_literal_pub(pptok_spell(&n->tok), &uns);
    n = n->next;
    while (n && n->tok.kind == CPPT_SPACE) n = n->next;
    const char *new_file = loc.file;
    if (n && n->tok.kind == CPPT_STRING_LIT) {
        const char *sp = pptok_spell(&n->tok);
        size_t len = strlen(sp);
        char tmp[4096];
        if (len >= 2 && sp[0] == '"' && sp[len-1] == '"')
            snprintf(tmp, sizeof tmp, "%.*s", (int)(len-2), sp+1);
        else snprintf(tmp, sizeof tmp, "%s", sp);
        new_file = intern_cstr(st->interns, tmp);
    }
    /* Actually update the reader's line number.
     * #line N means the directive line itself is N, so the next source line
     * is N+1. We set the reader to N so the newline after the directive
     * increments it to N+1. */
    if (rd) reader_set_line(rd, (int)new_line);
    emit_linemarker(st, (int)new_line, new_file);
    tl_free(&expanded);
}

static void handle_error(CppState *st, TokList *line, CppLoc loc, bool is_warning) {
    StrBuf sb = {0};
    bool first = true;
    for (TokNode *n = line->head; n; n = n->next) {
        if (n->tok.kind == CPPT_NEWLINE) break;
        if (!first) sb_push_ch(&sb, ' ');
        sb_push_cstr(&sb, pptok_spell(&n->tok));
        first = false;
    }
    emit_diag(st, is_warning ? CPP_DIAG_WARNING : CPP_DIAG_ERROR,
              loc, "%s", sb.buf ? sb.buf : "");
    sb_free(&sb);
}

static void handle_pragma(CppState *st, TokList *line, CppLoc loc,
                          const char *current_file) {
    /* Collect pragma text */
    StrBuf sb = {0};
    TokNode *first = line->head;
    while (first && first->tok.kind == CPPT_SPACE) first = first->next;

    /* #pragma once: mark this file as already-included with a sentinel macro */
    if (first && first->tok.kind == CPPT_IDENT &&
        strcmp(pptok_spell(&first->tok), "once") == 0) {
        if (current_file) {
            /* Derive a sentinel macro name from the filename:
             * "__SHARP_ONCE_<mangled_path>__"                             */
            StrBuf macro = {0};
            sb_push_cstr(&macro, "__SHARP_ONCE_");
            for (const char *p = current_file; *p; p++) {
                char c = *p;
                if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9'))
                    sb_push_ch(&macro, c);
                else
                    sb_push_ch(&macro, '_');
            }
            sb_push_cstr(&macro, "__");
            const char *mname = intern_cstr(st->interns, macro.buf);
            sb_free(&macro);
            CppLoc bloc = {intern_cstr(st->interns,"<pragma once>"),0,0};
            macro_define_object(st->macros, st->interns, mname, "1", bloc);
            guard_set(st, intern_cstr(st->interns, current_file), mname);
        }
        return;  /* do not emit #pragma once to output */
    }

    /* Macro-expand the pragma body so that #pragma pack(push, _CRT_PACKING)
     * with _CRT_PACKING=8 becomes "#pragma pack(push, 8)" instead of the raw
     * token string that would have a literal _CRT_PACKING identifier.      */
    TokList expanded = {0};
    macro_expand(line, st->macros, st->interns, st->diags, &expanded);

    bool pragma_first = true;
    for (TokNode *n = expanded.head; n; n = n->next) {
        if (!pragma_first) sb_push_ch(&sb, ' ');
        sb_push_cstr(&sb, pptok_spell(&n->tok));
        pragma_first = false;
    }

    tl_free(&expanded);

    bool suppress = false;
    if (st->pragma_cb)
        suppress = st->pragma_cb(NULL, sb.buf ? sb.buf : "", loc, st->pragma_ud);

    if (!suppress && in_live_branch(st)) {
        flush_pending_lm(st);
        sb_printf(&st->out_text, "#pragma %s\n", sb.buf ? sb.buf : "");
        st->out_newlines++;
    }
    sb_free(&sb);
}

/* Helper from expr.c — we need the public signature here. */
long parse_int_literal_pub(const char *sp, bool *is_unsigned);

/* =========================================================================
 * Dynamic built-in macros: __FILE__ and __LINE__
 * ====================================================================== */

static PPTok make_file_tok(CppState *st, CppLoc loc) {
    PPTok t = {0};
    t.kind = CPPT_STRING_LIT;
    t.loc  = loc;
    char buf[4096];
    snprintf(buf, sizeof buf, "\"%s\"", loc.file ? loc.file : "");
    sb_push_cstr(&t.spell, buf);
    return t;
}

static PPTok make_line_tok(CppState *st, CppLoc loc) {
    PPTok t = {0};
    t.kind = CPPT_PP_NUMBER;
    t.loc  = loc;
    char buf[32];
    snprintf(buf, sizeof buf, "%d", loc.line);
    sb_push_cstr(&t.spell, buf);
    return t;
}

static PPTok make_timestamp_tok(CppState *st, CppLoc loc) {
    PPTok t = {0};
    t.kind = CPPT_STRING_LIT;
    t.loc  = loc;
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    const char *days[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
    const char *mons[] = { "Jan","Feb","Mar","Apr","May","Jun",
                           "Jul","Aug","Sep","Oct","Nov","Dec" };
    char ts[256];
    snprintf(ts, sizeof ts, "\"%s %s %02d %02d:%02d:%02d %04d\"",
             days[tm->tm_wday], mons[tm->tm_mon], tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_year + 1900);
    sb_push_cstr(&t.spell, ts);
    return t;
}

/* =========================================================================
 * Token processing — read from reader, handle directives, expand macros,
 * emit to output.
 * ====================================================================== */

static void process_buf(CppState *st, CppReader *rd, CppLang lang) {
    bool at_bol = true;

    for (;;) {
        PPTok t = reader_next_tok(rd, st->keep_comments);
        if (t.kind == CPPT_EOF) { pptok_free(&t); break; }
        if (st->fatal)          { pptok_free(&t); break; }

        /* Track beginning-of-line */
        if (t.kind == CPPT_NEWLINE) {
            at_bol = true;
            if (in_live_branch(st)) {
                sb_push_ch(&st->out_text, '\n');
                st->out_newlines++;
            }
            pptok_free(&t);
            continue;
        }

        /* Skip comments (they were either kept or turned to spaces by lexer) */
        if (t.kind == CPPT_COMMENT) {
            if (in_live_branch(st) && st->keep_comments)
                sb_push_cstr(&st->out_text, pptok_spell(&t));
            pptok_free(&t);
            continue;
        }

        if (t.kind == CPPT_SPACE) {
            if (in_live_branch(st))
                sb_push_ch(&st->out_text, ' ');
            pptok_free(&t);
            continue;
        }

        /* ------------------------------------------------------------- */
        /* Directive: '#' at beginning of a logical line                   */
        if (t.kind == CPPT_PUNCT &&
            strcmp(pptok_spell(&t), "#") == 0 && at_bol) {
            CppLoc dir_loc = t.loc;
            pptok_free(&t);
            at_bol = false;

            /* Read the directive keyword */
            PPTok kw = reader_next_tok(rd, false);
            while (kw.kind == CPPT_SPACE) {
                pptok_free(&kw);
                kw = reader_next_tok(rd, false);
            }

            /* Null directive: lone '#' on a line */
            if (kw.kind == CPPT_NEWLINE || kw.kind == CPPT_EOF) {
                pptok_free(&kw);
                at_bol = true;
                continue;
            }

            /* Copy name before freeing kw — pptok_spell() points into kw.spell.buf */
            char dname_buf[64] = {0};
            if (kw.kind == CPPT_IDENT) {
                const char *sp = pptok_spell(&kw);
                size_t l = strlen(sp);
                if (l >= sizeof dname_buf) l = sizeof dname_buf - 1;
                memcpy(dname_buf, sp, l);
            }
            const char *dname = dname_buf;

            /* Collect the rest of the directive line */
            TokList line = collect_directive_line(rd, false);
            pptok_free(&kw);

            /* Now dispatch (always process structural directives even in
             * dead branches so we can track nesting correctly).          */
            if      (strcmp(dname, "if")     == 0) {
                handle_if(st, &line, dir_loc);
            } else if (strcmp(dname, "ifdef")  == 0) {
                handle_ifdef(st, &line, dir_loc, false);
            } else if (strcmp(dname, "ifndef") == 0) {
                handle_ifdef(st, &line, dir_loc, true);
            } else if (strcmp(dname, "elif")   == 0) {
                handle_elif(st, &line, dir_loc);
            } else if (strcmp(dname, "else")   == 0) {
                handle_else(st, dir_loc);
            } else if (strcmp(dname, "endif")  == 0) {
                handle_endif(st, dir_loc);
            } else if (in_live_branch(st)) {
                /* Remaining directives only execute in live branches */
                if      (strcmp(dname, "include") == 0) {
                    handle_include(st, &line, dir_loc,
                                   reader_filename(rd), lang, NULL);
                } else if (strcmp(dname, "include_next") == 0) {
                    handle_include(st, &line, dir_loc,
                                   reader_filename(rd), lang,
                                   reader_filename(rd));  /* skip_until=current */
                } else if (strcmp(dname, "define") == 0) {
                    handle_define(st, &line, dir_loc);
                } else if (strcmp(dname, "undef")  == 0) {
                    handle_undef(st, &line, dir_loc);
                } else if (strcmp(dname, "line")   == 0) {
                    handle_line(st, rd, &line, dir_loc);
                } else if (strcmp(dname, "error")  == 0) {
                    handle_error(st, &line, dir_loc, false);
                } else if (strcmp(dname, "warning") == 0) {
                    handle_error(st, &line, dir_loc, true);
                } else if (strcmp(dname, "pragma") == 0) {
                    handle_pragma(st, &line, dir_loc, reader_filename(rd));
                } else if (strcmp(dname, "ident")  == 0) {
                    /* #ident is a legacy directive — silently ignored */
                } else {
                    emit_diag(st, CPP_DIAG_ERROR, dir_loc,
                              "unknown directive: #%s", dname);
                }
            }

            tl_free(&line);

            /* Queue a linemarker for after directives so that consecutive
             * #define / #ifdef / etc. lines don't flood the output with
             * back-to-back markers.  The marker fires lazily just before the
             * next real content token (via flush_pending_lm).              */
            queue_linemarker(st, reader_current_line(rd), reader_filename(rd));

            at_bol = true;
            continue;
        }

        at_bol = false;

        /* Skip tokens in dead conditional branches */
        if (!in_live_branch(st)) { pptok_free(&t); continue; }

        /* ------------------------------------------------------------- */
        /* Normal token — potentially a macro invocation                   */
        if (t.kind == CPPT_IDENT) {
            const char *name = pptok_spell(&t);

            /* Dynamic built-ins */
            if (strcmp(name, "__FILE__") == 0) {
                PPTok ft = make_file_tok(st, t.loc);
                ft.has_leading_space = t.has_leading_space;
                pptok_free(&t);
                emit_tok_text(st, &ft);
                pptok_free(&ft);
                continue;
            }
            if (strcmp(name, "__LINE__") == 0) {
                PPTok lt = make_line_tok(st, t.loc);
                lt.has_leading_space = t.has_leading_space;
                pptok_free(&t);
                emit_tok_text(st, &lt);
                pptok_free(&lt);
                continue;
            }
            if (strcmp(name, "__COUNTER__") == 0) {
                char cbuf[32];
                snprintf(cbuf, sizeof cbuf, "%d", st->counter++);
                PPTok ct = {0};
                ct.kind = CPPT_PP_NUMBER;
                ct.loc  = t.loc;
                ct.has_leading_space = t.has_leading_space;
                sb_push_cstr(&ct.spell, cbuf);
                pptok_free(&t);
                emit_tok_text(st, &ct);
                pptok_free(&ct);
                continue;
            }
            if (strcmp(name, "__TIMESTAMP__") == 0) {
                PPTok tt = make_timestamp_tok(st, t.loc);
                tt.has_leading_space = t.has_leading_space;
                pptok_free(&t);
                emit_tok_text(st, &tt);
                pptok_free(&tt);
                continue;
            }

            /* __attribute__((...)) — GCC extension.  Consume the double-parenthesized
             * argument and discard it entirely.  MSVC does not use this syntax.     */
            if (strcmp(name, "__attribute__") == 0) {
                pptok_free(&t);
                /* Skip whitespace to find first '(' */
                PPTok nx = reader_next_tok(rd, false);
                while (nx.kind == CPPT_SPACE || nx.kind == CPPT_NEWLINE) {
                    if (nx.kind == CPPT_NEWLINE && in_live_branch(st))
                        sb_push_ch(&st->out_text, '\n');
                    pptok_free(&nx);
                    nx = reader_next_tok(rd, false);
                }
                if (nx.kind == CPPT_PUNCT && strcmp(pptok_spell(&nx), "(") == 0) {
                    pptok_free(&nx);
                    /* Collect tokens until matching ')' of outer parens.
                     * __attribute__((...)) has nested parens.             */
                    int depth = 1;
                    while (depth > 0) {
                        PPTok at = reader_next_tok(rd, false);
                        if (at.kind == CPPT_EOF) { pptok_free(&at); break; }
                        if (at.kind == CPPT_PUNCT) {
                            const char *sp = pptok_spell(&at);
                            if (strcmp(sp, "(") == 0) depth++;
                            else if (strcmp(sp, ")") == 0) { depth--; pptok_free(&at); if (depth == 0) break; }
                        }
                        pptok_free(&at);
                    }
                    /* __attribute__ is discarded — output nothing */
                } else {
                    /* Not followed by '(' — pass through as identifier */
                    emit_tok_text(st, &t);
                    pptok_free(&nx);
                }
                continue;
            }

            /* __pragma(expr) — MSVC keyword.  Consume the parenthesized
             * argument and re-emit it verbatim so that cl.exe can handle
             * it natively.  Do NOT treat it as a #pragma directive —
             * __pragma is an inline form that must remain inline.      */
            if (strcmp(name, "__pragma") == 0) {
                /* Skip whitespace to find '(' */
                PPTok nx = reader_next_tok(rd, false);
                while (nx.kind == CPPT_SPACE || nx.kind == CPPT_NEWLINE) {
                    if (nx.kind == CPPT_NEWLINE && in_live_branch(st))
                        sb_push_ch(&st->out_text, '\n');
                    pptok_free(&nx);
                    nx = reader_next_tok(rd, false);
                }
                if (nx.kind == CPPT_PUNCT && strcmp(pptok_spell(&nx), "(") == 0) {
                    pptok_free(&nx);
                    /* Output: __pragma( */
                    if (in_live_branch(st)) {
                        flush_pending_lm(st);
                        if (t.has_leading_space) sb_push_ch(&st->out_text, ' ');
                        sb_push_cstr(&st->out_text, "__pragma(");
                    }
                    /* Collect and output tokens until matching ')' */
                    int depth = 1;
                    while (depth > 0) {
                        PPTok at = reader_next_tok(rd, false);
                        if (at.kind == CPPT_EOF) { pptok_free(&at); break; }
                        if (at.kind == CPPT_PUNCT) {
                            const char *sp = pptok_spell(&at);
                            if (strcmp(sp, "(") == 0) depth++;
                            else if (strcmp(sp, ")") == 0) { depth--; pptok_free(&at); if (depth == 0) break; }
                        }
                        if (in_live_branch(st)) {
                            if (at.has_leading_space) sb_push_ch(&st->out_text, ' ');
                            sb_push_cstr(&st->out_text, pptok_spell(&at));
                        }
                        pptok_free(&at);
                    }
                    if (in_live_branch(st)) sb_push_ch(&st->out_text, ')');
                    pptok_free(&t);
                } else {
                    /* Not followed by '(' — pass through as identifier */
                    emit_tok_text(st, &t);
                    pptok_free(&nx);
                }
                continue;
            }

            /* Try macro expansion */
            if (!t.hide && macro_lookup(st->macros, name)) {
                /* Expansion limits breached? Pass through unexpanded */
                if (macro_limits_breached(st->macros)) {
                    emit_tok_text(st, &t);
                    pptok_free(&t);
                    continue;
                }

                bool macro_had_leading_space = t.has_leading_space;

                /* Build a small input list starting with this token, then
                 * accumulate more if needed for function-like macros.    */
                TokList input = {0};
                tl_append(&input, t); /* t.spell ownership transferred */

                /* For function-like macros we need to grab ahead, but
                 * the expander handles that internally via the list.
                 * Here we feed a single-token list and let macro_expand
                 * pull from the reader when needed.                       */

                /* We need to prime the list with enough look-ahead for
                 * function-like macros (the argument list).  Peek.       */
                bool _is_func = macro_lookup_is_func(st->macros, name);
                if (_is_func) {
                    /* Collect tokens until matching ')' and append. */
                    PPTok nx = reader_next_tok(rd, false);
                    while (nx.kind == CPPT_SPACE || nx.kind == CPPT_NEWLINE) {
                        tl_append(&input, nx);
                        nx = reader_next_tok(rd, false);
                    }
                    if (nx.kind == CPPT_PUNCT &&
                        strcmp(pptok_spell(&nx), "(") == 0) {
                        tl_append(&input, nx);
                        int depth = 1;
                        while (depth > 0) {
                            PPTok at = reader_next_tok(rd, false);
                            tl_append(&input, at);
                            if (at.kind == CPPT_EOF) break;
                            if (at.kind == CPPT_PUNCT) {
                                if (strcmp(pptok_spell(&at), "(") == 0) depth++;
                                else if (strcmp(pptok_spell(&at), ")") == 0) depth--;
                            }
                        }
                    } else {
                        /* Not followed by '(' — pass macro name through. */
                        emit_tok_text(st, &input.head->tok);
                        tl_free(&input);
                        /* Put nx back by creating a one-token list */
                        emit_tok_text(st, &nx);
                        pptok_free(&nx);
                        continue;
                    }
                }

                TokList expanded = {0};
                macro_expand(&input, st->macros, st->interns, st->diags, &expanded);
                tl_free(&input);

                /* Propagate leading space from macro invocation to first expanded token */
                if (macro_had_leading_space && expanded.head)
                    expanded.head->tok.has_leading_space = true;

                /* Rescan: after expansion, if the result is a single IDENT that is
                 * a function-like macro, peek at the reader to see if the next
                 * non-whitespace token is `(`.  If so, collect the args, rescan,
                 * and re-expand.  Handles: #define A B  →  B(args).
                 * We peek by reading ahead and storing tokens, then using them
                 * in the rescan input (they won't be read again by the main loop). */
                {
                    bool rescanned = false;
                    if (expanded.head && expanded.head->next == NULL &&
                        expanded.head->tok.kind == CPPT_IDENT &&
                        !expanded.head->tok.hide &&
                        macro_lookup_is_func(st->macros, pptok_spell(&expanded.head->tok))) {
                        /* Peek ahead: read tokens until we find `(` or non-ws */
                        TokList peek_ws = {0};
                        PPTok first = reader_next_tok(rd, false);
                        while (first.kind == CPPT_SPACE || first.kind == CPPT_NEWLINE) {
                            tl_append_copy(&peek_ws, &first);
                            pptok_free(&first);
                            first = reader_next_tok(rd, false);
                        }

                        if (first.kind == CPPT_PUNCT &&
                            strcmp(pptok_spell(&first), "(") == 0) {
                            /* Build rescan input: expanded ident + peeked ws + ( + args + ) */
                            TokList r_input = {0};
                            {
                                PPTok cpy = expanded.head->tok;
                                cpy.spell = (StrBuf){0};
                                sb_push_cstr(&cpy.spell, pptok_spell(&expanded.head->tok));
                                cpy.has_leading_space = expanded.head->tok.has_leading_space;
                                tl_append(&r_input, cpy);
                            }
                            for (TokNode *wn = peek_ws.head; wn; wn = wn->next)
                                tl_append_copy(&r_input, &wn->tok);
                            tl_append_copy(&r_input, &first);

                            int depth = 1;
                            while (depth > 0) {
                                PPTok at = reader_next_tok(rd, false);
                                if (at.kind == CPPT_EOF) { pptok_free(&at); break; }
                                tl_append_copy(&r_input, &at);
                                if (at.kind == CPPT_PUNCT) {
                                    if (strcmp(pptok_spell(&at), "(") == 0) depth++;
                                    else if (strcmp(pptok_spell(&at), ")") == 0) depth--;
                                }
                                pptok_free(&at);
                            }
                            pptok_free(&first);
                            tl_free(&peek_ws);

                            TokList r_expanded = {0};
                            macro_expand(&r_input, st->macros, st->interns, st->diags, &r_expanded);
                            tl_free(&r_input);

                            for (TokNode *rn = r_expanded.head; rn; rn = rn->next)
                                emit_tok_text(st, &rn->tok);
                            tl_free(&r_expanded);
                            tl_free(&expanded);
                            rescanned = true;
                        } else {
                            /* Not `(` — emit the whitespace and first token back
                             * by emitting them normally, then the expanded token. */
                            pptok_free(&first);
                        }
                    }

                    if (!rescanned) {
                        for (TokNode *en = expanded.head; en; en = en->next)
                            emit_tok_text(st, &en->tok);
                    }
                    tl_free(&expanded);
                }
                continue;
            }
        }

        /* Pass token through as-is */
        emit_tok_text(st, &t);
        pptok_free(&t);
    }

    /* Warn about unclosed conditionals at end of each file */
    /* (Only report from the top-level file) */
}

static void process_file(CppState *st, const char *filename, CppLang lang) {
    CppReader *rd = reader_new_from_file(filename, st->interns, st->diags);
    if (!rd) {
        CppDiag d = { CPP_DIAG_FATAL, {filename, 0, 0},
                      cpp_xstrdup("cannot open file") };
        diag_push(st->diags, d);
        st->fatal = true;
        return;
    }
    emit_linemarker(st, 1, filename);
    process_buf(st, rd, lang);
    reader_free(rd);
}

/* =========================================================================
 * Public API (called from cpp.c)
 * ====================================================================== */

CppState *cpp_state_new(CppCtx *ctx, CppDiagArr *diags) {
    CppState *st = cpp_xmalloc(sizeof *st);
    memset(st, 0, sizeof *st);

    st->ctx               = ctx;
    st->interns           = &ctx->interns;
    st->diags             = diags;
    st->emit_linemarkers  = ctx->emit_linemarkers;
    st->keep_comments     = ctx->keep_comments;
    st->max_include_depth = ctx->max_include_depth;
    st->pragma_cb         = ctx->pragma_handler;
    st->pragma_ud         = ctx->pragma_userdata;
    st->macros            = macro_table_new(st->interns);

    /* Copy search paths */
    for (size_t i = 0; i < ctx->user_include_paths.len; i++)
        strarr_push(&st->user_paths, ctx->user_include_paths.data[i]);
    for (size_t i = 0; i < ctx->sys_include_paths.len; i++)
        strarr_push(&st->sys_paths, ctx->sys_include_paths.data[i]);

    return st;
}

void cpp_state_free(CppState *st) {
    macro_table_free(st->macros);
    /* Don't free user_paths/sys_paths — they are shallow-copied from CppCtx
     * and owned by the context. Only free the arrays, not the strings. */
    free(st->user_paths.data); st->user_paths.data = NULL;
    st->user_paths.len = st->user_paths.cap = 0;
    free(st->sys_paths.data); st->sys_paths.data = NULL;
    st->sys_paths.len = st->sys_paths.cap = 0;
    sb_free(&st->out_text);
    da_free(&st->out_tokens);
    for (int i = 0; i < GUARD_BUCKETS; i++) {
        GuardEntry *e = st->guards[i];
        while (e) { GuardEntry *nx = e->next; free(e); e = nx; }
    }
    free(st);
}

void cpp_state_run_file(CppState *st, const char *filename, CppLang lang) {
    install_builtins(st, lang);
    /* Apply command-line -D/-U AFTER builtins so -U can suppress them and
     * target-triple macros (from apply_target_macros in main.c) can override
     * the generic defaults set above.  This is the correct POSIX cc order. */
    apply_cmdline_macros(st);
    process_file(st, filename, lang);

    if (st->cond_depth > 0) {
        CppLoc loc = st->cond_stack[st->cond_depth-1].open_loc;
        emit_diag(st, CPP_DIAG_ERROR, loc, "unterminated #if at end of file");
    }
}

void cpp_state_run_buf(CppState *st, const char *buf, size_t len,
                        const char *filename, CppLang lang) {
    install_builtins(st, lang);
    apply_cmdline_macros(st);
    CppReader *rd = reader_new_from_buf(buf, len, filename,
                                         st->interns, st->diags);
    emit_linemarker(st, 1, filename);
    process_buf(st, rd, lang);
    reader_free(rd);

    if (st->cond_depth > 0) {
        CppLoc loc = st->cond_stack[st->cond_depth-1].open_loc;
        emit_diag(st, CPP_DIAG_ERROR, loc, "unterminated #if at end of file");
    }
}

const char  *cpp_state_text(const CppState *st)    { return st->out_text.buf; }
size_t       cpp_state_text_len(const CppState *st) { return st->out_text.len; }
CppTok      *cpp_state_tokens(const CppState *st)   { return st->out_tokens.data; }
size_t       cpp_state_ntokens(const CppState *st)  { return st->out_tokens.len;  }

/* Transfer ownership of the raw output text buffer to the caller.
 * After this call, st->out_text is empty (will not be double-freed).      */
char *cpp_state_take_text(CppState *st) { return sb_take(&st->out_text); }

/* Bridge used by cpp.c */
MacroTable *macro_state_table(CppState *st) { return st->macros; }

/* Used by directive.c's #line handler via directive.h declaration */
long parse_int_literal_pub(const char *sp, bool *is_unsigned) {
    const char *p = sp;
    int base = 10;
    unsigned long val = 0;
    *is_unsigned = false;
    if (*p == '0') {
        p++;
        if (*p == 'x' || *p == 'X') { base = 16; p++; }
        else base = 8;
    }
    for (; *p; p++) {
        char c = (char)tolower((unsigned char)*p);
        int digit;
        if      (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = 10 + (c - 'a');
        else break;
        if (digit >= base) break;
        val = val * (unsigned long)base + (unsigned long)digit;
    }
    while (*p == 'u' || *p == 'U') { *is_unsigned = true; p++; }
    return (long)val;
}

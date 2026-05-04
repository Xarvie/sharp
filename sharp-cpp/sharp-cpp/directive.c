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

/* Forward declaration */
static void fix_token_ptrs(CppState *st, const char *old_buf);


/* =========================================================================
 * Helpers
 * ====================================================================== */

static bool in_live_branch(const CppState *st) {
    for (int i = 0; i < st->cond_depth; i++)
        if (!st->cond_stack[i].in_true_branch)
            return false;
    return true;
}

/* Push a diagnostic into st->diags; bump st->fatal on FATAL level. */
static void emit_diag(CppState *st, CppDiagLevel lvl, CppLoc loc, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    CppDiag d = { lvl, loc, cpp_xstrdup(buf) };
    diag_push(st->diags, d);
    if (lvl == CPP_DIAG_FATAL) st->fatal = true;
}

/* Emit a CPP_DIAG_NOTE follow-up to a previous diagnostic.  The convention
 * is: emit the primary diagnostic (warning/error), then call this for any
 * supporting context the user needs to act on the message — e.g.
 *     emit_diag(st, CPP_DIAG_ERROR, here, "macro 'X' redefined");
 *     emit_note(st, prev_loc, "previous definition was here");                */
static void emit_note(CppState *st, CppLoc loc, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    CppDiag d = { CPP_DIAG_NOTE, loc, cpp_xstrdup(buf) };
    diag_push(st->diags, d);
}

/* Phase 4: warn if a directive line has unconsumed non-whitespace tokens
 * past `consumed`.  Called by the directives whose grammar finishes after
 * a known token (#endif: nothing; #else: nothing; #ifdef X: nothing after
 * the ident; #undef X: nothing after the ident).  Matches GCC's
 * -Wendif-labels family of warnings.
 *
 * `consumed` is the last token that was legitimately part of the
 * directive's argument; everything after it (skipping whitespace) is
 * trailing junk.  Pass NULL to warn on ALL non-whitespace tokens in `line`. */
static void warn_trailing(CppState *st, const char *dname, const TokList *line,
                          const TokNode *consumed, CppLoc loc) {
    TokNode *n = consumed ? consumed->next : line->head;
    while (n) {
        if (n->tok.kind != CPPT_SPACE &&
            n->tok.kind != CPPT_NEWLINE &&
            n->tok.kind != CPPT_COMMENT) {
            emit_diag(st, CPP_DIAG_WARNING, loc,
                      "extra tokens at end of #%s directive", dname);
            return;
        }
        n = n->next;
    }
}

/* Emit a linemarker: `# <line> "<file>"\n` (or with GCC flags appended:
 * `# <line> "<file>" 1 3\n`).  Also resets the newline counter so the
 * next flush_pending_lm() can detect whether a new marker is actually
 * needed.
 *
 * Phase 8: flag1 / flag2 are GCC linemarker flags (1 = entering new file,
 * 2 = returning from include, 3 = following file is a system header,
 * 4 = following file should be wrapped in extern "C").  Pass 0 to omit a
 * flag.  Per GCC docs at most two flags appear on a single linemarker
 * line; we honour that.  Existing callers that pass no flags use the
 * thin wrapper emit_linemarker() defined below.
 */
static void emit_linemarker_ex(CppState *st, int line, const char *file,
                                int flag1, int flag2, int flag3) {
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
    /* Phase 8 / R12: emit GCC-style flags after the filename.  GNU
     * linemarker flag values are: 1 = entering an include, 2 = returning
     * to the enclosing file, 3 = system header (suppresses some warnings),
     * 4 = "extern C" (carries through into C++ as treat-as-C semantics).
     * gcc and clang both emit `1 3 4` and `2 3 4` for system header
     * push/pop in C and C++ alike, so we follow suit.                  */
    if (flag1 == 0 && flag2 == 0 && flag3 == 0) {
        do { const char *_ob = st->out_text.buf; sb_printf(&st->out_text, "# %d \"%s\"\n", line, out_file); fix_token_ptrs(st, _ob); } while(0);
    } else if (flag2 == 0 && flag3 == 0) {
        do { const char *_ob = st->out_text.buf; sb_printf(&st->out_text, "# %d \"%s\" %d\n", line, out_file, flag1); fix_token_ptrs(st, _ob); } while(0);
    } else if (flag3 == 0) {
        do { const char *_ob = st->out_text.buf; sb_printf(&st->out_text, "# %d \"%s\" %d %d\n", line, out_file,
                  flag1, flag2); fix_token_ptrs(st, _ob); } while(0);
    } else {
        do { const char *_ob = st->out_text.buf; sb_printf(&st->out_text, "# %d \"%s\" %d %d %d\n", line, out_file,
                  flag1, flag2, flag3); fix_token_ptrs(st, _ob); } while(0);
    }
    st->last_lm_src_line = line;
    st->last_lm_src_file = file;  /* keep original for comparison */
    st->out_newlines     = 0;
}

/* Thin wrapper: emit a linemarker without GCC flags.  Most call sites
 * (entering a file, returning from #include, the implicit start-of-input
 * marker) want this.  Only handle_line preserves user-supplied flags by
 * calling emit_linemarker_ex directly. */
static void emit_linemarker(CppState *st, int line, const char *file) {
    emit_linemarker_ex(st, line, file, 0, 0, 0);
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
 * that runs of directives don't produce redundant markers.
 *
 * Phase 1: when ctx->keep_whitespace is true, an implicit single-space
 * CPPT_SPACE token is also pushed into the public token array immediately
 * before this token if t->has_leading_space is set.  The pptok layer never
 * emits standalone CPPT_SPACE tokens (it folds runs of horizontal whitespace
 * into the next token's has_leading_space flag), so this is the canonical
 * point at which "preserved" whitespace materialises in the public output.
 *
 * Newlines are handled separately by the caller (process_buf): they ARE
 * reified as CPPT_NEWLINE tokens by the lexer and the caller pushes them
 * directly when keep_whitespace is on.                                      */
/* fix_token_ptrs — call after any sb_push/sb_push_ch to st->out_text.
 * If the StrBuf was reallocated (buf address changed), all existing
 * out_tokens[].text pointers that pointed into the old buffer become stale.
 * This function patches them by the displacement delta.
 * old_buf must be the value of st->out_text.buf BEFORE the push call. */
static void fix_token_ptrs(CppState *st, const char *old_buf) {
    if (!old_buf || st->out_text.buf == old_buf) return; /* no realloc */
    ptrdiff_t delta = st->out_text.buf - old_buf;
    for (size_t _ftp_i = 0; _ftp_i < st->out_tokens.len; _ftp_i++) {
        /* Only fix pointers that are in the out_text buffer (not
         * separately heap-allocated concat tokens, which have concat_done). */
        if (!st->out_tokens.data[_ftp_i].concat_done &&
            st->out_tokens.data[_ftp_i].text)
            st->out_tokens.data[_ftp_i].text += delta;
    }
}

/* Convenience macro: save buf, push, fix. */
#define PUSH_CH(st, c) do {     const char *_ob = (st)->out_text.buf;     sb_push_ch(&(st)->out_text, (c));     fix_token_ptrs((st), _ob); } while (0)
#define PUSH_STR(st, s, n) do {     const char *_ob = (st)->out_text.buf;     sb_push(&(st)->out_text, (s), (n));     fix_token_ptrs((st), _ob); } while (0)
#define PUSH_CSTR(st, s) do {     const char *_ob = (st)->out_text.buf;     sb_push_cstr(&(st)->out_text, (s));     fix_token_ptrs((st), _ob); } while (0)

static void emit_tok_text(CppState *st, const PPTok *t) {
    /* Flush pending marker before any real (non-whitespace) content. */
    if (t->kind != CPPT_SPACE && t->kind != CPPT_COMMENT)
        flush_pending_lm(st);

    const char *sp = pptok_spell(t);
    size_t      slen = strlen(sp);

    /* Optionally synthesize a CPPT_SPACE token for any leading whitespace. */
    if (t->has_leading_space) {
        size_t ws_start = st->out_text.len;
        PUSH_CH(st, ' ');
        if (st->ctx->keep_whitespace) {
            CppTok ws = {
                .kind = CPPT_SPACE,
                .text = st->out_text.buf + ws_start,
                .len  = 1,
                .loc  = t->loc,
            };
            da_push(&st->out_tokens, ws);
        }
    }

    PUSH_STR(st, sp, slen);

    /* Record public CppTok pointing into the output buffer. */
    size_t start = st->out_text.len - slen;
    CppTok pub = {
        .kind       = t->kind,
        .text       = st->out_text.buf + start,
        .len        = slen,
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

/* Record that `filename` is already included, guarded by `macro`. */
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

/* Return true if `filename` has a live #pragma-once / include-guard. */
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

/* Define a numeric built-in macro (e.g. __LINE__). */
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
     * that need different values (freestanding → __STDC_HOSTED__=0, etc.).
     * Phase R9: __STDC_VERSION__ comes from ctx->lang_std (default C11);
     * In C89/C90 the macro is undefined entirely (introduced in C95). */
    install_builtin_num(mt, it, "__STDC__",         1);
    install_builtin_num(mt, it, "__STDC_HOSTED__",  1);
    if (st->ctx->lang_std >= 199409L) {
        /* Phase R18: gcc outputs __STDC_VERSION__ with L suffix (long literal).
         * Use install_builtin_str to emit e.g. "201710L" not "201710".         */
        char sv_buf[32];
        snprintf(sv_buf, sizeof sv_buf, "%ldL", st->ctx->lang_std);
        install_builtin_str(mt, it, "__STDC_VERSION__", sv_buf);
    }
    /* else: C89/C90 — leave __STDC_VERSION__ undefined */

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
        install_builtin_str(mt, it, "__SHARP_VERSION__", "\"0.11\"");
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
static void process_include_file(CppState *st, const char *filename,
                                  CppLang lang, bool is_sys_header);
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
 *                  including the one that CONTAINS `skip_until`.
 *                  Pass NULL for normal #include behaviour.
 *
 * Phase 6 fix: previously this compared the just-found file's full path
 * against `skip_until` (the source file's full path).  That never
 * matches, because we're looking for `<limits.h>` in dirs but
 * `skip_until` is the path of `<syslimits.h>` — different filename.  So
 * `skipping` stayed true forever and `#include_next` always failed.
 *
 * The correct semantics (matching GCC) is: extract the DIRECTORY of
 * `skip_until` (e.g. "/usr/lib/gcc/.../include") and skip search-path
 * entries until we've consumed the one equal to that directory.       */
static char *find_include_ex(CppState *st, const char *name, bool is_system,
                              const char *current_file, const char *skip_until,
                              bool *out_found_in_sys) {
    char path[4096];
    bool skipping = (skip_until != NULL);
    if (out_found_in_sys) *out_found_in_sys = false;

    /* Compute the directory that contains skip_until once, up front. */
    char skip_dir[4096] = {0};
    if (skip_until) {
        const char *slash = strrchr(skip_until, '/');
#ifdef _WIN32
        const char *bslash = strrchr(skip_until, '\\');
        if (bslash && (!slash || bslash > slash)) slash = bslash;
#endif
        if (slash) {
            size_t dlen = (size_t)(slash - skip_until);
            if (dlen >= sizeof skip_dir) dlen = sizeof skip_dir - 1;
            memcpy(skip_dir, skip_until, dlen);
            skip_dir[dlen] = '\0';
        } else {
            /* skip_until has no directory component — nothing to skip past;
             * #include_next just becomes a regular include. */
            skipping = false;
        }
    }

    /* Absolute path: try directly first (works on both POSIX and Windows) */
    if (name[0] == '/' || (name[0] && name[1] == ':')) {
        if (!skipping && access(name, R_OK) == 0)
            return cpp_xstrdup(name);
        /* absolute paths cannot match relative include_next logic */
        if (skipping) return NULL;
    }

    /* For "..." (quoted) form, first search relative to the including
     * file's directory.  This is intentionally skipped for <...> (system)
     * form: per GCC/clang behaviour and the spirit of C99 §6.10.2, system
     * headers must be looked up only in -isystem / default system paths,
     * never relative to the including file.  Without this guard, a header
     * in /usr/include/x86_64-linux-gnu/sys/wait.h that does
     * `#include <signal.h>` would resolve to its sibling sys/signal.h —
     * which is a forwarding header that `#include <signal.h>` in turn,
     * causing infinite recursion (Phase R1 found this on real glibc
     * headers via Lua's lauxlib.c).  Also skipped when skip_until is
     * active because #include_next never looks in the source's own dir. */
    if (current_file && !skipping && !is_system) {
        const char *slash = strrchr(current_file, '/');
#ifdef _WIN32
        const char *bslash = strrchr(current_file, '\\');
        if (bslash && (!slash || bslash > slash)) slash = bslash;
#endif
        if (slash) {
            size_t dir_len = (size_t)(slash - current_file + 1);
            snprintf(path, sizeof path, "%.*s%s", (int)dir_len, current_file, name);
            if (access(path, R_OK) == 0) {
                return cpp_xstrdup(path);
            }
        }
        /* If no directory separator (e.g. virtual filename like "<test>"),
         * fall through to try the name directly relative to CWD.          */
    }

    /* User include paths */
    if (!is_system) {
        for (size_t i = 0; i < st->user_paths.len; i++) {
            /* Check whether this directory IS the one to skip past. */
            if (skipping && strcmp(st->user_paths.data[i], skip_dir) == 0) {
                skipping = false;
                continue;
            }
            if (skipping) continue;
            snprintf(path, sizeof path, "%s/%s", st->user_paths.data[i], name);
            if (access(path, R_OK) == 0)
                return cpp_xstrdup(path);
        }
    }

    /* System include paths */
    for (size_t i = 0; i < st->sys_paths.len; i++) {
        if (skipping && strcmp(st->sys_paths.data[i], skip_dir) == 0) {
            skipping = false;
            continue;
        }
        if (skipping) continue;
        snprintf(path, sizeof path, "%s/%s", st->sys_paths.data[i], name);
        if (access(path, R_OK) == 0) {
            if (out_found_in_sys) *out_found_in_sys = true;
            return cpp_xstrdup(path);
        }
    }
    return NULL;
}

/* Convenience wrapper for standard #include */
static char *find_include(CppState *st, const char *name, bool is_system,
                           const char *current_file) {
    return find_include_ex(st, name, is_system, current_file, NULL, NULL);
}

/* Check if a header file exists in the search paths.
 *
 * Used by both `__has_include` (is_next=false) and `__has_include_next`
 * (is_next=true).  When is_next is true, we mimic the skip-by-directory
 * semantics of `#include_next`: any candidate path that resolves to the
 * directory containing `current_file` (or any directory before it in the
 * search list) is rejected, and only paths after that directory in the
 * search order are considered.  This matches GCC's documented behaviour
 * and lets headers that wrap a system header (e.g. fixincluded variants)
 * test for the next-in-chain version with `__has_include_next(<foo.h>)`.
 *
 * Phase 8: previously is_next was a stub that behaved identically to
 * __has_include.  Now it implements the real skip-dir logic, mirroring
 * find_include_ex (which Phase 6 fixed for the corresponding #include_next
 * directive).
 */
static bool cpp_has_include(CppState *st, const char *name, bool is_system,
                             bool is_next, const char *current_file) {
    /* For absolute paths, just check access() (skip-dir doesn't apply). */
    if (name[0] == '/' || (name[0] && name[1] == ':')) {
        if (is_next) return false;  /* absolute can't participate in skip chain */
        return access(name, R_OK) == 0;
    }

    char path[4096];

    /* Phase 8: compute skip_dir for __has_include_next, mirroring the
     * logic in find_include_ex.  If current_file has no directory
     * component (e.g. virtual filename like "<test>"), there's nothing
     * to skip past — degrade gracefully to plain __has_include semantics. */
    bool skipping = is_next;
    char skip_dir[4096] = {0};
    if (skipping && current_file) {
        const char *slash = strrchr(current_file, '/');
#ifdef _WIN32
        const char *bslash = strrchr(current_file, '\\');
        if (bslash && (!slash || bslash > slash)) slash = bslash;
#endif
        if (slash) {
            size_t dlen = (size_t)(slash - current_file);
            if (dlen >= sizeof skip_dir) dlen = sizeof skip_dir - 1;
            memcpy(skip_dir, current_file, dlen);
            skip_dir[dlen] = '\0';
        } else {
            /* No directory component — nothing to skip; degrade. */
            skipping = false;
        }
    } else if (skipping && !current_file) {
        /* No current_file to anchor the skip — degrade. */
        skipping = false;
    }

    /* For <...> search system/user paths */
    const StrArr *paths[] = { &st->user_paths, &st->sys_paths };
    for (int pi = 0; pi < 2; pi++) {
        const StrArr *arr = paths[pi];
        for (size_t i = 0; i < arr->len; i++) {
            /* Phase 8: for __has_include_next, skip directories up to and
             * including the one that holds current_file. */
            if (skipping && strcmp(arr->data[i], skip_dir) == 0) {
                skipping = false;
                continue;
            }
            if (skipping) continue;

            snprintf(path, sizeof path, "%s/%s", arr->data[i], name);
            if (access(path, R_OK) == 0) return true;
#ifdef _WIN32
            snprintf(path, sizeof path, "%s\\%s", arr->data[i], name);
            if (access(path, R_OK) == 0) return true;
#endif
        }
    }

    /* For "..." try relative to current directory too — but only for the
     * non-_next variant; #include_next never falls back to CWD. */
    if (!is_system && !is_next) {
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

    if (first && first->tok.kind == CPPT_STRING_LIT) {
        /* "name.h" — raw, no expansion */
        const char *sp = pptok_spell(&first->tok);
        size_t len = strlen(sp);
        if (len >= 2 && sp[0] == '"' && sp[len-1] == '"')
            snprintf(name, sizeof name, "%.*s", (int)(len-2), sp+1);
        else snprintf(name, sizeof name, "%s", sp);
        is_sys = false;
        /* Phase R4 (u_1_13): warn on extra tokens after the filename. */
        TokNode *trailing = first->next;
        while (trailing && (trailing->tok.kind == CPPT_SPACE ||
                            trailing->tok.kind == CPPT_NEWLINE)) trailing = trailing->next;
        if (trailing) {
            emit_diag(st, CPP_DIAG_WARNING, trailing->tok.loc,
                "extra tokens at end of #include directive");
        }
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
        /* Phase R4 (u_1_13): warn on extra tokens after closing `>`. */
        if (n) {
            TokNode *trailing = n->next;
            while (trailing && (trailing->tok.kind == CPPT_SPACE ||
                                trailing->tok.kind == CPPT_NEWLINE)) trailing = trailing->next;
            if (trailing) {
                emit_diag(st, CPP_DIAG_WARNING, trailing->tok.loc,
                    "extra tokens at end of #include directive");
            }
        }
    } else {
        /* Not a direct header name — macro-expand and re-parse */
        TokList expanded = {0};
        macro_expand(line, st->macros, st->interns, st->diags, &expanded);

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

    /* Sharp mode: #include "file.sp" is a hard error.  Sharp modules must be
     * imported with 'import "file.sp"', not with the C #include mechanism.
     * Macro #defines do not propagate through import (spec §模块系统).
     * Only the "..." (user) form is checked here; <file.sp> is nonsensical
     * and will simply fail file-not-found, which is a sufficient error.    */
    if (lang == CPP_LANG_SHARP && !is_sys) {
        size_t _nlen = strlen(name);
        if (_nlen >= 3 && memcmp(name + _nlen - 3, ".sp", 3) == 0) {
            emit_diag(st, CPP_DIAG_ERROR, loc,
                "#include \"%s\": use 'import \"%s\"' to import Sharp modules",
                name, name);
            return;
        }
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
    bool found_in_sys = false;
    char *found = find_include_ex(st, name, is_sys, current_file, skip_until,
                                  &found_in_sys);
    if (!found) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "file not found: %s", name);
        /* Phase 4: emit notes listing the search paths actually consulted,
         * so the user can quickly see where the lookup failed.  This mirrors
         * Clang's "<...> search starts here:" hint.                        */
        if (!is_sys && current_file) {
            const char *slash = strrchr(current_file, '/');
#ifdef _WIN32
            const char *bslash = strrchr(current_file, '\\');
            if (bslash && (!slash || bslash > slash)) slash = bslash;
#endif
            if (slash) {
                char dirbuf[4096];
                size_t dlen = (size_t)(slash - current_file);
                if (dlen >= sizeof(dirbuf)) dlen = sizeof(dirbuf) - 1;
                memcpy(dirbuf, current_file, dlen);
                dirbuf[dlen] = '\0';
                emit_note(st, loc, "  searched (relative to including file): %s/", dirbuf);
            }
        }
        if (!is_sys) {
            for (size_t i = 0; i < st->user_paths.len; i++)
                emit_note(st, loc, "  searched (user, -I): %s",
                          st->user_paths.data[i]);
        }
        for (size_t i = 0; i < st->sys_paths.len; i++)
            emit_note(st, loc, "  searched (system): %s",
                      st->sys_paths.data[i]);
        return;
    }

    const char *found_interned = intern_cstr(st->interns, found);
    free(found);

    /* Second guard check with the resolved absolute path.
     * The first check (above) uses the raw #include name; this one uses the
     * canonical path so that #pragma once guards stored under the absolute
     * path are honoured even when included via a relative name. */
    if (guard_already_included(st, found_interned)) return;

    /* Phase R12: the file is treated as a "system header" if it was
     * either requested with `<...>` or actually found in a -isystem dir
     * (a `"..."` include can fall through into system paths).  GCC/clang
     * `-E` mark such files with linemarker flag 3; some macro guards
     * also gate diagnostics ("system header") on this status.            */
    bool is_sys_header = is_sys || found_in_sys;

    st->include_depth++;
    /* Phase R4: per C99 §6.10.2, an included file must contain a
     * self-balanced sequence of #if/#endif directives.  Snapshot the
     * cond-stack depth before processing and verify it's the same
     * after.  mcpp's e_17_5.c exercises this with `unbal1.h` (a bare
     * #endif) and `unbal2.h` (a #if without matching #endif).        */
    int cond_depth_before = st->cond_depth;
    /* Phase R12: emit `# 1 "<include>" 1 [3]` to match gcc/clang.
     * process_include_file forwards the flags into emit_linemarker_ex.   */
    process_include_file(st, found_interned, lang, is_sys_header);
    if (st->cond_depth < cond_depth_before) {
        emit_diag(st, CPP_DIAG_ERROR, loc,
                  "in file '%s': '#endif' without matching '#if'",
                  found_interned);
    } else if (st->cond_depth > cond_depth_before) {
        emit_diag(st, CPP_DIAG_ERROR, loc,
                  "in file '%s': unterminated '#if' / '#ifdef' (no matching '#endif')",
                  found_interned);
        /* Pop the unterminated frames so the outer file's bookkeeping
         * stays consistent and we don't blame the next #endif on the
         * unrelated state.                                            */
        while (st->cond_depth > cond_depth_before) st->cond_depth--;
    }
    st->include_depth--;
    /* Phase R12: returning to enclosing file — emit `# N "..." 2 [3 [4]]`.
     * Flag 2 marks "exiting include"; flags 3+4 carry through if the
     * *enclosing* file is itself a system header.                       */
    int pop_f2 = st->cur_file_is_sys ? 3 : 0;
    int pop_f3 = st->cur_file_is_sys ? 4 : 0;
    emit_linemarker_ex(st, loc.line + 1, current_file, 2, pop_f2, pop_f3);
}

/* Process a #define directive line. */
static void handle_define(CppState *st, TokList *line, CppLoc loc) {
    MacroDef *def = macro_parse_define(line, st->interns, st->diags);
    if (def) macro_define(st->macros, def, st->diags, loc);
}

/* Process a #undef directive line. */
static void handle_undef(CppState *st, TokList *line, CppLoc loc) {
    TokNode *n = line->head;
    while (n && n->tok.kind == CPPT_SPACE) n = n->next;
    if (!n || n->tok.kind != CPPT_IDENT) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "expected macro name after #undef");
        return;
    }
    macro_undef(st->macros, pptok_spell(&n->tok));
    warn_trailing(st, "undef", line, n, loc);
}

/* Push a conditional frame with the given truth value. */
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
                const char *mname = pptok_spell(&nx->tok);
                /* Phase R13: __has_* family are built-in operators, not
                 * real macros, but `defined(__has_attribute)` must be 1.
                 * glibc's <sys/cdefs.h> uses this pattern to guard its
                 * __glibc_has_attribute() wrapper.                      */
                bool is_has_x = is_has_family(mname);
                bool def = is_has_x || macro_lookup(mt, mname) != NULL;
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

/* Resolve `__has_include("file")` / `__has_include(<file>)` /
 * `__has_include_next(...)` in-place, replacing each call with pp-number
 * "1" or "0" (C23 §6.10.1).  Must run BEFORE macro expansion because the
 * header name inside the parens must not be expanded.
 *
 * Phase 8: current_file is now passed through and used by
 * __has_include_next to compute skip_dir.  Previously is_next was
 * silently dropped (the stub treated _next identically to plain).
 */
static TokList resolve_has_include(CppState *st, const TokList *in,
                                    const char *current_file) {
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
                            
                            result = cpp_has_include(st, name_buf, false,
                                                      is_next, current_file);
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
                        
                        result = cpp_has_include(st, hdr.buf, true,
                                                  is_next, current_file);
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

/* Phase 8: current_file is now threaded through to support
 * __has_include_next's skip-by-directory semantics. */
static TokList resolve_all_has_include(CppState *st, const TokList *in,
                                        const char *current_file) {
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
        TokList next = resolve_has_include(st, &cur, current_file);
        tl_free(&cur);
        cur = next;
    }
    return cur;
}

/* Process #if <constant-expression>. */
static void handle_if(CppState *st, TokList *line, CppLoc loc) {
    /* Phase R5: if we're inside a dead branch already, the nested #if
     * pushes a frame but its expression must not be evaluated.  C99
     * §6.10.1 specifies that controlling expressions in dead groups
     * are skipped along with the group itself.  Without this guard,
     * something like
     *     #if 0
     *     #  if EXPR_THAT_WOULD_ERROR
     *     #  endif
     *     #endif
     * raises the inner expression's diagnostics even though the inner
     * #if is in a code path that the standard says is not processed.
     * xxhash.h's `#if XXH_HAS_BUILTIN(__builtin_unreachable)` inside
     * an inactive `#if XXH_INLINE_ALL` block exercises this.        */
    if (!in_live_branch(st)) {
        handle_if_common(st, false, loc);
        return;
    }
    /* Step 1: resolve defined() BEFORE macro expansion (ISO C11 §6.10.1p4) */
    TokList preresolved = resolve_all_defined(st->macros, line);
    /* Step 1b: resolve __has_include(...) (C23 §6.10.1).
     * Phase 8: thread loc.file through so __has_include_next can compute
     * its skip-dir relative to the file being preprocessed. */
    TokList hi_resolved = resolve_all_has_include(st, &preresolved, loc.file);
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

/* Process #ifdef / #ifndef <name>. */
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
    /* Phase R5: treat the __has_* family as implicitly "defined" for
     * the purpose of `#ifdef __has_builtin` / `#if defined(__has_…)`
     * checks.  These names are not real macros (their `(...)` form is
     * intercepted in expr.c, see Phase R4) but modern C headers
     * — xxhash.h, glibc's <features.h>, libc++ — guard their use with
     * `#ifdef __has_builtin` to avoid #if-time evaluation on older
     * compilers.  Without this, sharp-cpp falls into the fallback arm
     * which often re-defines the macro to expand to literal `0`,
     * producing self-inflicted "#if 0(x)" strictness errors.         */
    const char *idname = pptok_spell(&n->tok);
    bool is_has_x = is_has_family(idname);
    bool defined = is_has_x || macro_lookup(st->macros, idname) != NULL;
    handle_if_common(st, invert ? !defined : defined, loc);
    /* GCC-style: warn if anything follows the macro name. */
    warn_trailing(st, invert ? "ifndef" : "ifdef", line, n, loc);
}

/* Process #elif <constant-expression>. */
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
    /* Phase R5: if the parent (outer) frame is itself inactive, this
     * #elif is in dead code regardless of its expression value.  Skip
     * the evaluation entirely so we don't surface diagnostics from
     * tokens the standard says we shouldn't process.  Same rationale
     * as the parallel guard in handle_if.                            */
    {
        bool outer_live = true;
        for (int i = 0; i < st->cond_depth - 1; i++)
            if (!st->cond_stack[i].in_true_branch) { outer_live = false; break; }
        if (!outer_live) {
            f->in_true_branch = false;
            return;
        }
    }

    /* Step 1: resolve defined() BEFORE macro expansion (ISO C11 §6.10.1p4) */
    TokList preresolved = resolve_all_defined(st->macros, line);
    /* Step 1b: resolve __has_include(...) (C23 §6.10.1).
     * Phase 8: thread loc.file through for __has_include_next. */
    TokList hi_resolved = resolve_all_has_include(st, &preresolved, loc.file);
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

/* Process #else. */
static void handle_else(CppState *st, TokList *line, CppLoc loc) {
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
    /* GCC-style: warn (don't error) if there are extra tokens after #else.
     * Comments are filtered above so legacy patterns using a comment as a
     * label after #else don't trigger the warning.                         */
    warn_trailing(st, "else", line, NULL, loc);
}

/* Process #endif. */
static void handle_endif(CppState *st, TokList *line, CppLoc loc) {
    if (st->cond_depth == 0) {
        emit_diag(st, CPP_DIAG_ERROR, loc, "#endif without #if");
        return;
    }
    st->cond_depth--;
    warn_trailing(st, "endif", line, NULL, loc);
}

static void handle_line(CppState *st, CppReader *rd, TokList *line, CppLoc loc) {
    /* #line digit-sequence ["filename"] [flag1 [flag2]]
     *
     * The trailing flag1/flag2 form is GCC's linemarker extension: when
     * read back from a preprocessed file, `# 1 "stdio.h" 1 3` means
     * "entering a system header".  Phase 8 added support for these flags
     * in handle_line so that round-tripping our own output through
     * sharp-cpp again preserves the system-header annotation.
     *
     * Recognised flags (per GCC docs):
     *   1  push: this is the start of a newly-included file
     *   2  pop:  resuming the file outer to the just-finished include
     *   3  the file is a system header
     *   4  the contents should be implicitly wrapped in extern "C"
     *
     * Unknown digit-only flags are silently ignored (GCC behaviour).
     * Non-digit trailing tokens are caught by the existing dispatch flow. */
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
    /* C99 §6.10.4/3 requires the line number to be in [1,2147483647];
     * C90 §3.8.4 limited it to [1,32767].  Phase R9: under -ansi /
     * -std=c89/c90, sharp-cpp enforces the C90 range so that mcpp's
     * u_1_17s.c (which expects #line 32768 to be rejected) passes.   */
    long max_line = (st->ctx->lang_std >= 199409L) ? 2147483647L : 32767L;
    if (new_line < 1 || new_line > max_line) {
        emit_diag(st, CPP_DIAG_ERROR, loc,
                  "#line value %ld is out of range (must be 1..%ld)",
                  new_line, max_line);
        tl_free(&expanded);
        return;
    }
    n = n->next;
    while (n && n->tok.kind == CPPT_SPACE) n = n->next;
    const char *new_file = loc.file;
    if (n && n->tok.kind == CPPT_STRING_LIT) {
        const char *sp = pptok_spell(&n->tok);
        size_t len = strlen(sp);
        /* Phase R4 (e_7_4): C99 §6.10.4/4 requires the optional s-char
         * sequence to be a plain string, not a wide one.  Spelling
         * starts with `L`, `u`, `U`, or `u8` for the wide/unicode
         * forms — reject those.                                       */
        if (len > 0 && (sp[0] == 'L' || sp[0] == 'u' || sp[0] == 'U')) {
            emit_diag(st, CPP_DIAG_ERROR, n->tok.loc,
                      "#line filename must be a narrow string literal "
                      "(not a valid filename: %s)", sp);
            tl_free(&expanded);
            return;
        }
        char tmp[4096];
        if (len >= 2 && sp[0] == '"' && sp[len-1] == '"')
            snprintf(tmp, sizeof tmp, "%.*s", (int)(len-2), sp+1);
        else snprintf(tmp, sizeof tmp, "%s", sp);
        new_file = intern_cstr(st->interns, tmp);
        n = n->next;
    }

    /* Phase 8 / R12: collect up to three trailing GCC flag digits (1/2/3/4).
     * Anything else (non-numeric, out-of-range value, or a fourth flag)
     * is silently discarded — this matches GCC's lenient parser. */
    int flag1 = 0, flag2 = 0, flag3 = 0;
    int flags_seen = 0;
    while (n) {
        while (n && n->tok.kind == CPPT_SPACE) n = n->next;
        if (!n || n->tok.kind == CPPT_NEWLINE) break;
        if (n->tok.kind != CPPT_PP_NUMBER) break;
        bool funs;
        long fv = (long)parse_int_literal_pub(pptok_spell(&n->tok), &funs);
        if (fv >= 1 && fv <= 4) {
            if (flags_seen == 0)      flag1 = (int)fv;
            else if (flags_seen == 1) flag2 = (int)fv;
            else if (flags_seen == 2) flag3 = (int)fv;
            /* further flags ignored */
            flags_seen++;
        }
        n = n->next;
    }

    /* Phase 4: actually update the reader's line number AND filename.
     * Previously only the line was updated, so `#line N "file"` would
     * emit a correct linemarker but `__FILE__` (which reads the loc of the
     * next token from the reader) still gave the original filename.
     *
     * `#line N` means the directive line itself is N, so the next source
     * line is N+1. We set the reader to N so the newline at the end of
     * this directive line increments it to N+1. */
    if (rd) {
        reader_set_line(rd, (int)new_line);
        reader_set_file(rd, new_file);
    }
    /* Phase 8 / R12: emit any preserved GCC flags so consumers downstream
     * (e.g. another sharp-cpp instance reading our output) see the
     * system-header / push / pop / extern-C annotation that the original
     * input carried. */
    emit_linemarker_ex(st, (int)new_line, new_file, flag1, flag2, flag3);
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

/* =========================================================================
 * Sharp deferred-conditional helpers
 * ====================================================================== */

/*
 * has_at_intrinsic — return true if the token list contains at least one
 * CPPT_AT_INTRINSIC token (@has_operator or @static_assert in v0.11).
 * Used to detect #if conditions that require deferred evaluation by the
 * Sharp frontend rather than immediate preprocessing evaluation.
 */
static bool has_at_intrinsic(const TokList *toks) {
    for (TokNode *n = toks->head; n; n = n->next)
        if (n->tok.kind == CPPT_AT_INTRINSIC) return true;
    return false;
}

/*
 * emit_deferred_directive — write a preprocessor directive verbatim to
 * both the text output (out_text) and the public token array (out_tokens).
 *
 * This is called when in deferred-conditional mode: the preprocessor
 * cannot evaluate #if conditions that contain @ intrinsics, so it passes
 * the entire #if...#endif block through to the Sharp frontend.
 *
 * The emitted text looks like:   #<dname> <line tokens>\n
 * Each token also appears in out_tokens so the Sharp frontend can parse
 * the directive structure without re-lexing the text.
 *
 * Nothing is emitted when the enclosing conditional context is dead
 * (i.e. in_live_branch(st) is false), preserving the invariant that dead
 * branches produce no output.
 */
static void emit_deferred_directive(CppState *st, const char *dname,
                                    const TokList *line, CppLoc loc) {
    if (!in_live_branch(st)) return;
    flush_pending_lm(st);

    /* '#' */
    {
        size_t start = st->out_text.len;
        PUSH_CH(st, '#');
        CppTok pub = {0};
        pub.kind = CPPT_PUNCT;
        pub.text = st->out_text.buf + start;
        pub.len  = 1;
        pub.loc  = loc;
        da_push(&st->out_tokens, pub);
    }

    /* directive keyword — no space between '#' and keyword so the output
     * reads as "#if", "#endif", etc. (space is permitted by the standard
     * but omitting it avoids confusion with line-marker syntax "# N").  */
    {
        size_t start = st->out_text.len;
        size_t dlen  = strlen(dname);
        PUSH_STR(st, dname, dlen);
        CppTok pub = {0};
        pub.kind = CPPT_IDENT;
        pub.text = st->out_text.buf + start;
        pub.len  = dlen;
        pub.loc  = loc;
        da_push(&st->out_tokens, pub);
    }

    /* condition / argument tokens from the directive line */
    for (TokNode *n = line->head; n; n = n->next) {
        const PPTok *t = &n->tok;
        if (t->kind == CPPT_NEWLINE) break;

        const char *sp  = pptok_spell(t);
        size_t      slen = strlen(sp);

        /* Preserve inter-token spacing */
        if (t->has_leading_space || t->kind == CPPT_SPACE)
            PUSH_CH(st, ' ');

        if (t->kind == CPPT_SPACE || slen == 0) continue;

        size_t start = st->out_text.len;
        PUSH_STR(st, sp, slen);

        CppTok pub = {0};
        pub.kind = t->kind;
        pub.text = st->out_text.buf + start;
        pub.len  = slen;
        pub.loc  = t->loc;
        da_push(&st->out_tokens, pub);
    }

    PUSH_CH(st, '\n');
    st->out_newlines++;
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
        /* Ensure pragma appears on its own line (may be called from
         * emit_pragma_text mid-token-stream, e.g. macro expansion)   */
        if (st->out_text.len > 0 &&
            st->out_text.buf[st->out_text.len - 1] != '\n')
            PUSH_CH(st, '\n');
        do { const char *_ob = st->out_text.buf; sb_printf(&st->out_text, "#pragma %s\n", sb.buf ? sb.buf : ""); fix_token_ptrs(st, _ob); } while(0);
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

/* Phase R12: emit a single post-expansion token, performing on-the-fly
 * substitution for the dynamic builtins __FILE__ / __LINE__ / __COUNTER__.
 *
 * Without this helper, a macro body containing __FILE__ or __LINE__
 * (e.g. glibc's <assert.h> assert macro) would emit the literal
 * identifier "__FILE__" instead of the source filename.  process_buf()'s
 * main loop already special-cases these for raw input tokens; this
 * helper does the same for tokens coming OUT of macro expansion.
 *
 * Per C99 §6.10.8.1, __FILE__ / __LINE__ in a macro replacement use the
 * INVOCATION site as their reference location, not the body's own loc.
 * Caller provides `inv_loc` — the location of the macro identifier as
 * read from the source file.                                          */
/* Emit a _Pragma("X") as `#pragma X\n`, ensuring it appears on its own line.
 * C99 §6.10.9: after de-stringizing the argument, it is treated as a
 * #pragma directive.  gcc -E outputs it as `#pragma X` on a separate line.
 *
 * Phase R17: route through handle_pragma so that _Pragma("once") activates
 * the include-guard mechanism, matching gcc -E behaviour.                  */
static void emit_pragma_text(CppState *st, const char *raw_string_lit,
                             const char *current_file, CppLoc loc) {
    /* De-stringize: strip outer quotes, \" → ", \\ → \ */
    const char *p = raw_string_lit;
    if (*p == '"') p++;
    StrBuf ds = {0};
    while (*p && !(*p == '"' && *(p+1) == '\0')) {
        if (p[0] == '\\' && (p[1] == '"' || p[1] == '\\')) {
            sb_push_ch(&ds, p[1]); p += 2;
        } else {
            sb_push_ch(&ds, *p++);
        }
    }
    sb_push_ch(&ds, '\0');
    const char *pragma_body = ds.buf ? ds.buf : "";

    /* Tokenise the de-stringized body and pass to handle_pragma so that
     * _Pragma("once") activates the include-guard just like #pragma once. */
    CppDiagArr dummy = {0};
    InternTable *it = st->interns;
    CppReader *rd = reader_new_from_buf(pragma_body, strlen(pragma_body),
                                       "<_Pragma>", it, &dummy);
    TokList tl = {0};
    for (;;) {
        PPTok tk = reader_next_tok(rd, false);
        if (tk.kind == CPPT_EOF || tk.kind == CPPT_NEWLINE)
            { pptok_free(&tk); break; }
        tl_append(&tl, tk);
    }
    reader_free(rd);
    free(dummy.data);

    handle_pragma(st, &tl, loc, current_file);
    tl_free(&tl);
    sb_free(&ds);
}

static void emit_post_expansion_tok(CppState *st, const PPTok *t, CppLoc inv_loc) {
    if (t->kind == CPPT_IDENT) {
        const char *name = pptok_spell(t);
        if (strcmp(name, "__FILE__") == 0) {
            PPTok ft = make_file_tok(st, inv_loc);
            ft.has_leading_space = t->has_leading_space;
            emit_tok_text(st, &ft);
            pptok_free(&ft);
            return;
        }
        if (strcmp(name, "__LINE__") == 0) {
            PPTok lt = make_line_tok(st, inv_loc);
            lt.has_leading_space = t->has_leading_space;
            emit_tok_text(st, &lt);
            pptok_free(&lt);
            return;
        }
        if (strcmp(name, "__COUNTER__") == 0) {
            char cbuf[32];
            snprintf(cbuf, sizeof cbuf, "%d", st->counter++);
            PPTok ct = {0};
            ct.kind = CPPT_PP_NUMBER;
            ct.loc  = inv_loc;
            ct.has_leading_space = t->has_leading_space;
            sb_push_cstr(&ct.spell, cbuf);
            emit_tok_text(st, &ct);
            pptok_free(&ct);
            return;
        }
    }
    emit_tok_text(st, t);
}

/* Phase R16: emit a TokList of expanded tokens, converting any
 * _Pragma("X") sequences into `#pragma X\n` (C99 §6.10.9).
 * Without this, _Pragma tokens from macro expansions (e.g. glib's
 * GLIB_DEPRECATED_IN_*) pass through as raw _Pragma(...) tokens,
 * diverging from gcc -E which emits `#pragma X`.                */
static void emit_expanded_list(CppState *st, TokList *tl, CppLoc inv_loc,
                               const char *current_file) {
    for (TokNode *en = tl->head; en; en = en->next) {
        const PPTok *tok = &en->tok;
        /* Detect _Pragma ( "string" ) in the expanded token stream */
        if (tok->kind == CPPT_IDENT &&
            strcmp(pptok_spell(tok), "_Pragma") == 0) {
            /* Scan forward for: optional spaces, (, optional spaces,
             * string literal, optional spaces, ) */
            TokNode *n = en->next;
            while (n && n->tok.kind == CPPT_SPACE) n = n->next;
            if (!n || n->tok.kind != CPPT_PUNCT ||
                strcmp(pptok_spell(&n->tok), "(") != 0) goto passthrough;
            n = n->next;
            while (n && n->tok.kind == CPPT_SPACE) n = n->next;
            if (!n || n->tok.kind != CPPT_STRING_LIT) goto passthrough;
            TokNode *str_node = n;
            n = n->next;
            while (n && n->tok.kind == CPPT_SPACE) n = n->next;
            if (!n || n->tok.kind != CPPT_PUNCT ||
                strcmp(pptok_spell(&n->tok), ")") != 0) goto passthrough;
            /* De-stringize and emit #pragma */
            flush_pending_lm(st);
            emit_pragma_text(st, pptok_spell(&str_node->tok), current_file, inv_loc);
            en = n;  /* advance past the closing ) */
            continue;
        passthrough:;
        }
        emit_post_expansion_tok(st, tok, inv_loc);
    }
}

/* =========================================================================
 * Token processing — read from reader, handle directives, expand macros,
 * emit to output.
 * ====================================================================== */

static void process_buf(CppState *st, CppReader *rd, CppLang lang) {
    /* Activate Sharp-language tokenization (@ intrinsic recognition) when
     * processing a .sp file.  Must be set before the first reader_next_tok
     * call so that the very first token is classified correctly.          */
    reader_set_sharp_mode(rd, lang == CPP_LANG_SHARP);

    bool at_bol = true;

    for (;;) {
        PPTok t = reader_next_tok(rd, st->keep_comments);
        if (t.kind == CPPT_EOF) { pptok_free(&t); break; }
        if (st->fatal)          { pptok_free(&t); break; }

        /* Track beginning-of-line */
        if (t.kind == CPPT_NEWLINE) {
            at_bol = true;
            if (in_live_branch(st)) {
                size_t start = st->out_text.len;
                PUSH_CH(st, '\n');
                st->out_newlines++;
                /* Phase 1: when the public-API client requested whitespace
                 * preservation, emit a CPPT_NEWLINE token referring to the
                 * '\n' we just appended.  Otherwise the public token array
                 * is whitespace-free and consumers reconstruct layout from
                 * out_text.                                                */
                if (st->ctx->keep_whitespace) {
                    CppTok pub = {
                        .kind = CPPT_NEWLINE,
                        .text = st->out_text.buf + start,
                        .len  = 1,
                        .loc  = t.loc,
                    };
                    da_push(&st->out_tokens, pub);
                }
            }
            pptok_free(&t);
            continue;
        }

        /* Skip comments (they were either kept or turned to spaces by lexer) */
        if (t.kind == CPPT_COMMENT) {
            if (in_live_branch(st) && st->keep_comments) {
                size_t start = st->out_text.len;
                const char *sp = pptok_spell(&t);
                size_t      slen = strlen(sp);
                PUSH_STR(st, sp, slen);
                if (st->ctx->keep_whitespace) {
                    CppTok pub = {
                        .kind = CPPT_COMMENT,
                        .text = st->out_text.buf + start,
                        .len  = slen,
                        .loc  = t.loc,
                    };
                    da_push(&st->out_tokens, pub);
                }
            }
            pptok_free(&t);
            continue;
        }

        if (t.kind == CPPT_SPACE) {
            if (in_live_branch(st)) {
                size_t start = st->out_text.len;
                PUSH_CH(st, ' ');
                if (st->ctx->keep_whitespace) {
                    CppTok pub = {
                        .kind = CPPT_SPACE,
                        .text = st->out_text.buf + start,
                        .len  = 1,
                        .loc  = t.loc,
                    };
                    da_push(&st->out_tokens, pub);
                }
            }
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

            /* Read the directive keyword.  Comments arrive as CPPT_SPACE
             * tokens here when keep_comments=false (Phase R4 normalised
             * comment-to-space in the lexer per C99 §5.1.1.2 phase 3),
             * so the SPACE-skip below correctly consumes intervening
             * comments such as `# COMMENT define FOO`.                  */
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
            bool linemarker_form = false;  /* Phase 8: # N "file" form */
            if (kw.kind == CPPT_IDENT) {
                const char *sp = pptok_spell(&kw);
                size_t l = strlen(sp);
                if (l >= sizeof dname_buf) l = sizeof dname_buf - 1;
                memcpy(dname_buf, sp, l);
            } else if (kw.kind == CPPT_PP_NUMBER) {
                /* Phase 8: GCC linemarker syntax `# N "file" [F1 [F2]]`.
                 * Synthesise dname="line" so the dispatch table routes
                 * this to handle_line, and prepend kw to the directive
                 * line so handle_line sees the leading number.  The
                 * `#line` keyword variant remains the canonical form;
                 * this branch only fires for the bare-`#`-then-number
                 * form that GCC emits in -E output and that any tool
                 * round-tripping through us would produce.            */
                memcpy(dname_buf, "line", 4);
                linemarker_form = true;
            }
            const char *dname = dname_buf;

            /* Collect the rest of the directive line */
            TokList line = collect_directive_line(rd, false);
            if (linemarker_form) {
                /* Phase 8: splice the leading PP_NUMBER token onto the
                 * front of the collected line so handle_line can parse
                 * it uniformly.  We copy because kw is freed below. */
                tl_prepend(&line, pptok_copy(&kw));
            }
            pptok_free(&kw);

            /* -------------------------------------------------------
             * Deferred conditional block passthrough (Sharp mode).
             *
             * When st->deferred_nest > 0 we are inside the body of a
             * #if whose condition contained @-intrinsic tokens that the
             * preprocessor cannot evaluate.  Every directive inside such
             * a block — including nested #if / #endif — is emitted
             * verbatim so the Sharp frontend receives the complete block.
             *
             * Nesting rules:
             *   #if / #ifdef / #ifndef  → deferred_nest++
             *   #endif                  → deferred_nest--  (0 → exits mode)
             *   all others              → emitted verbatim as-is
             *
             * Live-branch liveness is NOT tracked for the deferred block
             * (no CondFrame is pushed).  The in_live_branch(st) check in
             * emit_deferred_directive() uses the outer context correctly
             * because no frame was pushed for the deferred #if.           */
            if (st->deferred_nest > 0) {
                if (strcmp(dname, "if")     == 0 ||
                    strcmp(dname, "ifdef")  == 0 ||
                    strcmp(dname, "ifndef") == 0) {
                    st->deferred_nest++;
                    emit_deferred_directive(st, dname, &line, dir_loc);
                } else if (strcmp(dname, "endif") == 0) {
                    st->deferred_nest--;
                    emit_deferred_directive(st, "endif", &line, dir_loc);
                } else {
                    /* #elif, #else, #define, #undef, #pragma, … inside the
                     * deferred block — emit all verbatim.                 */
                    emit_deferred_directive(st, dname, &line, dir_loc);
                }
                tl_free(&line);
                queue_linemarker(st, reader_current_line(rd), reader_filename(rd));
                at_bol = true;
                continue;
            }

            /* Now dispatch structural directives normally.
             * (These must always be processed even in dead branches so
             *  we can track cond-stack nesting correctly.)                */
            if      (strcmp(dname, "if")     == 0) {
                /* v0.12: `#if @has_operator(T, op)` is deprecated.
                 * Use `if (@has_operator(T, op)) { ... }` instead —
                 * the Sharp frontend handles it as a compile-time branch
                 * with dead-branch suppression (like C++ `if constexpr`).
                 *
                 * The old deferred-passthrough mechanism is removed; the
                 * `#if @` form was never functional end-to-end because
                 * the Sharp parser did not handle the re-emitted `#`
                 * token in the token stream.  Emit a deprecation warning
                 * and fall through to normal #if evaluation, which will
                 * fail on the @-intrinsic token — letting the user know
                 * they need to migrate to `if (@...)`. */
                if (lang == CPP_LANG_SHARP &&
                    in_live_branch(st) &&
                    has_at_intrinsic(&line)) {
                    emit_diag(st, CPP_DIAG_WARNING, dir_loc,
                        "#if @has_operator(...) is deprecated; "
                        "use `if (@has_operator(...)) { ... }` instead "
                        "(Sharp v0.12)");
                }
                handle_if(st, &line, dir_loc);
            } else if (strcmp(dname, "ifdef")  == 0) {
                handle_ifdef(st, &line, dir_loc, false);
            } else if (strcmp(dname, "ifndef") == 0) {
                handle_ifdef(st, &line, dir_loc, true);
            } else if (strcmp(dname, "elif")   == 0) {
                handle_elif(st, &line, dir_loc);
            } else if (strcmp(dname, "else")   == 0) {
                handle_else(st, &line, dir_loc);
            } else if (strcmp(dname, "endif")  == 0) {
                handle_endif(st, &line, dir_loc);
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

            /* Phase R4: _Pragma operator (C99 §6.10.9).
             * `_Pragma ( "..." )` is syntactically equivalent to
             * `#pragma ...` (after destringizing the operand).  It is
             * macro-replaceable: e.g.
             *     #define DOPRAGMA(s) _Pragma(#s)
             *     DOPRAGMA(once)
             * works because `#s` gets stringified, then the resulting
             * `_Pragma("once")` is processed here.
             *
             * sharp-cpp doesn't act on most pragmas, so this code only
             * (a) reports an error if the operand isn't a string literal
             * (mcpp e_pragma) and (b) consumes the entire construct
             * so it doesn't leak into the output stream.                */
            if (strcmp(name, "_Pragma") == 0) {
                pptok_free(&t);
                /* Skip whitespace to find `(` */
                PPTok lp = reader_next_tok(rd, false);
                while (lp.kind == CPPT_SPACE) {
                    pptok_free(&lp);
                    lp = reader_next_tok(rd, false);
                }
                if (lp.kind != CPPT_PUNCT ||
                    strcmp(pptok_spell(&lp), "(") != 0) {
                    /* Not _Pragma( ... ) — emit the IDENT we already
                     * consumed and re-emit lp.                           */
                    PPTok pt = {0};
                    pt.kind = CPPT_IDENT;
                    pt.loc  = t.loc;
                    sb_push_cstr(&pt.spell, "_Pragma");
                    emit_tok_text(st, &pt);
                    pptok_free(&pt);
                    emit_tok_text(st, &lp);
                    pptok_free(&lp);
                    continue;
                }
                pptok_free(&lp);
                /* Skip whitespace to find STRING_LIT */
                PPTok s = reader_next_tok(rd, false);
                while (s.kind == CPPT_SPACE) {
                    pptok_free(&s);
                    s = reader_next_tok(rd, false);
                }
                if (s.kind != CPPT_STRING_LIT) {
                    emit_diag(st, CPP_DIAG_ERROR, t.loc,
                        "_Pragma takes a parenthesized string literal");
                    /* Skip until matching `)` so we resync */
                    int depth = 1;
                    while (depth > 0) {
                        if (s.kind == CPPT_EOF) break;
                        if (s.kind == CPPT_PUNCT) {
                            const char *sp = pptok_spell(&s);
                            if (strcmp(sp, "(") == 0) depth++;
                            else if (strcmp(sp, ")") == 0) {
                                depth--;
                                if (depth == 0) { pptok_free(&s); break; }
                            }
                        }
                        pptok_free(&s);
                        s = reader_next_tok(rd, false);
                    }
                    continue;
                }
                /* Phase R16: save string content BEFORE freeing s */
                char pragma_raw_buf[512];
                {
                    const char *sp = pptok_spell(&s);
                    size_t n = strlen(sp);
                    if (n >= sizeof pragma_raw_buf) n = sizeof pragma_raw_buf - 1;
                    memcpy(pragma_raw_buf, sp, n);
                    pragma_raw_buf[n] = '\0';
                }
                pptok_free(&s);
                /* Skip whitespace to find `)` */
                PPTok rp = reader_next_tok(rd, false);
                while (rp.kind == CPPT_SPACE) {
                    pptok_free(&rp);
                    rp = reader_next_tok(rd, false);
                }
                if (rp.kind != CPPT_PUNCT ||
                    strcmp(pptok_spell(&rp), ")") != 0)
                    emit_diag(st, CPP_DIAG_ERROR, t.loc,
                        "_Pragma: expected ')' after string literal");
                pptok_free(&rp);
                if (in_live_branch(st))
                    emit_pragma_text(st, pragma_raw_buf,
                                     reader_filename(rd), t.loc);
                continue;
            } /* end _Pragma */

            /* Phase R12: __attribute__((...)) — GCC extension.  Previously sharp-cpp
             * stripped this entirely at PP time, but that's wrong: __attribute__ is
             * a *compiler frontend* concern (it affects code generation, ABI, etc.)
             * and gcc/clang's `-E` output preserves it verbatim.  Stripping it at PP
             * time silently corrupts ABI-relevant headers — e.g. <stddef.h>'s
             * max_align_t lost its __aligned__(__alignof__(long long)) specifier,
             * making sharp-cpp's output ABI-incompatible with gcc's.
             *
             * Correct behaviour: pass __attribute__ through untouched and let the
             * downstream compiler handle it.  Same for __pragma below.            */

            /* __pragma(expr) — MSVC keyword.  Consume the parenthesized
             * argument and re-emit it verbatim so that cl.exe can handle
             * it natively.  Do NOT treat it as a #pragma directive —
             * __pragma is an inline form that must remain inline.      */
            if (strcmp(name, "__pragma") == 0) {
                /* Skip whitespace to find '(' */
                PPTok nx = reader_next_tok(rd, false);
                while (nx.kind == CPPT_SPACE || nx.kind == CPPT_NEWLINE) {
                    if (nx.kind == CPPT_NEWLINE && in_live_branch(st))
                        PUSH_CH(st, '\n');
                    pptok_free(&nx);
                    nx = reader_next_tok(rd, false);
                }
                if (nx.kind == CPPT_PUNCT && strcmp(pptok_spell(&nx), "(") == 0) {
                    pptok_free(&nx);
                    /* Output: __pragma( */
                    if (in_live_branch(st)) {
                        flush_pending_lm(st);
                        if (t.has_leading_space) PUSH_CH(st, ' ');
                        PUSH_CSTR(st, "__pragma(");
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
                            if (at.has_leading_space) PUSH_CH(st, ' ');
                            PUSH_CSTR(st, pptok_spell(&at));
                        }
                        pptok_free(&at);
                    }
                    if (in_live_branch(st)) PUSH_CH(st, ')');
                    pptok_free(&t);
                } else {
                    /* Not followed by '(' — pass through as identifier */
                    emit_tok_text(st, &t);
                    pptok_free(&nx);
                }
                continue;
            }

            /* Try macro expansion.
             *
             * Phase 10: cache the MacroDef* from this lookup and reuse it
             * below when checking is_func.  Previously this loop did two
             * separate hash+strcmp passes ({lookup, then lookup_is_func})
             * for every identifier in the source — gprof showed
             * macro_lookup_is_func at 20%+ self-time after Phase 9.  The
             * cached pointer lets us drop the second pass to a single
             * memory read (def->is_func).                                */
            MacroDef *_macdef = !t.hide ? macro_lookup(st->macros, name) : NULL;
            if (_macdef) {
                /* Expansion limits breached? Pass through unexpanded */
                if (macro_limits_breached(st->macros)) {
                    emit_tok_text(st, &t);
                    pptok_free(&t);
                    continue;
                }

                bool macro_had_leading_space = t.has_leading_space;

                /* Phase R12: capture the invocation site loc BEFORE
                 * `tl_append(&input, t)` transfers ownership of t.  Used
                 * downstream when the expansion contains __FILE__ /
                 * __LINE__ — those should refer to the caller, not the
                 * macro body's own loc.                                  */
                CppLoc macro_inv_loc = t.loc;

                /* Build a small input list starting with this token, then
                 * accumulate more if needed for function-like macros.    */
                TokList input = {0};
                tl_append(&input, t); /* t.spell ownership transferred */

                /* For function-like macros we need to grab ahead, but
                 * the expander handles that internally via the list.
                 * Here we feed a single-token list and let macro_expand
                 * pull from the reader when needed.                       */

                /* We need to prime the list with enough look-ahead for
                 * function-like macros (the argument list).  Phase 10:
                 * use the cached _macdef rather than re-querying.        */
                bool _is_func = macro_def_is_func(_macdef);
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
                        bool arg_at_bol = false;  
                        while (depth > 0) {
                            PPTok at = reader_next_tok(rd, false);
                            /* Phase R7: detect a directive-like line
                             * (`#` at start-of-line) inside the macro
                             * argument list.  C99 §6.10.3/11 says this
                             * is undefined behaviour; mcpp's u_1_25.c
                             * exercises it.  We emit a warning rather
                             * than an error to avoid breaking real-
                             * world code that hits this corner; GCC
                             * also warns under -Wpedantic.            */
                            if (arg_at_bol && at.kind == CPPT_PUNCT &&
                                strcmp(pptok_spell(&at), "#") == 0) {
                                emit_diag(st, CPP_DIAG_WARNING, at.loc,
                                    "embedding a directive within macro arguments has undefined behavior");
                            }
                            arg_at_bol = (at.kind == CPPT_NEWLINE);
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
                    /* Phase R18: avoid second hash lookup — macro_lookup once,
                     * then read is_func from the returned pointer.             */
                    MacroDef *_rescan_def = NULL;
                    if (expanded.head && expanded.head->next == NULL &&
                        expanded.head->tok.kind == CPPT_IDENT &&
                        !expanded.head->tok.hide)
                        _rescan_def = macro_lookup(st->macros,
                                         pptok_spell(&expanded.head->tok));
                    if (_rescan_def && macro_def_is_func(_rescan_def)) {
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

                            emit_expanded_list(st, &r_expanded, macro_inv_loc, reader_filename(rd));
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
                        emit_expanded_list(st, &expanded, macro_inv_loc, reader_filename(rd));
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
    /* Phase R12: propagate trigraph mode from CppCtx to the reader. */
    reader_set_trigraphs(rd, st->ctx->trigraphs);
    emit_linemarker(st, 1, filename);
    process_buf(st, rd, lang);
    reader_free(rd);
}

/* Phase R12: variant of process_file used by handle_include — emits the
 * `# 1 "<file>" 1 [3]` linemarker that gcc/clang emit on push, and
 * tracks `cur_file_is_sys` for the nested processing duration so that
 * the matching pop linemarker (emitted by handle_include after this
 * returns) carries the correct flag-3 status.                          */
static void process_include_file(CppState *st, const char *filename,
                                  CppLang lang, bool is_sys_header) {
    CppReader *rd = reader_new_from_file(filename, st->interns, st->diags);
    if (!rd) {
        CppDiag d = { CPP_DIAG_FATAL, {filename, 0, 0},
                      cpp_xstrdup("cannot open file") };
        diag_push(st->diags, d);
        st->fatal = true;
        return;
    }
    
    reader_set_trigraphs(rd, st->ctx->trigraphs);
    bool saved_is_sys = st->cur_file_is_sys;
    st->cur_file_is_sys = is_sys_header;
    /* Phase R12: gcc/clang emit `1 3 4` for system-header push.  Flag 4
     * is the "extern-C" annotation — meaningless in C-mode but emitted
     * universally so downstream tools (consuming -E output) get a
     * faithful copy of the original gcc/clang linemarker.               */
    emit_linemarker_ex(st, 1, filename, 1,
                       is_sys_header ? 3 : 0,
                       is_sys_header ? 4 : 0);
    process_buf(st, rd, lang);
    reader_free(rd);
    st->cur_file_is_sys = saved_is_sys;
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
    /* Phase R12: propagate trigraph mode from CppCtx. */
    reader_set_trigraphs(rd, st->ctx->trigraphs);
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

/* parse_int_literal_pub is now defined in expr.c — Phase 3 unified the
 * integer-literal parsing under one Value-typed implementation, and this
 * minimal wrapper used to live here for #line.  The expr.c version is
 * exposed via directive.h.                                              */

/* POSIX for fileno() */
#define _POSIX_C_SOURCE 200809L
/*
 * pptok.c — Preprocessing Tokenizer (Translation phases 1–3)
 *
 * Phase 1: Trigraph replacement (§5.2.1.1)
 * Phase 2: Line splicing — backslash-newline pairs deleted (§5.1.1.2 p1.2)
 * Phase 3: Tokenization into preprocessing tokens (§6.4)
 *
 * The reader is a lazy character source that layers these three transforms.
 * It never allocates the entire source in expanded form; instead each phase
 * is a thin wrapper around the previous one.
 *
 * Exported:
 *   CppReader  — opaque reader object
 *   reader_new / reader_free
 *   reader_next_tok   — produce the next preprocessing token
 */

#include "cpp_internal.h"
#include "pptok.h"

#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#define fileno _fileno
#endif

/* =========================================================================
 * Raw source buffer with position tracking
 * ====================================================================== */

typedef struct {
    const char *buf;
    size_t      len;
    size_t      pos;
    int         line;
    int         col;
    const char *filename;  /* for diagnostics */
    /* Phase R12: when false, '??x' sequences are not translated to
     * the trigraph-mapped char in phase 1.  Default false to match
     * `gcc -E` (which only warns about trigraphs, doesn't process). */
    bool        trigraphs;
} RawBuf;

/* Phase 9: rb_* are tiny (a couple of memory reads + branch) and called
 * tens of millions of times per typical input.  Marking them `static
 * inline` lets the optimiser inline them into the lexer hot loop, which
 * gprof showed was the dominant bottleneck (rb-layer + ph1/ph2 = 65% of
 * self-time).                                                          */
static inline int rb_peek(const RawBuf *rb) {
    return (rb->pos < rb->len) ? (unsigned char)rb->buf[rb->pos] : -1;
}
static inline int rb_peek2(const RawBuf *rb) {
    return (rb->pos + 1 < rb->len) ? (unsigned char)rb->buf[rb->pos + 1] : -1;
}
static inline int rb_adv(RawBuf *rb) {
    if (CPP_UNLIKELY(rb->pos >= rb->len)) return -1;
    int c = (unsigned char)rb->buf[rb->pos++];
    if (CPP_UNLIKELY(c == '\n')) { rb->line++; rb->col = 1; }
    else                          { rb->col++; }
    return c;
}

/* =========================================================================
 * Phase 1: Trigraph translation
 * Returns the logical character at the current position (applying trigraph).
 * ====================================================================== */

static int trigraph_char(int q) {
    /* q is the character following '??' */
    switch (q) {
    case '=':  return '#';
    case '(':  return '[';
    case '/':  return '\\';
    case ')':  return ']';
    case '\'': return '^';
    case '<':  return '{';
    case '!':  return '|';
    case '>':  return '}';
    case '-':  return '~';
    default:   return -1;
    }
}

/* Read one phase-1 character (trigraph-translated).
 *
 * Phase 9: marked inline + trigraph branch annotated UNLIKELY so the
 * compiler can pull the entire fast path (just rb_adv + return c) into
 * the calling lexer loop.  Trigraphs are virtually never used in modern
 * C code, so the cold path's heavier work — three rb_* calls plus a
 * trigraph_char lookup — must not block inlining of the common path. */
static inline int ph1_get(RawBuf *rb) {
    int c = rb_adv(rb);
    if (CPP_UNLIKELY(rb->trigraphs && c == '?' && rb_peek(rb) == '?')) {
        int q = rb_peek2(rb);
        int mapped = trigraph_char(q);
        if (mapped >= 0) {
            rb_adv(rb); /* consume second '?' */
            rb_adv(rb); /* consume trigger char */
            return mapped;
        }
    }
    return c;
}

/* =========================================================================
 * Phase 2: Line splicing
 * A `\` immediately followed by `\n` is deleted (along with the newline).
 * ====================================================================== */

/* The splicing layer sits atop RawBuf. We pre-read up to 2 raw chars ahead
 * to detect \\\n sequences without consuming them prematurely.            */

/* (No separate struct needed — we implement it inline in the Reader.) */

/* =========================================================================
 * CppReader — full definition
 * ====================================================================== */

struct CppReader {
    /* Source management */
    char       *owned_buf;    /* NULL if buffer was provided externally    */
    RawBuf      raw;

    /* Phase-2 lookahead: we buffer at most one logically-sliced char.     */
    int         ph2_buf;      /* -2 = uninitialised, -1 = EOF              */
    int         ph2_line;
    int         ph2_col;

    /* Location of the *current* logical character (after ph2 get).        */
    int         cur_line;
    int         cur_col;

    /* Intern table reference (not owned) */
    InternTable *interns;

    /* Diagnostic list (shared with CppState) */
    CppDiagArr *diags;

    /* at-beginning-of-line flag for the *next* token */
    bool         at_bol;

    /* Has the last real token been preceded by whitespace? */
    bool         had_space;

    /* Sharp-language mode: when true '@' followed by an ident start is
     * tokenised as CPPT_AT_INTRINSIC instead of two separate tokens.    */
    bool         sharp_mode;

    /* Phase R12: process trigraphs (??=, ??(, ...) per C99 §5.1.1.2 phase 1.
     * Default false to match `gcc -E` (which only warns).  Set via
     * cpp_set_trigraphs() / reader_set_trigraphs() before reading begins.
     * The flag is sampled in ph1_get's hot path.                         */
    bool         trigraphs;
};

/* -------------------------------------------------------------------------
 * Phase 2: get one logical character (with line-splicing).
 * Returns -1 on EOF.
 *
 * Phase 9: split into a fast inline path and a cold out-of-line slow
 * path.  Profiling showed ph2_raw_get accounted for 19% of self-time on
 * typical inputs, but the line-splice case (\\\n or \\\r\n) is rare in
 * almost all real source.  The split lets the inlined fast path collapse
 * to "ph1_get + branch on c == '\\\\'", while the splice loop stays in
 * its own out-of-line function that the compiler can size-optimise
 * without inhibiting the hot path.
 *
 * The slow path is reached only after we already saw a backslash from
 * ph1, so it starts with that knowledge and proceeds to look for the
 * \\\n / \\\r\n splice continuation.
 * ---------------------------------------------------------------------- */

static int ph2_raw_get_slow(CppReader *rd);  /* forward decl, noinline */

static inline int ph2_raw_get(CppReader *rd) {
    int c = ph1_get(&rd->raw);
    if (CPP_LIKELY(c != '\\')) return c;
    return ph2_raw_get_slow(rd);
}

static int ph2_raw_get_slow(CppReader *rd) {
    /* We already consumed a '\\' via ph1_get.  Check whether it begins
     * a line splice (\\\n or \\\r\n); if so, drop it and the newline,
     * fetch the next logical character, and loop in case that one is
     * also a backslash starting another splice.                      */
    int c = '\\';
    while (c == '\\') {
        int next = rb_peek(&rd->raw);
        if (next == '\n') {
            rb_adv(&rd->raw); /* consume '\n' */
            c = ph1_get(&rd->raw);
            /* Loop: the next char might also start a splice. */
        } else if (next == '\r') {
            /* Windows CRLF splice: \\\r\n */
            int next2 = rb_peek2(&rd->raw);
            if (next2 == '\n') {
                rb_adv(&rd->raw); rb_adv(&rd->raw);
                c = ph1_get(&rd->raw);
            } else break;
        } else break;
    }
    return c;
}

static int reader_getc(CppReader *rd) {
    int c = ph2_raw_get(rd);
    rd->cur_line = rd->raw.line;
    rd->cur_col  = rd->raw.col;
    return c;
}

static int reader_peekc(CppReader *rd) {
    /* Save state, get, restore — only works because ph2_raw_get is a pure
     * function of RawBuf state.                                           */
    size_t saved_pos  = rd->raw.pos;
    int    saved_line = rd->raw.line;
    int    saved_col  = rd->raw.col;
    int c = ph2_raw_get(rd);
    rd->raw.pos  = saved_pos;
    rd->raw.line = saved_line;
    rd->raw.col  = saved_col;
    return c;
}

/* =========================================================================
 * Reader construction / destruction
 * ====================================================================== */

CppReader *reader_new_from_file(const char *filename,
                                InternTable *interns,
                                CppDiagArr *diags) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    /* Read entire file, handling pipes and special files (st_size == 0). */
    struct stat st;
    fstat(fileno(f), &st);
    size_t nr = 0;
    char  *buf;

    if (st.st_size > 0) {
        /* Regular file: allocate exactly */
        size_t sz = (size_t)st.st_size;
        buf = (char *)cpp_xmalloc(sz + 2);
        nr  = fread(buf, 1, sz, f);
    } else {
        /* Pipe / stdin / special: read in chunks */
        size_t cap = 4096;
        buf = (char *)cpp_xmalloc(cap);
        size_t n;
        while ((n = fread(buf + nr, 1, cap - nr, f)) > 0) {
            nr += n;
            if (nr == cap) {
                cap *= 2;
                buf  = (char *)cpp_xrealloc(buf, cap);
            }
        }
        buf = (char *)cpp_xrealloc(buf, nr + 2);
    }
    fclose(f);
    buf[nr] = '\0';

    CppReader *rd = cpp_xmalloc(sizeof *rd);
    memset(rd, 0, sizeof *rd);
    rd->owned_buf      = buf;
    rd->raw.buf        = buf;
    rd->raw.len        = nr;
    rd->raw.pos        = 0;
    rd->raw.line       = 1;
    rd->raw.col        = 1;
    rd->raw.filename   = intern_cstr(interns, filename);
    rd->interns        = interns;
    rd->diags          = diags;
    rd->at_bol         = true;
    return rd;
}

CppReader *reader_new_from_buf(const char *buf, size_t len,
                               const char *filename,
                               InternTable *interns,
                               CppDiagArr *diags) {
    CppReader *rd = cpp_xmalloc(sizeof *rd);
    memset(rd, 0, sizeof *rd);
    rd->owned_buf    = NULL;
    rd->raw.buf      = buf;
    rd->raw.len      = len;
    rd->raw.pos      = 0;
    rd->raw.line     = 1;
    rd->raw.col      = 1;
    rd->raw.filename = intern_cstr(interns, filename);
    rd->interns      = interns;
    rd->diags        = diags;
    rd->at_bol       = true;
    return rd;
}

void reader_free(CppReader *rd) {
    if (!rd) return;
    free(rd->owned_buf);
    free(rd);
}

const char *reader_filename(const CppReader *rd) { return rd->raw.filename; }
int reader_current_line(const CppReader *rd) { return rd->raw.line; }
void reader_set_line(CppReader *rd, int line) { rd->raw.line = line; }
/* Phase 4: support `#line N "filename"` rewriting __FILE__.  The string is
 * borrowed (must stay alive for the reader's lifetime); callers in
 * directive.c pass an interned string from CppState's intern table. */
void reader_set_file(CppReader *rd, const char *filename) { rd->raw.filename = filename; }
int reader_current_col(const CppReader *rd) { return rd->raw.col; }
void reader_set_sharp_mode(CppReader *rd, bool sharp) { rd->sharp_mode = sharp; }

/* Phase R12: enable/disable trigraph processing.  Plumbs the flag into
 * the inner RawBuf since ph1_get reads from RawBuf, not CppReader.    */
void reader_set_trigraphs(CppReader *rd, bool on) {
    rd->trigraphs   = on;
    rd->raw.trigraphs = on;
}

/* =========================================================================
 * Tokenizer helpers
 * ====================================================================== */

static CppLoc current_loc(const CppReader *rd) {
    return (CppLoc){ rd->raw.filename, rd->raw.line, rd->raw.col };
}

static bool is_ident_start(int c) {
    return isalpha(c) || c == '_' || c == '$';
}
static bool is_ident_cont(int c) {
    return isalnum(c) || c == '_' || c == '$';
}
static bool is_pp_number_start(int c, int next) {
    /* A preprocessing number starts with an optional `.` followed by a digit
     * (§6.4.8).  A leading `.` that is *not* followed by a digit is NOT a
     * pp-number.                                                            */
    return isdigit(c) || (c == '.' && isdigit(next));
}

/* Consume a C-style block comment.  Called after the opening delimiter has
 * been consumed.  Returns true if the comment closed normally, false on
 * unterminated (EOF reached before a closing star-slash).
 *
 * Phase 1 hardening: previously this function returned silently on EOF and
 * the caller had no way to detect or diagnose an unterminated comment.  The
 * status return lets reader_next_tok emit a CPP_DIAG_ERROR pointing at the
 * opening delimiter. */
static bool skip_block_comment(CppReader *rd, StrBuf *spell, bool keep) {
    if (keep) sb_push_cstr(spell, "/*");

    /* Phase R18: fast path for keep=false (the overwhelmingly common case).
     * Scan the raw buffer directly with memchr, bypassing ph1/ph2 overhead.
     * Block comments cannot contain trigraphs or line-splices that affect
     * the star-slash terminator, so direct raw scanning is safe.           */
    if (!keep) {
        const char *buf = rd->raw.buf + rd->raw.pos;
        const char *end = rd->raw.buf + rd->raw.len;
        for (;;) {
            const char *star = (const char *)memchr(buf, '*', (size_t)(end - buf));
            if (!star) { rd->raw.pos = rd->raw.len; return false; /* unterminated */ }
            /* Count newlines we're skipping over for line tracking */
            for (const char *p = buf; p < star; p++)
                if (*p == '\n') { rd->raw.line++; rd->raw.col = 1; }
                else             rd->raw.col++;
            rd->raw.pos = (size_t)(star - rd->raw.buf) + 1;
            rd->raw.col++;  /* for the '*' itself */
            if (rd->raw.pos < rd->raw.len && rd->raw.buf[rd->raw.pos] == '/') {
                rd->raw.pos++;
                rd->raw.col++;
                rd->ph2_buf = -2;  /* invalidate ph2 lookahead */
                rd->cur_line = rd->raw.line;
                rd->cur_col  = rd->raw.col;
                return true;
            }
            buf = rd->raw.buf + rd->raw.pos;
        }
    }

    for (;;) {
        int c = reader_peekc(rd);
        if (c < 0) return false;
        reader_getc(rd);
        if (keep) sb_push_ch(spell, (char)c);
        if (c == '*') {
            int d = reader_peekc(rd);
            if (d == '/') {
                reader_getc(rd);
                if (keep) sb_push_ch(spell, '/');
                return true;
            }
        }
    }
}

/* Consume a C99 line comment (`//`).  Called after `//` has been consumed.
 * Always returns true — line comments are allowed to abut EOF (no diagnostic). */
static void skip_line_comment(CppReader *rd, StrBuf *spell, bool keep) {
    if (keep) sb_push_cstr(spell, "//");
    for (;;) {
        int c = reader_peekc(rd);
        if (c < 0 || c == '\n') return;
        reader_getc(rd);
        if (keep) sb_push_ch(spell, (char)c);
    }
}

/* Read a string or character literal body (everything including delimiters).
 * `delim` is '\'' or '"'.  Pushes the opening delim as the first character,
 * then consumes characters up to and including the matching closing delim.
 *
 * Returns true on a properly-closed literal, false on unterminated (EOF or
 * an unescaped logical newline before the closing delim).  On unterminated,
 * the trailing newline (if any) is NOT consumed — it remains for the next
 * tokenization step so that line tracking and at_bol stay correct.
 *
 * Phase-2 line splicing has already deleted any `\\<NL>` pairs, so a literal
 * '\n' encountered here is always a real (unescaped) source newline. */
static bool lex_quoted(CppReader *rd, StrBuf *spell, char delim) {
    sb_push_ch(spell, delim);
    for (;;) {
        int c = reader_peekc(rd);
        if (c < 0)     return false; /* EOF before close */
        if (c == '\n') return false; /* unescaped newline — leave for caller */
        reader_getc(rd);
        sb_push_ch(spell, (char)c);
        if (c == '\\') {
            /* Escape sequence: consume the next character verbatim.
             * If the next char is a newline (which phase-2 should have
             * already spliced out, but we are defensive) or EOF, the
             * literal is unterminated.                                  */
            int esc = reader_peekc(rd);
            if (esc < 0)     return false;
            if (esc == '\n') return false;
            reader_getc(rd);
            sb_push_ch(spell, (char)esc);
        } else if (c == delim) {
            return true;
        }
    }
}

/* =========================================================================
 * Phase 3: Tokenization — reader_next_tok
 *
 * Produces one preprocessing token.  Whitespace and newlines are returned
 * as CPPT_SPACE / CPPT_NEWLINE (callers may discard them).
 * ====================================================================== */

PPTok reader_next_tok(CppReader *rd, bool keep_comments) {
    PPTok tok;
    memset(&tok, 0, sizeof tok);

    /* Skip horizontal whitespace, accumulate leading-space flag.
     * Phase R18: avoid peek+get per space character. For the common
     * case where the next raw byte is a plain ASCII space (no line-splice
     * or trigraph), read it directly with rb_adv to halve the call count.
     * Any non-plain character falls back to the ph2 path for correctness. */
    bool had_space = false;
    for (;;) {
        /* Fast path: inspect raw buffer directly for the common case of
         * plain ASCII spaces/tabs — never part of a line-splice.        */
        if (rd->raw.pos < rd->raw.len) {
            unsigned char c = (unsigned char)rd->raw.buf[rd->raw.pos];
            if (c == ' ' || c == '\t') {
                rd->raw.pos++;
                rd->raw.col++;
                rd->cur_col = rd->raw.col;
                had_space = true;
                continue;
            }
            /* Not space/tab: might be \r, \f, \v, or start of \<nl> splice.
             * Use ph2 to resolve the logical character.                    */
        }
        int c = reader_peekc(rd);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v') {
            reader_getc(rd);   /* consume via ph2 (handles splices) */
            had_space = true;
        } else break;
    }

    tok.loc              = current_loc(rd);
    tok.at_bol           = rd->at_bol;
    tok.has_leading_space = had_space;
    rd->at_bol           = false;

    int c = reader_getc(rd);
    if (c < 0) {
        tok.kind = CPPT_EOF;
        return tok;
    }

    /* ------------------------------------------------------------------ */
    /* Newline */
    if (c == '\n') {
        rd->at_bol = true;
        tok.kind = CPPT_NEWLINE;
        sb_push_ch(&tok.spell, '\n');
        return tok;
    }

    /* ------------------------------------------------------------------ */
    /* Comments
     *
     * Phase R4: per C99 §5.1.1.2 phase 3 a comment is replaced by ONE
     * SPACE CHARACTER.  When the caller doesn't want comment tokens
     * (keep_comments=false), we honour the spec by re-typing the token
     * as CPPT_SPACE — not just clearing its spell.  Previously the
     * token stayed CPPT_COMMENT but with an empty / single-space spell,
     * which let comment tokens slip past every `while (kind == SPACE)`
     * skip-loop in directive.c and macro.c.  mcpp's n_5.c exposed this
     * via `# COMMENT define FOO bar` — the inner comment was a non-space
     * token between '#' and 'define', breaking directive recognition. */
    if (c == '/') {
        int d = reader_peekc(rd);
        if (d == '*') {
            reader_getc(rd);
            bool ok = skip_block_comment(rd, &tok.spell, keep_comments);
            tok.kind = CPPT_COMMENT;
            if (!ok) {
                CppDiag diag = { CPP_DIAG_ERROR, tok.loc,
                                 cpp_xstrdup("unterminated /* comment") };
                diag_push(rd->diags, diag);
            }
            if (!keep_comments) {
                sb_free(&tok.spell);
                sb_push_cstr(&tok.spell, " ");
                tok.kind = CPPT_SPACE;          /* phase 3: comment → space */
            }
            return tok;
        }
        if (d == '/') {
            reader_getc(rd);
            skip_line_comment(rd, &tok.spell, keep_comments);
            tok.kind = CPPT_COMMENT;
            if (!keep_comments) {
                sb_free(&tok.spell);
                sb_push_cstr(&tok.spell, " ");  /* still equivalent to one space */
                tok.kind = CPPT_SPACE;
            }
            return tok;
        }
        /* Fall through: '/' not followed by '*' or '/' */
    }

    /* ------------------------------------------------------------------ */
    /* Wide / Unicode string and character literal prefixes:
     *     L"..."   L'...'    (wide,        wchar_t / wint_t)
     *     u"..."   u'...'    (UTF-16,      char16_t)
     *     U"..."   U'...'    (UTF-32,      char32_t)
     *     u8"..."  u8'...'   (UTF-8;       char / char_t)
     *
     * Must be tested BEFORE the identifier handler — otherwise `L` would be
     * consumed as a 1-character identifier and the following `"` would
     * become a separate string literal, splitting one token into two.
     *
     * Strategy: peek 1 char (or 2 for `u8`).  If a quote follows, commit to
     * the prefixed-literal path.  Otherwise restore reader state and fall
     * through to the identifier handler so that `Lvalue`, `u8type`, etc.
     * tokenize correctly as plain identifiers.                            */
    if (c == 'L' || c == 'u' || c == 'U') {
        int next = reader_peekc(rd);

        /* Single-letter prefix followed by '"' or '\'' */
        if (next == '"' || next == '\'') {
            sb_push_ch(&tok.spell, (char)c);
            reader_getc(rd); /* consume the opening quote */
            bool ok = lex_quoted(rd, &tok.spell, (char)next);
            tok.kind = (next == '"') ? CPPT_STRING_LIT : CPPT_CHAR_CONST;
            if (!ok) {
                CppDiag diag = { CPP_DIAG_ERROR, tok.loc,
                                 cpp_xstrdup(next == '"'
                                              ? "unterminated string literal"
                                              : "unterminated character constant") };
                diag_push(rd->diags, diag);
            }
            return tok;
        }

        /* Two-letter prefix `u8` followed by '"' or '\'' */
        if (c == 'u' && next == '8') {
            /* Save reader state: if the char AFTER '8' is not a quote, we
             * must restore so the identifier handler can see this as `u8…`. */
            size_t saved_pos  = rd->raw.pos;
            int    saved_line = rd->raw.line;
            int    saved_col  = rd->raw.col;

            reader_getc(rd); /* consume '8' */
            int n2 = reader_peekc(rd);
            if (n2 == '"' || n2 == '\'') {
                sb_push_ch(&tok.spell, 'u');
                sb_push_ch(&tok.spell, '8');
                reader_getc(rd); /* consume the opening quote */
                bool ok = lex_quoted(rd, &tok.spell, (char)n2);
                tok.kind = (n2 == '"') ? CPPT_STRING_LIT : CPPT_CHAR_CONST;
                if (!ok) {
                    CppDiag diag = { CPP_DIAG_ERROR, tok.loc,
                                     cpp_xstrdup(n2 == '"'
                                                  ? "unterminated string literal"
                                                  : "unterminated character constant") };
                    diag_push(rd->diags, diag);
                }
                return tok;
            }
            /* Not a u8-prefixed literal — restore so 'u8…' is an identifier. */
            rd->raw.pos  = saved_pos;
            rd->raw.line = saved_line;
            rd->raw.col  = saved_col;
        }
        /* Fall through to identifier handler with c still being L/u/U. */
    }

    /* ------------------------------------------------------------------ */
    /* Identifier or keyword.
     * Phase R7: support UCN (universal character names) inside identifiers
     * per C99 §6.4.2.1.  `\uXXXX` and `\UXXXXXXXX` count as ident chars.
     * The UCN is preserved verbatim in the spell so two macros named
     * `macro\u5B57` and `macro\U00006F22` are distinguishable from each
     * other and from `macro` (mcpp's n_ucn1.c).  Also validate the
     * code-point ranges per C99 §6.4.3 (e_ucn.c):
     *   - reject < 0xA0 except $ @ ` (control chars must error)
     *   - reject D800..DFFF (surrogates) for C
     *   - reject incomplete (too-short) UCN sequences as syntax error  */
    if (is_ident_start(c)) {
        tok.kind = CPPT_IDENT;
        sb_push_ch(&tok.spell, (char)c);
        for (;;) {
            int d = reader_peekc(rd);
            if (d == '\\') {
                /* Possible UCN escape inside identifier.  Consume `\`
                 * then check the next char.  If it's `u`/`U` we have
                 * a UCN; otherwise we treat the `\` as outside the
                 * ident — we must "unget" it conceptually, but since
                 * the reader has no unget, we instead just include
                 * the backslash in the spell as best-effort.  In well-
                 * formed C this case never happens (a stray `\` in an
                 * ident position is a syntax error anyway).            */
                reader_getc(rd);  /* consume `\` */
                int after_bs = reader_peekc(rd);
                if (after_bs == 'u' || after_bs == 'U') {
                    int hex_digits = (after_bs == 'u') ? 4 : 8;
                    sb_push_ch(&tok.spell, '\\');
                    reader_getc(rd);
                    sb_push_ch(&tok.spell, (char)after_bs);
                    /* Read exactly hex_digits hex chars */
                    uint32_t cp = 0;
                    int got = 0;
                    for (int i = 0; i < hex_digits; i++) {
                        int hd = reader_peekc(rd);
                        if (!((hd >= '0' && hd <= '9') ||
                              (hd >= 'a' && hd <= 'f') ||
                              (hd >= 'A' && hd <= 'F'))) break;
                        sb_push_ch(&tok.spell, (char)hd);
                        reader_getc(rd);
                        if (hd >= '0' && hd <= '9')      cp = cp*16 + (hd - '0');
                        else if (hd >= 'a' && hd <= 'f') cp = cp*16 + (hd - 'a' + 10);
                        else                              cp = cp*16 + (hd - 'A' + 10);
                        got++;
                    }
                    if (got != hex_digits) {
                        if (rd->diags) {
                            CppDiag dg = { CPP_DIAG_ERROR, tok.loc,
                                cpp_xstrdup("incomplete universal-character-name") };
                            diag_push(rd->diags, dg);
                        }
                        continue;
                    }
                    bool bad = false;
                    if (cp < 0xA0 && cp != 0x24 && cp != 0x40 && cp != 0x60) bad = true;
                    if (cp >= 0xD800 && cp <= 0xDFFF) bad = true;
                    if (cp > 0x10FFFF) bad = true;
                    if (bad && rd->diags) {
                        char msg[160];
                        snprintf(msg, sizeof msg,
                            "universal-character-name '\\%c%0*X' specifies an invalid value",
                            (char)after_bs, hex_digits, cp);
                        CppDiag dg = { CPP_DIAG_ERROR, tok.loc, cpp_xstrdup(msg) };
                        diag_push(rd->diags, dg);
                    }
                    continue;
                }
                /* Stray `\` not followed by u/U — break out of ident.
                 * (We've already consumed the `\`; emitting it as part
                 * of a punctuator is a separate concern.)             */
                break;
            }
            if (!is_ident_cont(d)) break;
            sb_push_ch(&tok.spell, (char)d);
            reader_getc(rd);
        }
        return tok;
    }

    /* ------------------------------------------------------------------ */
    /* Preprocessing number */
    if (is_pp_number_start(c, reader_peekc(rd))) {
        tok.kind = CPPT_PP_NUMBER;
        sb_push_ch(&tok.spell, (char)c);
        for (;;) {
            int d = reader_peekc(rd);
            /* A pp-number may contain: digits, letters, '_', '.', and
             * 'e±' / 'E±' / 'p±' / 'P±' exponent parts.               */
            if (isalnum(d) || d == '_' || d == '.') {
                sb_push_ch(&tok.spell, (char)d);
                reader_getc(rd);
                if ((d == 'e' || d == 'E' || d == 'p' || d == 'P')) {
                    int e = reader_peekc(rd);
                    if (e == '+' || e == '-') {
                        sb_push_ch(&tok.spell, (char)e);
                        reader_getc(rd);
                    }
                }
            } else break;
        }
        return tok;
    }

    /* ------------------------------------------------------------------ */
    /* String literal */
    if (c == '"') {
        tok.kind = CPPT_STRING_LIT;
        bool ok = lex_quoted(rd, &tok.spell, '"');
        if (!ok) {
            CppDiag diag = { CPP_DIAG_ERROR, tok.loc,
                             cpp_xstrdup("unterminated string literal") };
            diag_push(rd->diags, diag);
        }
        return tok;
    }

    /* ------------------------------------------------------------------ */
    /* Character constant */
    if (c == '\'') {
        tok.kind = CPPT_CHAR_CONST;
        bool ok = lex_quoted(rd, &tok.spell, '\'');
        if (!ok) {
            CppDiag diag = { CPP_DIAG_ERROR, tok.loc,
                             cpp_xstrdup("unterminated character constant") };
            diag_push(rd->diags, diag);
        }
        return tok;
    }

    /* ------------------------------------------------------------------ */
    /* Sharp @ intrinsic: @ident  (Sharp mode only)
     *
     * When sharp_mode is active and the current character is '@' followed
     * immediately by an identifier start character, we consume the entire
     * "@ident" string as a single CPPT_AT_INTRINSIC token.  The spell
     * contains the verbatim text including the '@' prefix.
     *
     * Sharp v0.11 defines exactly two @ intrinsics:
     *   @has_operator(T, <op>)     — T defines the given operator
     *   @static_assert(cond, msg)  — compile-time assertion
     *
     * All other @ident forms are passed through as CPPT_AT_INTRINSIC
     * and rejected by the language frontend.
     *
     * In C mode '@' falls through to the CPPT_PUNCT handler below.       */
    if (c == '@' && rd->sharp_mode) {
        int d = reader_peekc(rd);
        if (is_ident_start(d)) {
            tok.kind = CPPT_AT_INTRINSIC;
            sb_push_ch(&tok.spell, '@');
            /* Consume the identifier characters */
            for (;;) {
                int e = reader_peekc(rd);
                if (!is_ident_cont(e)) break;
                sb_push_ch(&tok.spell, (char)e);
                reader_getc(rd);
            }
            return tok;
        }
        /* '@' not followed by ident — fall through to CPPT_PUNCT */
    }

    /* ------------------------------------------------------------------ */
    /* Phase R4: catch control characters that have no place in PP source.
     * Per C99 §5.1.1.2 the source character set excludes most control
     * characters; mcpp's u_1_5.c uses 0x1E (RS) and 0x1F (US) and
     * expects "invalid character" diagnostics.  Newline (0x0A), tab
     * (0x09), vertical-tab (0x0B), form-feed (0x0C), and carriage-return
     * (0x0D) are explicitly part of the source set and handled
     * earlier.  Anything else < 0x20 lands here.                       */
    if (c > 0 && c < 0x20 && c != '\t' && c != '\n' && c != '\v' &&
        c != '\f' && c != '\r') {
        CppDiag diag = { CPP_DIAG_ERROR, tok.loc,
                         NULL };
        char buf[80];
        snprintf(buf, sizeof buf,
                 "invalid character 0x%02x in source", (unsigned)c);
        diag.msg = cpp_xstrdup(buf);
        diag_push(rd->diags, diag);
        /* Continue with PUNCT processing so we still produce a token
         * and don't desynchronise downstream.                         */
    }

    /* ------------------------------------------------------------------ */
    /* Punctuators (ISO §6.4.6) — longest-match */
    tok.kind = CPPT_PUNCT;
    sb_push_ch(&tok.spell, (char)c);
    int n = reader_peekc(rd);

#define ADVC() do { sb_push_ch(&tok.spell,(char)n); reader_getc(rd); n=reader_peekc(rd); } while(0)

    switch (c) {
    case '#':
        if (n == '#') { ADVC(); } /* ## token-paste */
        break;
    case '.':
        if (n == '.' && reader_peekc(rd) == '.') {
            /* "..." — peek ahead further */
            /* We peeked one char ahead; need to check the one after that. */
            /* Simple: consume one and check next. */
            size_t saved = rd->raw.pos; int sl = rd->raw.line, sc = rd->raw.col;
            reader_getc(rd); /* consume second '.' */
            int n3 = reader_peekc(rd);
            if (n3 == '.') {
                sb_push_ch(&tok.spell, '.');
                sb_push_ch(&tok.spell, '.');
                reader_getc(rd);
            } else {
                /* only ".." — put back */
                rd->raw.pos  = saved;
                rd->raw.line = sl;
                rd->raw.col  = sc;
            }
        }
        break;
    case '-':
        if (n == '>') { ADVC(); }
        else if (n == '-') { ADVC(); }
        else if (n == '=') { ADVC(); }
        break;
    case '+':
        if (n == '+') { ADVC(); }
        else if (n == '=') { ADVC(); }
        break;
    case '&':
        if (n == '&') { ADVC(); }
        else if (n == '=') { ADVC(); }
        break;
    case '|':
        if (n == '|') { ADVC(); }
        else if (n == '=') { ADVC(); }
        break;
    case '<':
        if (n == '<') { ADVC(); if (n == '=') { ADVC(); } }
        else if (n == '=') { ADVC(); }
        else if (n == ':') { ADVC(); } /* digraph [  */
        else if (n == '%') { ADVC(); } /* digraph {  */
        break;
    case '>':
        if (n == '>') { ADVC(); if (n == '=') { ADVC(); } }
        else if (n == '=') { ADVC(); }
        break;
    case '=':
        if (n == '=') { ADVC(); }
        break;
    case '!':
        if (n == '=') { ADVC(); }
        break;
    case '*':
        if (n == '=') { ADVC(); }
        break;
    case '/':
        if (n == '=') { ADVC(); }
        break;
    case '%':
        if (n == '=') { ADVC(); }
        else if (n == '>') { ADVC(); } /* digraph } */
        else if (n == ':') {
            ADVC();
            /* %:%: digraph ## */
            if (n == '%') {
                size_t sv = rd->raw.pos; int sl = rd->raw.line, sc = rd->raw.col;
                reader_getc(rd);
                if (reader_peekc(rd) == ':') {
                    sb_push_ch(&tok.spell, ':');
                    reader_getc(rd);
                    sb_push_ch(&tok.spell, ':');
                } else {
                    rd->raw.pos = sv; rd->raw.line = sl; rd->raw.col = sc;
                }
            }
        }
        break;
    case '^':
        if (n == '=') { ADVC(); }
        break;
    case ':':
        if (n == '>') { ADVC(); } /* digraph ] */
        break;
    default:
        break;
    }
#undef ADVC

    return tok;
}

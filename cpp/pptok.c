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
} RawBuf;

static int rb_peek(const RawBuf *rb) {
    return (rb->pos < rb->len) ? (unsigned char)rb->buf[rb->pos] : -1;
}
static int rb_peek2(const RawBuf *rb) {
    return (rb->pos + 1 < rb->len) ? (unsigned char)rb->buf[rb->pos + 1] : -1;
}
static int rb_adv(RawBuf *rb) {
    if (rb->pos >= rb->len) return -1;
    int c = (unsigned char)rb->buf[rb->pos++];
    if (c == '\n') { rb->line++; rb->col = 1; }
    else            { rb->col++; }
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

/* Read one phase-1 character (trigraph-translated). */
static int ph1_get(RawBuf *rb) {
    int c = rb_adv(rb);
    if (c == '?' && rb_peek(rb) == '?') {
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
};

/* -------------------------------------------------------------------------
 * Phase 2: get one logical character (with line-splicing).
 * Returns -1 on EOF.
 * ---------------------------------------------------------------------- */

static int ph2_raw_get(CppReader *rd) {
    /* Reads one phase-1 character, then checks for \\\n.                 */
    int c = ph1_get(&rd->raw);
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
int reader_current_col(const CppReader *rd) { return rd->raw.col; }

/* =========================================================================
 * Tokenizer helpers
 * ====================================================================== */

static CppLoc current_loc(const CppReader *rd) {
    return (CppLoc){ rd->raw.filename, rd->raw.line, rd->raw.col };
}

static bool is_ident_start(int c) {
    return isalpha(c) || c == '_';
}
static bool is_ident_cont(int c) {
    return isalnum(c) || c == '_';
}
static bool is_pp_number_start(int c, int next) {
    /* A preprocessing number starts with an optional `.` followed by a digit
     * (§6.4.8).  A leading `.` that is *not* followed by a digit is NOT a
     * pp-number.                                                            */
    return isdigit(c) || (c == '.' && isdigit(next));
}

/* Consume a C-style block comment.  Called after the opening delimiter has been consumed. */
static void skip_block_comment(CppReader *rd, StrBuf *spell, bool keep) {
    if (keep) sb_push_cstr(spell, "/*");
    for (;;) {
        int c = reader_getc(rd);
        if (c < 0) return; /* unterminated — let the caller diagnose */
        if (keep) sb_push_ch(spell, (char)c);
        if (c == '*') {
            int d = reader_peekc(rd);
            if (d == '/') {
                reader_getc(rd);
                if (keep) sb_push_ch(spell, '/');
                return;
            }
        }
    }
}

/* Consume a C99 line comment (`//`).  Called after `//` has been consumed. */
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
 * `delim` is '\'' or '"'. */
static void lex_quoted(CppReader *rd, StrBuf *spell, char delim) {
    sb_push_ch(spell, delim);
    for (;;) {
        int c = reader_getc(rd);
        if (c < 0 || c == '\n') break; /* unterminated */
        sb_push_ch(spell, (char)c);
        if (c == '\\') {
            int esc = reader_getc(rd);
            if (esc >= 0) sb_push_ch(spell, (char)esc);
        } else if (c == delim) break;
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

    /* Skip horizontal whitespace, accumulate leading-space flag. */
    bool had_space = false;
    for (;;) {
        int c = reader_peekc(rd);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v') {
            reader_getc(rd);
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
    /* Comments */
    if (c == '/') {
        int d = reader_peekc(rd);
        if (d == '*') {
            reader_getc(rd);
            skip_block_comment(rd, &tok.spell, keep_comments);
            tok.kind = CPPT_COMMENT;
            if (!keep_comments) { sb_free(&tok.spell); sb_push_cstr(&tok.spell, " "); }
            return tok;
        }
        if (d == '/') {
            reader_getc(rd);
            skip_line_comment(rd, &tok.spell, keep_comments);
            tok.kind = CPPT_COMMENT;
            if (!keep_comments) { sb_free(&tok.spell); /* no text emitted */ }
            return tok;
        }
        /* Fall through: '/' not followed by '*' or '/' */
    }

    /* ------------------------------------------------------------------ */
    /* Identifier or keyword */
    if (is_ident_start(c)) {
        tok.kind = CPPT_IDENT;
        sb_push_ch(&tok.spell, (char)c);
        for (;;) {
            int d = reader_peekc(rd);
            if (!is_ident_cont(d)) break;
            sb_push_ch(&tok.spell, (char)d);
            reader_getc(rd);
        }
        return tok;
    }

    /* ------------------------------------------------------------------ */
    /* Wide / Unicode string / char prefix: L, u, U, u8 */
    if ((c == 'L' || c == 'u' || c == 'U') ||
        (c == 'u' && reader_peekc(rd) == '8')) {
        char prefix[3] = {0};
        int pi = 0;
        prefix[pi++] = (char)c;
        int d = reader_peekc(rd);
        if (c == 'u' && d == '8') {
            reader_getc(rd);
            prefix[pi++] = '8';
            d = reader_peekc(rd);
        }
        if (d == '"' || d == '\'') {
            reader_getc(rd);
            sb_push(&tok.spell, prefix, pi);
            lex_quoted(rd, &tok.spell, (char)d);
            tok.kind = (d == '"') ? CPPT_STRING_LIT : CPPT_CHAR_CONST;
            return tok;
        }
        /* Not a string prefix — treat as identifier start */
        tok.kind = CPPT_IDENT;
        sb_push(&tok.spell, prefix, pi);
        for (;;) {
            d = reader_peekc(rd);
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
        lex_quoted(rd, &tok.spell, '"');
        return tok;
    }

    /* ------------------------------------------------------------------ */
    /* Character constant */
    if (c == '\'') {
        tok.kind = CPPT_CHAR_CONST;
        lex_quoted(rd, &tok.spell, '\'');
        return tok;
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

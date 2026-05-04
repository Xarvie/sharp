/*
 * lex.c — Sharp Frontend: Phase 1 Second-Stage Lexer.
 *
 * Phase 1: maps cpp's pp-token stream to SharpTok[]:
 *   - keyword promotion via lookup table
 *   - punctuator subdivision via text-length switch
 *   - PP_NUMBER → INT_LIT / FLOAT_LIT with value parsing
 *   - CHAR_CONST → CHAR_LIT with escape-sequence decoding
 *   - STRING_LIT pass-through
 *   - AT_INTRINSIC pass-through
 *   - whitespace / comment tokens dropped
 */

#include "lex.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

/* =========================================================================
 * Keyword table
 * ====================================================================== */

typedef struct { const char *word; SharpTokKind kind; } KwEntry;

/* Phase 1: keyword lookup table — sorted by first char then length for
 * fast switch-based dispatch.  Sharp keywords ('defer', 'operator', 'null',
 * 'this') live here too; they are promoted alongside C keywords. */
static const KwEntry kw_table[] = {
    /* a */ { "auto",             STOK_AUTO          },
    /* b */ { "break",            STOK_BREAK         },
    /* c */ { "case",             STOK_CASE          },
            { "char",             STOK_CHAR          },
            { "const",            STOK_CONST         },
            { "continue",         STOK_CONTINUE      },
    /* d */ { "default",          STOK_DEFAULT       },
            { "defer",            STOK_DEFER         },  /* Phase 1: Sharp */
            { "do",               STOK_DO            },
            { "double",           STOK_DOUBLE        },
    /* e */ { "else",             STOK_ELSE          },
            { "enum",             STOK_ENUM          },
            { "extern",           STOK_EXTERN        },
    /* f */ { "float",            STOK_FLOAT         },
            { "for",              STOK_FOR           },
    /* g */ { "goto",             STOK_GOTO          },
    /* i */ { "if",               STOK_IF            },
            { "import",           STOK_IMPORT        },  /* Phase 1: Sharp spec §模块系统 */
            { "inline",           STOK_INLINE        },
            { "int",              STOK_INT           },
    /* l */ { "long",             STOK_LONG          },
    /* n */ { "null",             STOK_NULL          },  /* Phase 1: Sharp */
    /* o */ { "operator",         STOK_OPERATOR      },  /* Phase 1: Sharp */
    /* r */ { "register",         STOK_REGISTER      },
            { "restrict",         STOK_RESTRICT      },
            { "return",           STOK_RETURN        },
    /* s */ { "short",            STOK_SHORT         },
            { "signed",           STOK_SIGNED        },
            { "sizeof",           STOK_SIZEOF        },
            { "static",           STOK_STATIC        },
            { "struct",           STOK_STRUCT        },
            { "switch",           STOK_SWITCH        },
    /* t */ { "this",             STOK_THIS          },  /* Phase 1: Sharp */
            { "typedef",          STOK_TYPEDEF       },
    /* u */ { "union",            STOK_UNION         },
            { "unsigned",         STOK_UNSIGNED      },
    /* v */ { "void",             STOK_VOID          },
            { "volatile",         STOK_VOLATILE      },
    /* w */ { "while",            STOK_WHILE         },
    /* _ */ { "_Alignas",         STOK__ALIGNAS      },
            { "_Alignof",         STOK__ALIGNOF      },
            { "_Atomic",          STOK__ATOMIC       },
            { "_Bool",            STOK__BOOL         },
            { "_Complex",         STOK__COMPLEX      },
            { "_Generic",         STOK__GENERIC      },
            { "_Imaginary",       STOK__IMAGINARY    },
            { "_Noreturn",        STOK__NORETURN     },
            { "_Static_assert",   STOK__STATIC_ASSERT},
            { "_Thread_local",    STOK__THREAD_LOCAL },
            /* GCC alias spellings of standard C keywords (used by glibc
             * headers under -ansi / -std=c89 to remain identifier-safe).
             * Map each to the canonical token kind so the parser sees
             * them identically to their ISO forms. */
            { "__restrict",       STOK_RESTRICT      },
            { "__restrict__",     STOK_RESTRICT      },
            { "__inline",         STOK_INLINE        },
            { "__inline__",       STOK_INLINE        },
            { "__const",          STOK_CONST         },
            { "__const__",        STOK_CONST         },
            { "__volatile",       STOK_VOLATILE      },
            { "__volatile__",     STOK_VOLATILE      },
            { "__signed",         STOK_SIGNED        },
            { "__signed__",       STOK_SIGNED        },
            /* GCC extension: __attribute__ and the older alias __attribute.
             * Always followed by `((…))` payload, swallowed by parse.c. */
            { "__attribute__",    STOK_ATTRIBUTE     },
            { "__attribute",      STOK_ATTRIBUTE     },
            /* GCC asm-name extension: declarator-suffix
             * `extern int foo(int) __asm__("name");` — symbol rename.
             * Sharp accepts and discards.  Three spellings recognised. */
            { "asm",              STOK_ASM           },
            { "__asm",            STOK_ASM           },
            { "__asm__",          STOK_ASM           },
};
static const size_t kw_count = sizeof kw_table / sizeof kw_table[0];

/* Phase 1: O(n) linear scan — keyword count is small (~50).
 * Replace with a perfect hash in a future optimisation phase if profiling
 * shows it as a hot spot.
 *
 * Phase R1: when `sharp_kw` is false (caller is processing C source),
 * skip the five Sharp-specific keywords so identifiers named
 * `null` / `defer` / `operator` / `this` / `import` in C code parse as
 * plain IDENT.  cJSON 1.7.18 has `cJSON *null = ...`; standard C does
 * not reserve any of these names. */
static SharpTokKind keyword_kind(const char *text, size_t len, bool sharp_kw) {
    for (size_t i = 0; i < kw_count; i++) {
        const char *w = kw_table[i].word;
        size_t wlen   = strlen(w);
        if (wlen == len && strncmp(w, text, len) == 0) {
            if (!sharp_kw) {
                SharpTokKind k = kw_table[i].kind;
                /* Sharp-only keywords stay identifiers in C mode. */
                if (k == STOK_DEFER || k == STOK_IMPORT ||
                    k == STOK_NULL  || k == STOK_OPERATOR ||
                    k == STOK_THIS)
                    return STOK_IDENT;
            }
            return kw_table[i].kind;
        }
    }
    return STOK_IDENT;
}

/* =========================================================================
 * Punctuator subdivision
 * ====================================================================== */

static SharpTokKind punct_kind(const char *text, size_t len) {
    if (len == 1) {
        switch (text[0]) {
        case '{': return STOK_LBRACE;    case '}': return STOK_RBRACE;
        case '[': return STOK_LBRACKET;  case ']': return STOK_RBRACKET;
        case '(': return STOK_LPAREN;    case ')': return STOK_RPAREN;
        case ';': return STOK_SEMI;      case ':': return STOK_COLON;
        case ',': return STOK_COMMA;     case '?': return STOK_QUESTION;
        case '~': return STOK_TILDE;     case '#': return STOK_HASH;
        case '.': return STOK_DOT;
        case '+': return STOK_PLUS;      case '-': return STOK_MINUS;
        case '*': return STOK_STAR;      case '/': return STOK_SLASH;
        case '%': return STOK_PERCENT;   case '&': return STOK_AMP;
        case '|': return STOK_PIPE;      case '^': return STOK_CARET;
        case '!': return STOK_BANG;      case '=': return STOK_EQ;
        case '<': return STOK_LT;        case '>': return STOK_GT;
        default:  return STOK_OTHER;
        }
    }
    if (len == 2) {
        char a = text[0], b = text[1];
        if (a=='-' && b=='>') return STOK_ARROW;
        if (a=='+' && b=='+') return STOK_PLUSPLUS;
        if (a=='-' && b=='-') return STOK_MINUSMINUS;
        if (a=='+' && b=='=') return STOK_PLUSEQ;
        if (a=='-' && b=='=') return STOK_MINUSEQ;
        if (a=='*' && b=='=') return STOK_STAREQ;
        if (a=='/' && b=='=') return STOK_SLASHEQ;
        if (a=='%' && b=='=') return STOK_PERCENTEQ;
        if (a=='&' && b=='&') return STOK_AMPAMP;
        if (a=='&' && b=='=') return STOK_AMPEQ;
        if (a=='|' && b=='|') return STOK_PIPEPIPE;
        if (a=='|' && b=='=') return STOK_PIPEEQ;
        if (a=='^' && b=='=') return STOK_CARETEQ;
        if (a=='!' && b=='=') return STOK_BANGEQ;
        if (a=='=' && b=='=') return STOK_EQEQ;
        if (a=='<' && b=='<') return STOK_LTLT;
        if (a=='<' && b=='=') return STOK_LTEQ;
        if (a=='>' && b=='>') return STOK_GTGT;
        if (a=='>' && b=='=') return STOK_GTEQ;
        if (a=='#' && b=='#') return STOK_HASHHASH;
        return STOK_OTHER;
    }
    if (len == 3) {
        if (text[0]=='.' && text[1]=='.' && text[2]=='.') return STOK_ELLIPSIS;
        if (text[0]=='<' && text[1]=='<' && text[2]=='=') return STOK_LTLTEQ;
        if (text[0]=='>' && text[1]=='>' && text[2]=='=') return STOK_GTGTEQ;
        return STOK_OTHER;
    }
    return STOK_OTHER;
}

/* =========================================================================
 * Number literal parsing
 * ====================================================================== */

/* Return true if the PP_NUMBER text represents a floating-point constant. */
static bool is_float(const char *text, size_t len) {
    bool hex = len >= 2 && text[0] == '0' && (text[1]=='x' || text[1]=='X');
    for (size_t i = 0; i < len; i++) {
        char c = text[i];
        if (c == '.') return true;
        /* Decimal exponent (only in non-hex context). */
        if (!hex && (c == 'e' || c == 'E')) return true;
        /* Binary exponent (hex float). */
        if (hex  && (c == 'p' || c == 'P')) return true;
    }
    return false;
}

/* Parse integer suffix: consume u/U, l/L, ll/LL at the end of text.
 * Returns the length of the suffix consumed (0 if none). */
static size_t int_suffix_len(const char *text, size_t len) {
    size_t s = len;
    /* Consume ll/LL or l/L first (order matters for ll). */
    if (s >= 3 && (text[s-2]=='l'||text[s-2]=='L') && text[s-1]==text[s-2]) s -= 2;
    else if (s >= 2 && (text[s-1]=='l'||text[s-1]=='L')) s--;
    /* Then u/U. */
    if (s >= 2 && (text[s-1]=='u'||text[s-1]=='U')) s--;
    else if (s > 0 && s < len && (text[s-1]=='u'||text[s-1]=='U')) s--;
    return len - s;
}

/* Re-check if the suffix contains 'u' or 'U'. */
static bool suffix_is_unsigned(const char *text, size_t len) {
    for (size_t i = 0; i < len; i++)
        if (text[i]=='u' || text[i]=='U') return true;
    return false;
}
static bool suffix_has_ll(const char *text, size_t len) {
    for (size_t i = 0; i+1 < len; i++)
        if ((text[i]=='l'||text[i]=='L') && text[i+1]==text[i]) return true;
    return false;
}
static bool suffix_has_l(const char *text, size_t len) {
    if (suffix_has_ll(text, len)) return false;
    for (size_t i = 0; i < len; i++)
        if (text[i]=='l' || text[i]=='L') return true;
    return false;
}

static void parse_int_literal(SharpTok *t, FeDiagArr *diags) {
    const char *text = t->text;
    size_t len       = t->len;

    /* Strip suffix. */
    size_t suf_len   = int_suffix_len(text, len);
    size_t body_len  = len - suf_len;
    const char *suf  = text + body_len;
    bool u_suffix    = suffix_is_unsigned(suf, suf_len);
    bool ll_suffix   = suffix_has_ll(suf, suf_len);
    bool l_suffix    = suffix_has_l(suf, suf_len);

    /* Copy body to a NUL-terminated buffer for strtoull. */
    char buf[128];
    if (body_len >= sizeof buf) body_len = sizeof buf - 1;
    memcpy(buf, text, body_len);
    buf[body_len] = '\0';

    /* Determine base from prefix. */
    int base = 10;
    if (body_len >= 2 && buf[0] == '0' && (buf[1]=='x' || buf[1]=='X')) {
        base = 16;
    } else if (body_len >= 2 && buf[0] == '0' && (buf[1]=='b' || buf[1]=='B')) {
        /* Phase 1: binary literal (GCC extension). */
        base = 2;
        /* strtoull doesn't handle 0b; convert manually. */
        uint64_t val = 0;
        for (size_t i = 2; i < body_len; i++) {
            val = (val << 1) | (unsigned)(buf[i] - '0');
        }
        t->u.uval      = val;
        t->is_unsigned  = u_suffix || (val > (uint64_t)INT64_MAX);
        t->is_longlong  = ll_suffix;
        t->is_long      = l_suffix;
        if (!t->is_unsigned) t->u.ival = (int64_t)val;
        return;
    } else if (body_len >= 2 && buf[0] == '0') {
        base = 8;
    }

    errno = 0;
    unsigned long long val = strtoull(buf, NULL, base);
    if (errno == ERANGE) {
        /* Overflow: emit warning, keep saturated value. */
        FE_WARNING(diags, t->loc,
            "integer constant '%.*s' overflows unsigned 64-bit range",
            (int)len, text);
    }

    t->u.uval      = (uint64_t)val;
    /* Per C11 §6.4.4.1/5: hex/octal with high bit set are unsigned. */
    bool high_bit = (val > (uint64_t)INT64_MAX);
    t->is_unsigned  = u_suffix || high_bit;
    t->is_longlong  = ll_suffix;
    t->is_long      = l_suffix;
    if (!t->is_unsigned)
        t->u.ival = (int64_t)val;
}

static void parse_float_literal(SharpTok *t, FeDiagArr *diags) {
    /* Strip trailing suffix (f/F/l/L). */
    const char *text = t->text;
    size_t len       = t->len;
    if (len > 0) {
        char last = text[len-1];
        if (last=='f'||last=='F'||last=='l'||last=='L') len--;
    }
    char buf[256];
    if (len >= sizeof buf) len = sizeof buf - 1;
    memcpy(buf, text, len);
    buf[len] = '\0';

    errno = 0;
    t->u.fval = strtod(buf, NULL);
    if (errno == ERANGE)
        FE_WARNING(diags, t->loc,
            "floating constant '%.*s' out of range", (int)t->len, text);
    (void)diags;
}

/* =========================================================================
 * Character constant parsing
 * ====================================================================== */

/* Decode one escape sequence starting at *p (after the backslash).
 * Advances *p past the consumed characters. */
static int64_t decode_escape(const char **p, const char *end) {
    if (*p >= end) return '\\';
    char c = *(*p)++;
    switch (c) {
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
    case '?':  return '?';
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7': {
        /* Octal: up to 3 digits. */
        int64_t val = c - '0';
        for (int i = 0; i < 2 && *p < end &&
                        **p >= '0' && **p <= '7'; i++)
            val = (val << 3) | (*(*p)++ - '0');
        return val;
    }
    case 'x': {
        /* Hex: any number of hex digits. */
        int64_t val = 0;
        while (*p < end && isxdigit((unsigned char)**p)) {
            int d = isdigit((unsigned char)**p) ? (**p - '0')
                  : (tolower((unsigned char)**p) - 'a' + 10);
            val = (val << 4) | d;
            (*p)++;
        }
        return val;
    }
    case 'u': {
        /* UCN \uXXXX */
        int64_t val = 0;
        for (int i = 0; i < 4 && *p < end && isxdigit((unsigned char)**p); i++) {
            int d = isdigit((unsigned char)**p) ? (**p - '0')
                  : (tolower((unsigned char)**p) - 'a' + 10);
            val = (val << 4) | d;
            (*p)++;
        }
        return val;
    }
    case 'U': {
        /* UCN \UXXXXXXXX */
        int64_t val = 0;
        for (int i = 0; i < 8 && *p < end && isxdigit((unsigned char)**p); i++) {
            int d = isdigit((unsigned char)**p) ? (**p - '0')
                  : (tolower((unsigned char)**p) - 'a' + 10);
            val = (val << 4) | d;
            (*p)++;
        }
        return val;
    }
    default:   return c;
    }
}

static void parse_char_literal(SharpTok *t, FeDiagArr *diags) {
    const char *text = t->text;
    size_t len       = t->len;

    /* Skip prefix (L / u / U / u8). */
    const char *p = text;
    const char *end = text + len;
    bool wide_or_unsigned = false;
    if (p < end && (*p=='L' || *p=='u' || *p=='U')) {
        if (p+1 < end && p[0]=='u' && p[1]=='8') p += 2;
        else { wide_or_unsigned = (*p=='u' || *p=='U'); p++; }
    }

    /* Skip opening quote. */
    if (p < end && *p == '\'') p++;

    /* Decode character(s). */
    int64_t val = 0;
    int char_count = 0;
    while (p < end && *p != '\'') {
        int64_t ch;
        if (*p == '\\') { p++; ch = decode_escape(&p, end); }
        else            { ch = (unsigned char)*p++; }
        /* Multi-char: GCC big-endian packing. */
        val = (val << 8) | (ch & 0xFF);
        char_count++;
    }

    t->is_unsigned = wide_or_unsigned;
    if (wide_or_unsigned) {
        t->u.uval = (uint64_t)val;
    } else {
        t->u.ival = val;
    }

    if (char_count > 1)
        FE_WARNING(diags, t->loc,
            "multi-character character constant '%.*s'", (int)len, text);
}

/* =========================================================================
 * lex_run
 * ====================================================================== */

SharpTok *lex_run(const CppTok *tokens, size_t ntokens,
                  const char *filename, FeDiagArr *diags,
                  size_t *out_count, bool sharp_keywords) {
    (void)filename; /* used only for future sourcemap integration */

    /* Over-allocate: at most ntokens+1 SharpToks (including EOF sentinel). */
    size_t cap  = ntokens + 1;
    SharpTok *out = malloc(cap * sizeof *out);
    if (!out) { perror("sharp-fe lex"); abort(); }
    size_t n = 0;

    for (size_t i = 0; i < ntokens; i++) {
        CppTok ct = tokens[i];

        /* Skip whitespace / newlines / comments — cpp may emit these when
         * keep_whitespace is enabled; we discard them here. */
        if (ct.kind == CPPT_SPACE    ||
            ct.kind == CPPT_NEWLINE  ||
            ct.kind == CPPT_COMMENT  ||
            ct.kind == CPPT_EOF)
            continue;

        /* Placemarkers are internal cpp artefacts; skip. */
        if (ct.kind == CPPT_PLACEMARKER) continue;

        /* GCC's `__extension__` keyword tells gcc not to warn about the
         * non-ISO construct that follows.  Sharp does not emit ISO-only
         * warnings in the first place, so we can simply drop it from
         * the token stream — the parser never has to know it existed.
         * Used by glibc's <stdlib.h>, <math.h> and friends. */
        if (ct.kind == CPPT_IDENT && ct.text && ct.len == 13 &&
            memcmp(ct.text, "__extension__", 13) == 0) {
            continue;
        }

        SharpTok st = {0};
        /* Copy the token text into a fresh heap buffer so that SharpTok
         * is fully independent of the CppResult / CppCtx lifetime.
         * This prevents use-after-free when the CPP's internal out_text
         * StrBuf is reallocated (old absolute text pointers become stale). */
        st.text       = (ct.text && ct.len > 0)
                        ? cpp_xstrndup(ct.text, ct.len)
                        : cpp_xstrndup("", 0);
        st.len        = ct.len;
        st.loc        = ct.loc;
        st.from_macro = ct.from_macro;

        switch (ct.kind) {
        case CPPT_IDENT:
            /* Phase 1: keyword promotion (Phase R1: gated by mode). */
            st.kind = keyword_kind(ct.text, ct.len, sharp_keywords);
            break;

        case CPPT_PUNCT:
            st.kind = punct_kind(ct.text, ct.len);
            break;

        case CPPT_PP_NUMBER:
            if (is_float(ct.text, ct.len)) {
                st.kind = STOK_FLOAT_LIT;
                parse_float_literal(&st, diags);
            } else {
                st.kind = STOK_INT_LIT;
                parse_int_literal(&st, diags);
            }
            break;

        case CPPT_CHAR_CONST:
            st.kind = STOK_CHAR_LIT;
            parse_char_literal(&st, diags);
            break;

        case CPPT_STRING_LIT:
        case CPPT_HEADER_NAME:
            st.kind = STOK_STRING_LIT;
            break;

        case CPPT_AT_INTRINSIC:
            st.kind = STOK_AT_INTRINSIC;
            break;

        case CPPT_OTHER:
        default:
            st.kind = STOK_OTHER;
            break;
        }

        /* ISO C phase-6: adjacent string literal tokens are concatenated.
         * `"foo" "bar"` is one logical literal `"foobar"`.  We merge by
         * looking back at the previous emitted token: if both are
         * STRING_LIT we splice the two contents (stripping inner quotes)
         * and update the existing token, then skip emitting a new one.
         * Macro expansion in cpp can produce such pairs even when the
         * source text has no adjacency, e.g. `LUA_VERSION` →
         * `"Lua " "5" "." "4"`. */
        if (st.kind == STOK_STRING_LIT && n > 0 &&
            out[n-1].kind == STOK_STRING_LIT) {
            SharpTok *prev = &out[n-1];
            /* Each spelled-as token still has its surrounding quotes.
             * Combined form keeps a single pair of outer quotes; merge
             * by stripping prev's trailing `"` and st's leading `"`.
             *
             * Defensive minimum: both texts must be at least 2 chars
             * (the empty string `""` is two chars).  Fall back to no
             * merge if either is malformed. */
            if (prev->len >= 2 && st.len >= 2 &&
                prev->text[prev->len-1] == '"' && st.text[0] == '"') {
                size_t inner_prev = prev->len - 1;   /* drop closing " */
                size_t inner_st   = st.len   - 1;    /* drop opening " */
                size_t total      = inner_prev + inner_st;
                char *combined    = cpp_xmalloc(total + 1);
                memcpy(combined, prev->text, inner_prev);
                memcpy(combined + inner_prev, st.text + 1, inner_st);
                combined[total] = '\0';
                free((char*)prev->text);
                prev->text = combined;
                prev->len  = total;
                free((char*)st.text);
                continue;  /* don't emit `st` — it has been folded in */
            }
        }

        out[n++] = st;
    }

    /* EOF sentinel. */
    SharpTok eof = {0};
    eof.kind = STOK_EOF;
    out[n] = eof;

    *out_count = n;
    return out;
}

/* =========================================================================
 * Supporting functions
 * ====================================================================== */

void lex_free(SharpTok *toks) {
    if (!toks) return;
    /* Free each token's text copy (allocated by cpp_xstrndup in lex_run). */
    for (size_t i = 0; toks[i].kind != STOK_EOF || i == 0; i++) {
        free((char *)toks[i].text);
        if (toks[i].kind == STOK_EOF) break;
    }
    free(toks);
}

bool lex_is_keyword(SharpTokKind k) {
    /* Phase 1: range covers all C11 + Sharp keywords including STOK_IMPORT. */
    return k >= STOK_AUTO && k <= STOK_IMPORT;
}

const char *lex_tok_kind_name(SharpTokKind k) {
    static const char *names[] = {
        "EOF",
        /* C11 keywords */
        "auto","break","case","char","const","continue","default","do",
        "double","else","enum","extern","float","for","goto","if","inline",
        "int","long","register","restrict","return","short","signed","sizeof",
        "static","struct","switch","typedef","union","unsigned","void",
        "volatile","while",
        "_Alignas","_Alignof","_Atomic","_Bool","_Complex","_Generic",
        "_Imaginary","_Noreturn","_Static_assert","_Thread_local",
        /* Sharp keywords */
        "defer","operator","null","this","import","extern_struct",
        /* Identity */
        "IDENT",
        /* Literals */
        "INT_LIT","FLOAT_LIT","CHAR_LIT","STRING_LIT",
        /* Punctuators */
        "{","}","[","]","(",")",
        ";",":",",","?","~","#","##","...",
        ".","->",
        "+","++","+=",
        "-","--","-=",
        "*","*=",
        "/","/=",
        "%","%=",
        "&","&&","&=",
        "|","||","|=",
        "^","^=",
        "!","!=",
        "=","==",
        "<","<<","<=","<<=",
        ">",">>",">=",">>=",
        /* Sharp */
        "AT_INTRINSIC",
        /* Other */
        "OTHER",
    };
    if ((size_t)k >= sizeof names / sizeof names[0]) return "?";
    return names[k];
}

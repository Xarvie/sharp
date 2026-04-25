/* lexer.c - tokenizer for Sharp (2-token lookahead). */
#include "sharp.h"

typedef struct { const char* s; int len; TokKind kind; } Keyword;

static const Keyword kws[] = {
    {"struct",   6, TK_STRUCT},  {"impl",     4, TK_IMPL},
    {"union",    5, TK_UNION},   {"const",    5, TK_CONST},
    {"enum",     4, TK_ENUM},
    {"static",   6, TK_STATIC},  {"return",   6, TK_RETURN},
    {"if",       2, TK_IF},      {"else",     4, TK_ELSE},
    {"while",    5, TK_WHILE},   {"for",      3, TK_FOR},
    {"true",     4, TK_TRUE},    {"false",    5, TK_FALSE},
    {"null",     4, TK_NULL},    {"let",      3, TK_LET},
    {"var",      3, TK_VAR},
    {"unsafe",   6, TK_UNSAFE},  {"break",    5, TK_BREAK},
    {"continue", 8, TK_CONTINUE},{"sizeof",   6, TK_SIZEOF},
    {"as",       2, TK_AS},      {"operator",  8, TK_OPERATOR},
    {"extern",   6, TK_EXTERN},
    {"print",    5, TK_PRINT},
    {"println",  7, TK_PRINTLN},
    {"void",     4, TK_VOID},    {"bool",     4, TK_BOOL},
    {"i8",  2, TK_I8},  {"i16", 3, TK_I16}, {"i32", 3, TK_I32}, {"i64", 3, TK_I64},
    {"u8",  2, TK_U8},  {"u16", 3, TK_U16}, {"u32", 3, TK_U32}, {"u64", 3, TK_U64},
    {"f32", 3, TK_F32}, {"f64", 3, TK_F64},
    {"isize", 5, TK_ISIZE}, {"usize", 5, TK_USIZE},

    /* C-style type keywords */
    {"short",    5, TK_SHORT},
    {"long",     4, TK_LONG},
    {"int",      3, TK_INT_TYPE},
    {"unsigned", 8, TK_UNSIGNED},
    {"signed",   6, TK_SIGNED},
    {"char",     4, TK_CHAR},
    {"float",    5, TK_FLOAT_TYPE},
    {"double",   6, TK_DOUBLE_TYPE},
    {"typedef",  7, TK_TYPEDEF},

    /* C extensions — wchar_t is a typedef name, NOT a keyword.
     * It is handled in parse_type by name matching, not lexing. */
    {"__int64",  7, TK___INT64},
    {"_Bool",    5, TK_BOOL},

    /* C declaration modifiers */
    {"inline",      6, TK___INLINE},
    {"__inline__",  10, TK___INLINE__},
    {"__inline",    8, TK___INLINE},
    {"__extension__", 13, TK___EXTENSION__},
    {"__thread",     8, TK___THREAD},
    {"__attribute__", 13, TK___ATTRIBUTE__},
    {"__declspec",    10, TK___DECLSPEC},
    {"_Static_assert", 14, TK_STATIC_ASSERT},

    /* calling conventions */
    {"__cdecl",     7, TK___CDECL},
    {"__stdcall",   9, TK___STDCALL},
    {"__fastcall", 10, TK___FASTCALL},
    {"__unaligned", 11, TK___UNALIGNED},
    {"__restrict", 10, TK___RESTRICT},
    {"__restrict__", 12, TK___RESTRICT__},
    {"restrict", 8, TK_RESTRICT},
    {"__asm__", 7, TK_ASM},
    {"__asm", 5, TK_ASM},
    {"asm", 3, TK_ASM},
};
static const int nkws = (int)(sizeof(kws) / sizeof(kws[0]));

static Tok lex_read_raw(Lexer* lx);

void lex_init(Lexer* lx, const char* src, const char* filename) {
    lx->src      = src;
    lx->cur      = src;
    lx->line     = 1;
    lx->col      = 1;
    lx->filename = filename;
    lx->peek0    = lex_read_raw(lx);
    lx->peek1    = lex_read_raw(lx);
}

Tok lex_peek(Lexer* lx)  { return lx->peek0; }
Tok lex_peek2(Lexer* lx) { return lx->peek1; }

/* Compare an identifier token's text with a C string. */
bool lex_ident_is(Tok t, const char* name) {
    if (t.kind != TK_IDENT) return false;
    size_t nlen = strlen(name);
    if (t.len != (int)nlen) return false;
    return memcmp(t.start, name, nlen) == 0;
}

/* Peek n tokens ahead (0 = current, 1 = next, etc.).
 * Uses save/restore for lookahead beyond 2 tokens. */
Tok lex_peek_n(Lexer* lx, int n) {
    if (n == 0) return lx->peek0;
    if (n == 1) return lx->peek1;
    LexerState s = lex_save(lx);
    /* Advance n times to get to the nth token ahead */
    for (int i = 0; i < n; i++) lex_next(lx);
    /* lex_peek returns peek0, which is now the nth token from the original position */
    Tok t = lex_peek(lx);
    lex_restore(lx, s);
    return t;
}

Tok lex_next(Lexer* lx) {
    Tok r = lx->peek0;
    lx->peek0 = lx->peek1;
    lx->peek1 = lex_read_raw(lx);
    return r;
}

LexerState lex_save(Lexer* lx) {
    LexerState s;
    s.cur   = lx->cur;
    s.line  = lx->line;
    s.col   = lx->col;
    s.peek0 = lx->peek0;
    s.peek1 = lx->peek1;
    return s;
}

void lex_restore(Lexer* lx, LexerState s) {
    lx->cur   = s.cur;
    lx->line  = s.line;
    lx->col   = s.col;
    lx->peek0 = s.peek0;
    lx->peek1 = s.peek1;
}

/* ---------- internals ---------- */
static int pc(Lexer* lx, int ahead) { return (unsigned char)lx->cur[ahead]; }

static int adv(Lexer* lx) {
    int c = (unsigned char)*lx->cur;
    if (c == 0) return 0;
    lx->cur++;
    if (c == '\n') { lx->line++; lx->col = 1; }
    else           { lx->col++; }
    return c;
}

static void skip_ws(Lexer* lx) {
    for (;;) {
        int c = pc(lx, 0);
        if (c == 0) return;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { adv(lx); continue; }
        if (c == '/' && pc(lx, 1) == '/') {
            while (pc(lx, 0) && pc(lx, 0) != '\n') adv(lx);
            continue;
        }
        if (c == '/' && pc(lx, 1) == '*') {
            adv(lx); adv(lx);
            while (pc(lx, 0)) {
                if (pc(lx, 0) == '*' && pc(lx, 1) == '/') { adv(lx); adv(lx); break; }
                adv(lx);
            }
            continue;
        }
        return;
    }
}

static bool is_id0(int c) { return isalpha(c) || c == '_'; }
static bool is_id(int c)  { return isalnum(c) || c == '_'; }

static Tok read_ident(Lexer* lx) {
    Tok t = {0};
    t.start = lx->cur; t.line = lx->line; t.col = lx->col;
    while (is_id(pc(lx, 0))) adv(lx);
    t.len = (int)(lx->cur - t.start);
    for (int i = 0; i < nkws; i++) {
        if (kws[i].len == t.len && memcmp(kws[i].s, t.start, (size_t)t.len) == 0) {
            t.kind = kws[i].kind;
            return t;
        }
    }
    t.kind = TK_IDENT;
    return t;
}

static Tok read_number(Lexer* lx) {
    Tok t = {0};
    t.start = lx->cur; t.line = lx->line; t.col = lx->col;
    bool is_float = false;

    if (pc(lx, 0) == '0' && (pc(lx, 1) == 'x' || pc(lx, 1) == 'X')) {
        adv(lx); adv(lx);
        while (isxdigit(pc(lx, 0))) adv(lx);
        t.len = (int)(lx->cur - t.start);
        char buf[64]; int n = t.len < 63 ? t.len : 63;
        memcpy(buf, t.start, (size_t)n); buf[n] = 0;
        t.ival = strtoll(buf, NULL, 0);
        t.kind = TK_INT;
        return t;
    }

    while (isdigit(pc(lx, 0))) adv(lx);
    if (pc(lx, 0) == '.' && isdigit(pc(lx, 1))) {
        is_float = true; adv(lx);
        while (isdigit(pc(lx, 0))) adv(lx);
    }
    if (pc(lx, 0) == 'e' || pc(lx, 0) == 'E') {
        is_float = true; adv(lx);
        if (pc(lx, 0) == '+' || pc(lx, 0) == '-') adv(lx);
        while (isdigit(pc(lx, 0))) adv(lx);
    }
    /* allow 1.0f etc. -> just consume */
    if (pc(lx, 0) == 'f' || pc(lx, 0) == 'F') { is_float = true; adv(lx); }

    /* MSVC-style integer suffixes: i8 i16 i32 i64 u8 u16 u32 u64 (and unsigned variants ui8..ui64) */
    if (!is_float) {
        static const struct { const char* s; int len; } int_suffixes[] = {
            {"ui64",4},{"ui32",4},{"ui16",4},{"ui8",3},
            {"i64",3},{"i32",3},{"i16",3},{"i8",2},
            {"u64",3},{"u32",3},{"u16",3},{"u8",2},
        };
        for (int i = 0; i < 12; i++) {
            int sl = int_suffixes[i].len;
            if (strncmp(lx->cur, int_suffixes[i].s, (size_t)sl) == 0) {
                char after = pc(lx, sl);
                if (!is_id(after)) {
                    for (int j = 0; j < sl; j++) adv(lx);
                    break;
                }
            }
        }
    }

    t.len = (int)(lx->cur - t.start);
    char buf[64]; int n = t.len < 63 ? t.len : 63;
    memcpy(buf, t.start, (size_t)n); buf[n] = 0;
    /* strip trailing 'f' */
    if (n > 0 && (buf[n-1] == 'f' || buf[n-1] == 'F')) buf[n-1] = 0;

    if (is_float) { t.fval = strtod(buf, NULL); t.kind = TK_FLOAT; }
    else          { t.ival = strtoll(buf, NULL, 10); t.kind = TK_INT; }
    return t;
}

static Tok read_string(Lexer* lx) {
    Tok t = {0};
    t.line = lx->line; t.col = lx->col;
    adv(lx); /* skip " */
    t.start = lx->cur;
    while (pc(lx, 0) && pc(lx, 0) != '"') {
        if (pc(lx, 0) == '\\' && pc(lx, 1)) { adv(lx); adv(lx); }
        else                                  adv(lx);
    }
    t.len = (int)(lx->cur - t.start);
    if (pc(lx, 0) == '"') adv(lx);
    else error_at(t.line, "unterminated string");
    t.kind = TK_STRING;
    return t;
}

static Tok read_char(Lexer* lx) {
    Tok t = {0};
    t.line = lx->line; t.col = lx->col;
    adv(lx); /* skip ' */
    t.start = lx->cur;
    if (pc(lx, 0) == '\\' && pc(lx, 1)) { adv(lx); adv(lx); }
    else if (pc(lx, 0))                   adv(lx);
    t.len = (int)(lx->cur - t.start);
    if (pc(lx, 0) == '\'') adv(lx);
    else error_at(t.line, "unterminated char literal");
    t.kind = TK_CHAR_LIT;
    return t;
}

static Tok read_punct(Lexer* lx) {
    Tok t = {0};
    t.start = lx->cur; t.line = lx->line; t.col = lx->col;
    int c = adv(lx);
    int c2 = pc(lx, 0);
    TokKind k = TK_EOF;

    switch (c) {
        case '(': k = TK_LPAREN;   break;
        case ')': k = TK_RPAREN;   break;
        case '{': k = TK_LBRACE;   break;
        case '}': k = TK_RBRACE;   break;
        case '[': k = TK_LBRACKET; break;
        case ']': k = TK_RBRACKET; break;
        case ',': k = TK_COMMA;    break;
        case ';': k = TK_SEMI;     break;
        case '~': k = TK_TILDE;    break;
        case '.':
            if (c2 == '.' && pc(lx, 1) == '.') { adv(lx); adv(lx); k = TK_ELLIPSIS; }
            else k = TK_DOT;
            break;
        case '^': k = TK_CARET;    break;
        case ':': if (c2 == ':') { adv(lx); k = TK_DCOLON; } else k = TK_COLON; break;
        case '+':
            if      (c2 == '=') { adv(lx); k = TK_PLUSEQ;   }
            else if (c2 == '+') { adv(lx); k = TK_PLUSPLUS; }
            else                  k = TK_PLUS;
            break;
        case '-':
            if      (c2 == '=') { adv(lx); k = TK_MINUSEQ;    }
            else if (c2 == '>') { adv(lx); k = TK_ARROW;      }
            else if (c2 == '-') { adv(lx); k = TK_MINUSMINUS; }
            else                  k = TK_MINUS;
            break;
        case '*': if (c2 == '=') { adv(lx); k = TK_STAREQ;  } else k = TK_STAR;    break;
        case '/': if (c2 == '=') { adv(lx); k = TK_SLASHEQ; } else k = TK_SLASH;   break;
        case '%': if (c2 == '=') { adv(lx); k = TK_PERCENTEQ;} else k = TK_PERCENT; break;
        case '=':
            if (c2 == '=') { adv(lx); k = TK_EQ; }
            else             k = TK_ASSIGN;
            break;
        case '!': if (c2 == '=') { adv(lx); k = TK_NEQ; } else k = TK_NOT; break;
        case '<':
            if      (c2 == '=') { adv(lx); k = TK_LE;  }
            else if (c2 == '<') { adv(lx); k = TK_SHL; }
            else                  k = TK_LT;
            break;
        case '>':
            if      (c2 == '=') { adv(lx); k = TK_GE;  }
            else if (c2 == '>') { adv(lx); k = TK_SHR; }
            else                  k = TK_GT;
            break;
        case '&': if (c2 == '&') { adv(lx); k = TK_AND_AND; } else k = TK_AMP;  break;
        case '|': if (c2 == '|') { adv(lx); k = TK_OR_OR;   } else k = TK_PIPE; break;
        case '?': k = TK_QUESTION; break;
        default:
            /* c was already consumed by adv(lx) at line 270, so no need to advance again */
            error_at(t.line, "unexpected character '%c' (0x%02x)", c, c);
            k = TK_EOF;
    }
    t.kind = k;
    t.len  = (int)(lx->cur - t.start);
    return t;
}

/* Consume a GCC-style linemarker:  # <line> "<file>" [flags...]
 * These are emitted by the preprocessor (cpp/) to preserve original source
 * locations.  The lexer absorbs them silently and updates lx->line /
 * lx->filename so that all subsequent tokens carry the correct location.
 * Returns true if a linemarker was consumed, false otherwise. */
static bool try_consume_linemarker(Lexer* lx) {
    /* A linemarker starts with '#' at column 1 of a fresh line (col is reset
     * to 1 by adv() on newline, so we check col == 1 before skip_ws).     */
    if (pc(lx, 0) != '#') return false;
    if (pc(lx, 1) == '#') return false;         /* ## token-paste — not a marker */

    /* Peek ahead: skip '#' and any spaces, then expect a digit.            */
    const char *save = lx->cur;
    int save_line = lx->line, save_col = lx->col;
    adv(lx);                              /* consume '#'                    */
    while (pc(lx, 0) == ' ' || pc(lx, 0) == '\t') adv(lx);

    if (!isdigit(pc(lx, 0))) {
        /* Not a linemarker — restore and let normal dispatch handle '#'    */
        lx->cur  = save;
        lx->line = save_line;
        lx->col  = save_col;
        return false;
    }

    /* Parse the line number */
    int new_line = 0;
    while (isdigit(pc(lx, 0))) {
        new_line = new_line * 10 + (pc(lx, 0) - '0');
        adv(lx);
    }

    /* Skip whitespace then parse optional filename string */
    while (pc(lx, 0) == ' ' || pc(lx, 0) == '\t') adv(lx);

    if (pc(lx, 0) == '"') {
        adv(lx);                          /* consume opening '"'            */
        const char *fname_start = lx->cur;
        while (pc(lx, 0) && pc(lx, 0) != '"' && pc(lx, 0) != '\n')
            adv(lx);
        size_t fname_len = (size_t)(lx->cur - fname_start);
        if (pc(lx, 0) == '"') adv(lx);   /* consume closing '"'            */

        /* Intern the filename so it outlives the source buffer             */
        char *fname = (char*)malloc(fname_len + 1);
        memcpy(fname, fname_start, fname_len);
        fname[fname_len] = '\0';
        lx->filename = fname;             /* NOTE: intentional minor leak   */
        /* (In a long-running process use an arena; for a one-shot compiler
         *  this is acceptable — total linemarkers ~ O(files) << O(tokens).) */
    }

    /* The linemarker number is the line of the NEXT source line.  After this
     * function returns, skip_ws will read the trailing '\n' via adv() which
     * increments lx->line.  Pre-decrement so the post-increment lands on the
     * correct value.                                                         */
    lx->line = new_line - 1;
    lx->col  = 1;

    /* Consume the rest of the linemarker line (flags: 1 2 3 4, etc.)        */
    while (pc(lx, 0) && pc(lx, 0) != '\n') adv(lx);
    return true;
}

static Tok lex_read_raw(Lexer* lx) {
    skip_ws(lx);
    /* After skip_ws consumed any preceding newlines, the cursor may land on
     * a preprocessor linemarker (#) at column 1.  Consume it transparently
     * and try again until we get a real token.                              */
    while (lx->col == 1 && pc(lx, 0) == '#' && pc(lx, 1) != '#') {
        if (!try_consume_linemarker(lx)) {
            break; // 不是linemarker，退出循环交给正常token处理逻辑
        }
        skip_ws(lx);
    }
    /* Skip remaining preprocessor directive lines (e.g. #pragma GCC system_header).
     * These are emitted by the preprocessor but are not Sharp source tokens. */
    if (pc(lx, 0) == '#') {
        while (pc(lx, 0) && pc(lx, 0) != '\n') adv(lx);
        return lex_read_raw(lx);
    }
    int c = pc(lx, 0);
    if (c == 0) {
        Tok t = {0};
        t.kind = TK_EOF; t.line = lx->line; t.col = lx->col; t.start = lx->cur;
        return t;
    }
    if (is_id0(c)) return read_ident(lx);
    if (isdigit(c)) return read_number(lx);
    if (c == '"')   return read_string(lx);
    if (c == '\'')  return read_char(lx);
    return read_punct(lx);
}

/* debug helper; only a subset of names defined - enough for diagnostics */
const char* tok_name(TokKind k) {
    switch (k) {
        case TK_EOF:      return "EOF";
        case TK_INT:      return "int-literal";
        case TK_FLOAT:    return "float-literal";
        case TK_STRING:   return "string-literal";
        case TK_CHAR_LIT: return "char-literal";
        case TK_IDENT:    return "identifier";
        case TK_STRUCT:   return "'struct'";
        case TK_IMPL:     return "'impl'";
        case TK_RETURN:   return "'return'";
        case TK_IF:       return "'if'";
        case TK_ELSE:     return "'else'";
        case TK_WHILE:    return "'while'";
        case TK_FOR:      return "'for'";
        case TK_LPAREN:   return "'('";
        case TK_RPAREN:   return "')'";
        case TK_LBRACE:   return "'{'";
        case TK_RBRACE:   return "'}'";
        case TK_SEMI:     return "';'";
        case TK_COMMA:    return "','";
        case TK_ASSIGN:   return "'='";
        default:          return "token";
    }
}

/* util.c - arena allocator, string buffer, errors, type constructors */
#include "sharp.h"

/* ===================================================================== *
 *   Arena
 * ===================================================================== */
#define ARENA_CHUNK (64 * 1024)

static void arena_grow(Arena** a, size_t min_size) {
    size_t cap = ARENA_CHUNK;
    if (cap < min_size) cap = min_size;
    Arena* n = (Arena*)malloc(sizeof(Arena) + cap);
    if (!n) fatal("out of memory");
    n->buf  = (char*)(n + 1);
    n->cap  = cap;
    n->used = 0;
    n->next = *a;
    *a = n;
}

void* arena_alloc(Arena** a, size_t size) {
    size = (size + 7u) & ~(size_t)7u;                 /* align to 8 */
    if (!*a || (*a)->used + size > (*a)->cap)
        arena_grow(a, size);
    void* p = (*a)->buf + (*a)->used;
    (*a)->used += size;
    return p;
}

void* arena_zalloc(Arena** a, size_t size) {
    void* p = arena_alloc(a, size);
    memset(p, 0, size);
    return p;
}

char* arena_strndup(Arena** a, const char* s, int n) {
    char* p = (char*)arena_alloc(a, (size_t)n + 1);
    memcpy(p, s, (size_t)n);
    p[n] = 0;
    return p;
}

void arena_free_all(Arena** a) {
    Arena* cur = *a;
    while (cur) {
        Arena* next = cur->next;
        free(cur);
        cur = next;
    }
    *a = NULL;
}

/* ===================================================================== *
 *   String buffer
 * ===================================================================== */
void sb_init(StrBuf* sb) {
    sb->cap  = 256;
    sb->data = (char*)malloc(sb->cap);
    if (!sb->data) fatal("out of memory");
    sb->len  = 0;
    sb->data[0] = 0;
}

void sb_free(StrBuf* sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len = sb->cap = 0;
}

static void sb_reserve(StrBuf* sb, size_t need) {
    while (sb->len + need + 1 > sb->cap) {
        sb->cap *= 2;
        sb->data = (char*)realloc(sb->data, sb->cap);
        if (!sb->data) fatal("out of memory");
    }
}

void sb_putc(StrBuf* sb, char c) {
    sb_reserve(sb, 1);
    sb->data[sb->len++] = c;
    sb->data[sb->len] = 0;
}

void sb_puts(StrBuf* sb, const char* s) {
    size_t n = strlen(s);
    sb_reserve(sb, n);
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
    sb->data[sb->len] = 0;
}

void sb_putn(StrBuf* sb, const char* s, int n) {
    sb_reserve(sb, (size_t)n);
    memcpy(sb->data + sb->len, s, (size_t)n);
    sb->len += (size_t)n;
    sb->data[sb->len] = 0;
}

void sb_printf(StrBuf* sb, const char* fmt, ...) {
    va_list ap, ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);
    int need = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (need < 0) { va_end(ap); return; }
    sb_reserve(sb, (size_t)need);
    vsnprintf(sb->data + sb->len, sb->cap - sb->len, fmt, ap);
    sb->len += (size_t)need;
    sb->data[sb->len] = 0;
    va_end(ap);
}

/* ===================================================================== *
 *   Errors & diagnostics
 *
 *   The diagnostic engine holds a pointer to the source text registered
 *   by main() so it can print a context line with a caret indicator
 *   under the offending token. Output format:
 *
 *     foo.sp:12:18: error[E3001]: cannot assign 'char*' to 'i32'
 *          12 |     i32 x = "hello";
 *             |                  ^~~~~~~
 *
 *   If the source or filename isn't registered (e.g. internal tests
 *   compiling from a string), we fall back to a single-line form.
 *   g_silent suppresses everything (used by parser speculation).
 * ===================================================================== */
int  g_error_count   = 0;
int  g_warning_count = 0;
bool g_silent        = false;
int  g_error_limit   = 25;

static const char* s_diag_filename = NULL;
static const char* s_diag_source   = NULL;

void diag_set_source(const char* filename, const char* source) {
    s_diag_filename = filename;
    s_diag_source   = source;
}

/* Find the start of the (1-based) line in s_diag_source. Returns NULL
 * if the source isn't registered or the line is out of range. */
static const char* line_start(int line) {
    if (!s_diag_source || line < 1) return NULL;
    const char* p = s_diag_source;
    int cur = 1;
    while (*p && cur < line) {
        if (*p == '\n') cur++;
        p++;
    }
    return (cur == line) ? p : NULL;
}

/* Length of the line at `start` up to but excluding the newline. */
static int line_length(const char* start) {
    int n = 0;
    while (start[n] && start[n] != '\n' && start[n] != '\r') n++;
    return n;
}

static const char* level_name(DiagLevel lvl) {
    switch (lvl) {
        case DIAG_ERROR:   return "error";
        case DIAG_WARNING: return "warning";
        case DIAG_NOTE:    return "note";
    }
    return "error";
}

/* ANSI colour sequences; only emitted if stderr is a TTY. */
static bool use_colour(void) {
    static int cached = -1;
    if (cached == -1) {
        /* Respect NO_COLOR; enable on TTY otherwise. */
        const char* no_colour = getenv("NO_COLOR");
        if (no_colour && *no_colour) cached = 0;
#ifdef _WIN32
        else cached = 0;
#else
        else { extern int isatty(int); extern int fileno(FILE*);
               cached = isatty(fileno(stderr)) ? 1 : 0; }
#endif
    }
    return cached != 0;
}
static const char* col_red(void)   { return use_colour() ? "\x1b[31;1m" : ""; }
static const char* col_yel(void)   { return use_colour() ? "\x1b[33;1m" : ""; }
static const char* col_blue(void)  { return use_colour() ? "\x1b[34;1m" : ""; }
static const char* col_bold(void)  { return use_colour() ? "\x1b[1m"    : ""; }
static const char* col_reset(void) { return use_colour() ? "\x1b[0m"    : ""; }

static const char* level_colour(DiagLevel lvl) {
    switch (lvl) {
        case DIAG_ERROR:   return col_red();
        case DIAG_WARNING: return col_yel();
        case DIAG_NOTE:    return col_blue();
    }
    return "";
}

void diag_emit(DiagLevel level, DiagCode code,
               int line, int col, int span,
               const char* fmt, ...) {
    if (g_silent) return;

    /* Header: filename:line:col: <level>[Ecode]: message */
    if (s_diag_filename) fprintf(stderr, "%s%s:", col_bold(), s_diag_filename);
    else                  fprintf(stderr, "%s<input>:", col_bold());
    if (line > 0) {
        fprintf(stderr, "%d:", line);
        if (col > 0) fprintf(stderr, "%d:", col);
    }
    fprintf(stderr, "%s ", col_reset());

    fprintf(stderr, "%s%s", level_colour(level), level_name(level));
    if (code != E_NONE) fprintf(stderr, "[E%04d]", (int)code);
    fprintf(stderr, ":%s ", col_reset());

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);

    /* Source context + caret. */
    const char* ls = line_start(line);
    if (ls) {
        int llen = line_length(ls);
        fprintf(stderr, "%s%6d |%s ", col_blue(), line, col_reset());
        fwrite(ls, 1, (size_t)llen, stderr);
        fputc('\n', stderr);
        fprintf(stderr, "%s       |%s ", col_blue(), col_reset());
        if (col > 0) {
            for (int i = 1; i < col; i++) {
                /* Preserve tab alignment. */
                fputc((i - 1 < llen && ls[i - 1] == '\t') ? '\t' : ' ', stderr);
            }
            fprintf(stderr, "%s^", level_colour(level));
            int tilde = span > 1 ? span - 1 : 0;
            for (int i = 0; i < tilde; i++) fputc('~', stderr);
            fprintf(stderr, "%s", col_reset());
        }
        fputc('\n', stderr);
    }

    if (level == DIAG_ERROR)   g_error_count++;
    if (level == DIAG_WARNING) g_warning_count++;

    if (g_error_count >= g_error_limit) {
        fprintf(stderr, "%serror%s: too many errors (%d); aborting\n",
                col_red(), col_reset(), g_error_count);
        exit(1);
    }
}

/* Legacy entrypoint — kept so existing callers still work during the
 * transition. Emits with E_PARSE_GENERIC and no column information. */
void error_at(int line, const char* fmt, ...) {
    if (g_silent) return;
    va_list ap;
    va_start(ap, fmt);
    /* Render message into a buffer, then forward so formatting is uniform. */
    char buf[1024];
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    diag_emit(DIAG_ERROR, E_PARSE_GENERIC, line, 0, 0, "%s", buf);
}

void fatal(const char* fmt, ...) {
    fprintf(stderr, "%sfatal%s: ", col_red(), col_reset());
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

/* ===================================================================== *
 *   Type constructors — thin shims over the interning types module.
 *
 *   These signatures are kept so legacy call sites that thread `Arena**`
 *   around keep compiling unchanged. The Arena* argument is IGNORED: the
 *   types module owns its own arena and returns interned Type*. Callers
 *   that use these shims get the same interning benefits as callers that
 *   go through types.h directly.
 * ===================================================================== */
#include "types.h"

Type* type_prim(Arena** a, TypeKind k)                       { (void)a; return ty_prim(k); }
Type* type_ptr (Arena** a, Type* base)                       { (void)a; return ty_ptr(base); }
Type* type_named(Arena** a, const char* name)                { (void)a; return ty_named(name); }
Type* type_named_generic(Arena** a, const char* name,
                         Type** targs, int ntargs)           { (void)a; return ty_generic(name, targs, ntargs); }
Type* type_const(Arena** a, Type* base)                      { (void)a; return ty_const(base); }
Type* type_func(Arena** a, Type* ret, Type** params, int nparams, bool variadic) {
    (void)a; return ty_func(ret, params, nparams, variadic);
}
Type* type_bitfield(Arena** a, Type* base, int width)        { (void)a; return ty_bitfield(base, width); }
Type* type_array(Arena** a, Type* elem, int size)            { (void)a; return ty_array(elem, size); }

bool type_is_primitive(TypeKind k) {
    return (k >= TY_VOID && k <= TY_USIZE);
}

bool type_is_pointerlike(Type* t) {
    return ty_is_pointer_like(t);
}

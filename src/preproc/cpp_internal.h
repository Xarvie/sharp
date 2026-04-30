/*
 * cpp_internal.h — Internal types and helpers shared across cpp sub-modules.
 */
#ifndef SHARP_CPP_INTERNAL_H
#define SHARP_CPP_INTERNAL_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpp.h"

/* =========================================================================
 * Named dynamic-array type for CppDiag (cross-module function signatures).
 * DA(T) produces anonymous structs — incompatible across TUs in signatures.
 * ====================================================================== */
typedef struct { CppDiag *data; size_t len, cap; } CppDiagArr;

static inline void diag_push(CppDiagArr *a, CppDiag d) {
    if (a->len == a->cap) {
        size_t nc = a->cap ? a->cap * 2 : 8;
        a->data = realloc(a->data, nc * sizeof *a->data);
        if (!a->data) { perror("cpp"); abort(); }
        a->cap = nc;
    }
    a->data[a->len++] = d;
}

/* =========================================================================
 * Generic dynamic array  — LOCAL VARIABLES ONLY, never in function params.
 * ====================================================================== */
#define DA(T)  struct { T *data; size_t len, cap; }

#define da_push(da, val) do {                                              \
    if ((da)->len == (da)->cap) {                                          \
        size_t _nc = (da)->cap ? (da)->cap * 2 : 8u;                      \
        (da)->data = realloc((da)->data, _nc * sizeof(*(da)->data));       \
        if (!(da)->data) { perror("cpp"); abort(); }                       \
        (da)->cap = _nc;                                                   \
    }                                                                      \
    (da)->data[(da)->len++] = (val);                                       \
} while (0)

#define da_free(da) \
    do { free((da)->data); (da)->data=NULL; (da)->len=(da)->cap=0; } while(0)

/* =========================================================================
 * String array — named type for path lists stored in CppCtx / CppState
 * ====================================================================== */
typedef struct { char **data; size_t len, cap; } StrArr;

static inline void strarr_push(StrArr *a, char *s) {
    if (a->len == a->cap) {
        size_t nc = a->cap ? a->cap * 2 : 8;
        a->data = realloc(a->data, nc * sizeof *a->data);
        if (!a->data) { perror("cpp"); abort(); }
        a->cap = nc;
    }
    a->data[a->len++] = s;
}

static inline void strarr_free_contents(StrArr *a) {
    for (size_t i = 0; i < a->len; i++) free(a->data[i]);
    free(a->data);
    a->data = NULL; a->len = a->cap = 0;
}

/* =========================================================================
 * Allocators
 * ====================================================================== */
static inline void *cpp_xmalloc(size_t n) {
    void *p = malloc(n ? n : 1);
    if (!p) { perror("cpp: malloc"); abort(); }
    return p;
}
static inline void *cpp_xrealloc(void *p, size_t n) {
    void *q = realloc(p, n ? n : 1);
    if (!q) { perror("cpp: realloc"); abort(); }
    return q;
}
static inline char *cpp_xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *d = (char *)cpp_xmalloc(n);
    memcpy(d, s, n);
    return d;
}
static inline char *cpp_xstrndup(const char *s, size_t len) {
    char *d = (char *)cpp_xmalloc(len + 1);
    memcpy(d, s, len);
    d[len] = '\0';
    return d;
}

/* =========================================================================
 * String builder
 * ====================================================================== */
typedef struct { char *buf; size_t len, cap; } StrBuf;

static inline void sb_grow(StrBuf *sb, size_t extra) {
    if (sb->len + extra + 1 <= sb->cap) return;
    size_t nc = sb->cap ? sb->cap * 2 : 256;
    while (nc <= sb->len + extra) nc *= 2;
    sb->buf = (char *)cpp_xrealloc(sb->buf, nc);
    sb->cap = nc;
}
static inline void sb_push_ch(StrBuf *sb, char c) {
    sb_grow(sb, 1);
    sb->buf[sb->len++] = c;
    sb->buf[sb->len]   = '\0';
}
static inline void sb_push(StrBuf *sb, const char *s, size_t n) {
    if (!n || !s) return;
    sb_grow(sb, n);
    memcpy(sb->buf + sb->len, s, n);
    sb->len += n;
    sb->buf[sb->len] = '\0';
}
static inline void sb_push_cstr(StrBuf *sb, const char *s) {
    if (s) sb_push(sb, s, strlen(s));
}
static inline void sb_printf(StrBuf *sb, const char *fmt, ...) {
    char tmp[4096];
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);
    if (n > 0) sb_push(sb, tmp, (size_t)n < sizeof tmp ? (size_t)n : sizeof tmp - 1);
}
static inline void sb_free(StrBuf *sb) {
    free(sb->buf); sb->buf = NULL; sb->len = sb->cap = 0;
}
static inline char *sb_take(StrBuf *sb) {
    char *p = sb->buf ? sb->buf : cpp_xstrdup("");
    sb->buf = NULL; sb->len = sb->cap = 0;
    return p;
}

/* =========================================================================
 * String intern table
 * ====================================================================== */
typedef struct InternEntry { char *str; struct InternEntry *next; } InternEntry;
#define INTERN_BUCKETS 1024
typedef struct { InternEntry *buckets[INTERN_BUCKETS]; } InternTable;

static inline uint32_t intern_hash(const char *s, size_t len) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; i++) h = (h ^ (uint8_t)s[i]) * 16777619u;
    return h;
}
static inline const char *intern_get(InternTable *t, const char *s, size_t len) {
    uint32_t h = intern_hash(s, len) & (INTERN_BUCKETS - 1);
    for (InternEntry *e = t->buckets[h]; e; e = e->next)
        if (strlen(e->str) == len && memcmp(e->str, s, len) == 0) return e->str;
    InternEntry *e = (InternEntry *)cpp_xmalloc(sizeof *e);
    e->str = cpp_xstrndup(s, len);
    e->next = t->buckets[h];
    t->buckets[h] = e;
    return e->str;
}
static inline const char *intern_cstr(InternTable *t, const char *s) {
    return intern_get(t, s, strlen(s));
}
static inline void intern_free(InternTable *t) {
    for (int i = 0; i < INTERN_BUCKETS; i++) {
        InternEntry *e = t->buckets[i];
        while (e) { InternEntry *nx = e->next; free(e->str); free(e); e = nx; }
        t->buckets[i] = NULL;
    }
}

/* =========================================================================
 * Preprocessing token (internal, mutable spelling buffer)
 * ====================================================================== */
typedef struct PPTok {
    CppTokKind kind;
    StrBuf     spell;
    CppLoc     loc;
    bool       from_macro;
    bool       hide;              /* blue-painted: suppress re-expansion   */
    bool       at_bol;
    bool       has_leading_space;
} PPTok;

static inline void       pptok_free(PPTok *t)         { sb_free(&t->spell); }
static inline const char *pptok_spell(const PPTok *t)  { return t->spell.buf ? t->spell.buf : ""; }
static inline PPTok pptok_copy(const PPTok *src) {
    PPTok d = *src; d.spell = (StrBuf){0};
    sb_push_cstr(&d.spell, pptok_spell(src));
    return d;
}

/* =========================================================================
 * Token list (singly-linked)
 * ====================================================================== */
typedef struct TokNode { PPTok tok; struct TokNode *next; } TokNode;
typedef struct { TokNode *head; TokNode *tail; size_t len; } TokList;

static inline void tl_append(TokList *l, PPTok tok) {
    TokNode *n = (TokNode *)cpp_xmalloc(sizeof *n);
    n->tok = tok; n->next = NULL;
    if (l->tail) l->tail->next = n; else l->head = n;
    l->tail = n; l->len++;
}
static inline void tl_append_copy(TokList *l, const PPTok *src) { tl_append(l, pptok_copy(src)); }
static inline void tl_free(TokList *l) {
    TokNode *n = l->head;
    while (n) { TokNode *nx = n->next; pptok_free(&n->tok); free(n); n = nx; }
    l->head = l->tail = NULL; l->len = 0;
}

/* =========================================================================
 * Forward declarations
 * ====================================================================== */
typedef struct MacroTable MacroTable;
typedef struct CppReader  CppReader;

/* =========================================================================
 * CppCtx — concrete definition (opaque in cpp.h)
 * ====================================================================== */
struct CppCtx {
    StrArr user_include_paths;
    StrArr sys_include_paths;
    /* Ordered list of command-line -D / -U ops: "D:NAME=val" or "U:NAME" */
    StrArr cmdline_ops;
    /* Legacy separate arrays kept for API compat — ops is the source of truth */
    StrArr cmdline_defines;    /* "NAME=value" */
    StrArr cmdline_undefines;  /* "NAME"        */

    bool keep_whitespace;
    bool emit_linemarkers;
    bool keep_comments;
    int  max_include_depth;

    CppPragmaHandler pragma_handler;
    void            *pragma_userdata;

    InternTable interns;
};

/* =========================================================================
 * CppState — the main preprocessor state machine (defined in directive.c)
 * ====================================================================== */

/* Include-guard tracking */
typedef struct GuardEntry {
    const char       *filename; /* interned */
    const char       *macro;    /* interned macro name, or NULL if not guarded */
    struct GuardEntry *next;
} GuardEntry;

#define GUARD_BUCKETS 256

/* Conditional stack entry */
typedef struct {
    bool in_true_branch;    /* we are currently inside the live branch */
    bool ever_true;         /* some branch was true (suppress further elif/else) */
    bool has_else;          /* #else seen */
    CppLoc open_loc;        /* location of the opening #if for error messages */
} CondFrame;

#define COND_STACK_MAX 256

struct CppState {
    CppCtx       *ctx;       /* owning context — needed for ordered cmdline apply */
    MacroTable   *macros;
    InternTable  *interns;
    CppDiagArr *diags;
    CppLang       lang;

    /* Options (from CppCtx) */
    bool emit_linemarkers;
    bool keep_comments;
    int  max_include_depth;

    /* Search paths */
    StrArr       user_paths;
    StrArr       sys_paths;

    /* Output */
    StrBuf       out_text;      /* accumulated preprocessed text */
    DA(CppTok)   out_tokens;    /* token array */

    /* Conditional stack */
    CondFrame    cond_stack[COND_STACK_MAX];
    int          cond_depth;

    /* Include stack */
    int          include_depth;

    /* Include guard map */
    GuardEntry  *guards[GUARD_BUCKETS];

    /* Pragma callback */
    CppPragmaHandler pragma_cb;
    void            *pragma_ud;

    /* Error flag */
    bool         fatal;

    /* __COUNTER__ value — incremented each time the macro is expanded */
    int          counter;

    /* Lazy linemarker state ------------------------------------------------
     * A marker is "pending" after each directive so that consecutive directives
     * don't flood the output with redundant markers.  The pending marker fires
     * only when actual (non-whitespace) content is about to be emitted, and
     * only if the expected output line doesn't already match the source line.
     * -------------------------------------------------------------------- */
    bool         pending_lm;          /* a marker is queued                  */
    int          pending_lm_line;     /* source line of pending marker        */
    const char  *pending_lm_file;     /* source file of pending marker        */

    int          last_lm_src_line;    /* source line declared in last marker  */
    const char  *last_lm_src_file;   /* source file declared in last marker  */
    int          out_newlines;        /* '\n' emitted to out_text since last
                                       * marker (used for redundancy check)   */
};

#endif /* SHARP_CPP_INTERNAL_H */

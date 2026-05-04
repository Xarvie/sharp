#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <stdlib.h>    /* realpath, malloc, free — must come before other headers */
#include <stdio.h>
#include <string.h>
#include <libgen.h>    /* dirname */
#include <limits.h>    /* PATH_MAX */
#include <unistd.h>    /* access, R_OK */
/*
 * import.c — Sharp import system implementation.
 *
 * Algorithm:
 *   import_resolve(ctx, file, file_path, diags):
 *     resolve_file(ctx, file, canon_of(file_path)):
 *       push canon onto ctx->stack
 *       for each AST_IMPORT node in file->decls (in-place splice walk):
 *         target = resolve_path(ctx, import_path, file->path)
 *         if target in stack → cycle error
 *         if target in done  → dedup (skip)
 *         else: parse target, resolve_file recursively, splice decls
 *       pop from stack; mark done
 */
#include "import.h"
#include "lex.h"
#include "ast.h"      /* Phase S5: astvec_free for splice_one leak fix */
#include "cpp.h"
#include "sharp_internal.h"

/* =========================================================================
 * ImportCtx
 * ======================================================================== */
struct ImportCtx {
    char  **search_paths; size_t nsearch, search_cap;
    char  **done;         size_t ndone,   done_cap;
    char  **stack;        size_t nstack,  stack_cap;
};

ImportCtx *import_ctx_new(void) {
    ImportCtx *ctx = calloc(1, sizeof *ctx);
    if (!ctx) abort();
    return ctx;
}
void import_ctx_free(ImportCtx *ctx) {
    if (!ctx) return;
    for (size_t i=0; i<ctx->nsearch; i++) free(ctx->search_paths[i]);
    for (size_t i=0; i<ctx->ndone;   i++) free(ctx->done[i]);
    for (size_t i=0; i<ctx->nstack;  i++) free(ctx->stack[i]);
    free(ctx->search_paths); free(ctx->done); free(ctx->stack);
    free(ctx);
}
void import_ctx_add_search_path(ImportCtx *ctx, const char *dir) {
    if (ctx->nsearch == ctx->search_cap) {
        ctx->search_cap = ctx->search_cap ? ctx->search_cap*2 : 4;
        ctx->search_paths = realloc(ctx->search_paths,
            ctx->search_cap * sizeof *ctx->search_paths);
        if (!ctx->search_paths) abort();
    }
    ctx->search_paths[ctx->nsearch++] = cpp_xstrdup(dir);
}

/* =========================================================================
 * Helpers
 * ======================================================================== */
static bool str_in(char **arr, size_t n, const char *s) {
    for (size_t i=0; i<n; i++)
        if (arr[i] && strcmp(arr[i], s)==0) return true;
    return false;
}
static void str_push(char ***arr, size_t *n, size_t *cap, const char *s) {
    if (*n == *cap) {
        *cap = *cap ? *cap*2 : 8;
        *arr = realloc(*arr, *cap * sizeof **arr);
        if (!*arr) abort();
    }
    (*arr)[(*n)++] = cpp_xstrdup(s);
}
static void str_pop_free(char **arr, size_t *n) {
    if (*n == 0) return;
    (*n)--;
    free(arr[*n]);
    arr[*n] = NULL;
}

/* -------------------------------------------------------------------------
 * Canonicalise a path safely (realpath for existing files, strdup fallback).
 * Returns heap-allocated string; caller owns.
 * ------------------------------------------------------------------------- */
static char *canonicalise(const char *path) {
    /* Call realpath once and store result. */
    char *r = realpath(path, NULL);
    if (r) return r;
    return cpp_xstrdup(path);
}

/* Resolve import_path relative to from_file's directory, then search paths.
 * Returns heap-allocated canonical path, or NULL if not found. */
static char *resolve_path(ImportCtx *ctx, const char *import_path,
                           const char *from_file) {
    char buf[PATH_MAX];

    /* 1. Relative to the importing file's directory. */
    if (from_file && from_file[0]) {
        char tmp[PATH_MAX];
        strncpy(tmp, from_file, PATH_MAX-1); tmp[PATH_MAX-1]='\0';
        char *dir = dirname(tmp);   /* modifies tmp; returns ptr into tmp */
        snprintf(buf, sizeof buf, "%s/%s", dir, import_path);
        if (access(buf, R_OK) == 0)
            return canonicalise(buf);
    }

    /* 2. Search paths. */
    for (size_t i=0; i<ctx->nsearch; i++) {
        snprintf(buf, sizeof buf, "%s/%s", ctx->search_paths[i], import_path);
        if (access(buf, R_OK) == 0)
            return canonicalise(buf);
    }

    return NULL;
}

/* =========================================================================
 * Read and parse a .sp source file from disk.
 * ======================================================================== */
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f); rewind(f);
    if (sz < 0) { fclose(f); return NULL; }
    char *buf = malloc((size_t)sz+1);
    if (!buf) { fclose(f); return NULL; }
    size_t nread = fread(buf, 1, (size_t)sz, f);
    buf[nread] = '\0'; fclose(f);
    return buf;
}

static AstNode *parse_sp_file(const char *path, FeDiagArr *diags) {
    char *src = read_file(path);
    if (!src) return NULL;
    CppCtx *ctx = cpp_ctx_new(); cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, src, strlen(src), path, CPP_LANG_SHARP);
    FeDiagArr ld={0}; size_t n=0;
    /* Phase R1: parse_sp_file is only called for `.sp` paths; always Sharp mode. */
    SharpTok *toks = lex_run(r.tokens, r.ntokens, path, &ld, &n, true);
    cpp_ctx_free(ctx); free(src);
    AstNode *ast = parse_file(toks, n, path, diags);
    lex_free(toks); cpp_result_free(&r);
    for (size_t i=0;i<ld.len;i++) free(ld.data[i].msg);
    free(ld.data);
    return ast;
}

/* =========================================================================
 * Core: resolve_file / splice_one
 * ======================================================================== */
static void resolve_file(ImportCtx *ctx, AstNode *file,
                          const char *canon_path, FeDiagArr *diags);

/* Process one AST_IMPORT node at decls[idx].
 * Returns the number of decls spliced in (>0), or 0 on skip/error.
 * If 0 is returned, decls[idx] has been set to NULL (caller compacts). */
static size_t splice_one(ImportCtx *ctx, AstNode *file, size_t idx,
                          FeDiagArr *diags) {
    AstNode *imp_node = file->u.file.decls.data[idx];
    const char *import_path = imp_node->u.import.path;

    char *canon = resolve_path(ctx, import_path, file->u.file.path);
    if (!canon) {
        FE_ERROR(diags, imp_node->loc,
            "import: file not found: '%s'", import_path);
        ast_node_free(imp_node);
        file->u.file.decls.data[idx] = NULL;
        return 0;
    }

    /* Cycle? (target is currently being processed) */
    if (str_in(ctx->stack, ctx->nstack, canon)) {
        FE_ERROR(diags, imp_node->loc,
            "import cycle detected involving '%s'", canon);
        ast_node_free(imp_node);
        file->u.file.decls.data[idx] = NULL;
        free(canon); return 0;
    }

    /* Dedup: already fully processed. */
    if (str_in(ctx->done, ctx->ndone, canon)) {
        ast_node_free(imp_node);
        file->u.file.decls.data[idx] = NULL;
        free(canon); return 0;
    }

    /* Parse + recursively resolve. */
    FeDiagArr sub_diags = {0};
    AstNode *imported = parse_sp_file(canon, &sub_diags);
    /* Propagate sub-file diagnostics. */
    for (size_t i=0; i<sub_diags.len; i++) {
        FE_ERROR(diags, sub_diags.data[i].loc, "%s", sub_diags.data[i].msg);
        free(sub_diags.data[i].msg);
    }
    free(sub_diags.data);
    if (!imported) {
        FE_ERROR(diags, imp_node->loc,
            "import: failed to read '%s'", canon);
        ast_node_free(imp_node);
        file->u.file.decls.data[idx] = NULL;
        free(canon); return 0;
    }

    /* Recursively resolve: pushes canon, processes, pops, marks done. */
    resolve_file(ctx, imported, canon, diags);
    free(canon);   /* safe: resolve_file copied canon into stack/done */

    /* Splice imported->decls into file->decls, replacing node at idx. */
    size_t ic = imported->u.file.decls.len;
    ast_node_free(imp_node);

    if (ic == 0) {
        file->u.file.decls.data[idx] = NULL;
        ast_node_free(imported);
        return 0;
    }

    size_t old_len = file->u.file.decls.len;
    size_t new_len = old_len - 1 + ic;
    AstNode **nd = malloc(new_len * sizeof *nd);
    if (!nd) abort();
    /* [0..idx-1]  |  imported decls  |  [idx+1..old_len-1] */
    for (size_t i=0;   i<idx;     i++) nd[i]       = file->u.file.decls.data[i];
    for (size_t i=0;   i<ic;      i++) nd[idx+i]   = imported->u.file.decls.data[i];
    for (size_t i=idx+1; i<old_len; i++) nd[ic+i-1] = file->u.file.decls.data[i];

    free(file->u.file.decls.data);
    file->u.file.decls.data = nd;
    file->u.file.decls.len  = new_len;
    file->u.file.decls.cap  = new_len;

    /* Zero imported's decls so ast_node_free won't double-free.
     * Phase S5: child pointers moved into nd[], but the backing array
     * itself is still owned by imported and must be freed.  Use
     * astvec_free (frees data + zeroes fields) rather than direct
     * field-zeroing — the previous code leaked the realloc'd buffer. */
    astvec_free(&imported->u.file.decls);
    ast_node_free(imported);
    return ic;
}

static void resolve_file(ImportCtx *ctx, AstNode *file,
                          const char *canon_path, FeDiagArr *diags) {
    if (!file || file->kind != AST_FILE) return;
    str_push(&ctx->stack, &ctx->nstack, &ctx->stack_cap, canon_path);

    size_t i = 0;
    while (i < file->u.file.decls.len) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) {
            /* Compact NULL placeholder. */
            size_t rest = file->u.file.decls.len - i - 1;
            if (rest > 0)
                memmove(&file->u.file.decls.data[i],
                        &file->u.file.decls.data[i+1],
                        rest * sizeof *file->u.file.decls.data);
            file->u.file.decls.len--;
            continue;
        }
        if (d->kind == AST_IMPORT) {
            size_t n = splice_one(ctx, file, i, diags);
            if (i < file->u.file.decls.len &&
                file->u.file.decls.data[i] == NULL) {
                /* NULL placeholder: compact and re-examine position i. */
                size_t rest = file->u.file.decls.len - i - 1;
                if (rest > 0)
                    memmove(&file->u.file.decls.data[i],
                            &file->u.file.decls.data[i+1],
                            rest * sizeof *file->u.file.decls.data);
                file->u.file.decls.len--;
                /* Don't increment i; re-examine same position. */
            } else {
                i += (n > 0) ? n : 1;
            }
        } else {
            i++;
        }
    }

    str_pop_free(ctx->stack, &ctx->nstack);
    str_push(&ctx->done, &ctx->ndone, &ctx->done_cap, canon_path);
}

/* =========================================================================
 * Public API
 * ======================================================================== */
void import_resolve(ImportCtx *ctx, AstNode *file,
                    const char *file_path, FeDiagArr *diags) {
    if (!ctx || !file || file->kind != AST_FILE) return;

    /* Canonicalise the root file path (call realpath once). */
    char *canon = canonicalise(file_path ? file_path : "<stdin>");

    /* Ensure file->u.file.path is set for relative imports. */
    if (!file->u.file.path)
        file->u.file.path = cpp_xstrdup(file_path ? file_path : "");

    resolve_file(ctx, file, canon, diags);
    free(canon);
}

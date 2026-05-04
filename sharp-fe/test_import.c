#define _GNU_SOURCE
/*
 * test_import.c — Phase: import system tests.
 *
 *   §1  Path resolution (relative + search path)
 *   §2  Single import: decls merged into caller
 *   §3  Multiple imports from one file
 *   §4  Import deduplication (same file imported twice = no-op)
 *   §5  Transitive imports (A imports B, B imports C)
 *   §6  Cycle detection (A imports B, B imports A → error)
 *   §7  Import + sema: cross-file type references work
 *   §8  Error cases: file not found
 */

#include "import.h"
#include "sema.h"
#include "parse.h"
#include "lex.h"
#include "cpp.h"
#include "sharp_internal.h"
#include "test_helpers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int g_pass = 0, g_fail = 0;

/* ── test directory setup ─────────────────────────────────────────────── */

/* Write a file to disk (temp dir). */
static void write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) { perror(path); return; }
    fputs(content, f);
    fclose(f);
}

/* Parse a Sharp source, resolve imports, return AST (caller owns). */
typedef struct {
    AstNode  *ast;
    FeDiagArr ld, pd, impd;
    SharpTok *toks;
    CppResult cpp;
} Parsed;

static Parsed parse_with_imports(const char *src, const char *src_path,
                                  ImportCtx *ictx) {
    Parsed p = {0};
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    p.cpp  = cpp_run_buf(ctx, src, strlen(src), src_path, CPP_LANG_SHARP);
    size_t n = 0;
    p.toks = lex_run(p.cpp.tokens, p.cpp.ntokens, src_path, &p.ld, &n, true);
    cpp_ctx_free(ctx);
    p.ast  = parse_file(p.toks, n, src_path, &p.pd);
    import_resolve(ictx, p.ast, src_path, &p.impd);
    return p;
}

static void parsed_free(Parsed *p) {
    ast_node_free(p->ast);
    lex_free(p->toks);
    cpp_result_free(&p->cpp);
    for (size_t i=0;i<p->ld.len;i++) free(p->ld.data[i].msg);
    for (size_t i=0;i<p->pd.len;i++) free(p->pd.data[i].msg);
    for (size_t i=0;i<p->impd.len;i++) free(p->impd.data[i].msg);
    free(p->ld.data); free(p->pd.data); free(p->impd.data);
    *p = (Parsed){0};
}

/* Count declarations of a given name in a file AST. */
static size_t count_decl(const AstNode *file, const char *name) {
    size_t n = 0;
    if (!file) return 0;
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        const char *dn = NULL;
        if      (d->kind == AST_FUNC_DEF)    dn = d->u.func_def.name;
        else if (d->kind == AST_STRUCT_DEF)   dn = d->u.struct_def.name;
        else if (d->kind == AST_VAR_DECL)     dn = d->u.var_decl.name;
        if (dn && strcmp(dn, name) == 0) n++;
    }
    return n;
}

/* =========================================================================
 * §1  Path resolution
 * ====================================================================== */
static void test_path_resolution(void) {
    printf("[1] Path resolution\n");

    /* Simple: write a file and import it by relative path */
    char tmpdir[64];
    snprintf(tmpdir, sizeof tmpdir, "/tmp/sharp_import_XXXXXX");
    if (!mkdtemp(tmpdir)) { perror("mkdtemp"); return; }

    char libpath[128], mainpath[128];
    snprintf(libpath,  sizeof libpath,  "%s/lib.sp",  tmpdir);
    snprintf(mainpath, sizeof mainpath, "%s/main.sp", tmpdir);

    write_file(libpath, "int helper() { return 1; }\n");

    char mainsrc[256];
    snprintf(mainsrc, sizeof mainsrc, "import \"lib.sp\";\nint main() { return 0; }\n");

    ImportCtx *ictx = import_ctx_new();
    Parsed p = parse_with_imports(mainsrc, mainpath, ictx);

    CHECK(p.impd.len == 0, "relative import: no errors");
    CHECK(count_decl(p.ast, "helper") == 1, "helper decl merged");
    CHECK(count_decl(p.ast, "main")   == 1, "main decl present");

    import_ctx_free(ictx);
    parsed_free(&p);

    /* Search path: add tmpdir, import without path prefix */
    ImportCtx *ictx2 = import_ctx_new();
    import_ctx_add_search_path(ictx2, tmpdir);

    char mainsrc2[256];
    snprintf(mainsrc2, sizeof mainsrc2,
             "import \"lib.sp\";\nint main2() { return 0; }\n");

    Parsed p2 = parse_with_imports(mainsrc2, "/nowhere/main.sp", ictx2);
    CHECK(p2.impd.len == 0, "search path import: no errors");
    CHECK(count_decl(p2.ast, "helper") == 1, "helper via search path");

    import_ctx_free(ictx2);
    parsed_free(&p2);
}

/* =========================================================================
 * §2  Single import: decls merged
 * ====================================================================== */
static void test_single_import(void) {
    printf("[2] Single import: decls merged\n");

    char tmpdir[64];
    snprintf(tmpdir, sizeof tmpdir, "/tmp/sharp_import_XXXXXX");
    if (!mkdtemp(tmpdir)) { perror("mkdtemp"); return; }

    char libpath[128], mainpath[128];
    snprintf(libpath,  sizeof libpath,  "%s/math.sp",  tmpdir);
    snprintf(mainpath, sizeof mainpath, "%s/prog.sp",  tmpdir);

    write_file(libpath,
        "struct Vec2 { float x; float y; }\n"
        "float add(float a, float b) { return a + b; }\n");

    ImportCtx *ictx = import_ctx_new();
    Parsed p = parse_with_imports(
        "import \"math.sp\";\nvoid use() { Vec2 v; }\n",
        mainpath, ictx);

    CHECK(p.impd.len == 0, "single import: no errors");
    CHECK(count_decl(p.ast, "Vec2") == 1, "Vec2 imported");
    CHECK(count_decl(p.ast, "add")  == 1, "add imported");
    CHECK(count_decl(p.ast, "use")  == 1, "use present");

    /* Imported decls come BEFORE local decls */
    size_t vec2_pos = 0, use_pos = 0;
    for (size_t i = 0; i < p.ast->u.file.decls.len; i++) {
        AstNode *d = p.ast->u.file.decls.data[i];
        if (!d) continue;
        if (d->kind == AST_STRUCT_DEF &&
            strcmp(d->u.struct_def.name, "Vec2") == 0) vec2_pos = i;
        if (d->kind == AST_FUNC_DEF &&
            strcmp(d->u.func_def.name, "use") == 0) use_pos = i;
    }
    CHECK(vec2_pos < use_pos, "imported decls before local decls");

    import_ctx_free(ictx);
    parsed_free(&p);
}

/* =========================================================================
 * §3  Multiple imports
 * ====================================================================== */
static void test_multi_import(void) {
    printf("[3] Multiple imports\n");

    char tmpdir[64];
    snprintf(tmpdir, sizeof tmpdir, "/tmp/sharp_import_XXXXXX");
    if (!mkdtemp(tmpdir)) { perror("mkdtemp"); return; }
    char a[128], b[128], main[128];
    snprintf(a,    sizeof a,    "%s/a.sp",    tmpdir);
    snprintf(b,    sizeof b,    "%s/b.sp",    tmpdir);
    snprintf(main, sizeof main, "%s/main.sp", tmpdir);

    write_file(a, "int fa() { return 1; }\n");
    write_file(b, "int fb() { return 2; }\n");

    ImportCtx *ictx = import_ctx_new();
    Parsed p = parse_with_imports(
        "import \"a.sp\";\nimport \"b.sp\";\nint main() { return 0; }\n",
        main, ictx);

    CHECK(p.impd.len == 0, "multi-import: no errors");
    CHECK(count_decl(p.ast, "fa")   == 1, "fa imported");
    CHECK(count_decl(p.ast, "fb")   == 1, "fb imported");
    CHECK(count_decl(p.ast, "main") == 1, "main present");

    import_ctx_free(ictx);
    parsed_free(&p);
}

/* =========================================================================
 * §4  Deduplication
 * ====================================================================== */
static void test_dedup(void) {
    printf("[4] Import deduplication\n");

    char tmpdir[64];
    snprintf(tmpdir, sizeof tmpdir, "/tmp/sharp_import_XXXXXX");
    if (!mkdtemp(tmpdir)) { perror("mkdtemp"); return; }
    char lib[128], main[128];
    snprintf(lib,  sizeof lib,  "%s/lib.sp",  tmpdir);
    snprintf(main, sizeof main, "%s/main.sp", tmpdir);

    write_file(lib, "int f() { return 1; }\n");

    ImportCtx *ictx = import_ctx_new();
    Parsed p = parse_with_imports(
        "import \"lib.sp\";\nimport \"lib.sp\";\nint g() { return 2; }\n",
        main, ictx);

    CHECK(p.impd.len == 0, "dedup: no errors");
    /* f appears exactly once despite two imports */
    CHECK(count_decl(p.ast, "f") == 1, "f appears exactly once (dedup)");

    import_ctx_free(ictx);
    parsed_free(&p);
}

/* =========================================================================
 * §5  Transitive imports
 * ====================================================================== */
static void test_transitive(void) {
    printf("[5] Transitive imports\n");

    char tmpdir[64];
    snprintf(tmpdir, sizeof tmpdir, "/tmp/sharp_import_XXXXXX");
    if (!mkdtemp(tmpdir)) { perror("mkdtemp"); return; }
    char a[128], b[128], main[128];
    snprintf(a,    sizeof a,    "%s/a.sp",    tmpdir);
    snprintf(b,    sizeof b,    "%s/b.sp",    tmpdir);
    snprintf(main, sizeof main, "%s/main.sp", tmpdir);

    write_file(b, "int from_b() { return 2; }\n");

    char a_content[256];
    snprintf(a_content, sizeof a_content,
             "import \"b.sp\";\nint from_a() { return 1; }\n");
    write_file(a, a_content);

    ImportCtx *ictx = import_ctx_new();
    Parsed p = parse_with_imports(
        "import \"a.sp\";\nint main() { return 0; }\n",
        main, ictx);

    CHECK(p.impd.len == 0, "transitive: no errors");
    CHECK(count_decl(p.ast, "from_b") == 1, "from_b transitively imported");
    CHECK(count_decl(p.ast, "from_a") == 1, "from_a imported");
    CHECK(count_decl(p.ast, "main")   == 1, "main present");

    import_ctx_free(ictx);
    parsed_free(&p);
}

/* =========================================================================
 * §6  Cycle detection
 * ====================================================================== */
static void test_cycle(void) {
    printf("[6] Cycle detection\n");

    char tmpdir[64];
    snprintf(tmpdir, sizeof tmpdir, "/tmp/sharp_import_XXXXXX");
    if (!mkdtemp(tmpdir)) { perror("mkdtemp"); return; }
    char a[128], b[128];
    snprintf(a, sizeof a, "%s/a.sp", tmpdir);
    snprintf(b, sizeof b, "%s/b.sp", tmpdir);

    char a_c[256], b_c[256];
    snprintf(a_c, sizeof a_c, "import \"b.sp\";\nint fa() { return 1; }\n");
    snprintf(b_c, sizeof b_c, "import \"a.sp\";\nint fb() { return 2; }\n");
    write_file(a, a_c);
    write_file(b, b_c);

    ImportCtx *ictx = import_ctx_new();
    Parsed p = parse_with_imports(
        "import \"a.sp\";\nint main() { return 0; }\n",
        a, ictx);

    CHECK(p.impd.len > 0, "cycle: error detected");

    import_ctx_free(ictx);
    parsed_free(&p);
}

/* =========================================================================
 * §7  Import + sema cross-file types
 * ====================================================================== */
static void test_import_sema(void) {
    printf("[7] Import + sema cross-file types\n");

    char tmpdir[64];
    snprintf(tmpdir, sizeof tmpdir, "/tmp/sharp_import_XXXXXX");
    if (!mkdtemp(tmpdir)) { perror("mkdtemp"); return; }
    char lib[128], main[128];
    snprintf(lib,  sizeof lib,  "%s/types.sp", tmpdir);
    snprintf(main, sizeof main, "%s/main.sp",  tmpdir);

    write_file(lib, "struct Point { float x; float y; }\n");

    ImportCtx *ictx = import_ctx_new();
    Parsed p = parse_with_imports(
        "import \"types.sp\";\nvoid f(Point p) { float x = p.x; }\n",
        main, ictx);

    /* Build scope + sema on merged AST */
    FeDiagArr sd={0}, sema_d={0};
    Scope    *scope = scope_build(p.ast, &sd);
    TyStore  *ts    = ty_store_new();
    SemaCtx  *sema  = sema_ctx_new(ts, scope, &sema_d);
    sema_check_file(sema, p.ast);

    CHECK(p.impd.len  == 0, "cross-file sema: no import errors");
    CHECK(sema_d.len  == 0, "cross-file sema: no sema errors");
    CHECK(count_decl(p.ast, "Point") == 1, "Point imported");

    sema_ctx_free(sema); ty_store_free(ts); scope_free_chain(scope);
    for(size_t i=0;i<sd.len;i++) free(sd.data[i].msg);
    for(size_t i=0;i<sema_d.len;i++) free(sema_d.data[i].msg);
    free(sd.data); free(sema_d.data);

    import_ctx_free(ictx);
    parsed_free(&p);
}

/* =========================================================================
 * §8  Error: file not found
 * ====================================================================== */
static void test_not_found(void) {
    printf("[8] File not found error\n");

    ImportCtx *ictx = import_ctx_new();
    Parsed p = parse_with_imports(
        "import \"nonexistent.sp\";\nint main() { return 0; }\n",
        "/tmp/dummy.sp", ictx);

    CHECK(p.impd.len > 0, "missing file: error reported");

    import_ctx_free(ictx);
    parsed_free(&p);
}

/* =========================================================================
 * main
 * ====================================================================== */
int main(void) {
    test_path_resolution();
    test_single_import();
    test_multi_import();
    test_dedup();
    test_transitive();
    test_cycle();
    test_import_sema();
    test_not_found();

    printf("\nImport results: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

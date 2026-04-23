/* test_cpp_regression9.c - C header inclusion and declaration parsing
 *
 * Tests:
 * 1. #include .h file expansion (preprocessor)
 * 2. C function declaration parsing (parser)
 * 3. Macro propagation from .h to .sp
 * 4. Include guard handling for .h files
 * 5. Multiple .h includes
 * 6. Mixed .sp and .h includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cpp/cpp.h"
#include "../sharp.h"

static int g_pass = 0, g_fail = 0;

#define CHECK(desc, expected, input) \
    do { \
        CppCtx *ctx = cpp_ctx_new(); \
        cpp_emit_linemarkers(ctx, false); \
        CppResult r = cpp_run_buf(ctx, (input), strlen(input), "<test>", CPP_LANG_SHARP); \
        if (r.text && strstr(r.text, (expected)) != NULL) { \
            printf("  PASS  %s\n", desc); g_pass++; \
        } else { \
            printf("  FAIL  %s: expected=[%s]\n", desc, expected); \
            printf("        text=[%s]\n", r.text ? r.text : "(null)"); \
            if (r.ndiags > 0) { \
                for (size_t i = 0; i < r.ndiags && i < 3; i++) \
                    printf("        diag[%zu]: %s\n", i, r.diags[i].msg); \
            } \
            g_fail++; \
        } \
        cpp_result_free(&r); \
        cpp_ctx_free(ctx); \
    } while (0)

/* Helper: create a temp file with given content */
static void write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

int main(void) {
    printf("=== cpp regression tests round 9: C header inclusion ===\n\n");

    /* Test 1: Basic .h file expansion */
    printf("--- 1: Basic .h file expansion ---\n");
    write_file("r9_basic.h",
        "#define R9_CONST 42\n"
        "int r9_func(int x);\n");

    CHECK(".h file macro expands", "= 42",
        "#include \"r9_basic.h\"\n"
        "int x = R9_CONST;\n");

    remove("r9_basic.h");

    /* Test 2: C function declaration in .h */
    printf("--- 2: C function declaration in .h ---\n");
    write_file("r9_funcs.h",
        "int c_add(int a, int b);\n"
        "void c_noop(void);\n");

    /* The preprocessor expands the .h, parser sees the declarations */
    {
        CppCtx* ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);

        const char* src =
            "#include \"r9_funcs.h\"\n"
            "int main() {\n"
            "    return c_add(1, 2);\n"
            "}\n";

        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<test>", CPP_LANG_SHARP);

        if (r.text && strstr(r.text, "c_add") != NULL) {
            printf("  PASS  .h function declaration visible in output\n"); g_pass++;
        } else {
            printf("  FAIL  c_add not found in preprocessed output\n");
            printf("        text=[%s]\n", r.text ? r.text : "(null)");
            g_fail++;
        }
        cpp_result_free(&r);
        cpp_ctx_free(ctx);
    }

    remove("r9_funcs.h");

    /* Test 3: Macro propagation from .h to .sp */
    printf("--- 3: Macro propagation ---\n");
    write_file("r9_macros.h",
        "#define R9_A 100\n"
        "#define R9_B 200\n"
        "#define R9_SUM (R9_A + R9_B)\n");

    CHECK("macros from .h are available", "R9_A + R9_B",
        "#include \"r9_macros.h\"\n"
        "int x = R9_SUM;\n");

    remove("r9_macros.h");

    /* Test 4: Include guard handling for .h files */
    printf("--- 4: Include guard handling ---\n");
    write_file("r9_guard.h",
        "#ifndef R9_GUARD_H\n"
        "#define R9_GUARD_H\n"
        "int guard_value = 1;\n"
        "#endif\n");

    {
        CppCtx* ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);

        const char* src =
            "#include \"r9_guard.h\"\n"
            "#include \"r9_guard.h\"\n"
            "#include \"r9_guard.h\"\n"
            "int x = guard_value;\n";

        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<test>", CPP_LANG_SHARP);

        int count = 0;
        const char* p = r.text;
        while (p) {
            p = strstr(p, "guard_value");
            if (p) { count++; p++; }
        }
        if (count == 2) { /* one definition + one usage */
            printf("  PASS  include guard prevents triple definition\n"); g_pass++;
        } else {
            printf("  FAIL  guard_value appeared %d times (expected 2)\n", count);
            printf("        text=[%s]\n", r.text ? r.text : "(null)");
            g_fail++;
        }
        cpp_result_free(&r);
        cpp_ctx_free(ctx);
    }

    remove("r9_guard.h");

    /* Test 5: Multiple .h includes */
    printf("--- 5: Multiple .h includes ---\n");
    write_file("r9_math.h",
        "#define R9_PI 314\n"
        "int r9_add(int a, int b);\n");

    write_file("r9_io.h",
        "#define R9_BUF_SIZE 1024\n"
        "void r9_print(int x);\n");

    CHECK("multiple .h macros visible", "= 1024",
        "#include \"r9_math.h\"\n"
        "#include \"r9_io.h\"\n"
        "int x = R9_BUF_SIZE;\n");

    remove("r9_math.h");
    remove("r9_io.h");

    /* Test 6: Mixed .sp and .h includes */
    printf("--- 6: Mixed .sp and .h includes ---\n");
    write_file("r9_util.h",
        "#define R9_SCALE 10\n");

    write_file("r9_util.sp",
        "#define R9_OFFSET 5\n");

    CHECK("macros from .h are available", "10 + 5",
        "#include \"r9_util.h\"\n"
        "#include \"r9_util.sp\"\n"
        "int x = R9_SCALE + R9_OFFSET;\n");

    remove("r9_util.h");
    remove("r9_util.sp");

    /* Test 7: Conditional compilation in .h */
    printf("--- 7: Conditional compilation in .h ---\n");
    write_file("r9_cond.h",
        "#define R9_ENABLE 1\n"
        "#if R9_ENABLE\n"
        "#define R9_ACTIVE 1\n"
        "#else\n"
        "#define R9_ACTIVE 0\n"
        "#endif\n");

    CHECK("conditionals in .h resolved", "= 1",
        "#include \"r9_cond.h\"\n"
        "int x = R9_ACTIVE;\n");

    remove("r9_cond.h");

    /* Test 8: Nested includes */
    printf("--- 8: Nested includes ---\n");
    write_file("r9_inner.h",
        "#define R9_INNER 99\n");

    write_file("r9_outer.h",
        "#include \"r9_inner.h\"\n"
        "#define R9_OUTER 1\n");

    CHECK("nested include expands inner macro", "= 99",
        "#include \"r9_outer.h\"\n"
        "int x = R9_INNER;\n");

    remove("r9_inner.h");
    remove("r9_outer.h");

    /* Summary */
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}

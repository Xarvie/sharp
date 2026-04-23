/* debug_preprocess.c - Dump preprocessed output */
#include <stdio.h>
#include <stdlib.h>
#include "../cpp/cpp.h"

int main() {
    /* Read test_stdio_simple.sp */
    FILE* f = fopen("../test_stdio_simple.sp", "r");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* src = malloc(len + 1);
    fread(src, 1, len, f);
    src[len] = 0;
    fclose(f);

    CppCtx* ctx = cpp_ctx_new();
    CppResult r = cpp_run_buf(ctx, src, len, "test_stdio_simple.sp", CPP_LANG_SHARP);

    if (r.text) {
        printf("=== PREPROCESSED OUTPUT (%zd bytes) ===\n", r.len);
        fwrite(r.text, 1, r.len, stdout);
        printf("\n=== END ===\n");
    }

    cpp_result_free(&r);
    cpp_ctx_free(ctx);
    free(src);
    return 0;
}

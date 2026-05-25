#define _POSIX_C_SOURCE 200809L
#include "cpp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    struct stat st; fstat(fileno(f), &st);
    char *buf = (char*)malloc((size_t)st.st_size + 1);
    size_t nread = fread(buf, 1, (size_t)st.st_size, f);
    buf[nread] = '\0';
    *out_len = nread;
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <file.c>\n", argv[0]);
        return 2;
    }
    CppCtx *ctx = cpp_ctx_new();
    cpp_probe_host_macros(ctx);
    cpp_emit_linemarkers(ctx, false);

    size_t len; char *src = read_file(argv[1], &len);
    if (!src) { cpp_ctx_free(ctx); return 2; }
    CppResult r = cpp_run_buf(ctx, src, len, argv[1]);
    if (r.text) {
        fwrite(r.text, 1, r.text_len, stdout);
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
    free(src);
    return 0;
}
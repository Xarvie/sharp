#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Inline the minimal code we need to test */
#include "sharp-cpp/cpp_sys_paths.h"
#include "sharp-cpp/cpp.h"

extern const char *cpp_find_zig_exe(void);
extern void cpp_detect_zig_sys_paths_from_zig(CppCtx *ctx, const char *target);

int main(void) {
    printf("=== cpp_sys_paths zig verbose test ===\n\n");
    
    const char *zig_exe = cpp_find_zig_exe();
    printf("Zig exe: %s\n", zig_exe ? zig_exe : "NOT FOUND");
    
    CppCtx *ctx = cpp_ctx_new();
    
    printf("\nDetecting sys paths from zig cc -E -v...\n");
    cpp_detect_zig_sys_paths_from_zig(ctx, "x86_64-linux-gnu");
    
    printf("\nFound %zu include paths:\n", cpp_sys_include_count(ctx));
    for (size_t i = 0; i < cpp_sys_include_count(ctx); i++) {
        const char *p = cpp_sys_include(ctx, i);
        printf("  [%zu] %s\n", i, p);
    }
    
    printf("\n=== Test complete ===\n");
    cpp_ctx_free(ctx);
    return 0;
}

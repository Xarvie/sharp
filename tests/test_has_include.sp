/* compiler: any */
/* Preprocessor: __has_include() (C23 §6.10.1) */
/* stdio.h should exist on all platforms */

#if __has_include(<stdio.h>)
#define HAS_STDIO 1
#else
#define HAS_STDIO 0
#endif

i32 main() {
    return HAS_STDIO;
}

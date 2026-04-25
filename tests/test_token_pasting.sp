/* compiler: any */
/* §30 双层宏展开 token-pasting */
#define CONCAT(a, b) a##b
#define CONCAT_EXPANDED(a, b) CONCAT(a, b)

i32 main() {
    return 0;
}

/*
 * sharpc bug: struct tag redefinition inside anonymous struct
 *
 * Expected: compiles without error (gcc/clang accept this)
 * Actual:   sharpc reports "redefinition of 'inner_tag'"
 *
 * Pattern from mbedtls2/include/mbedtls/ssl_internal.h:
 *   struct { struct inner_tag { ... }; } anonymous;
 *   typedef struct inner_tag inner_tag;
 */

struct outer {
    struct {
        int x;
        struct inner_tag {
            int a;
            int b;
        } items[4];
    } buffering;
};

typedef struct inner_tag inner_tag;

int main(void) {
    struct outer o;
    o.buffering.x = 1;
    o.buffering.items[0].a = 10;
    o.buffering.items[0].b = 20;
    inner_tag t = { .a = 3, .b = 4 };
    (void)o;
    (void)t;
    return 0;
}

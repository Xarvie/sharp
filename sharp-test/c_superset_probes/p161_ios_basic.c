/*
 * p161_ios_basic.c — Cross-compile probe: basic C compilation targeting iOS.
 *
 * Tests:
 *   1. Include standard C headers targeting iOS (stddef.h, stdint.h)
 *   2. Use standard C types and arithmetic
 *   3. Compile with --target aarch64-ios on macOS
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

size_t test_size_t(void) {
    return sizeof(size_t);
}

int test_stdint(void) {
    int8_t   i8  = 0;
    int16_t  i16 = 0;
    int32_t  i32 = 0;
    int64_t  i64 = 0;
    uint8_t  u8  = 0;
    uint16_t u16 = 0;
    uint32_t u32 = 0;
    uint64_t u64 = 0;
    (void)i8; (void)i16; (void)i32; (void)i64;
    (void)u8; (void)u16; (void)u32; (void)u64;
    return 0;
}

bool test_bool(bool x) {
    return !x;
}

int main(void) {
    test_size_t();
    test_stdint();
    test_bool(true);
    return 0;
}
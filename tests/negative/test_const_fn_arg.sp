/* Test: passing const pointer to non-const parameter should be rejected */
/* Expected error: const discard in function call */

void modify(u8* ptr) {
    *ptr = 0;
}

i32 main() {
    const u8* cptr = "hello";
    modify(cptr);   /* error: discards const */
    return 0;
}

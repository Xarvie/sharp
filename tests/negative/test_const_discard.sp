/* Test: assigning const pointer to non-const pointer should be rejected */
/* Expected error: const discard */

extern i32 puts(const u8* s);

i32 main() {
    const u8* cptr = "hello";
    u8* mptr = cptr;   /* error: discards const */
    return 0;
}

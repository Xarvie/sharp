/* Test: assigning const pointer to non-const pointer should be rejected */
/* Expected error: const discard */

extern int puts(const char* s);

int main() {
    const char* cptr = "hello";
    char* mptr = cptr;   /* error: discards const */
    return 0;
}
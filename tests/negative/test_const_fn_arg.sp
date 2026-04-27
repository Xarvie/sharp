/* Test: passing const pointer to non-const parameter should be rejected */
/* Expected error: const discard in function call */

void modify(char* ptr) {
    *ptr = 0;
}

int main() {
    const char* cptr = "hello";
    modify(cptr);   /* error: discards const */
    return 0;
}
/* compiler: any */
/* test_const_ptr.sp - Test const pointer variants */

extern int puts(const char* s);

int main() {
    const char* msg1 = "hello const";
    puts(msg1);
    return 0;
}
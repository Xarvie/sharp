#define MULTI_LINE(x) \
  { int a = x; \
    a++; }

void test(void) {
    MULTI_LINE(5);
}

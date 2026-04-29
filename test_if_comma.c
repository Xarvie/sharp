#define P99_HAS_COMMA(...) P00_HAS_COMMA_(__VA_ARGS__, 1, 0)
#define P00_HAS_COMMA_(_1, _2, _3, HAS, ...) HAS

/* Test: what does P99_HAS_COMMA produce? */
#if P99_HAS_COMMA(,)
int has_comma = 1;
#else
int has_comma = 0;
#endif

int main() { return has_comma; }

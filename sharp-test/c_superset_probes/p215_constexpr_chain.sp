/* p215 — constexpr: sizeof, array sizes, and constant context */
/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
constexpr int INT_BYTES = sizeof(int);
constexpr int CHAR_BYTES = sizeof(char);
constexpr int PTR_BYTES = sizeof(int *);

/* constexpr function computing sizes */
constexpr int total_bytes(int count) {
    return count * sizeof(int);
}

/* constexpr with sizeof in complex expression */
constexpr int elements_for_bytes(int bytes) {
    return bytes / sizeof(int);
}

/* constexpr array size computation */
constexpr int ARR_SIZE = elements_for_bytes(256);

/* constexpr with multiple sizeof */
constexpr int size_ratio(void) {
    return sizeof(int) / sizeof(char);
}

int main(void) {
    if (INT_BYTES != 4) return 1;
    if (CHAR_BYTES != 1) return 2;
    if (PTR_BYTES != 4 && PTR_BYTES != 8) return 3;
    if (total_bytes(10) != 40) return 4;
    if (elements_for_bytes(100) != 25) return 5;
    if (size_ratio() != 4) return 6;

    /* Array sized from constexpr */
    int arr[ARR_SIZE];
    if (sizeof(arr) != 1024) return 7;

    return 0;
}

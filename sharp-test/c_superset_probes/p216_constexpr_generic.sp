/* p216 — constexpr: generic function with compile-time constants */
/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
/* constexpr variable: base type sizes */
constexpr int INT_SIZE = sizeof(int);
constexpr int CHAR_SIZE = sizeof(char);

/* constexpr function: compute array element count */
constexpr int element_count(int total_bytes) {
    return total_bytes / sizeof(int);
}

/* constexpr function: default buffer capacity */
constexpr int default_capacity(void) {
    return 256;
}

/* Generic swap function */
void swap<T>(T* a, T* b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

/* Generic array fill with constexpr count */
void fill_array<T>(T* arr, T val, int count) {
    int i = 0;
    while (i < count) {
        arr[i] = val;
        i = i + 1;
    }
}

int main(void) {
    /* Test constexpr variables */
    if (INT_SIZE != 4) return 1;
    if (CHAR_SIZE != 1) return 2;
    
    /* Test constexpr function */
    if (element_count(1024) != 256) return 3;
    if (default_capacity() != 256) return 4;
    
    /* Test generic swap */
    int x = 10, y = 20;
    swap<int>(&x, &y);
    if (x != 20) return 5;
    if (y != 10) return 6;
    
    /* Test generic fill with constexpr count */
    int arr[4];
    fill_array<int>(arr, 42, 4);
    if (arr[0] != 42) return 7;
    if (arr[3] != 42) return 8;
    
    return 0;
}

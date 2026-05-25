/* p217 — constexpr: compile-time constants and functions (macro replacement) */
/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
/* Export constexpr variable - replaces #define BUFFER_SIZE 1024 */
constexpr int BUFFER_SIZE = 1024;

/* Export constexpr function - replaces macro MAX(a,b) */
constexpr int max(int a, int b) {
    return a > b ? a : b;
}

/* Export constexpr function - replaces macro MIN(a,b) */
constexpr int min(int a, int b) {
    return a < b ? a : b;
}

/* Export constexpr variable - replaces #define PAGE_SIZE 4096 */
constexpr int PAGE_SIZE = 4096;

/* Export constexpr function - replaces macro ARRAY_SIZE(arr) */
constexpr int array_elements(int total_bytes) {
    return total_bytes / sizeof(int);
}

/* Regular function using constexpr */
int compute_buffer_pages(void) {
    return BUFFER_SIZE * sizeof(int) / PAGE_SIZE;
}

int main(void) {
    /* Test constexpr variables */
    if (BUFFER_SIZE != 1024) return 1;
    if (PAGE_SIZE != 4096) return 2;
    
    /* Test constexpr functions */
    if (max(3, 7) != 7) return 3;
    if (min(3, 7) != 3) return 4;
    
    /* Test constexpr in array sizing */
    int buf[BUFFER_SIZE];
    if (sizeof(buf) != 4096) return 5;
    
    /* Test constexpr in expressions */
    int pages = compute_buffer_pages();
    if (pages != 1) return 6;
    
    /* Test constexpr with nested calls */
    if (max(min(5, 10), 3) != 5) return 7;
    if (min(max(5, 10), 8) != 8) return 8;
    
    return 0;
}

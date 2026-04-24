/* Phase 2: C declaration modifier tests */

/* __attribute__ on function declarations */
extern void log_msg(const char* msg) __attribute__((format(printf, 1, 2)));

/* __inline__ function */
extern __inline__ int add(int a, int b) {
    return a + b;
}

/* __declspec (MSVC extension, pass-through) */
extern __declspec(dllimport) int external_value;

/* Usage */
i32 main() {
    i32 x = add(1, 2);
    return 0;
}

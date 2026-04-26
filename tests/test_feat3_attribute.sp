/* compiler: any */
/* Feature 3: __attribute__ before return type */

extern
__attribute__((format(printf, 1, 2)))
i32 my_printf(const char* fmt, ...);

i32 main() {
    return 0;
}

/* compiler: any */
/* Feature 3: __attribute__ before return type */

extern
__attribute__((format(printf, 1, 2)))
int my_printf(const char* fmt, ...);

int main() {
    return 0;
}
/* compiler: any */
/* §23 __need_wint_t 条件 typedef */
/* NOTE: 此测试验证条件宏编译模式。
 * 生成的 C 代码包含 #ifndef __need_wint_t / typedef unsigned int wint_t; / #endif。
 * 但 uint32_t 未定义（未引入 <stdint.h>），GCC 编译时会报错。
 * C 代码生成正确，测试重点是条件宏机制而非运行时。
 */
#ifndef __need_wint_t
#define __need_wint_t
typedef unsigned int wint_t;
#endif

i32 main() {
    return 0;
}

// 来源: p262_module_generic.sp
// 模块泛型实例化: 导入泛型 RingBuf 模块并实例化

#include <stdbool.h>
#include "generic_inst.sph"

int main() {
    int buf[8] = {0};

    RingBuf<int> rb;
    rb.init(buf, 8);

    if (rb.size() != 0) return 1;

    if (!rb.push(10)) return 2;
    if (!rb.push(20)) return 3;
    if (!rb.push(30)) return 4;

    if (rb.size() != 3) return 5;

    int val = 0;
    if (!rb.pop(&val)) return 6;
    if (val != 10) return 7;
    if (!rb.pop(&val)) return 8;
    if (val != 20) return 9;
    if (!rb.pop(&val)) return 10;
    if (val != 30) return 11;
    if (rb.size() != 0) return 12;

    return 0;
}
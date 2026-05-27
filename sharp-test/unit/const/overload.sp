// 来源: p156_assoc_const.sp, p244_ext_const_overload.sp
// 关联函数(const方法) + 扩展const重载测试

#include <stdbool.h>

// === p156: 关联函数 (工厂) + const 方法 ===
class Counter {
    long value;
}
Counter Counter.from(long start) {
    Counter c; c.value = start; return c;
}
void Counter.inc(this) { this->value = this->value + 1; }
long Counter.get(this) const { return this->value; }

// === p244: const 重载 ===
class Buffer {
    int data;
};
int Buffer.read(this) const { return this->data; }
void Buffer.write(this, int v) { this->data = v; }
int read_const(const Buffer* b) { return b->read(); }

int main() {
    // --- p156: factory + const ---
    Counter c = Counter.from(100);
    if (c.get() != 100) return 1;
    c.inc();
    if (c.get() != 101) return 2;
    c.inc();
    c.inc();
    if (c.get() != 103) return 3;

    // --- p244: const overloading ---
    Buffer b = {0};
    b.write(42);
    if (b.read() != 42) return 4;
    const Buffer cb = b;
    if (read_const(&cb) != 42) return 5;

    return 0;
}
// 来源: p227_voidptr_erasure.sp, p229_func_ptr.sp
// C互通: void* 类型擦除 + 函数指针字段 + 扩展方法

#include <stdbool.h>

// === p227: void* type erasure ===
class AnyBox {
    void* ptr;
    int   tag;
};
int AnyBox.as_int(this) const {
    int* ip = (int*)this->ptr;
    if (this->ptr) return *ip;
    return 0;
}
float AnyBox.as_float(this) const {
    float* fp = (float*)this->ptr;
    if (this->ptr) return *fp;
    return 0.0f;
}
void AnyBox.set_int(this, int* p) {
    this->ptr = (void*)p;
    this->tag = 1;
}
void AnyBox.set_float(this, float* p) {
    this->ptr = (void*)p;
    this->tag = 2;
}
bool ptr_equals(void* a, void* b) { return a == b; }

// === p229: function pointer fields ===
class Button {
    int id;
    void (*on_click)(int id);
};
void Button.click(this) {
    if (this->on_click) this->on_click(this->id);
}
void Button.set_callback(this, void (*cb)(int)) {
    this->on_click = cb;
}
typedef int (*IntUnary)(int);
IntUnary make_doubler() { return 0; }

void cb_a(int id) { (void)id; }
void cb_b(int id) { (void)id; }

int main() {
    // --- p227: void* tests ---
    AnyBox box;
    int iv = 123;
    box.set_int(&iv);
    if (box.as_int() != 123) return 1;

    float fv = 3.14f;
    box.set_float(&fv);
    float rf = box.as_float();
    if (rf < 3.13f || rf > 3.15f) return 2;

    if (!ptr_equals(&iv, &iv)) return 3;
    if (ptr_equals(&iv, &fv)) return 4;

    AnyBox empty = {0};
    if (empty.as_int() != 0) return 5;

    // --- p229: func_ptr tests ---
    Button b;
    b.id = 1;
    b.set_callback(cb_a);
    b.click();
    b.id = 2;
    b.set_callback(cb_b);
    b.click();
    Button b2 = {0};
    b2.click();  /* null callback check */

    return 0;
}
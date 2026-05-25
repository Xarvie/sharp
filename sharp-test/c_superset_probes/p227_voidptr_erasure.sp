/* p227 — void* type erasure patterns
 *
 * Verifies that Sharp supports void* as a generic erasure mechanism
 * (C-style containers), including:
 *   - void* fields in structs
 *   - Extension methods that cast void* back to typed pointer
 *   - Passing typed &var to void* parameters
 */

#include <stdbool.h>

/* ===== C-style generic box via void* ===== */
class AnyBox {
    void* ptr;
    int   tag;   /* type discriminator */
};

/* Extension methods that cast void* back */
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

/* ===== free function that takes void* ===== */
bool ptr_equals(void* a, void* b) {
    return a == b;
}

int main() {
    AnyBox box;

    /* --- int boxing --- */
    int iv = 123;
    box.set_int(&iv);
    int r1 = box.as_int();
    if (r1 != 123) return 1;

    /* --- float boxing --- */
    float fv = 3.14f;
    box.set_float(&fv);
    float r2 = box.as_float();
    if (r2 < 3.13f || r2 > 3.15f) return 2;

    /* --- void* comparison --- */
    if (!ptr_equals(&iv, &iv)) return 3;
    if (ptr_equals(&iv, &fv)) return 4;

    /* --- null void* (zero-init) --- */
    AnyBox empty = {0};
    /* empty.ptr is NULL from zero-init */
    if (empty.as_int() != 0) return 5;

    return 0;
}
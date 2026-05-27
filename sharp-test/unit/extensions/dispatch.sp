// 来源: p241_ext_method_samename.sp, p242_ext_vs_freefunc.sp, p243_ext_escape.sp
// 扩展方法分发: 同名方法、vs自由函数、escape分析

#include <stdbool.h>

// === p241: 同名扩展方法 on different types ===
class Point2D { int x; int y; };
class Point3D { int x; int y; int z; };

int Point2D.magnitude_sq(this) const { return this->x * this->x + this->y * this->y; }
int Point3D.magnitude_sq(this) const { return this->x * this->x + this->y * this->y + this->z * this->z; }

// === p242: 扩展方法 vs 自由函数 ===
struct Vec { int x; int y; };

// 自由函数版本
int vec_dot_free(struct Vec a, struct Vec b) { return a.x * b.x + a.y * b.y; }

// 扩展方法版本 (namespace-like)
int Vec.dot(this, struct Vec b) { return this->x * b.x + this->y * b.y; }

// === p243: escape analysis (返回值生命周期) ===
class Data { int val; };
int Data.get_val(this) { return this->val; }
void Data.set_val(this, int v) { this->val = v; }

int compute_sum(class Data a, class Data b) {
    return a.get_val() + b.get_val();
}

int main() {
    // --- p241: 同名方法 ---
    Point2D p2; p2.x = 3; p2.y = 4;
    Point3D p3; p3.x = 1; p3.y = 2; p3.z = 2;
    int mag2 = p2.magnitude_sq();
    int mag3 = p3.magnitude_sq();
    if (mag2 != 25) return 1;
    if (mag3 != 9) return 2;

    // --- p242: extension vs free function ---
    struct Vec va = {2, 3};
    struct Vec vb = {5, 7};
    int dot_ext = va.dot(vb);        // 2*5 + 3*7 = 31
    int dot_free = vec_dot_free(va, vb);
    if (dot_ext != 31) return 3;
    if (dot_free != 31) return 4;

    // --- p243: escape analysis ---
    Data a; a.set_val(10);
    Data b; b.set_val(20);
    int s = compute_sum(a, b);
    if (s != 30) return 5;
    if (a.get_val() != 10) return 6;

    return 0;
}
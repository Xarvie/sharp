/*
 * p154_operator_full.sp — 运算符重载全组合测试
 * Tests: all 12 operator overloads (+, -, *, /, %, ==, !=, <, >, <=, >=, [])
 *        on both generic and non-generic structs.
 */
/* FIXED: generic struct operator instantiation — operator functions forward-declared */
#include <stdio.h>
#include <stdbool.h>

int g_passed = 0;
int g_failed = 0;

void check(int cond, const char* msg) {
    if (cond) { g_passed = g_passed + 1; printf("  PASS: %s\n", msg); }
    else { g_failed = g_failed + 1; printf("  FAIL: %s\n", msg); }
}

/* === IntBox: all arithmetic + comparison operators === */
class IntBox {
    int value;
}

IntBox IntBox.operator+(this, IntBox other) const {
    IntBox r;
    r.value = this->value + other.value;
    return r;
}
IntBox IntBox.operator-(this, IntBox other) const {
    IntBox r;
    r.value = this->value - other.value;
    return r;
}
IntBox IntBox.operator*(this, IntBox other) const {
    IntBox r;
    r.value = this->value * other.value;
    return r;
}
IntBox IntBox.operator/(this, IntBox other) const {
    IntBox r;
    r.value = this->value / other.value;
    return r;
}
IntBox IntBox.operator%(this, IntBox other) const {
    IntBox r;
    r.value = this->value % other.value;
    return r;
}
bool IntBox.operator==(this, IntBox other) const {
    return this->value == other.value;
}
bool IntBox.operator!=(this, IntBox other) const {
    return this->value != other.value;
}
bool IntBox.operator<(this, IntBox other) const {
    return this->value < other.value;
}
bool IntBox.operator>(this, IntBox other) const {
    return this->value > other.value;
}
bool IntBox.operator<=(this, IntBox other) const {
    return this->value <= other.value;
}
bool IntBox.operator>=(this, IntBox other) const {
    return this->value >= other.value;
}

void test_intbox_arithmetic() {
    IntBox a; a.value = 10;
    IntBox b; b.value = 3;

    IntBox c = a + b; check(c.value == 13, "IntBox +");
    IntBox d = a - b; check(d.value == 7, "IntBox -");
    IntBox e = a * b; check(e.value == 30, "IntBox *");
    IntBox f = a / b; check(f.value == 3, "IntBox /");
    IntBox g = a % b; check(g.value == 1, "IntBox %%");
}

void test_intbox_comparison() {
    IntBox a; a.value = 10;
    IntBox b; b.value = 3;
    IntBox c; c.value = 10;

    check(a == c, "IntBox ==");
    check(a == a, "IntBox == self");
    check(!(a == b), "IntBox == false");
    check(a != b, "IntBox !=");
    check(!(a != c), "IntBox != false");
    check(b < a, "IntBox <");
    check(!(a < b), "IntBox < false");
    check(a > b, "IntBox >");
    check(!(b > a), "IntBox > false");
    check(b <= a, "IntBox <=");
    check(a <= c, "IntBox <= equal");
    check(a >= b, "IntBox >=");
    check(a >= c, "IntBox >= equal");
}

/* === FloatBox: test float operators === */
class FloatBox {
    float value;
}

FloatBox FloatBox.operator+(this, FloatBox other) const {
    FloatBox r;
    r.value = this->value + other.value;
    return r;
}
FloatBox FloatBox.operator-(this, FloatBox other) const {
    FloatBox r;
    r.value = this->value - other.value;
    return r;
}
bool FloatBox.operator==(this, FloatBox other) const {
    return this->value == other.value;
}

void test_floatbox() {
    FloatBox a; a.value = 3.0f;
    FloatBox b; b.value = 4.0f;

    FloatBox c = a + b;
    check((int)c.value == 7, "FloatBox +");
    FloatBox d = b - a;
    check((int)d.value == 1, "FloatBox -");
    check(!(a == b), "FloatBox != by value");
    check(a == a, "FloatBox == self");
}

/* === ArrayLike: operator[] on non-generic struct === */
class ArrayLike {
    int items[4];
}

int ArrayLike.operator[](this, long i) const { return this->items[i]; }

void test_arraylike() {
    ArrayLike arr;
    arr.items[0] = 10;
    arr.items[1] = 20;
    arr.items[2] = 30;
    arr.items[3] = 40;

    check(arr[0] == 10, "ArrayLike[0]");
    check(arr[1] == 20, "ArrayLike[1]");
    check(arr[2] == 30, "ArrayLike[2]");
    check(arr[3] == 40, "ArrayLike[3]");
}

/* === Free-function operator === */
struct Point {
    int x; int y;
}

struct Point operator+(struct Point a, struct Point b) {
    struct Point r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

void test_free_operator() {
    struct Point a; a.x = 1; a.y = 2;
    struct Point b; b.x = 3; b.y = 4;
    struct Point c = a + b;
    check(c.x == 4, "free op+ .x");
    check(c.y == 6, "free op+ .y");
}

/* === 泛型 class 上所有运算符 === */
class Num<T> {
    T value;
}

Num<T> Num<T>.operator+(this, Num<T> other) const {
    Num<T> r;
    r.value = this->value + other.value;
    return r;
}
bool Num<T>.operator==(this, Num<T> other) const {
    return this->value == other.value;
}
bool Num<T>.operator!=(this, Num<T> other) const {
    return this->value != other.value;
}
bool Num<T>.operator<(this, Num<T> other) const {
    return this->value < other.value;
}
bool Num<T>.operator>(this, Num<T> other) const {
    return this->value > other.value;
}
T Num<T>.operator[](this, long i) const {
    if (i == 0) return this->value;
    return this->value;
}

void test_generic_num_ops() {
    Num<int> a; a.value = 100;
    Num<int> b; b.value = 50;

    Num<int> c = a + b;
    check(c.value == 150, "Num<int> +");

    check(a == a, "Num<int> == self");
    check(!(a == b), "Num<int> == false");
    check(a != b, "Num<int> !=");
    check(!(a < b), "Num<int> < false");
    check(b < a, "Num<int> <");
    check(a > b, "Num<int> >");
    check(!(b > a), "Num<int> > false");
    check(a[0] == 100, "Num<int>[0]");
}

void test_generic_num_float_ops() {
    Num<float> a; a.value = 7.0f;
    Num<float> b; b.value = 3.0f;

    Num<float> c = a + b;
    check((int)c.value == 10, "Num<float> +");
    check(a == a, "Num<float> == self");
    check(!(a == b), "Num<float> == false");
    check(b < a, "Num<float> <");
}

int main() {
    printf("=== p154_operator_full ===\n");

    test_intbox_arithmetic();
    test_intbox_comparison();
    test_floatbox();
    test_arraylike();
    test_free_operator();
    test_generic_num_ops();
    test_generic_num_float_ops();

    printf("  passed=%d  failed=%d\n", g_passed, g_failed);
    return g_failed;
}
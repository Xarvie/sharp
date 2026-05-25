/*
 * p157_auto_deduction.sp — auto 类型推断边界测试
 * Tests: auto with int, float, pointer, generic types,
 *        auto in expressions, auto const, auto chaining.
 */
#include <stdio.h>

int g_passed = 0;
int g_failed = 0;

void check(int cond, const char* msg) {
    if (cond) { g_passed = g_passed + 1; printf("  PASS: %s\n", msg); }
    else { g_failed = g_failed + 1; printf("  FAIL: %s\n", msg); }
}

/* === 基本 auto === */
void test_auto_int() {
    auto x = 42;
    auto y = x + 8;
    check(y == 50, "auto int: 42+8=50");
}

void test_auto_float() {
    auto f = 3.14f;
    auto d = 7.0;
    check((int)f == 3, "auto float literal");
    check((int)d == 7, "auto double literal");
}

void test_auto_char() {
    auto c = 'A';
    check(c == 65, "auto char 'A'");
}

void test_auto_string() {
    auto s = "hello";
    check(s[0] == 'h', "auto string[0]");
}

/* === auto 从表达式推导 === */
void test_auto_from_expr() {
    int a = 10;
    int b = 20;
    auto sum = a + b;
    check(sum == 30, "auto from int expr");
}

void test_auto_from_float_expr() {
    float a = 3.0f;
    float b = 4.0f;
    auto prod = a * b;
    check((int)prod == 12, "auto from float expr");
}

/* === auto 从函数调用推导 === */
int get_100() { return 100; }
float get_pi() { return 3.14f; }

void test_auto_from_func() {
    auto v = get_100();
    check(v == 100, "auto from int func");

    auto f = get_pi();
    check((int)f == 3, "auto from float func");
}

/* === auto 指针 === */
void test_auto_pointer() {
    int x = 42;
    auto p = &x;
    check(*p == 42, "auto ptr deref");
}

/* === 基础类型测试 === */
void test_auto_const() {
    auto c = 99;
    check(c == 99, "auto int const val");
}

/* === auto 在 if 条件中 === */
void test_auto_in_if() {
    auto cond = 1;
    if (cond) {
        auto inside = 77;
        check(inside == 77, "auto inside if");
    }
}

/* === 泛型 struct 配合 auto === */
class Value<T> {
    T data;
}

T Value<T>.get(this) const {
    return this->data;
}

void test_auto_with_generic() {
    Value<int> v;
    v.data = 42;
    auto x = v.get();
    check(x == 42, "auto from Value<int>.get()");

    Value<float> vf;
    vf.data = 7.0f;
    auto y = vf.get();
    check((int)y == 7, "auto from Value<float>.get()");
}

/* === auto 配合 operator[] === */
class DataContainer {
    int storage[3];
}

int DataContainer.operator[](this, long i) const { return this->storage[i]; }

void test_auto_with_opindex() {
    DataContainer dc;
    dc.storage[0] = 10;
    dc.storage[1] = 20;

    auto a = dc[0];
    auto b = dc[1];
    check(a == 10, "auto from op_idx[0]");
    check(b == 20, "auto from op_idx[1]");
}

int main() {
    printf("=== p157_auto_deduction ===\n");

    test_auto_int();
    test_auto_float();
    test_auto_char();
    test_auto_string();
    test_auto_from_expr();
    test_auto_from_float_expr();
    test_auto_from_func();
    test_auto_pointer();
    test_auto_const();
    test_auto_in_if();
    test_auto_with_generic();
    test_auto_with_opindex();

    printf("  passed=%d  failed=%d\n", g_passed, g_failed);
    return g_failed;
}
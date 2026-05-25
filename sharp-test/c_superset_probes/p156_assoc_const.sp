/*
 * p156_assoc_const.sp — 关联函数与const方法边界测试
 * Tests: associated functions (auto-detected: no 'this' usage),
 *        const methods on generic and non-generic structs,
 *        const/non-const method mix, factory patterns, type.method() call syntax.
 */
/* FIXED(e2e): generic method second specialization — AST_CAST collects operand */
#include <stdio.h>
#include <stdbool.h>

int g_passed = 0;
int g_failed = 0;

void check(int cond, const char* msg) {
    if (cond) { g_passed = g_passed + 1; printf("  PASS: %s\n", msg); }
    else { g_failed = g_failed + 1; printf("  FAIL: %s\n", msg); }
}

/* === 关联函数（static method）：工厂模式 === */
class Counter {
    long value;
}

Counter Counter.from(long start) {
    Counter c;
    c.value = start;
    return c;
}

void Counter.inc(this) {
    this->value = this->value + 1;
}

long Counter.get(this) const {
    return this->value;
}

void test_counter_factory() {
    Counter c = Counter.from(100);
    check(c.get() == 100, "Counter.from(100)");
    c.inc();
    check(c.get() == 101, "Counter.inc()");
    c.inc();
    c.inc();
    check(c.get() == 103, "Counter.inc() x3");
}

/* === 关联函数搭配 const 方法 === */
class Point {
    int x; int y;
}

Point Point.origin() {
    Point p;
    p.x = 0;
    p.y = 0;
    return p;
}

Point Point.at(int px, int py) {
    Point p;
    p.x = px;
    p.y = py;
    return p;
}

int Point.get_x(this) const {
    return this->x;
}

int Point.get_y(this) const {
    return this->y;
}

bool Point.is_origin(this) const {
    return this->x == 0 && this->y == 0;
}

void test_point_assoc() {
    Point o = Point.origin();
    check(o.is_origin(), "Point.origin().is_origin()");

    Point p = Point.at(10, 20);
    check(p.get_x() == 10, "Point.at(10,20).x");
    check(p.get_y() == 20, "Point.at(10,20).y");
    check(!p.is_origin(), "!Point.at(10,20).is_origin()");
}

/* === const 方法链式调用 === */
class ChainVal {
    int value;
}

int ChainVal.get(this) const {
    return this->value;
}

bool ChainVal.equals(this, int v) const {
    return this->value == v;
}

void ChainVal.set_val(this, int v) {
    this->value = v;
}

void test_chain_const() {
    ChainVal cv; cv.value = 0;
    cv.set_val(42);
    check(cv.get() == 42, "ChainVal.set_val(42)");
    check(cv.equals(42), "ChainVal.equals(42)");
    check(!cv.equals(0), "ChainVal.equals(0) false");
}

/* === 泛型 struct 上的关联函数 === */
class Wrapper<T> {
    T value;
}

Wrapper<T> Wrapper<T>.of(T v) {
    Wrapper<T> w;
    w.value = v;
    return w;
}

T Wrapper<T>.get(this) const {
    return this->value;
}

void Wrapper<T>.set(this, T v) {
    this->value = v;
}

void test_wrapper_assoc() {
    Wrapper<int> w = Wrapper<int>.of(255);
    check(w.get() == 255, "Wrapper<int>.of(255)");

    Wrapper<float> wf = Wrapper<float>.of(7.5f);
    check((int)wf.get() == 7, "Wrapper<float>.of(7.5)");
}

/* === const 方法返回结构体 === */
class Pair {
    int first;
    int second;
}

Pair Pair.from_values(int a, int b) {
    Pair p;
    p.first = a;
    p.second = b;
    return p;
}

int Pair.sum(this) const {
    return this->first + this->second;
}

int Pair.product(this) const {
    return this->first * this->second;
}

void test_pair_assoc_const() {
    Pair p = Pair.from_values(7, 3);
    check(p.sum() == 10, "Pair(7,3).sum() == 10");
    check(p.product() == 21, "Pair(7,3).product() == 21");
}

/* === 泛型关联函数返回泛型 struct === */
class Maybe<T> {
    T value;
    int present;
}

Maybe<T> Maybe<T>.some(T v) {
    Maybe<T> m;
    m.value = v;
    m.present = 1;
    return m;
}

int Maybe<T>.is_present(this) const {
    return this->present;
}

T Maybe<T>.unwrap(this) const {
    return this->value;
}

void Maybe<T>.clear(this) {
    this->present = 0;
}

void test_maybe_assoc() {
    Maybe<int> ms = Maybe<int>.some(42);
    check(ms.is_present(), "Maybe<int>.some(42).is_present()");
    check(ms.unwrap() == 42, "Maybe<int>.some(42).unwrap()");

    ms.clear();
    check(!ms.is_present(), "Maybe<int>.clear() → not present");
}

int main() {
    printf("=== p156_assoc_const ===\n");

    test_counter_factory();
    test_point_assoc();
    test_chain_const();
    test_wrapper_assoc();
    test_pair_assoc_const();
    test_maybe_assoc();

    printf("  passed=%d  failed=%d\n", g_passed, g_failed);
    return g_failed;
}
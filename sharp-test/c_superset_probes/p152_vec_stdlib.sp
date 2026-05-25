/*
 * p152_vec_stdlib.sp — Comprehensive generic Vec<T> standard library
 *
 * Tests:
 * - Generic struct with 12 methods (int, float, Vec<int> 三个实例化)
 * - Const methods / non-const methods
 * - Methods returning T (generic return type in struct body)
 * - Methods returning Vec<T> (struct type, not T)
 * - Nested generics: Vec<Vec<int>>
 * - auto type deduction
 * - const method calling another const method (chaining)
 * - **Standalone generic function returning T: <T> T first(Vec<T>* v)**
 *   This tests Phase 16: <T> T prefix syntax for standalone generic functions
 *   where T is used as the return type (T resolved via parameter type inference).
 * - **operator[] overload on generic struct: v[2] → Vec__int__op_idx(&v, 2)**
 *   This tests Phase 18: generic operator[] read support with scope/subst fixes.
 *
 * Note: Methods using T in comparisons (+, -, <, >) require @has_operator(T, op)
 * constraint support on generic type parameters — that is a deeper feature, deferred.
 */
/* FIXED: generic struct operator[] and operator== instantiation */
#include <stdbool.h>

class Vec<T> {
    T* data;
    long len;
    long cap;
}

/* Non-const mutating method */
void Vec<T>.push(this, T item) {
    if (this->len >= this->cap) {
        long new_cap = (this->cap == 0) ? 4 : this->cap * 2;
        this->cap = new_cap;
    }
    this->data[this->len] = item;
    this->len = this->len + 1;
}

void Vec<T>.pop(this) {
    if (this->len > 0) this->len = this->len - 1;
}

void Vec<T>.clear(this) {
    this->len = 0;
}

void Vec<T>.set(this, long i, T value) {
    this->data[i] = value;
}

/* Const methods returning T */
T Vec<T>.get(this, long i) const {
    return this->data[i];
}

T Vec<T>.first(this) const {
    return this->data[0];
}

T Vec<T>.last(this) const {
    return this->data[this->len - 1];
}

/* Const methods returning built-in types */
long Vec<T>.size(this) const {
    return this->len;
}

long Vec<T>.capacity(this) const {
    return this->cap;
}

bool Vec<T>.is_empty(this) const {
    return this->len == 0;
}

/* Test: method returning struct type (Vec<T>, not T) */
Vec<T> Vec<T>.clone(this) const {
    Vec<T> r;
    r.data = this->data;
    r.len = this->len;
    r.cap = this->cap;
    return r;
}

/* operator[]: subscript read (requires T to be indexable) */
T Vec<T>.operator[](this, long i) const {
    return this->data[i];
}

/* operator==: element-wise comparison (requires T supports ==) */
bool Vec<T>.operator==(this, Vec<T> other) const {
    if (this->len != other.len) return false;
    long i = 0;
    while (i < this->len) {
        if (!(this->data[i] == other.data[i])) return false;
        i = i + 1;
    }
    return true;
}

/*
 * PHASE 16 TEST: Standalone generic function with T as return type.
 * Syntax: <T> T first(Vec<T>* v)
 * The <T> prefix is parsed BEFORE the return type, so T is registered
 * as a generic param before parse_decl_specifiers runs for the return type.
 * Type inference: Vec<int>* → T=int → return type becomes int.
 */
<T> T first(Vec<T>* v) {
    return v->data[0];
}

int g_passed = 0;
int g_failed = 0;

void check(int cond, const char* msg) {
    if (cond) {
        g_passed = g_passed + 1;
    } else {
        g_failed = g_failed + 1;
    }
}

void test_vec_int() {
    int buf[10];
    Vec<int> v;
    v.data = buf;
    v.len = 0;
    v.cap = 10;

    check(v.size() == 0, "empty init");
    check(v.is_empty(), "is_empty");
    check(v.capacity() == 10, "capacity");

    v.push(10);
    v.push(20);
    v.push(30);
    v.push(40);
    v.push(50);

    check(v.size() == 5, "size after push");
    check(!v.is_empty(), "not empty");
    check(v.get(0) == 10, "get 0");
    check(v.get(4) == 50, "get 4");
    check(v.first() == 10, "first");
    check(v.last() == 50, "last");

    v.pop();
    check(v.size() == 4, "size after pop");
    check(v.last() == 40, "last after pop");

    v.set(0, 99);
    check(v.get(0) == 99, "set");

    v.clear();
    check(v.size() == 0, "size after clear");
    check(v.is_empty(), "is_empty after clear");

    /* operator[] read test */
    v.push(100);
    v.push(200);
    v.push(300);
    check(v[0] == 100, "op_idx [0]");
    check(v[1] == 200, "op_idx [1]");
    check(v[2] == 300, "op_idx [2]");
    int x = v[1];
    check(x == 200, "op_idx assign");
}

void test_vec_float() {
    float buf[8];
    Vec<float> v;
    v.data = buf;
    v.len = 0;
    v.cap = 8;

    v.push(1.5);
    v.push(2.5);
    v.push(3.5);

    check(v.size() == 3, "float size");
    check(v.first() == 1.5, "float first");
    check(v.last() == 3.5, "float last");
}

void test_vec_nested() {
    Vec<Vec<int>> vv;
    vv.data = (Vec<int>*)0;
    vv.len = 0;
    vv.cap = 5;

    check(vv.size() == 0, "nested size");
    check(vv.is_empty(), "nested is_empty");
    check(vv.capacity() == 5, "nested capacity");
}

void test_vec_clone() {
    int buf1[4];
    Vec<int> v1;
    v1.data = buf1;
    v1.len = 0;
    v1.cap = 4;

    v1.push(10);
    v1.push(20);

    Vec<int> v2 = v1.clone();
    check(v2.size() == 2, "clone size");
    check(v2.first() == 10, "clone first");
    check(v2.last() == 20, "clone last");
}

void test_phase16_standalone_return_t() {
    int buf[4];
    Vec<int> v;
    v.data = buf;
    v.len = 0;
    v.cap = 4;

    v.push(42);
    v.push(99);

    /* Phase 16: <T> T first(Vec<T>* v) — T resolved from Vec<int>* → T=int */
    int ires = first(&v);
    check(ires == 42, "phase16 standalone first int");
}

void test_phase16_standalone_return_t_float() {
    float buf2[4];
    Vec<float> vf;
    vf.data = buf2;
    vf.len = 0;
    vf.cap = 4;

    vf.push(314);
    vf.push(271);

    /* Phase 16: <T> T first(Vec<T>* v) — Vec<float>* → T=float */
    float fres = first(&vf);
    check(fres == 314, "phase16 standalone first float");
}

void test_auto_deduction() {
    auto x = 42;
    auto f = 3.14;
    auto p = &x;
    check(x == 42, "auto int");
    check(f == 3.14, "auto float");
    check(*p == 42, "auto pointer");
}

void test_chaining() {
    int buf[4];
    Vec<int> v;
    v.data = buf;
    v.len = 0;
    v.cap = 4;

    v.push(10);
    v.push(20);
    v.push(30);

    long s = v.size();
    long c = v.capacity();
    bool e = v.is_empty();
    check(s == 3, "chaining size");
    check(c == 4, "chaining capacity");
    check(!e, "chaining not empty");
}

void test_vec_operator_eq() {
    int buf1[4], buf2[4], buf3[4];
    Vec<int> v1;
    v1.data = buf1;
    v1.len = 0;
    v1.cap = 4;
    v1.push(10);
    v1.push(20);

    Vec<int> v2;
    v2.data = buf2;
    v2.len = 0;
    v2.cap = 4;
    v2.push(10);
    v2.push(20);

    Vec<int> v3;
    v3.data = buf3;
    v3.len = 0;
    v3.cap = 4;
    v3.push(10);
    v3.push(99);

    check(v1 == v2, "operator== equal");
    check(!(v1 == v3), "operator== not equal");
    check(v1.len != v3.len || v1.data[1] != v3.data[1], "operator== diff detect");
}

int main() {
    test_auto_deduction();
    test_chaining();
    test_vec_int();
    test_vec_float();
    test_vec_nested();
    test_vec_clone();
    test_phase16_standalone_return_t();
    test_phase16_standalone_return_t_float();
    test_vec_operator_eq();

    if (g_failed > 0) return 1;
    return 0;
}
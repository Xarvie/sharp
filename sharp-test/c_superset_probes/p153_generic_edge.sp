/*
 * p153_generic_edge.sp — 泛型边界测试
 * Tests: multi-param generics, deep nesting, generic-dependent expressions,
 *        generic structs with multiple type params, complex return type patterns.
 */
/* FIXED(e2e): multi-param generic associated function instantiation — AST_CAST collects operand */
#include <stdio.h>
#include <stdbool.h>

int g_passed = 0;
int g_failed = 0;

void check(int cond, const char* msg) {
    if (cond) { g_passed = g_passed + 1; printf("  PASS: %s\n", msg); }
    else { g_failed = g_failed + 1; printf("  FAIL: %s\n", msg); }
}

/* === 双类型参数泛型 Map<K, V> === */
class Map<K, V> {
    K key;
    V val;
}

Map<K, V> Map<K, V>.make(K k, V v) {
    Map<K, V> m;
    m.key = k;
    m.val = v;
    return m;
}

K Map<K, V>.get_key(this) const { return this->key; }
V Map<K, V>.get_val(this) const { return this->val; }

/* operator== only works on non-generic structs currently.
   Test value access methods instead. */
V Map<K, V>.value(this) const { return this->val; }

/* Method that uses both generic params */
bool Map<K, V>.key_equal(this, K other) const {
    return this->key == other;
}

/* === 三层嵌套泛型 === */
class Triple<T> {
    T value;
}

Triple<T> Triple<T>.wrap(T v) {
    Triple<T> t;
    t.value = v;
    return t;
}

T Triple<T>.unwrap(this) const { return this->value; }

/* === Container<Vec<T>> pattern === */
class Container<T> {
    T item;
}

void Container<T>.set(this, T t) { this->item = t; }
T Container<T>.get(this) const { return this->item; }

/* === 泛型中的 pointer 类型 === */
class Box<T> {
    T* ptr;
}

void Box<T>.set_ptr(this, T* p) { this->ptr = p; }
T Box<T>.get_deref(this) const { return *this->ptr; }

/* === 单一方法 struct，极限瘦 === */
class Id<T> {
    T value;
}

T Id<T>.get(this) const { return this->value; }

/* === 泛型 standalone 函数：双参数 === */
<K, V> K fst(Map<K, V>* m) { return m->key; }
<K, V> V snd(Map<K, V>* m) { return m->val; }
<T> T id(T x) { return x; }

void test_map_int_string() {
    Map<int, char*> m = Map<int, char*>.make(42, "hello");
    check(m.get_key() == 42, "Map<int,char*> key is 42");
    check(m.get_val() == "hello", "Map<int,char*> val is hello");
    check(m.value() == "hello", "Map<K,V>.value()");
    check(m.key_equal(42), "Map<K,V>.key_equal(42)");
    check(!m.key_equal(99), "Map<K,V>.key_equal(99) false");
}

void test_map_float_int() {
    Map<float, int> m = Map<float, int>.make(3.0f, 7);
    check((int)m.get_key() == 3, "Map<float,int> key 3");
    check(m.get_val() == 7, "Map<float,int> val 7");
}

void test_triple_nesting() {
    int val = 42;
    Triple<int> t = Triple<int>.wrap(val);
    check(t.unwrap() == 42, "Triple<int> unwrap");
}

void test_container_int() {
    Container<int> c;
    c.set(777);
    check(c.get() == 777, "Container<int> get");
}

void test_container_float() {
    Container<float> c;
    c.set(7.0f);
    check((int)c.get() == 7, "Container<float> get");
}

void test_box_int() {
    int x = 123;
    Box<int> b;
    b.set_ptr(&x);
    check(b.get_deref() == 123, "Box<int> get_deref");
}

void test_id_int() {
    Id<int> z;
    z.value = 55;
    check(z.get() == 55, "Id<int>.get");
}

void test_id_double() {
    Id<double> z;
    z.value = 7.5;
    check((int)z.get() == 7, "Id<double>.get");
}

void test_standalone_fst_snd() {
    Map<int, char*> m = Map<int, char*>.make(10, "ten");
    int k = fst<int>(&m);
    char* v = snd<int>(&m);
    check(k == 10, "fst<int>");
    check(v == "ten", "snd<int>");
}

void test_standalone_id() {
    int a = id<int>(999);
    float b = id<float>(7.0f);
    check(a == 999, "id<int>(999)");
    check((int)b == 7, "id<float>(7)");
}

int main() {
    printf("=== p153_generic_edge ===\n");

    test_map_int_string();
    test_map_float_int();
    test_triple_nesting();
    test_container_int();
    test_container_float();
    test_box_int();
    test_id_int();
    test_id_double();
    test_standalone_fst_snd();
    test_standalone_id();

    printf("  passed=%d  failed=%d\n", g_passed, g_failed);
    return g_failed;
}
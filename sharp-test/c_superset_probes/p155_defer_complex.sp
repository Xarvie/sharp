/*
 * p155_defer_complex.sp — defer 复杂场景测试
 * Tests: defer in loops, multiple defers in same scope, defer in nested scopes,
 *        defer returning value from function, defer with struct operations.
 */
#include <stdio.h>

int g_passed = 0;
int g_failed = 0;

void check(int cond, const char* msg) {
    if (cond) { g_passed = g_passed + 1; printf("  PASS: %s\n", msg); }
    else { g_failed = g_failed + 1; printf("  FAIL: %s\n", msg); }
}

/* === 单个 defer 清理资源 === */
void test_single_defer() {
    int count = 10;
    {
        defer { count = count - 1; }
        count = count + 5;
    }
    check(count == 14, "single defer: 10+5-1=14");
}

/* === 多个 defer，LIFO 执行顺序 === */
void test_multi_defer_lifo() {
    int x = 0;
    {
        defer { x = x * 10; }   /* 第三个执行 */
        defer { x = x + 3; }    /* 第二个执行 */
        defer { x = x + 2; }    /* 第一个执行 */
        x = 1;
    }
    /* 执行顺序: x+2 → 1+2=3 → x+3 → 3+3=6 → x*10 → 60 */
    check(x == 60, "multi defer LIFO: (1+2+3)*10=60");
}

/* === defer 在 if 分支中 === */
void test_defer_in_if() {
    int val = 0;
    {
        if (val == 0) {
            defer { val = 10; }
            val = 5;
        }
    }
    check(val == 10, "defer in if: val set to 10 after block");
}

/* === defer 在循环中，每次迭代都触发 === */
void test_defer_in_loop() {
    int sum = 0;
    {
        long i = 0;
        while (i < 5) {
            defer { sum = sum + 1; }
            i = i + 1;
        }
    }
    check(sum == 5, "defer in loop: each iter triggers defer");
}

/* === defer 在嵌套作用域中 === */
void test_defer_nested_scope() {
    int outer = 0;
    int inner = 0;
    {
        defer { outer = 1; }
        {
            defer { inner = 1; }
        }
        /* inner defer should have run, outer not yet */
        check(inner == 1, "defer nested: inner already fired");
    }
    check(outer == 1, "defer nested: outer fired after block");
}

/* === defer 与 return 交互 === */
int test_defer_with_return() {
    int val = 0;
    {
        defer { val = 100; }
        return val;
    }
    return val;
}

void test_defer_return() {
    int r = test_defer_with_return();
    /* defer runs at end of block, after return. But C semantics: 
       the defer moves after the body, before the closing brace.
       So the defer fires but return value is already captured. */
    check(r == 0, "defer with return: return value is 0 (before defer)");
}

/* === defer 修改外部结构体 === */
struct Counter {
    int value;
}

void test_defer_on_struct() {
    struct Counter c;
    c.value = 0;
    {
        defer { c.value = 42; }
        c.value = 10;
    }
    check(c.value == 42, "defer on struct: value becomes 42");
}

/* === 多个 defer 混合不同类型操作 === */
void test_defer_mixed() {
    int a = 0, b = 0, c = 0;
    {
        defer { c = 30; }
        defer { b = 20; }
        defer { a = 10; }
    }
    check(a == 10, "defer mixed: a=10");
    check(b == 20, "defer mixed: b=20");
    check(c == 30, "defer mixed: c=30");
}

int main() {
    printf("=== p155_defer_complex ===\n");

    test_single_defer();
    test_multi_defer_lifo();
    test_defer_in_if();
    test_defer_in_loop();
    test_defer_nested_scope();
    test_defer_return();
    test_defer_on_struct();
    test_defer_mixed();

    printf("  passed=%d  failed=%d\n", g_passed, g_failed);
    return g_failed;
}
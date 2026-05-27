// 来源: p274_string_literal.sp, p275_sizeof.sp
// C代码生成: 字符串字面量 + sizeof

#include <stdbool.h>

// === p274: string literal ===
int str_len(const char* s) {
    int n = 0;
    while (s[n] != 0) n = n + 1;
    return n;
}
int str_eq(const char* a, const char* b) {
    int i = 0;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i]) return 0;
        i = i + 1;
    }
    return a[i] == b[i];
}

// === p275: sizeof ===
class Vec4 {
    int x;
    int y;
    int z;
    int w;
};
typedef int IntArr[10];

int main() {
    // --- p274 ---
    if (str_len("hello") != 5) return 1;
    if (str_len("") != 0) return 2;
    if (!str_eq("abc", "abc")) return 3;
    if (str_eq("abc", "abd")) return 4;
    if (str_eq("ab", "abc")) return 5;
    const char* null_str = (const char*)0;
    if (null_str != (const char*)0) return 6;

    // --- p275 ---
    if (sizeof(int) != 4)  return 10;
    if (sizeof(Vec4) != 16) return 11;
    if (sizeof(Vec4*) != 8) return 12;
    int arr[5] = {0};
    if (sizeof(arr) != 20) return 13;
    if (sizeof(IntArr) != 40) return 14;
    if (sizeof(Vec4) / sizeof(int) != 4) return 15;

    return 0;
}
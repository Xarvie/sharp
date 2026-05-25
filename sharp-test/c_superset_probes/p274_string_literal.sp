/* p274 — String literal usage patterns
 *
 * Tests various string literal patterns compatible with C string handling.
 */

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

int main() {
    /* Basic string literal */
    if (str_len("hello") != 5) return 1;
    if (str_len("") != 0) return 2;

    /* Compare */
    if (!str_eq("abc", "abc")) return 3;
    if (str_eq("abc", "abd")) return 4;
    if (str_eq("ab", "abc")) return 5;

    /* Null check */
    const char* null_str = (const char*)0;
    if (null_str != (const char*)0) return 6;

    return 0;
}
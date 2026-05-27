
#include <stdlib.h>

#line 22 "unit/codegen/string_sizeof.sp"
typedef struct Vec4 Vec4;
struct Vec4 {
    int x;
    int y;
    int z;
    int w;
};

#line 28 "unit/codegen/string_sizeof.sp"
typedef int IntArr[10];

#line 7 "unit/codegen/string_sizeof.sp"
int str_len(const char * s) {

#line 8 "unit/codegen/string_sizeof.sp"
    int n = 0;

#line 9 "unit/codegen/string_sizeof.sp"
    while (s[n] != 0) 
        n = n + 1;

#line 10 "unit/codegen/string_sizeof.sp"
    return n;
}

#line 12 "unit/codegen/string_sizeof.sp"
int str_eq(const char * a, const char * b) {

#line 13 "unit/codegen/string_sizeof.sp"
    int i = 0;

#line 14 "unit/codegen/string_sizeof.sp"
    while (a[i] != 0 && b[i] != 0) {

#line 15 "unit/codegen/string_sizeof.sp"
        if (a[i] != b[i]) 
            return 0;

#line 16 "unit/codegen/string_sizeof.sp"
        i = i + 1;
    }

#line 18 "unit/codegen/string_sizeof.sp"
    return a[i] == b[i];
}

#line 30 "unit/codegen/string_sizeof.sp"
int main() {

#line 32 "unit/codegen/string_sizeof.sp"
    if (str_len("hello") != 5) 
        return 1;

#line 33 "unit/codegen/string_sizeof.sp"
    if (str_len("") != 0) 
        return 2;

#line 34 "unit/codegen/string_sizeof.sp"
    if (!str_eq("abc", "abc")) 
        return 3;

#line 35 "unit/codegen/string_sizeof.sp"
    if (str_eq("abc", "abd")) 
        return 4;

#line 36 "unit/codegen/string_sizeof.sp"
    if (str_eq("ab", "abc")) 
        return 5;

#line 37 "unit/codegen/string_sizeof.sp"
    const char * null_str = (const char *)0;

#line 38 "unit/codegen/string_sizeof.sp"
    if (null_str != (const char *)0) 
        return 6;

#line 41 "unit/codegen/string_sizeof.sp"
    if (sizeof(int) != 4) 
        return 10;

#line 42 "unit/codegen/string_sizeof.sp"
    if (sizeof(Vec4) != 16) 
        return 11;

#line 43 "unit/codegen/string_sizeof.sp"
    if (sizeof(Vec4 *) != 8) 
        return 12;

#line 44 "unit/codegen/string_sizeof.sp"
    int arr[5] = { 0 };

#line 45 "unit/codegen/string_sizeof.sp"
    if (sizeof(arr) != 20) 
        return 13;

#line 46 "unit/codegen/string_sizeof.sp"
    if (sizeof(IntArr) != 40) 
        return 14;

#line 47 "unit/codegen/string_sizeof.sp"
    if (sizeof(Vec4) / sizeof(int) != 4) 
        return 15;

#line 49 "unit/codegen/string_sizeof.sp"
    return 0;
}


#line 22 "sharp-test/unit/codegen/string_sizeof.sp"
typedef struct Vec4 Vec4;
struct Vec4 {
    int x;
    int y;
    int z;
    int w;
};

#line 28 "sharp-test/unit/codegen/string_sizeof.sp"
typedef int IntArr[10];

#line 7 "sharp-test/unit/codegen/string_sizeof.sp"
int str_len(const char * s) {

#line 8 "sharp-test/unit/codegen/string_sizeof.sp"
    int n = 0;

#line 9 "sharp-test/unit/codegen/string_sizeof.sp"
    while (s[n] != 0) 
        n = n + 1;

#line 10 "sharp-test/unit/codegen/string_sizeof.sp"
    return n;
}

#line 12 "sharp-test/unit/codegen/string_sizeof.sp"
int str_eq(const char * a, const char * b) {

#line 13 "sharp-test/unit/codegen/string_sizeof.sp"
    int i = 0;

#line 14 "sharp-test/unit/codegen/string_sizeof.sp"
    while (a[i] != 0 && b[i] != 0) {

#line 15 "sharp-test/unit/codegen/string_sizeof.sp"
        if (a[i] != b[i]) 
            return 0;

#line 16 "sharp-test/unit/codegen/string_sizeof.sp"
        i = i + 1;
    }

#line 18 "sharp-test/unit/codegen/string_sizeof.sp"
    return a[i] == b[i];
}

#line 30 "sharp-test/unit/codegen/string_sizeof.sp"
int main() {

#line 32 "sharp-test/unit/codegen/string_sizeof.sp"
    if (str_len("hello") != 5) 
        return 1;

#line 33 "sharp-test/unit/codegen/string_sizeof.sp"
    if (str_len("") != 0) 
        return 2;

#line 34 "sharp-test/unit/codegen/string_sizeof.sp"
    if (!str_eq("abc", "abc")) 
        return 3;

#line 35 "sharp-test/unit/codegen/string_sizeof.sp"
    if (str_eq("abc", "abd")) 
        return 4;

#line 36 "sharp-test/unit/codegen/string_sizeof.sp"
    if (str_eq("ab", "abc")) 
        return 5;

#line 37 "sharp-test/unit/codegen/string_sizeof.sp"
    const char * null_str = (const char *)0;

#line 38 "sharp-test/unit/codegen/string_sizeof.sp"
    if (null_str != (const char *)0) 
        return 6;

#line 41 "sharp-test/unit/codegen/string_sizeof.sp"
    if (sizeof(int) != 4) 
        return 10;

#line 42 "sharp-test/unit/codegen/string_sizeof.sp"
    if (sizeof(Vec4) != 16) 
        return 11;

#line 43 "sharp-test/unit/codegen/string_sizeof.sp"
    if (sizeof(Vec4 *) != 8) 
        return 12;

#line 44 "sharp-test/unit/codegen/string_sizeof.sp"
    int arr[5] = { 0 };

#line 45 "sharp-test/unit/codegen/string_sizeof.sp"
    if (sizeof(arr) != 20) 
        return 13;

#line 46 "sharp-test/unit/codegen/string_sizeof.sp"
    if (sizeof(IntArr) != 40) 
        return 14;

#line 47 "sharp-test/unit/codegen/string_sizeof.sp"
    if (sizeof(Vec4) / sizeof(int) != 4) 
        return 15;

#line 49 "sharp-test/unit/codegen/string_sizeof.sp"
    return 0;
}

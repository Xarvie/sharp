
#line 22 "sharp-test/unit/codegen/string_sizeof.ce"
typedef struct Vec4 Vec4;
struct Vec4 {
    int x;
    int y;
    int z;
    int w;
};

#line 28 "sharp-test/unit/codegen/string_sizeof.ce"
typedef int IntArr[10];

#line 7 "sharp-test/unit/codegen/string_sizeof.ce"
int str_len(const char * s) {

#line 8 "sharp-test/unit/codegen/string_sizeof.ce"
    int n = 0;

#line 9 "sharp-test/unit/codegen/string_sizeof.ce"
    while (s[n] != 0) 
        n = n + 1;

#line 10 "sharp-test/unit/codegen/string_sizeof.ce"
    return n;
}

#line 12 "sharp-test/unit/codegen/string_sizeof.ce"
int str_eq(const char * a, const char * b) {

#line 13 "sharp-test/unit/codegen/string_sizeof.ce"
    int i = 0;

#line 14 "sharp-test/unit/codegen/string_sizeof.ce"
    while (a[i] != 0 && b[i] != 0) {

#line 15 "sharp-test/unit/codegen/string_sizeof.ce"
        if (a[i] != b[i]) 
            return 0;

#line 16 "sharp-test/unit/codegen/string_sizeof.ce"
        i = i + 1;
    }

#line 18 "sharp-test/unit/codegen/string_sizeof.ce"
    return a[i] == b[i];
}

#line 30 "sharp-test/unit/codegen/string_sizeof.ce"
int main() {

#line 32 "sharp-test/unit/codegen/string_sizeof.ce"
    if (str_len("hello") != 5) 
        return 1;

#line 33 "sharp-test/unit/codegen/string_sizeof.ce"
    if (str_len("") != 0) 
        return 2;

#line 34 "sharp-test/unit/codegen/string_sizeof.ce"
    if (!str_eq("abc", "abc")) 
        return 3;

#line 35 "sharp-test/unit/codegen/string_sizeof.ce"
    if (str_eq("abc", "abd")) 
        return 4;

#line 36 "sharp-test/unit/codegen/string_sizeof.ce"
    if (str_eq("ab", "abc")) 
        return 5;

#line 37 "sharp-test/unit/codegen/string_sizeof.ce"
    const char * null_str = (const char *)0;

#line 38 "sharp-test/unit/codegen/string_sizeof.ce"
    if (null_str != (const char *)0) 
        return 6;

#line 41 "sharp-test/unit/codegen/string_sizeof.ce"
    if (sizeof(int) != 4) 
        return 10;

#line 42 "sharp-test/unit/codegen/string_sizeof.ce"
    if (sizeof(Vec4) != 16) 
        return 11;

#line 43 "sharp-test/unit/codegen/string_sizeof.ce"
    if (sizeof(Vec4 *) != 8) 
        return 12;

#line 44 "sharp-test/unit/codegen/string_sizeof.ce"
    int arr[5] = { 0 };

#line 45 "sharp-test/unit/codegen/string_sizeof.ce"
    if (sizeof(arr) != 20) 
        return 13;

#line 46 "sharp-test/unit/codegen/string_sizeof.ce"
    if (sizeof(IntArr) != 40) 
        return 14;

#line 47 "sharp-test/unit/codegen/string_sizeof.ce"
    if (sizeof(Vec4) / sizeof(int) != 4) 
        return 15;

#line 50 "sharp-test/unit/codegen/string_sizeof.ce"
    typedef struct Outer Outer;
struct Outer {
    Vec4 inner;
    int flag;
};


#line 54 "sharp-test/unit/codegen/string_sizeof.ce"
    if (sizeof(Outer) != 20) 
        return 16;

#line 55 "sharp-test/unit/codegen/string_sizeof.ce"
    if (sizeof(Outer) - sizeof(Vec4) != 4) 
        return 17;

#line 58 "sharp-test/unit/codegen/string_sizeof.ce"
    const char * msg = "sharp";

#line 59 "sharp-test/unit/codegen/string_sizeof.ce"
    int slen = 0;

#line 60 "sharp-test/unit/codegen/string_sizeof.ce"
    while (msg[slen] != 0) 
        slen = slen + 1;

#line 61 "sharp-test/unit/codegen/string_sizeof.ce"
    if (slen != 5) 
        return 18;

#line 62 "sharp-test/unit/codegen/string_sizeof.ce"
    if (msg[0] != 's') 
        return 19;

#line 63 "sharp-test/unit/codegen/string_sizeof.ce"
    if (msg[4] != 'p') 
        return 20;

#line 65 "sharp-test/unit/codegen/string_sizeof.ce"
    return 0;
}

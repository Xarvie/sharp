
#line 69 "sharp-test/unit/macros/macro_advanced.ce"
int main(void) {

#line 70 "sharp-test/unit/macros/macro_advanced.ce"
    int result = 0;

#line 77 "sharp-test/unit/macros/macro_advanced.ce"
    if (((5) * (5)) != 25) 
        return 1;

#line 82 "sharp-test/unit/macros/macro_advanced.ce"
    if (((((3) * (3))) + (((4) * (4)))) != 25) 
        return 2;

#line 85 "sharp-test/unit/macros/macro_advanced.ce"
    if (((7) + (7)) != 14) 
        return 3;

#line 91 "sharp-test/unit/macros/macro_advanced.ce"
    if (((((3) + (3))) + (((3) + (3)))) != 12) 
        return 4;

#line 98 "sharp-test/unit/macros/macro_advanced.ce"
    int var1 = 100;

#line 99 "sharp-test/unit/macros/macro_advanced.ce"
    if (var1 != 100) 
        return 5;

#line 102 "sharp-test/unit/macros/macro_advanced.ce"
    int var2 = 200;

#line 103 "sharp-test/unit/macros/macro_advanced.ce"
    if (var2 != 200) 
        return 6;

#line 106 "sharp-test/unit/macros/macro_advanced.ce"
    int tmp3 = 300;

#line 107 "sharp-test/unit/macros/macro_advanced.ce"
    if (tmp3 != 300) 
        return 7;

#line 110 "sharp-test/unit/macros/macro_advanced.ce"
    int my_func = 42;

#line 111 "sharp-test/unit/macros/macro_advanced.ce"
    if (my_func != 42) 
        return 8;

#line 115 "sharp-test/unit/macros/macro_advanced.ce"
    int xyz = 999;

#line 116 "sharp-test/unit/macros/macro_advanced.ce"
    if (xyz != 999) 
        return 9;

#line 123 "sharp-test/unit/macros/macro_advanced.ce"
    const char * s1 = "hello";

#line 124 "sharp-test/unit/macros/macro_advanced.ce"
    if (s1[0] != 'h' || s1[4] != 'o' || s1[5] != '\0') 
        return 10;

#line 127 "sharp-test/unit/macros/macro_advanced.ce"
    const char * s2 = "a+b";

#line 128 "sharp-test/unit/macros/macro_advanced.ce"
    if (s2[0] != 'a' || s2[1] != '+' || s2[2] != 'b') 
        return 11;

#line 131 "sharp-test/unit/macros/macro_advanced.ce"
    const char * s3 = "3";

#line 132 "sharp-test/unit/macros/macro_advanced.ce"
    if (s3[0] != '3' || s3[1] != '\0') 
        return 12;

#line 138 "sharp-test/unit/macros/macro_advanced.ce"
    result = 0;

#line 143 "sharp-test/unit/macros/macro_advanced.ce"
    result = result + 10;

#line 150 "sharp-test/unit/macros/macro_advanced.ce"
    if (result != 10) 
        return 13;

#line 153 "sharp-test/unit/macros/macro_advanced.ce"
    result = result + 1;

#line 159 "sharp-test/unit/macros/macro_advanced.ce"
    if (result != 11) 
        return 14;

#line 162 "sharp-test/unit/macros/macro_advanced.ce"
    result = result + 1;

#line 164 "sharp-test/unit/macros/macro_advanced.ce"
    if (result != 12) 
        return 15;

#line 167 "sharp-test/unit/macros/macro_advanced.ce"
    result = result + 1;

#line 169 "sharp-test/unit/macros/macro_advanced.ce"
    if (result != 13) 
        return 16;

#line 172 "sharp-test/unit/macros/macro_advanced.ce"
    result = result + 1;

#line 174 "sharp-test/unit/macros/macro_advanced.ce"
    if (result != 14) 
        return 17;

#line 177 "sharp-test/unit/macros/macro_advanced.ce"
    result = result + 1;

#line 179 "sharp-test/unit/macros/macro_advanced.ce"
    if (result != 15) 
        return 18;

#line 3 "sharp-test/unit/macros/macro_advanced_once.h"
    int pragma_once_val = 77;

#line 189 "sharp-test/unit/macros/macro_advanced.ce"
    if (pragma_once_val != 77) 
        return 19;

#line 196 "sharp-test/unit/macros/macro_advanced.ce"
{

#line 197 "sharp-test/unit/macros/macro_advanced.ce"
        int arr[3] = { 10, 20, 30 };

#line 198 "sharp-test/unit/macros/macro_advanced.ce"
        if (arr[0] != 10) 
            return 20;

#line 199 "sharp-test/unit/macros/macro_advanced.ce"
        if (arr[1] != 20) 
            return 21;

#line 200 "sharp-test/unit/macros/macro_advanced.ce"
        if (arr[2] != 30) 
            return 22;
    }

#line 204 "sharp-test/unit/macros/macro_advanced.ce"
    if (3 != 3) 
        return 23;

#line 207 "sharp-test/unit/macros/macro_advanced.ce"
    if (1 != 1) 
        return 24;

#line 210 "sharp-test/unit/macros/macro_advanced.ce"
    if (5 != 5) 
        return 25;

#line 213 "sharp-test/unit/macros/macro_advanced.ce"
{

#line 214 "sharp-test/unit/macros/macro_advanced.ce"
        int vw = 42;

#line 215 "sharp-test/unit/macros/macro_advanced.ce"
        if (vw != 42) 
            return 26;
    }

#line 219 "sharp-test/unit/macros/macro_advanced.ce"
{
        int a = 1, b = 2;

#line 221 "sharp-test/unit/macros/macro_advanced.ce"
        int vw = a + b;

#line 222 "sharp-test/unit/macros/macro_advanced.ce"
        if (vw != 3) 
            return 27;
    }

#line 230 "sharp-test/unit/macros/macro_advanced.ce"
    if (20 != 20) 
        return 28;

#line 234 "sharp-test/unit/macros/macro_advanced.ce"
    if (555 != 555) 
        return 29;

#line 238 "sharp-test/unit/macros/macro_advanced.ce"
{

#line 239 "sharp-test/unit/macros/macro_advanced.ce"
        int temp_check = 0;

#line 243 "sharp-test/unit/macros/macro_advanced.ce"
        temp_check = 2;

#line 245 "sharp-test/unit/macros/macro_advanced.ce"
        if (temp_check != 2) 
            return 30;
    }

#line 251 "sharp-test/unit/macros/macro_advanced.ce"
    if (100 != 100) 
        return 31;

#line 260 "sharp-test/unit/macros/macro_advanced.ce"
    if (2 != 2) 
        return 32;

#line 262 "sharp-test/unit/macros/macro_advanced.ce"
    return 0;
}


#include <stdlib.h>

#line 81 "unit/defer/complex.sp"
typedef struct Counter Counter;
struct Counter {
    int value;
};

#line 5 "unit/defer/complex.sp"
void test_single_defer() {

#line 6 "unit/defer/complex.sp"
    int count = 10;

#line 7 "unit/defer/complex.sp"
{

#line 9 "unit/defer/complex.sp"
        count = count + 5;
        
#line 8 "unit/defer/complex.sp"
{
            count = count - 1;
        }
    }

#line 11 "unit/defer/complex.sp"
    if (count != 14) 
        return;
}

#line 15 "unit/defer/complex.sp"
void test_multi_defer_lifo() {

#line 16 "unit/defer/complex.sp"
    int x = 0;

#line 17 "unit/defer/complex.sp"
{

#line 21 "unit/defer/complex.sp"
        x = 1;
        
#line 20 "unit/defer/complex.sp"
{
            x = x + 2;
        }
        
#line 19 "unit/defer/complex.sp"
{
            x = x + 3;
        }
        
#line 18 "unit/defer/complex.sp"
{
            x = x * 10;
        }
    }

#line 23 "unit/defer/complex.sp"
    if (x != 60) 
        return;
}

#line 27 "unit/defer/complex.sp"
void test_defer_in_if() {

#line 28 "unit/defer/complex.sp"
    int val = 0;

#line 29 "unit/defer/complex.sp"
{

#line 30 "unit/defer/complex.sp"
        if (val == 0) {

#line 32 "unit/defer/complex.sp"
            val = 5;
            
#line 31 "unit/defer/complex.sp"
{
                val = 10;
            }
        }
    }

#line 35 "unit/defer/complex.sp"
    if (val != 10) 
        return;
}

#line 39 "unit/defer/complex.sp"
void test_defer_in_loop() {

#line 40 "unit/defer/complex.sp"
    int sum = 0;

#line 41 "unit/defer/complex.sp"
{

#line 42 "unit/defer/complex.sp"
        long i = 0;

#line 43 "unit/defer/complex.sp"
        while (i < 5) {

#line 45 "unit/defer/complex.sp"
            i = i + 1;
            
#line 44 "unit/defer/complex.sp"
{
                sum = sum + 1;
            }
        }
    }

#line 48 "unit/defer/complex.sp"
    if (sum != 5) 
        return;
}

#line 52 "unit/defer/complex.sp"
void test_defer_nested_scope() {

#line 53 "unit/defer/complex.sp"
    int outer = 0;

#line 54 "unit/defer/complex.sp"
    int inner = 0;

#line 55 "unit/defer/complex.sp"
{

#line 57 "unit/defer/complex.sp"
{
            
#line 58 "unit/defer/complex.sp"
{
                inner = 1;
            }
        }

#line 60 "unit/defer/complex.sp"
        if (inner != 1) {
            
#line 56 "unit/defer/complex.sp"
{
                outer = 1;
            }
            return;
        }
        {
            outer = 1;
        }
    }

#line 62 "unit/defer/complex.sp"
    if (outer != 1) 
        return;
}

#line 66 "unit/defer/complex.sp"
int test_defer_with_return() {

#line 67 "unit/defer/complex.sp"
    int val = 0;

#line 68 "unit/defer/complex.sp"
{

#line 70 "unit/defer/complex.sp"
        int __sharp_ret = val;
        
#line 69 "unit/defer/complex.sp"
{
            val = 100;
        }
        return __sharp_ret;
    }

#line 72 "unit/defer/complex.sp"
    return val;
}

#line 75 "unit/defer/complex.sp"
void test_defer_return() {

#line 76 "unit/defer/complex.sp"
    int r = test_defer_with_return();

#line 77 "unit/defer/complex.sp"
    if (r != 0) 
        return;
}

#line 85 "unit/defer/complex.sp"
void test_defer_on_struct() {

#line 86 "unit/defer/complex.sp"
    Counter c;

#line 87 "unit/defer/complex.sp"
    c.value = 0;

#line 88 "unit/defer/complex.sp"
{

#line 90 "unit/defer/complex.sp"
        c.value = 10;
        
#line 89 "unit/defer/complex.sp"
{
            c.value = 42;
        }
    }

#line 92 "unit/defer/complex.sp"
    if (c.value != 42) 
        return;
}

#line 96 "unit/defer/complex.sp"
void test_defer_mixed() {
    int a = 0, b = 0, c = 0;

#line 98 "unit/defer/complex.sp"
{
        
#line 101 "unit/defer/complex.sp"
{
            a = 10;
        }
        
#line 100 "unit/defer/complex.sp"
{
            b = 20;
        }
        
#line 99 "unit/defer/complex.sp"
{
            c = 30;
        }
    }

#line 103 "unit/defer/complex.sp"
    if (a != 10) 
        return;

#line 104 "unit/defer/complex.sp"
    if (b != 20) 
        return;

#line 105 "unit/defer/complex.sp"
    if (c != 30) 
        return;
}

#line 108 "unit/defer/complex.sp"
int main() {

#line 109 "unit/defer/complex.sp"
    test_single_defer();

#line 110 "unit/defer/complex.sp"
    test_multi_defer_lifo();

#line 111 "unit/defer/complex.sp"
    test_defer_in_if();

#line 112 "unit/defer/complex.sp"
    test_defer_in_loop();

#line 113 "unit/defer/complex.sp"
    test_defer_nested_scope();

#line 114 "unit/defer/complex.sp"
    test_defer_return();

#line 115 "unit/defer/complex.sp"
    test_defer_on_struct();

#line 116 "unit/defer/complex.sp"
    test_defer_mixed();

#line 118 "unit/defer/complex.sp"
    return 0;
}

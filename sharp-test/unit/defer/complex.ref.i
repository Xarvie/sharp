
#line 81 "sharp-test/unit/defer/complex.ce"
typedef struct Counter Counter;
struct Counter {
    int value;
};

#line 5 "sharp-test/unit/defer/complex.ce"
void test_single_defer() {

#line 6 "sharp-test/unit/defer/complex.ce"
    int count = 10;

#line 7 "sharp-test/unit/defer/complex.ce"
{

#line 9 "sharp-test/unit/defer/complex.ce"
        count = count + 5;
        
#line 8 "sharp-test/unit/defer/complex.ce"
{
            count = count - 1;
        }
    }

#line 11 "sharp-test/unit/defer/complex.ce"
    if (count != 14) 
        return;
}

#line 15 "sharp-test/unit/defer/complex.ce"
void test_multi_defer_lifo() {

#line 16 "sharp-test/unit/defer/complex.ce"
    int x = 0;

#line 17 "sharp-test/unit/defer/complex.ce"
{

#line 21 "sharp-test/unit/defer/complex.ce"
        x = 1;
        
#line 20 "sharp-test/unit/defer/complex.ce"
{
            x = x + 2;
        }
        
#line 19 "sharp-test/unit/defer/complex.ce"
{
            x = x + 3;
        }
        
#line 18 "sharp-test/unit/defer/complex.ce"
{
            x = x * 10;
        }
    }

#line 23 "sharp-test/unit/defer/complex.ce"
    if (x != 60) 
        return;
}

#line 27 "sharp-test/unit/defer/complex.ce"
void test_defer_in_if() {

#line 28 "sharp-test/unit/defer/complex.ce"
    int val = 0;

#line 29 "sharp-test/unit/defer/complex.ce"
{

#line 30 "sharp-test/unit/defer/complex.ce"
        if (val == 0) {

#line 32 "sharp-test/unit/defer/complex.ce"
            val = 5;
            
#line 31 "sharp-test/unit/defer/complex.ce"
{
                val = 10;
            }
        }
    }

#line 35 "sharp-test/unit/defer/complex.ce"
    if (val != 10) 
        return;
}

#line 39 "sharp-test/unit/defer/complex.ce"
void test_defer_in_loop() {

#line 40 "sharp-test/unit/defer/complex.ce"
    int sum = 0;

#line 41 "sharp-test/unit/defer/complex.ce"
{

#line 42 "sharp-test/unit/defer/complex.ce"
        long i = 0;

#line 43 "sharp-test/unit/defer/complex.ce"
        while (i < 5) {

#line 45 "sharp-test/unit/defer/complex.ce"
            i = i + 1;
            
#line 44 "sharp-test/unit/defer/complex.ce"
{
                sum = sum + 1;
            }
        }
    }

#line 48 "sharp-test/unit/defer/complex.ce"
    if (sum != 5) 
        return;
}

#line 52 "sharp-test/unit/defer/complex.ce"
void test_defer_nested_scope() {

#line 53 "sharp-test/unit/defer/complex.ce"
    int outer = 0;

#line 54 "sharp-test/unit/defer/complex.ce"
    int inner = 0;

#line 55 "sharp-test/unit/defer/complex.ce"
{

#line 57 "sharp-test/unit/defer/complex.ce"
{
            
#line 58 "sharp-test/unit/defer/complex.ce"
{
                inner = 1;
            }
        }

#line 60 "sharp-test/unit/defer/complex.ce"
        if (inner != 1) {
            
#line 56 "sharp-test/unit/defer/complex.ce"
{
                outer = 1;
            }
            return;
        }
        {
            outer = 1;
        }
    }

#line 62 "sharp-test/unit/defer/complex.ce"
    if (outer != 1) 
        return;
}

#line 66 "sharp-test/unit/defer/complex.ce"
int test_defer_with_return() {

#line 67 "sharp-test/unit/defer/complex.ce"
    int val = 0;

#line 68 "sharp-test/unit/defer/complex.ce"
{

#line 70 "sharp-test/unit/defer/complex.ce"
        int __sharp_ret = val;
        
#line 69 "sharp-test/unit/defer/complex.ce"
{
            val = 100;
        }
        return __sharp_ret;
    }

#line 72 "sharp-test/unit/defer/complex.ce"
    return val;
}

#line 75 "sharp-test/unit/defer/complex.ce"
void test_defer_return() {

#line 76 "sharp-test/unit/defer/complex.ce"
    int r = test_defer_with_return();

#line 77 "sharp-test/unit/defer/complex.ce"
    if (r != 0) 
        return;
}

#line 85 "sharp-test/unit/defer/complex.ce"
void test_defer_on_struct() {

#line 86 "sharp-test/unit/defer/complex.ce"
    Counter c;

#line 87 "sharp-test/unit/defer/complex.ce"
    c.value = 0;

#line 88 "sharp-test/unit/defer/complex.ce"
{

#line 90 "sharp-test/unit/defer/complex.ce"
        c.value = 10;
        
#line 89 "sharp-test/unit/defer/complex.ce"
{
            c.value = 42;
        }
    }

#line 92 "sharp-test/unit/defer/complex.ce"
    if (c.value != 42) 
        return;
}

#line 96 "sharp-test/unit/defer/complex.ce"
void test_defer_mixed() {
    int a = 0, b = 0, c = 0;

#line 98 "sharp-test/unit/defer/complex.ce"
{
        
#line 101 "sharp-test/unit/defer/complex.ce"
{
            a = 10;
        }
        
#line 100 "sharp-test/unit/defer/complex.ce"
{
            b = 20;
        }
        
#line 99 "sharp-test/unit/defer/complex.ce"
{
            c = 30;
        }
    }

#line 103 "sharp-test/unit/defer/complex.ce"
    if (a != 10) 
        return;

#line 104 "sharp-test/unit/defer/complex.ce"
    if (b != 20) 
        return;

#line 105 "sharp-test/unit/defer/complex.ce"
    if (c != 30) 
        return;
}

#line 108 "sharp-test/unit/defer/complex.ce"
int error_handler(int err) {

#line 109 "sharp-test/unit/defer/complex.ce"
    int cleanup = 0;

#line 110 "sharp-test/unit/defer/complex.ce"
{

#line 112 "sharp-test/unit/defer/complex.ce"
        if (err != 0) {
            int __sharp_ret = err;
            
#line 111 "sharp-test/unit/defer/complex.ce"
{
                cleanup = 1;
            }
            return __sharp_ret;
        }
        {
            cleanup = 1;
        }
    }

#line 114 "sharp-test/unit/defer/complex.ce"
    if (cleanup != 1) 
        return 0;

#line 115 "sharp-test/unit/defer/complex.ce"
    return 0;
}

#line 118 "sharp-test/unit/defer/complex.ce"
void test_error_handler() {

#line 119 "sharp-test/unit/defer/complex.ce"
    int r1 = error_handler(0);

#line 120 "sharp-test/unit/defer/complex.ce"
    if (r1 != 0) 
        return;

#line 121 "sharp-test/unit/defer/complex.ce"
    int r2 = error_handler(42);

#line 122 "sharp-test/unit/defer/complex.ce"
    if (r2 != 42) 
        return;
}

#line 125 "sharp-test/unit/defer/complex.ce"
void test_defer_array() {

#line 126 "sharp-test/unit/defer/complex.ce"
    int arr[3] = { 0, 0, 0 };

#line 127 "sharp-test/unit/defer/complex.ce"
{
        
#line 130 "sharp-test/unit/defer/complex.ce"
{
            arr[0] = 10;
        }
        
#line 129 "sharp-test/unit/defer/complex.ce"
{
            arr[1] = 20;
        }
        
#line 128 "sharp-test/unit/defer/complex.ce"
{
            arr[2] = 30;
        }
    }

#line 132 "sharp-test/unit/defer/complex.ce"
    if (arr[0] != 10) 
        return;

#line 133 "sharp-test/unit/defer/complex.ce"
    if (arr[1] != 20) 
        return;

#line 134 "sharp-test/unit/defer/complex.ce"
    if (arr[2] != 30) 
        return;
}

#line 137 "sharp-test/unit/defer/complex.ce"
int main() {

#line 138 "sharp-test/unit/defer/complex.ce"
    test_single_defer();

#line 139 "sharp-test/unit/defer/complex.ce"
    test_multi_defer_lifo();

#line 140 "sharp-test/unit/defer/complex.ce"
    test_defer_in_if();

#line 141 "sharp-test/unit/defer/complex.ce"
    test_defer_in_loop();

#line 142 "sharp-test/unit/defer/complex.ce"
    test_defer_nested_scope();

#line 143 "sharp-test/unit/defer/complex.ce"
    test_defer_return();

#line 144 "sharp-test/unit/defer/complex.ce"
    test_defer_on_struct();

#line 145 "sharp-test/unit/defer/complex.ce"
    test_defer_mixed();

#line 146 "sharp-test/unit/defer/complex.ce"
    test_error_handler();

#line 147 "sharp-test/unit/defer/complex.ce"
    test_defer_array();

#line 149 "sharp-test/unit/defer/complex.ce"
    return 0;
}

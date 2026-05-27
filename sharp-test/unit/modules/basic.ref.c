#include <stdio.h>
#include <__stddef_size_t.h>
#include <__stdarg___gnuc_va_list.h>

#include <stdlib.h>

#line 8 "unit/modules/basic.sph"
struct Point2D {
    float x;
    float y;
};

#line 9 "unit/modules/basic.sph"
struct Rectangle {
    float width;
    float height;
    struct Point2D origin;
};

#line 11 "unit/modules/basic.sph"
typedef int Int32;

#line 12 "unit/modules/basic.sph"
typedef float Float32;

#line 14 "unit/modules/basic.sph"
const double PI = 3.14159265358979323846;

#line 15 "unit/modules/basic.sph"
int VERSION = 1;

#line 17 "unit/modules/basic.sph"
enum Color {
    RED,
    GREEN,
    BLUE
};

#line 18 "unit/modules/basic.sph"
enum Status {
    OK = 0,
    ERROR = -1,
    PENDING = 1
};

#line 19 "unit/modules/basic.sph"
enum Status DEFAULT_STATUS = OK;

#line 4 "unit/modules/basic.sph"
int add(int a, int b) {
    return a + b;
}

#line 5 "unit/modules/basic.sph"
float multiply(float x, float y) {
    return x * y;
}

#line 6 "unit/modules/basic.sph"
void greet(char * name) {
    (void)name;
}

#line 7 "unit/modules/basic.sp"
int main(void) {

#line 8 "unit/modules/basic.sp"
    int sum = add(10, 20);

#line 9 "unit/modules/basic.sp"
    float product = multiply(3.0f, 4.0f);

#line 10 "unit/modules/basic.sp"
    greet((char *)"World");

#line 12 "unit/modules/basic.sp"
    struct Point2D pt;

#line 13 "unit/modules/basic.sp"
    pt.x = 1.5f;

#line 14 "unit/modules/basic.sp"
    pt.y = 2.5f;

#line 16 "unit/modules/basic.sp"
    struct Rectangle rect;

#line 17 "unit/modules/basic.sp"
    rect.width = 10.0f;

#line 18 "unit/modules/basic.sp"
    rect.height = 20.0f;

#line 19 "unit/modules/basic.sp"
    rect.origin = pt;

#line 21 "unit/modules/basic.sp"
    Int32 value = 42;

#line 22 "unit/modules/basic.sp"
    Float32 fvalue = 3.14f;

#line 24 "unit/modules/basic.sp"
    double pi_copy = PI;

#line 25 "unit/modules/basic.sp"
    int ver = VERSION;

#line 27 "unit/modules/basic.sp"
    enum Color c = RED;

#line 28 "unit/modules/basic.sp"
    enum Status s = DEFAULT_STATUS;

#line 30 "unit/modules/basic.sp"
    printf("sum=%d product=%.2f\n", sum, product);

#line 31 "unit/modules/basic.sp"
    printf("pt=(%.1f,%.1f) rect=%.1fx%.1f\n", pt.x, pt.y, rect.width, rect.height);

#line 33 "unit/modules/basic.sp"
    if (sum != 30) 
        return 1;

#line 34 "unit/modules/basic.sp"
    if (product < 11.99f || product > 12.01f) 
        return 2;

#line 35 "unit/modules/basic.sp"
    if (value != 42) 
        return 3;

#line 36 "unit/modules/basic.sp"
    if (ver != 1) 
        return 4;

#line 37 "unit/modules/basic.sp"
    if (rect.origin.x != 1.5f || rect.origin.y != 2.5f) 
        return 5;

#line 38 "unit/modules/basic.sp"
    if (c != RED) 
        return 6;

#line 39 "unit/modules/basic.sp"
    if (s != OK) 
        return 7;

#line 41 "unit/modules/basic.sp"
    return 0;
}

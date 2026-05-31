
#line 8 "sharp-test/unit/modules/basic.he"
struct Point2D {
    float x;
    float y;
};

#line 9 "sharp-test/unit/modules/basic.he"
struct Rectangle {
    float width;
    float height;
    struct Point2D origin;
};

#line 11 "sharp-test/unit/modules/basic.he"
typedef int Int32;

#line 12 "sharp-test/unit/modules/basic.he"
typedef float Float32;

#line 14 "sharp-test/unit/modules/basic.he"
const double PI = 3.14159265358979323846;

#line 15 "sharp-test/unit/modules/basic.he"
int VERSION = 1;

#line 17 "sharp-test/unit/modules/basic.he"
enum Color {
    RED,
    GREEN,
    BLUE
};

#line 18 "sharp-test/unit/modules/basic.he"
enum Status {
    OK = 0,
    ERROR = -1,
    PENDING = 1
};

#line 19 "sharp-test/unit/modules/basic.he"
enum Status DEFAULT_STATUS = OK;

#line 4 "sharp-test/unit/modules/basic.he"
int add(int a, int b) {
    return a + b;
}

#line 5 "sharp-test/unit/modules/basic.he"
float multiply(float x, float y) {
    return x * y;
}

#line 6 "sharp-test/unit/modules/basic.he"
void greet(char * name) {
    (void)name;
}

#line 6 "sharp-test/unit/modules/basic.ce"
int main(void) {

#line 7 "sharp-test/unit/modules/basic.ce"
    int sum = add(10, 20);

#line 8 "sharp-test/unit/modules/basic.ce"
    float product = multiply(3.0f, 4.0f);

#line 9 "sharp-test/unit/modules/basic.ce"
    greet((char *)"World");

#line 11 "sharp-test/unit/modules/basic.ce"
    struct Point2D pt;

#line 12 "sharp-test/unit/modules/basic.ce"
    pt.x = 1.5f;

#line 13 "sharp-test/unit/modules/basic.ce"
    pt.y = 2.5f;

#line 15 "sharp-test/unit/modules/basic.ce"
    struct Rectangle rect;

#line 16 "sharp-test/unit/modules/basic.ce"
    rect.width = 10.0f;

#line 17 "sharp-test/unit/modules/basic.ce"
    rect.height = 20.0f;

#line 18 "sharp-test/unit/modules/basic.ce"
    rect.origin = pt;

#line 20 "sharp-test/unit/modules/basic.ce"
    Int32 value = 42;

#line 21 "sharp-test/unit/modules/basic.ce"
    Float32 fvalue = 3.14f;

#line 23 "sharp-test/unit/modules/basic.ce"
    double pi_copy = PI;

#line 24 "sharp-test/unit/modules/basic.ce"
    int ver = VERSION;

#line 26 "sharp-test/unit/modules/basic.ce"
    enum Color c = RED;

#line 27 "sharp-test/unit/modules/basic.ce"
    enum Status s = DEFAULT_STATUS;

#line 29 "sharp-test/unit/modules/basic.ce"
    if (sum != 30) 
        return 1;

#line 30 "sharp-test/unit/modules/basic.ce"
    if (product < 11.99f || product > 12.01f) 
        return 2;

#line 31 "sharp-test/unit/modules/basic.ce"
    if (value != 42) 
        return 3;

#line 32 "sharp-test/unit/modules/basic.ce"
    if (ver != 1) 
        return 4;

#line 33 "sharp-test/unit/modules/basic.ce"
    if (rect.origin.x != 1.5f || rect.origin.y != 2.5f) 
        return 5;

#line 34 "sharp-test/unit/modules/basic.ce"
    if (c != RED) 
        return 6;

#line 35 "sharp-test/unit/modules/basic.ce"
    if (s != OK) 
        return 7;

#line 37 "sharp-test/unit/modules/basic.ce"
    return 0;
}


#line 6 "sharp-test/unit/introspection/v013_intrinsics.ce"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 10 "sharp-test/unit/introspection/v013_intrinsics.ce"
int Point__hash(Point * this);

#line 11 "sharp-test/unit/introspection/v013_intrinsics.ce"
Point Point__op_add(Point * this, Point o);

#line 15 "sharp-test/unit/introspection/v013_intrinsics.ce"
struct NoHash {
    int v;
};

#line 10 "sharp-test/unit/introspection/v013_intrinsics.ce"
int Point__hash(Point * this) {
    return this->x * 31 + this->y;
}

#line 11 "sharp-test/unit/introspection/v013_intrinsics.ce"
Point Point__op_add(Point * this, Point o) {

#line 12 "sharp-test/unit/introspection/v013_intrinsics.ce"
    Point r;
    r.x = this->x + o.x;
    r.y = this->y + o.y;
    return r;
}

#line 19 "sharp-test/unit/introspection/v013_intrinsics.ce"
int main() {

#line 21 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @has_method */ 1) 
        return 1;

#line 22 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 25 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @has_field */ 1) 
        return 10;

#line 26 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @has_field */ 1) 
        return 11;

#line 27 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 28 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 31 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_arithmetic */ 1) 
        return 20;

#line 32 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_arithmetic */ 1) 
        return 21;

#line 33 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_arithmetic */ 1) 
        return 22;

#line 34 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_arithmetic */ 1) 
        return 23;

#line 35 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_arithmetic */ 1) 
        return 24;

#line 36 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_arithmetic */ 1) 
        return 25;

#line 37 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_arithmetic */ 1) 
        return 26;

#line 38 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 39 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 40 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 43 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_pointer */ 1) 
        return 30;

#line 44 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_pointer */ 1) 
        return 31;

#line 45 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_pointer */ 1) 
        return 32;

#line 46 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_pointer */ 1) 
        return 33;

#line 47 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 48 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 51 "sharp-test/unit/introspection/v013_intrinsics.ce"
    const char * n_int = "int";

#line 52 "sharp-test/unit/introspection/v013_intrinsics.ce"
    const char * n_pt = "Point";

#line 53 "sharp-test/unit/introspection/v013_intrinsics.ce"
    const char * n_pp = "Pint";

#line 54 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (n_int[0] != 'i') 
        return 40;

#line 55 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (n_pt[0] != 'P') 
        return 41;

#line 56 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (n_pp[0] != 'P') 
        return 42;

#line 59 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 63 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 69 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_pointer */ 1) 
        return 50;

#line 70 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (!/* @is_pointer */ 1) 
        return 51;

#line 71 "sharp-test/unit/introspection/v013_intrinsics.ce"

#line 74 "sharp-test/unit/introspection/v013_intrinsics.ce"
    const char * n_long = "long";

#line 75 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (n_long[0] != 'l') 
        return 53;

#line 76 "sharp-test/unit/introspection/v013_intrinsics.ce"
    const char * n_float = "float";

#line 77 "sharp-test/unit/introspection/v013_intrinsics.ce"
    if (n_float[0] != 'f') 
        return 54;

#line 79 "sharp-test/unit/introspection/v013_intrinsics.ce"
    return 0;
}

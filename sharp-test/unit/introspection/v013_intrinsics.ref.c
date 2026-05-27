#include <stdbool.h>

#include <stdlib.h>

#line 6 "unit/introspection/v013_intrinsics.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 10 "unit/introspection/v013_intrinsics.sp"
int Point__hash(Point * this);

#line 11 "unit/introspection/v013_intrinsics.sp"
Point Point__op_add(Point * this, Point o);

#line 15 "unit/introspection/v013_intrinsics.sp"
struct NoHash {
    int v;
};

#line 10 "unit/introspection/v013_intrinsics.sp"
int Point__hash(Point * this) {
    return this->x * 31 + this->y;
}

#line 11 "unit/introspection/v013_intrinsics.sp"
Point Point__op_add(Point * this, Point o) {

#line 12 "unit/introspection/v013_intrinsics.sp"
    Point r;
    r.x = this->x + o.x;
    r.y = this->y + o.y;
    return r;
}

#line 19 "unit/introspection/v013_intrinsics.sp"
int main() {

#line 21 "unit/introspection/v013_intrinsics.sp"
    if (!/* @has_method */ 1) 
        return 1;

#line 22 "unit/introspection/v013_intrinsics.sp"

#line 25 "unit/introspection/v013_intrinsics.sp"
    if (!/* @has_field */ 1) 
        return 10;

#line 26 "unit/introspection/v013_intrinsics.sp"
    if (!/* @has_field */ 1) 
        return 11;

#line 27 "unit/introspection/v013_intrinsics.sp"

#line 28 "unit/introspection/v013_intrinsics.sp"

#line 31 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_arithmetic */ 1) 
        return 20;

#line 32 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_arithmetic */ 1) 
        return 21;

#line 33 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_arithmetic */ 1) 
        return 22;

#line 34 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_arithmetic */ 1) 
        return 23;

#line 35 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_arithmetic */ 1) 
        return 24;

#line 36 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_arithmetic */ 1) 
        return 25;

#line 37 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_arithmetic */ 1) 
        return 26;

#line 38 "unit/introspection/v013_intrinsics.sp"

#line 39 "unit/introspection/v013_intrinsics.sp"

#line 40 "unit/introspection/v013_intrinsics.sp"

#line 43 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_pointer */ 1) 
        return 30;

#line 44 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_pointer */ 1) 
        return 31;

#line 45 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_pointer */ 1) 
        return 32;

#line 46 "unit/introspection/v013_intrinsics.sp"
    if (!/* @is_pointer */ 1) 
        return 33;

#line 47 "unit/introspection/v013_intrinsics.sp"

#line 48 "unit/introspection/v013_intrinsics.sp"

#line 51 "unit/introspection/v013_intrinsics.sp"
    const char * n_int = "int";

#line 52 "unit/introspection/v013_intrinsics.sp"
    const char * n_pt = "Point";

#line 53 "unit/introspection/v013_intrinsics.sp"
    const char * n_pp = "Pint";

#line 54 "unit/introspection/v013_intrinsics.sp"
    if (n_int[0] != 'i') 
        return 40;

#line 55 "unit/introspection/v013_intrinsics.sp"
    if (n_pt[0] != 'P') 
        return 41;

#line 56 "unit/introspection/v013_intrinsics.sp"
    if (n_pp[0] != 'P') 
        return 42;

#line 59 "unit/introspection/v013_intrinsics.sp"

#line 63 "unit/introspection/v013_intrinsics.sp"

#line 68 "unit/introspection/v013_intrinsics.sp"
    return 0;
}

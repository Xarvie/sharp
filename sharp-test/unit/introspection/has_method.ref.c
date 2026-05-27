
#include <stdlib.h>

#line 7 "unit/introspection/has_method.sp"
typedef struct Foo Foo;
struct Foo {
    int x;
};

#line 10 "unit/introspection/has_method.sp"
int Foo__get_x(Foo * this);

#line 13 "unit/introspection/has_method.sp"
typedef struct Inner Inner;
struct Inner {
    int a;
};

#line 16 "unit/introspection/has_method.sp"
typedef struct Outer Outer;
struct Outer {
    Inner inner;
    int b;
};

#line 22 "unit/introspection/has_method.sp"
typedef int MyInt;

#line 23 "unit/introspection/has_method.sp"
typedef float MyFloat;

#line 24 "unit/introspection/has_method.sp"
typedef long isize;

#line 25 "unit/introspection/has_method.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 10 "unit/introspection/has_method.sp"
int Foo__get_x(Foo * this) {
    return this->x;
}

#line 27 "unit/introspection/has_method.sp"
int main() {

#line 29 "unit/introspection/has_method.sp"
    Foo f;
    f.x = 42;

#line 30 "unit/introspection/has_method.sp"
{

#line 31 "unit/introspection/has_method.sp"
        if (Foo__get_x(&f) != 42) 
            return 1;
    }

#line 33 "unit/introspection/has_method.sp"

#line 34 "unit/introspection/has_method.sp"
    /* @static_assert */ 1;

#line 37 "unit/introspection/has_method.sp"
    Outer o = { 0 };

#line 38 "unit/introspection/has_method.sp"
{
        o.inner.a = 1;
    }

#line 39 "unit/introspection/has_method.sp"
{
        o.b = 2;
    }

#line 40 "unit/introspection/has_method.sp"
{
    }

#line 41 "unit/introspection/has_method.sp"

#line 42 "unit/introspection/has_method.sp"
    /* @static_assert */ 1;

#line 43 "unit/introspection/has_method.sp"
    if (o.inner.a != 1) 
        return 2;

#line 44 "unit/introspection/has_method.sp"
    if (o.b != 2) 
        return 3;

#line 47 "unit/introspection/has_method.sp"
{
    }

#line 48 "unit/introspection/has_method.sp"
{
    }

#line 49 "unit/introspection/has_method.sp"

#line 50 "unit/introspection/has_method.sp"
{
    }

#line 51 "unit/introspection/has_method.sp"
    /* @static_assert */ 1;

#line 53 "unit/introspection/has_method.sp"
    return 0;
}

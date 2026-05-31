
#line 7 "sharp-test/unit/introspection/has_method.ce"
typedef struct Foo Foo;
struct Foo {
    int x;
};

#line 10 "sharp-test/unit/introspection/has_method.ce"
int Foo__get_x(Foo * this);

#line 13 "sharp-test/unit/introspection/has_method.ce"
typedef struct Inner Inner;
struct Inner {
    int a;
};

#line 16 "sharp-test/unit/introspection/has_method.ce"
typedef struct Outer Outer;
struct Outer {
    Inner inner;
    int b;
};

#line 22 "sharp-test/unit/introspection/has_method.ce"
typedef int MyInt;

#line 23 "sharp-test/unit/introspection/has_method.ce"
typedef float MyFloat;

#line 24 "sharp-test/unit/introspection/has_method.ce"
typedef long isize;

#line 25 "sharp-test/unit/introspection/has_method.ce"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 27 "sharp-test/unit/introspection/has_method.ce"
typedef struct WithDebug WithDebug;
struct WithDebug {
    int val;
};

#line 30 "sharp-test/unit/introspection/has_method.ce"
void WithDebug__debug_print(WithDebug * this);

#line 31 "sharp-test/unit/introspection/has_method.ce"
int WithDebug__get_val(WithDebug * this);

#line 33 "sharp-test/unit/introspection/has_method.ce"
typedef struct NoDebug NoDebug;
struct NoDebug {
    int val;
};

#line 36 "sharp-test/unit/introspection/has_method.ce"
int NoDebug__get_val(NoDebug * this);

#line 10 "sharp-test/unit/introspection/has_method.ce"
int Foo__get_x(Foo * this) {
    return this->x;
}

#line 30 "sharp-test/unit/introspection/has_method.ce"
void WithDebug__debug_print(WithDebug * this) {
    (void)this;
}

#line 31 "sharp-test/unit/introspection/has_method.ce"
int WithDebug__get_val(WithDebug * this) {
    return this->val;
}

#line 36 "sharp-test/unit/introspection/has_method.ce"
int NoDebug__get_val(NoDebug * this) {
    return this->val;
}

#line 38 "sharp-test/unit/introspection/has_method.ce"
int main() {

#line 40 "sharp-test/unit/introspection/has_method.ce"
    Foo f;
    f.x = 42;

#line 41 "sharp-test/unit/introspection/has_method.ce"
{

#line 42 "sharp-test/unit/introspection/has_method.ce"
        if (Foo__get_x(&f) != 42) 
            return 1;
    }

#line 44 "sharp-test/unit/introspection/has_method.ce"

#line 45 "sharp-test/unit/introspection/has_method.ce"
    /* @static_assert */ 1;

#line 48 "sharp-test/unit/introspection/has_method.ce"
    Outer o = { 0 };

#line 49 "sharp-test/unit/introspection/has_method.ce"
{
        o.inner.a = 1;
    }

#line 50 "sharp-test/unit/introspection/has_method.ce"
{
        o.b = 2;
    }

#line 51 "sharp-test/unit/introspection/has_method.ce"
{
    }

#line 52 "sharp-test/unit/introspection/has_method.ce"

#line 53 "sharp-test/unit/introspection/has_method.ce"
    /* @static_assert */ 1;

#line 54 "sharp-test/unit/introspection/has_method.ce"
    if (o.inner.a != 1) 
        return 2;

#line 55 "sharp-test/unit/introspection/has_method.ce"
    if (o.b != 2) 
        return 3;

#line 58 "sharp-test/unit/introspection/has_method.ce"
{
    }

#line 59 "sharp-test/unit/introspection/has_method.ce"
{
    }

#line 60 "sharp-test/unit/introspection/has_method.ce"

#line 61 "sharp-test/unit/introspection/has_method.ce"
{
    }

#line 62 "sharp-test/unit/introspection/has_method.ce"
    /* @static_assert */ 1;

#line 65 "sharp-test/unit/introspection/has_method.ce"
    WithDebug wd;
    wd.val = 42;

#line 66 "sharp-test/unit/introspection/has_method.ce"
    NoDebug nd;
    nd.val = 99;

#line 68 "sharp-test/unit/introspection/has_method.ce"
{

#line 69 "sharp-test/unit/introspection/has_method.ce"
        WithDebug__debug_print(&wd);
    }

#line 71 "sharp-test/unit/introspection/has_method.ce"

#line 74 "sharp-test/unit/introspection/has_method.ce"
    if (WithDebug__get_val(&wd) != 42) 
        return 111;

#line 75 "sharp-test/unit/introspection/has_method.ce"
    if (NoDebug__get_val(&nd) != 99) 
        return 112;

#line 77 "sharp-test/unit/introspection/has_method.ce"
    return 0;
}

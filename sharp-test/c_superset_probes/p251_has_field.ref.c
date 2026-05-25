

#line 8 "p251_has_field.sp"
typedef struct Inner Inner;
struct Inner {
    int a;
};

#line 12 "p251_has_field.sp"
typedef struct Outer Outer;
struct Outer {
    Inner inner;
    int b;
};

#line 17 "p251_has_field.sp"
int main() {

#line 18 "p251_has_field.sp"
    Outer o = { 0 };

#line 20 "p251_has_field.sp"
{

#line 22 "p251_has_field.sp"
        o.inner.a = 1;
    }

#line 25 "p251_has_field.sp"
{

#line 26 "p251_has_field.sp"
        o.b = 2;
    }

#line 29 "p251_has_field.sp"
{
    }

#line 33 "p251_has_field.sp"

#line 37 "p251_has_field.sp"
    /* @static_assert */ 1;

#line 39 "p251_has_field.sp"
    if (o.inner.a != 1) 
        return 1;

#line 40 "p251_has_field.sp"
    if (o.b != 2) 
        return 2;

#line 42 "p251_has_field.sp"
    return 0;
}

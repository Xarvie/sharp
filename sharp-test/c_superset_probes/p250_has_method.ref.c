

#line 8 "p250_has_method.sp"
typedef struct Foo Foo;
struct Foo {
    int x;
};

#line 13 "p250_has_method.sp"
int Foo__get_x(Foo * this);
int Foo__get_x(Foo * this) {
    return this->x;
}

#line 15 "p250_has_method.sp"
int main() {

#line 16 "p250_has_method.sp"
    Foo f;
    f.x = 42;

#line 18 "p250_has_method.sp"
{

#line 20 "p250_has_method.sp"
        if (Foo__get_x(&f) != 42) 
            return 1;
    }

#line 26 "p250_has_method.sp"

#line 31 "p250_has_method.sp"
    /* @static_assert */ 1;

#line 33 "p250_has_method.sp"
    return 0;
}

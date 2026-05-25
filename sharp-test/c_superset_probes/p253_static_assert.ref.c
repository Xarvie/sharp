

#line 8 "p253_static_assert.sp"
typedef struct A A;
struct A {
    int x;
};

#line 9 "p253_static_assert.sp"
typedef struct B B;
struct B {
    int x;
};

#line 10 "p253_static_assert.sp"
typedef struct C C;
struct C {
    float y;
};

#line 12 "p253_static_assert.sp"
int main() {

#line 14 "p253_static_assert.sp"
    /* @static_assert */ 1;

#line 15 "p253_static_assert.sp"
    /* @static_assert */ 1;

#line 16 "p253_static_assert.sp"
    /* @static_assert */ 1;

#line 19 "p253_static_assert.sp"

#line 24 "p253_static_assert.sp"
    if (!/* @has_field */ 0) {
    }
    else {

#line 27 "p253_static_assert.sp"
        return 98;
    }

#line 30 "p253_static_assert.sp"
    return 0;
}


#line 4 "sharp-test/unit/introspection/compile_time.ce"
typedef struct HasMethod HasMethod;
struct HasMethod {
    int val;
};

#line 7 "sharp-test/unit/introspection/compile_time.ce"
void HasMethod__do_thing(HasMethod * this);

#line 8 "sharp-test/unit/introspection/compile_time.ce"
int HasMethod__get_val(HasMethod * this);

#line 10 "sharp-test/unit/introspection/compile_time.ce"
typedef struct NoMethod NoMethod;
struct NoMethod {
    int val;
};

#line 14 "sharp-test/unit/introspection/compile_time.ce"
typedef struct HasOps HasOps;
struct HasOps {
    int v;
};

#line 17 "sharp-test/unit/introspection/compile_time.ce"
int HasOps__op_add(HasOps * this, int x);

#line 18 "sharp-test/unit/introspection/compile_time.ce"
bool HasOps__op_eq(HasOps * this, HasOps o);

#line 7 "sharp-test/unit/introspection/compile_time.ce"
void HasMethod__do_thing(HasMethod * this) {
    (void)this;
}

#line 8 "sharp-test/unit/introspection/compile_time.ce"
int HasMethod__get_val(HasMethod * this) {
    return this->val;
}

#line 17 "sharp-test/unit/introspection/compile_time.ce"
int HasOps__op_add(HasOps * this, int x) {
    return this->v + x;
}

#line 18 "sharp-test/unit/introspection/compile_time.ce"
bool HasOps__op_eq(HasOps * this, HasOps o) {
    return this->v == o.v;
}

#line 20 "sharp-test/unit/introspection/compile_time.ce"
int main() {

#line 22 "sharp-test/unit/introspection/compile_time.ce"
    /* @static_assert */ 1;

#line 23 "sharp-test/unit/introspection/compile_time.ce"
    /* @static_assert */ 1;

#line 24 "sharp-test/unit/introspection/compile_time.ce"
    /* @static_assert */ 1;

#line 27 "sharp-test/unit/introspection/compile_time.ce"
    /* @static_assert */ 1;

#line 28 "sharp-test/unit/introspection/compile_time.ce"
    /* @static_assert */ 1;

#line 29 "sharp-test/unit/introspection/compile_time.ce"
    /* @static_assert */ 1;

#line 32 "sharp-test/unit/introspection/compile_time.ce"
{

#line 33 "sharp-test/unit/introspection/compile_time.ce"
        HasMethod hm;
        hm.val = 42;

#line 34 "sharp-test/unit/introspection/compile_time.ce"
        HasMethod__do_thing(&hm);

#line 35 "sharp-test/unit/introspection/compile_time.ce"
        if (HasMethod__get_val(&hm) != 42) 
            return 1;
    }

#line 39 "sharp-test/unit/introspection/compile_time.ce"
{

#line 40 "sharp-test/unit/introspection/compile_time.ce"
        HasOps a;
        a.v = 10;

#line 41 "sharp-test/unit/introspection/compile_time.ce"
        int r = HasOps__op_add(&a, 5);

#line 42 "sharp-test/unit/introspection/compile_time.ce"
        if (r != 15) 
            return 2;
    }

#line 46 "sharp-test/unit/introspection/compile_time.ce"
    if (!/* @has_method */ 0) {

#line 47 "sharp-test/unit/introspection/compile_time.ce"
        int x = 99;

#line 48 "sharp-test/unit/introspection/compile_time.ce"
        if (x != 99) 
            return 3;
    }

#line 51 "sharp-test/unit/introspection/compile_time.ce"
    return 0;
}

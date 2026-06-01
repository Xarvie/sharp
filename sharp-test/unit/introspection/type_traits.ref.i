
#line 4 "sharp-test/unit/introspection/type_traits.ce"
typedef struct TestStruct TestStruct;
struct TestStruct {
    int x;
    int y;
};

#line 9 "sharp-test/unit/introspection/type_traits.ce"
int main() {

#line 11 "sharp-test/unit/introspection/type_traits.ce"
    /* @static_assert */ 1;

#line 12 "sharp-test/unit/introspection/type_traits.ce"
    /* @static_assert */ 1;

#line 13 "sharp-test/unit/introspection/type_traits.ce"
    /* @static_assert */ 1;

#line 16 "sharp-test/unit/introspection/type_traits.ce"
    /* @static_assert */ 1;

#line 17 "sharp-test/unit/introspection/type_traits.ce"
    /* @static_assert */ 1;

#line 18 "sharp-test/unit/introspection/type_traits.ce"
    /* @static_assert */ 1;

#line 21 "sharp-test/unit/introspection/type_traits.ce"
    int ai = 1;

#line 22 "sharp-test/unit/introspection/type_traits.ce"
    float af = 1.0f;

#line 23 "sharp-test/unit/introspection/type_traits.ce"
    if (ai != 1) 
        return 1;

#line 24 "sharp-test/unit/introspection/type_traits.ce"
    if (af < 0.99f || af > 1.01f) 
        return 2;

#line 27 "sharp-test/unit/introspection/type_traits.ce"
    int * pi = &ai;

#line 28 "sharp-test/unit/introspection/type_traits.ce"
    if (*pi != 1) 
        return 3;

#line 31 "sharp-test/unit/introspection/type_traits.ce"
    if (!/* @has_field */ 1) 
        return 4;

#line 32 "sharp-test/unit/introspection/type_traits.ce"
    if (!/* @has_field */ 1) 
        return 5;

#line 33 "sharp-test/unit/introspection/type_traits.ce"

#line 36 "sharp-test/unit/introspection/type_traits.ce"
    const char * n = "int";

#line 37 "sharp-test/unit/introspection/type_traits.ce"
    if (n[0] != 'i') 
        return 7;

#line 39 "sharp-test/unit/introspection/type_traits.ce"
    return 0;
}

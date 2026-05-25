#include <stdbool.h>
#include <stdio.h>
#include <__stddef_size_t.h>
#include <__stdarg___gnuc_va_list.h>

#include <stdlib.h>

#line 10 "sharp-test/c_superset_probes/p154_operator_full.sp"
int g_passed = 0;

#line 11 "sharp-test/c_superset_probes/p154_operator_full.sp"
int g_failed = 0;

#line 19 "sharp-test/c_superset_probes/p154_operator_full.sp"
typedef struct IntBox IntBox;
struct IntBox {
    int value;
};

#line 23 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_add(IntBox * this, IntBox other);

#line 28 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_sub(IntBox * this, IntBox other);

#line 33 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_mul(IntBox * this, IntBox other);

#line 38 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_div(IntBox * this, IntBox other);

#line 43 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_mod(IntBox * this, IntBox other);

#line 48 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_eq(IntBox * this, IntBox other);

#line 51 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_ne(IntBox * this, IntBox other);

#line 54 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_lt(IntBox * this, IntBox other);

#line 57 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_gt(IntBox * this, IntBox other);

#line 60 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_le(IntBox * this, IntBox other);

#line 63 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_ge(IntBox * this, IntBox other);

#line 99 "sharp-test/c_superset_probes/p154_operator_full.sp"
typedef struct FloatBox FloatBox;
struct FloatBox {
    float value;
};

#line 103 "sharp-test/c_superset_probes/p154_operator_full.sp"
FloatBox FloatBox__op_add(FloatBox * this, FloatBox other);

#line 108 "sharp-test/c_superset_probes/p154_operator_full.sp"
FloatBox FloatBox__op_sub(FloatBox * this, FloatBox other);

#line 113 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool FloatBox__op_eq(FloatBox * this, FloatBox other);

#line 130 "sharp-test/c_superset_probes/p154_operator_full.sp"
typedef struct ArrayLike ArrayLike;
struct ArrayLike {
    int items[4];
};

#line 134 "sharp-test/c_superset_probes/p154_operator_full.sp"
int ArrayLike__op_idx(ArrayLike * this, long i);

#line 150 "sharp-test/c_superset_probes/p154_operator_full.sp"
struct Point {
    int x;
    int y;
};

#line 170 "sharp-test/c_superset_probes/p154_operator_full.sp"

#line 170 "sharp-test/c_superset_probes/p154_operator_full.sp"
typedef struct Num__int Num__int;
struct Num__int {
    int value;
};


typedef struct Num__float Num__float;
struct Num__float {
    float value;
};



#line 174 "sharp-test/c_superset_probes/p154_operator_full.sp"
Num__int Num__int__op_add(Num__int * this, Num__int other);

#line 179 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool Num__int__op_eq(Num__int * this, Num__int other);

#line 182 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool Num__int__op_ne(Num__int * this, Num__int other);

#line 185 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool Num__int__op_lt(Num__int * this, Num__int other);

#line 188 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool Num__int__op_gt(Num__int * this, Num__int other);

#line 191 "sharp-test/c_superset_probes/p154_operator_full.sp"
int Num__int__op_idx(Num__int * this, long i);

#line 174 "sharp-test/c_superset_probes/p154_operator_full.sp"
Num__float Num__float__op_add(Num__float * this, Num__float other);

#line 179 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool Num__float__op_eq(Num__float * this, Num__float other);

#line 185 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool Num__float__op_lt(Num__float * this, Num__float other);

#line 13 "sharp-test/c_superset_probes/p154_operator_full.sp"
void check(int cond, const char * msg) {

#line 14 "sharp-test/c_superset_probes/p154_operator_full.sp"
    if (cond) {
        g_passed = g_passed + 1;
        printf("  PASS: %s\n", msg);
    }
    else {

#line 15 "sharp-test/c_superset_probes/p154_operator_full.sp"
        g_failed = g_failed + 1;
        printf("  FAIL: %s\n", msg);
    }
}

#line 23 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_add(IntBox * this, IntBox other) {

#line 24 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox r;

#line 25 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value + other.value;

#line 26 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}

#line 28 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_sub(IntBox * this, IntBox other) {

#line 29 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox r;

#line 30 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value - other.value;

#line 31 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}

#line 33 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_mul(IntBox * this, IntBox other) {

#line 34 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox r;

#line 35 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value * other.value;

#line 36 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}

#line 38 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_div(IntBox * this, IntBox other) {

#line 39 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox r;

#line 40 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value / other.value;

#line 41 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}

#line 43 "sharp-test/c_superset_probes/p154_operator_full.sp"
IntBox IntBox__op_mod(IntBox * this, IntBox other) {

#line 44 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox r;

#line 45 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value % other.value;

#line 46 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}

#line 48 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_eq(IntBox * this, IntBox other) {

#line 49 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value == other.value;
}

#line 51 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_ne(IntBox * this, IntBox other) {

#line 52 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value != other.value;
}

#line 54 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_lt(IntBox * this, IntBox other) {

#line 55 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value < other.value;
}

#line 57 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_gt(IntBox * this, IntBox other) {

#line 58 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value > other.value;
}

#line 60 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_le(IntBox * this, IntBox other) {

#line 61 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value <= other.value;
}

#line 63 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool IntBox__op_ge(IntBox * this, IntBox other) {

#line 64 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value >= other.value;
}

#line 67 "sharp-test/c_superset_probes/p154_operator_full.sp"
void test_intbox_arithmetic() {

#line 68 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox a;
    a.value = 10;

#line 69 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox b;
    b.value = 3;

#line 71 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox c = IntBox__op_add(&a, b);
    check(c.value == 13, "IntBox +");

#line 72 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox d = IntBox__op_sub(&a, b);
    check(d.value == 7, "IntBox -");

#line 73 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox e = IntBox__op_mul(&a, b);
    check(e.value == 30, "IntBox *");

#line 74 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox f = IntBox__op_div(&a, b);
    check(f.value == 3, "IntBox /");

#line 75 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox g = IntBox__op_mod(&a, b);
    check(g.value == 1, "IntBox %%");
}

#line 78 "sharp-test/c_superset_probes/p154_operator_full.sp"
void test_intbox_comparison() {

#line 79 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox a;
    a.value = 10;

#line 80 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox b;
    b.value = 3;

#line 81 "sharp-test/c_superset_probes/p154_operator_full.sp"
    IntBox c;
    c.value = 10;

#line 83 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_eq(&a, c), "IntBox ==");

#line 84 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_eq(&a, a), "IntBox == self");

#line 85 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(IntBox__op_eq(&a, b)), "IntBox == false");

#line 86 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_ne(&a, b), "IntBox !=");

#line 87 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(IntBox__op_ne(&a, c)), "IntBox != false");

#line 88 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_lt(&b, a), "IntBox <");

#line 89 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(IntBox__op_lt(&a, b)), "IntBox < false");

#line 90 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_gt(&a, b), "IntBox >");

#line 91 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(IntBox__op_gt(&b, a)), "IntBox > false");

#line 92 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_le(&b, a), "IntBox <=");

#line 93 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_le(&a, c), "IntBox <= equal");

#line 94 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_ge(&a, b), "IntBox >=");

#line 95 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(IntBox__op_ge(&a, c), "IntBox >= equal");
}

#line 103 "sharp-test/c_superset_probes/p154_operator_full.sp"
FloatBox FloatBox__op_add(FloatBox * this, FloatBox other) {

#line 104 "sharp-test/c_superset_probes/p154_operator_full.sp"
    FloatBox r;

#line 105 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value + other.value;

#line 106 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}

#line 108 "sharp-test/c_superset_probes/p154_operator_full.sp"
FloatBox FloatBox__op_sub(FloatBox * this, FloatBox other) {

#line 109 "sharp-test/c_superset_probes/p154_operator_full.sp"
    FloatBox r;

#line 110 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value - other.value;

#line 111 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}

#line 113 "sharp-test/c_superset_probes/p154_operator_full.sp"
_Bool FloatBox__op_eq(FloatBox * this, FloatBox other) {

#line 114 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value == other.value;
}

#line 117 "sharp-test/c_superset_probes/p154_operator_full.sp"
void test_floatbox() {

#line 118 "sharp-test/c_superset_probes/p154_operator_full.sp"
    FloatBox a;
    a.value = 3.0f;

#line 119 "sharp-test/c_superset_probes/p154_operator_full.sp"
    FloatBox b;
    b.value = 4.0f;

#line 121 "sharp-test/c_superset_probes/p154_operator_full.sp"
    FloatBox c = FloatBox__op_add(&a, b);

#line 122 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check((int)c.value == 7, "FloatBox +");

#line 123 "sharp-test/c_superset_probes/p154_operator_full.sp"
    FloatBox d = FloatBox__op_sub(&b, a);

#line 124 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check((int)d.value == 1, "FloatBox -");

#line 125 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(FloatBox__op_eq(&a, b)), "FloatBox != by value");

#line 126 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(FloatBox__op_eq(&a, a), "FloatBox == self");
}

#line 134 "sharp-test/c_superset_probes/p154_operator_full.sp"
int ArrayLike__op_idx(ArrayLike * this, long i) {
    return this->items[i];
}

#line 136 "sharp-test/c_superset_probes/p154_operator_full.sp"
void test_arraylike() {

#line 137 "sharp-test/c_superset_probes/p154_operator_full.sp"
    ArrayLike arr;

#line 138 "sharp-test/c_superset_probes/p154_operator_full.sp"
    arr.items[0] = 10;

#line 139 "sharp-test/c_superset_probes/p154_operator_full.sp"
    arr.items[1] = 20;

#line 140 "sharp-test/c_superset_probes/p154_operator_full.sp"
    arr.items[2] = 30;

#line 141 "sharp-test/c_superset_probes/p154_operator_full.sp"
    arr.items[3] = 40;

#line 143 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(ArrayLike__op_idx(&arr, 0) == 10, "ArrayLike[0]");

#line 144 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(ArrayLike__op_idx(&arr, 1) == 20, "ArrayLike[1]");

#line 145 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(ArrayLike__op_idx(&arr, 2) == 30, "ArrayLike[2]");

#line 146 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(ArrayLike__op_idx(&arr, 3) == 40, "ArrayLike[3]");
}

#line 154 "sharp-test/c_superset_probes/p154_operator_full.sp"
struct Point operator_add__Point__Point(struct Point a, struct Point b) {

#line 155 "sharp-test/c_superset_probes/p154_operator_full.sp"
    struct Point r;

#line 156 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.x = a.x + b.x;

#line 157 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.y = a.y + b.y;

#line 158 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}

#line 161 "sharp-test/c_superset_probes/p154_operator_full.sp"
void test_free_operator() {

#line 162 "sharp-test/c_superset_probes/p154_operator_full.sp"
    struct Point a;
    a.x = 1;
    a.y = 2;

#line 163 "sharp-test/c_superset_probes/p154_operator_full.sp"
    struct Point b;
    b.x = 3;
    b.y = 4;

#line 164 "sharp-test/c_superset_probes/p154_operator_full.sp"
    struct Point c = operator_add__Point__Point(a, b);

#line 165 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(c.x == 4, "free op+ .x");

#line 166 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(c.y == 6, "free op+ .y");
}

#line 196 "sharp-test/c_superset_probes/p154_operator_full.sp"
void test_generic_num_ops() {

#line 197 "sharp-test/c_superset_probes/p154_operator_full.sp"
    Num__int a;
    a.value = 100;

#line 198 "sharp-test/c_superset_probes/p154_operator_full.sp"
    Num__int b;
    b.value = 50;

#line 200 "sharp-test/c_superset_probes/p154_operator_full.sp"
    Num__int c = Num__int__op_add(&a, b);

#line 201 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(c.value == 150, "Num<int> +");

#line 203 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(Num__int__op_eq(&a, a), "Num<int> == self");

#line 204 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(Num__int__op_eq(&a, b)), "Num<int> == false");

#line 205 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(Num__int__op_ne(&a, b), "Num<int> !=");

#line 206 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(Num__int__op_lt(&a, b)), "Num<int> < false");

#line 207 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(Num__int__op_lt(&b, a), "Num<int> <");

#line 208 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(Num__int__op_gt(&a, b), "Num<int> >");

#line 209 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(Num__int__op_gt(&b, a)), "Num<int> > false");

#line 210 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(Num__int__op_idx(&a, 0) == 100, "Num<int>[0]");
}

#line 213 "sharp-test/c_superset_probes/p154_operator_full.sp"
void test_generic_num_float_ops() {

#line 214 "sharp-test/c_superset_probes/p154_operator_full.sp"
    Num__float a;
    a.value = 7.0f;

#line 215 "sharp-test/c_superset_probes/p154_operator_full.sp"
    Num__float b;
    b.value = 3.0f;

#line 217 "sharp-test/c_superset_probes/p154_operator_full.sp"
    Num__float c = Num__float__op_add(&a, b);

#line 218 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check((int)c.value == 10, "Num<float> +");

#line 219 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(Num__float__op_eq(&a, a), "Num<float> == self");

#line 220 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(!(Num__float__op_eq(&a, b)), "Num<float> == false");

#line 221 "sharp-test/c_superset_probes/p154_operator_full.sp"
    check(Num__float__op_lt(&b, a), "Num<float> <");
}

#line 224 "sharp-test/c_superset_probes/p154_operator_full.sp"
int main() {

#line 225 "sharp-test/c_superset_probes/p154_operator_full.sp"
    printf("=== p154_operator_full ===\n");

#line 227 "sharp-test/c_superset_probes/p154_operator_full.sp"
    test_intbox_arithmetic();

#line 228 "sharp-test/c_superset_probes/p154_operator_full.sp"
    test_intbox_comparison();

#line 229 "sharp-test/c_superset_probes/p154_operator_full.sp"
    test_floatbox();

#line 230 "sharp-test/c_superset_probes/p154_operator_full.sp"
    test_arraylike();

#line 231 "sharp-test/c_superset_probes/p154_operator_full.sp"
    test_free_operator();

#line 232 "sharp-test/c_superset_probes/p154_operator_full.sp"
    test_generic_num_ops();

#line 233 "sharp-test/c_superset_probes/p154_operator_full.sp"
    test_generic_num_float_ops();

#line 235 "sharp-test/c_superset_probes/p154_operator_full.sp"
    printf("  passed=%d  failed=%d\n", g_passed, g_failed);

#line 236 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return g_failed;
}

#line 174 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) Num__int Num__int__op_add(Num__int * this, Num__int other) {

#line 175 "sharp-test/c_superset_probes/p154_operator_full.sp"
    Num__int r;

#line 176 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value + other.value;

#line 177 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}


#line 179 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) _Bool Num__int__op_eq(Num__int * this, Num__int other) {

#line 180 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value == other.value;
}


#line 182 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) _Bool Num__int__op_ne(Num__int * this, Num__int other) {

#line 183 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value != other.value;
}


#line 185 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) _Bool Num__int__op_lt(Num__int * this, Num__int other) {

#line 186 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value < other.value;
}


#line 188 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) _Bool Num__int__op_gt(Num__int * this, Num__int other) {

#line 189 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value > other.value;
}


#line 191 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) int Num__int__op_idx(Num__int * this, long i) {

#line 192 "sharp-test/c_superset_probes/p154_operator_full.sp"
    if (i == 0) 
        return this->value;

#line 193 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value;
}


#line 174 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) Num__float Num__float__op_add(Num__float * this, Num__float other) {

#line 175 "sharp-test/c_superset_probes/p154_operator_full.sp"
    Num__float r;

#line 176 "sharp-test/c_superset_probes/p154_operator_full.sp"
    r.value = this->value + other.value;

#line 177 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return r;
}


#line 179 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) _Bool Num__float__op_eq(Num__float * this, Num__float other) {

#line 180 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value == other.value;
}


#line 185 "sharp-test/c_superset_probes/p154_operator_full.sp"
__attribute__((weak)) _Bool Num__float__op_lt(Num__float * this, Num__float other) {

#line 186 "sharp-test/c_superset_probes/p154_operator_full.sp"
    return this->value < other.value;
}


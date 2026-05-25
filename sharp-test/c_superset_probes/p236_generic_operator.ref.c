#include <stdbool.h>


#line 11 "sharp-test/c_superset_probes/p236_generic_operator.sp"
typedef struct Vec2__int Vec2__int;
struct Vec2__int {
    int x;
    int y;
};



#line 16 "sharp-test/c_superset_probes/p236_generic_operator.sp"
_Bool Vec2__int__equals(Vec2__int * this, Vec2__int other);

#line 20 "sharp-test/c_superset_probes/p236_generic_operator.sp"
void Vec2__int__add_into(Vec2__int * this, Vec2__int other);

#line 25 "sharp-test/c_superset_probes/p236_generic_operator.sp"
int Vec2__int__sum(Vec2__int * this);

#line 29 "sharp-test/c_superset_probes/p236_generic_operator.sp"
int main() {

#line 30 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    Vec2__int a;

#line 31 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    a.x = 1;
    a.y = 2;

#line 32 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    Vec2__int b;

#line 33 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    b.x = 1;
    b.y = 2;

#line 34 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    Vec2__int c;

#line 35 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    c.x = 3;
    c.y = 4;

#line 37 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    if (!Vec2__int__equals(&a, b)) 
        return 1;

#line 38 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    if (Vec2__int__equals(&a, c)) 
        return 2;

#line 40 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    Vec2__int__add_into(&a, c);

#line 41 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    if (a.x != 4) 
        return 3;

#line 42 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    if (a.y != 6) 
        return 4;

#line 44 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    if (Vec2__int__sum(&a) != 10) 
        return 5;

#line 46 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    return 0;
}

#line 16 "sharp-test/c_superset_probes/p236_generic_operator.sp"
__attribute__((weak)) _Bool Vec2__int__equals(Vec2__int * this, Vec2__int other) {

#line 17 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    return this->x == other.x && this->y == other.y;
}


#line 20 "sharp-test/c_superset_probes/p236_generic_operator.sp"
__attribute__((weak)) void Vec2__int__add_into(Vec2__int * this, Vec2__int other) {

#line 21 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    this->x = this->x + other.x;

#line 22 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    this->y = this->y + other.y;
}


#line 25 "sharp-test/c_superset_probes/p236_generic_operator.sp"
__attribute__((weak)) int Vec2__int__sum(Vec2__int * this) {

#line 26 "sharp-test/c_superset_probes/p236_generic_operator.sp"
    return this->x + this->y;
}


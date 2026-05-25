#include <stdbool.h>


#line 8 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
typedef struct Maybe__int Maybe__int;
struct Maybe__int {
    _Bool has;
    int val;
};


typedef struct Maybe__float Maybe__float;
struct Maybe__float {
    _Bool has;
    float val;
};



#line 13 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
_Bool Maybe__int__has_val(Maybe__int * this);

#line 15 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
void Maybe__int__set_val(Maybe__int * this, int v);

#line 14 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
int Maybe__int__get_val(Maybe__int * this);

#line 13 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
_Bool Maybe__float__has_val(Maybe__float * this);

#line 15 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
void Maybe__float__set_val(Maybe__float * this, float v);

#line 14 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
float Maybe__float__get_val(Maybe__float * this);

#line 17 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
int main() {

#line 18 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    Maybe__int mi = { 0 };

#line 19 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    if (Maybe__int__has_val(&mi)) 
        return 1;

#line 21 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    Maybe__int__set_val(&mi, 42);

#line 22 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    if (!Maybe__int__has_val(&mi)) 
        return 2;

#line 23 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    if (Maybe__int__get_val(&mi) != 42) 
        return 3;

#line 25 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    Maybe__float mf = { 0 };

#line 26 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    if (Maybe__float__has_val(&mf)) 
        return 4;

#line 28 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    Maybe__float__set_val(&mf, 1.5f);

#line 29 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    if (Maybe__float__get_val(&mf) < 1.49f || Maybe__float__get_val(&mf) > 1.51f) 
        return 5;

#line 31 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
    return 0;
}

#line 13 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
__attribute__((weak)) _Bool Maybe__int__has_val(Maybe__int * this) {
    return this->has;
}


#line 15 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
__attribute__((weak)) void Maybe__int__set_val(Maybe__int * this, int v) {
    this->has = 1;
    this->val = v;
}


#line 14 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
__attribute__((weak)) int Maybe__int__get_val(Maybe__int * this) {
    return this->val;
}


#line 13 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
__attribute__((weak)) _Bool Maybe__float__has_val(Maybe__float * this) {
    return this->has;
}


#line 15 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
__attribute__((weak)) void Maybe__float__set_val(Maybe__float * this, float v) {
    this->has = 1;
    this->val = v;
}


#line 14 "sharp-test/c_superset_probes/p235_generic_zeroinit.sp"
__attribute__((weak)) float Maybe__float__get_val(Maybe__float * this) {
    return this->val;
}


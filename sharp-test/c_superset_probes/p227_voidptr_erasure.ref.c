#include <stdbool.h>


#line 13 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
typedef struct AnyBox AnyBox;
struct AnyBox {
    void * ptr;
    int tag;
};

#line 19 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
int AnyBox__as_int(AnyBox * this);

#line 25 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
float AnyBox__as_float(AnyBox * this);

#line 31 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
void AnyBox__set_int(AnyBox * this, int * p);

#line 36 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
void AnyBox__set_float(AnyBox * this, float * p);

#line 19 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
int AnyBox__as_int(AnyBox * this) {

#line 20 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    int * ip = (int *)this->ptr;

#line 21 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    if (this->ptr) 
        return *ip;

#line 22 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    return 0;
}

#line 25 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
float AnyBox__as_float(AnyBox * this) {

#line 26 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    float * fp = (float *)this->ptr;

#line 27 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    if (this->ptr) 
        return *fp;

#line 28 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    return 0.0f;
}

#line 31 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
void AnyBox__set_int(AnyBox * this, int * p) {

#line 32 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    this->ptr = (void *)p;

#line 33 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    this->tag = 1;
}

#line 36 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
void AnyBox__set_float(AnyBox * this, float * p) {

#line 37 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    this->ptr = (void *)p;

#line 38 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    this->tag = 2;
}

#line 42 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
_Bool ptr_equals(void * a, void * b) {

#line 43 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    return a == b;
}

#line 46 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
int main() {

#line 47 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    AnyBox box;

#line 50 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    int iv = 123;

#line 51 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    AnyBox__set_int(&box, &iv);

#line 52 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    int r1 = AnyBox__as_int(&box);

#line 53 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    if (r1 != 123) 
        return 1;

#line 56 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    float fv = 3.14f;

#line 57 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    AnyBox__set_float(&box, &fv);

#line 58 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    float r2 = AnyBox__as_float(&box);

#line 59 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    if (r2 < 3.13f || r2 > 3.15f) 
        return 2;

#line 62 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    if (!ptr_equals(&iv, &iv)) 
        return 3;

#line 63 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    if (ptr_equals(&iv, &fv)) 
        return 4;

#line 66 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    AnyBox empty = { 0 };

#line 68 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    if (AnyBox__as_int(&empty) != 0) 
        return 5;

#line 70 "sharp-test/c_superset_probes/p227_voidptr_erasure.sp"
    return 0;
}

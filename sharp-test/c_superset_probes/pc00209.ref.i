
#line 3 "sharp-test/c_superset_probes/pc00209.c"
enum E * e;

#line 4 "sharp-test/c_superset_probes/pc00209.c"
const enum E * e1;

#line 5 "sharp-test/c_superset_probes/pc00209.c"
enum E const * e2;

#line 6 "sharp-test/c_superset_probes/pc00209.c"
struct S * s;

#line 7 "sharp-test/c_superset_probes/pc00209.c"
const struct S * s1;

#line 8 "sharp-test/c_superset_probes/pc00209.c"
struct S const * s2;

#line 12 "sharp-test/c_superset_probes/pc00209.c"
typedef int (*fptr1)(int);

#line 13 "sharp-test/c_superset_probes/pc00209.c"
int f1(int (*)(int), int);

#line 14 "sharp-test/c_superset_probes/pc00209.c"
typedef int (*fptr2)(int x);

#line 15 "sharp-test/c_superset_probes/pc00209.c"
int f2(int (*)(int x), int);

#line 16 "sharp-test/c_superset_probes/pc00209.c"
typedef int (*fptr3)(int);

#line 17 "sharp-test/c_superset_probes/pc00209.c"
int f3(int (*)(int), int);

#line 18 "sharp-test/c_superset_probes/pc00209.c"
typedef int (*fptr4[4])(int);

#line 19 "sharp-test/c_superset_probes/pc00209.c"
int f4(int (*[4])(int), int);

#line 20 "sharp-test/c_superset_probes/pc00209.c"
typedef int (*fptr5)(fptr1);

#line 21 "sharp-test/c_superset_probes/pc00209.c"
int f5(int (*)(int (*)(int)), fptr1);

#line 42 "sharp-test/c_superset_probes/pc00209.c"
int f8(int[4], int);

#line 3 "sharp-test/c_superset_probes/pc00209.c"

#line 6 "sharp-test/c_superset_probes/pc00209.c"

#line 13 "sharp-test/c_superset_probes/pc00209.c"
int f1(int (*)(int), int);

#line 15 "sharp-test/c_superset_probes/pc00209.c"
int f2(int (*)(int x), int);

#line 17 "sharp-test/c_superset_probes/pc00209.c"
int f3(int (*)(int), int);

#line 19 "sharp-test/c_superset_probes/pc00209.c"
int f4(int (*[4])(int), int);

#line 21 "sharp-test/c_superset_probes/pc00209.c"
int f5(int (*)(int (*)(int)), fptr1);

#line 22 "sharp-test/c_superset_probes/pc00209.c"
int f1(fptr1 fp, int i) {

#line 24 "sharp-test/c_superset_probes/pc00209.c"
    return (*fp)(i);
}

#line 26 "sharp-test/c_superset_probes/pc00209.c"
int f2(fptr2 fp, int i) {

#line 28 "sharp-test/c_superset_probes/pc00209.c"
    return (*fp)(i);
}

#line 30 "sharp-test/c_superset_probes/pc00209.c"
int f3(fptr3 fp, int i) {

#line 32 "sharp-test/c_superset_probes/pc00209.c"
    return (*fp)(i);
}

#line 34 "sharp-test/c_superset_probes/pc00209.c"
int f4(fptr4 fp, int i) {

#line 36 "sharp-test/c_superset_probes/pc00209.c"
    return (*fp[i])(i);
}

#line 38 "sharp-test/c_superset_probes/pc00209.c"
int f5(fptr5 fp, fptr1 i) {

#line 40 "sharp-test/c_superset_probes/pc00209.c"
    return fp(i);
}

#line 42 "sharp-test/c_superset_probes/pc00209.c"
int f8(int[4], int);

#line 43 "sharp-test/c_superset_probes/pc00209.c"
int main() {
    return 0;
}

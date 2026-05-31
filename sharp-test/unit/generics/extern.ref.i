
#line 12 "sharp-test/unit/generics/extern.ce"

#line 13 "sharp-test/unit/generics/extern.ce"

#line 16 "sharp-test/unit/generics/extern.ce"

#line 37 "sharp-test/unit/generics/extern.ce"

#line 40 "sharp-test/unit/generics/extern.ce"

#line 37 "sharp-test/unit/generics/extern.ce"
typedef struct Vec2__int Vec2__int;
struct Vec2__int {
    int * data;
    int len;
};


typedef struct Vec2__Vec2__int Vec2__Vec2__int;
struct Vec2__Vec2__int {
    Vec2__int * data;
    int len;
};



#line 16 "sharp-test/unit/generics/extern.ce"
typedef struct Vec__int Vec__int;
struct Vec__int {
    int * data;
    long len;
    long cap;
};



#line 5 "sharp-test/unit/generics/extern.ce"
void swap__int(int * a, int * b);
void swap__float(float * a, float * b);

#line 26 "sharp-test/unit/generics/extern.ce"
void Vec__int__push(Vec__int * this, int item);

#line 22 "sharp-test/unit/generics/extern.ce"
long Vec__int__size(Vec__int * this);

#line 32 "sharp-test/unit/generics/extern.ce"
int Vec__int__at(Vec__int * this, long i);

#line 5 "sharp-test/unit/generics/extern.ce"

#line 22 "sharp-test/unit/generics/extern.ce"

#line 26 "sharp-test/unit/generics/extern.ce"

#line 32 "sharp-test/unit/generics/extern.ce"

#line 42 "sharp-test/unit/generics/extern.ce"
Vec2__Vec2__int flatten(Vec2__Vec2__int v) {

#line 43 "sharp-test/unit/generics/extern.ce"
    return v;
}

#line 46 "sharp-test/unit/generics/extern.ce"
int main() {
    float fa = 1.0f, fb = 2.0f;

#line 49 "sharp-test/unit/generics/extern.ce"
    swap__float(&fa, &fb);

#line 50 "sharp-test/unit/generics/extern.ce"
    if ((int)(fa + fb - 3.0f) != 0) 
        return 1;

#line 53 "sharp-test/unit/generics/extern.ce"
    int arr[10];

#line 54 "sharp-test/unit/generics/extern.ce"
    Vec__int v = { arr, 0, 10 };

#line 55 "sharp-test/unit/generics/extern.ce"
    Vec__int__push(&v, 100);

#line 56 "sharp-test/unit/generics/extern.ce"
    Vec__int__push(&v, 200);

#line 57 "sharp-test/unit/generics/extern.ce"
    if (Vec__int__size(&v) != 2) 
        return 2;

#line 58 "sharp-test/unit/generics/extern.ce"
    if (Vec__int__at(&v, 0) != 100) 
        return 3;

#line 59 "sharp-test/unit/generics/extern.ce"
    if (Vec__int__at(&v, 1) != 200) 
        return 4;

#line 62 "sharp-test/unit/generics/extern.ce"
    Vec2__Vec2__int x;

#line 63 "sharp-test/unit/generics/extern.ce"
    flatten(x);

#line 67 "sharp-test/unit/generics/extern.ce"
    int ext_arr[4];

#line 68 "sharp-test/unit/generics/extern.ce"
    Vec__int ev = { ext_arr, 0, 4 };

#line 69 "sharp-test/unit/generics/extern.ce"
    Vec__int__push(&ev, 7);

#line 70 "sharp-test/unit/generics/extern.ce"
    Vec__int__push(&ev, 8);

#line 71 "sharp-test/unit/generics/extern.ce"
    if (Vec__int__size(&ev) != 2) 
        return 5;

#line 72 "sharp-test/unit/generics/extern.ce"
    if (Vec__int__at(&ev, 0) != 7) 
        return 6;

#line 74 "sharp-test/unit/generics/extern.ce"
    return 0;
}

#line 5 "sharp-test/unit/generics/extern.ce"
__attribute__((weak)) void swap__int(int * a, int * b) {

#line 6 "sharp-test/unit/generics/extern.ce"
    int tmp = *a;

#line 7 "sharp-test/unit/generics/extern.ce"
    *a = *b;

#line 8 "sharp-test/unit/generics/extern.ce"
    *b = tmp;
}


#line 5 "sharp-test/unit/generics/extern.ce"
__attribute__((weak)) void swap__float(float * a, float * b) {

#line 6 "sharp-test/unit/generics/extern.ce"
    float tmp = *a;

#line 7 "sharp-test/unit/generics/extern.ce"
    *a = *b;

#line 8 "sharp-test/unit/generics/extern.ce"
    *b = tmp;
}


#line 26 "sharp-test/unit/generics/extern.ce"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int item) {

#line 27 "sharp-test/unit/generics/extern.ce"
    if (this->len >= this->cap) 
        return;

#line 28 "sharp-test/unit/generics/extern.ce"
    this->data[this->len] = item;

#line 29 "sharp-test/unit/generics/extern.ce"
    this->len = this->len + 1;
}


#line 22 "sharp-test/unit/generics/extern.ce"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 23 "sharp-test/unit/generics/extern.ce"
    return this->len;
}


#line 32 "sharp-test/unit/generics/extern.ce"
__attribute__((weak)) int Vec__int__at(Vec__int * this, long i) {

#line 33 "sharp-test/unit/generics/extern.ce"
    return this->data[i];
}


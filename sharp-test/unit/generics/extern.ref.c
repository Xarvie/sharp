
#line 12 "sharp-test/unit/generics/extern.sp"

#line 13 "sharp-test/unit/generics/extern.sp"

#line 16 "sharp-test/unit/generics/extern.sp"

#line 37 "sharp-test/unit/generics/extern.sp"

#line 40 "sharp-test/unit/generics/extern.sp"

#line 37 "sharp-test/unit/generics/extern.sp"
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



#line 16 "sharp-test/unit/generics/extern.sp"
typedef struct Vec__int Vec__int;
struct Vec__int {
    int * data;
    long len;
    long cap;
};



#line 5 "sharp-test/unit/generics/extern.sp"
void swap__int(int * a, int * b);
void swap__float(float * a, float * b);

#line 26 "sharp-test/unit/generics/extern.sp"
void Vec__int__push(Vec__int * this, int item);

#line 22 "sharp-test/unit/generics/extern.sp"
long Vec__int__size(Vec__int * this);

#line 32 "sharp-test/unit/generics/extern.sp"
int Vec__int__at(Vec__int * this, long i);

#line 5 "sharp-test/unit/generics/extern.sp"

#line 22 "sharp-test/unit/generics/extern.sp"

#line 26 "sharp-test/unit/generics/extern.sp"

#line 32 "sharp-test/unit/generics/extern.sp"

#line 42 "sharp-test/unit/generics/extern.sp"
Vec2__Vec2__int flatten(Vec2__Vec2__int v) {

#line 43 "sharp-test/unit/generics/extern.sp"
    return v;
}

#line 46 "sharp-test/unit/generics/extern.sp"
int main() {
    float fa = 1.0f, fb = 2.0f;

#line 49 "sharp-test/unit/generics/extern.sp"
    swap__float(&fa, &fb);

#line 50 "sharp-test/unit/generics/extern.sp"
    if ((int)(fa + fb - 3.0f) != 0) 
        return 1;

#line 53 "sharp-test/unit/generics/extern.sp"
    int arr[10];

#line 54 "sharp-test/unit/generics/extern.sp"
    Vec__int v = { arr, 0, 10 };

#line 55 "sharp-test/unit/generics/extern.sp"
    Vec__int__push(&v, 100);

#line 56 "sharp-test/unit/generics/extern.sp"
    Vec__int__push(&v, 200);

#line 57 "sharp-test/unit/generics/extern.sp"
    if (Vec__int__size(&v) != 2) 
        return 2;

#line 58 "sharp-test/unit/generics/extern.sp"
    if (Vec__int__at(&v, 0) != 100) 
        return 3;

#line 59 "sharp-test/unit/generics/extern.sp"
    if (Vec__int__at(&v, 1) != 200) 
        return 4;

#line 62 "sharp-test/unit/generics/extern.sp"
    Vec2__Vec2__int x;

#line 63 "sharp-test/unit/generics/extern.sp"
    flatten(x);

#line 65 "sharp-test/unit/generics/extern.sp"
    return 0;
}

#line 5 "sharp-test/unit/generics/extern.sp"
__attribute__((weak)) void swap__int(int * a, int * b) {

#line 6 "sharp-test/unit/generics/extern.sp"
    int tmp = *a;

#line 7 "sharp-test/unit/generics/extern.sp"
    *a = *b;

#line 8 "sharp-test/unit/generics/extern.sp"
    *b = tmp;
}


#line 5 "sharp-test/unit/generics/extern.sp"
__attribute__((weak)) void swap__float(float * a, float * b) {

#line 6 "sharp-test/unit/generics/extern.sp"
    float tmp = *a;

#line 7 "sharp-test/unit/generics/extern.sp"
    *a = *b;

#line 8 "sharp-test/unit/generics/extern.sp"
    *b = tmp;
}


#line 26 "sharp-test/unit/generics/extern.sp"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int item) {

#line 27 "sharp-test/unit/generics/extern.sp"
    if (this->len >= this->cap) 
        return;

#line 28 "sharp-test/unit/generics/extern.sp"
    this->data[this->len] = item;

#line 29 "sharp-test/unit/generics/extern.sp"
    this->len = this->len + 1;
}


#line 22 "sharp-test/unit/generics/extern.sp"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 23 "sharp-test/unit/generics/extern.sp"
    return this->len;
}


#line 32 "sharp-test/unit/generics/extern.sp"
__attribute__((weak)) int Vec__int__at(Vec__int * this, long i) {

#line 33 "sharp-test/unit/generics/extern.sp"
    return this->data[i];
}


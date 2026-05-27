#include <stdbool.h>

#include <stdlib.h>

#line 7 "unit/generics/embed.sp"

#line 17 "unit/generics/embed.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 22 "unit/generics/embed.sp"

#line 7 "unit/generics/embed.sp"
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



#line 22 "unit/generics/embed.sp"
typedef struct Labeled__int Labeled__int;
struct Labeled__int {
    int label;
    Point pos;
};


typedef struct Labeled__float Labeled__float;
struct Labeled__float {
    float label;
    Point pos;
};



#line 12 "unit/generics/embed.sp"
_Bool Maybe__int__has_val(Maybe__int * this);

#line 14 "unit/generics/embed.sp"
void Maybe__int__set_val(Maybe__int * this, int v);

#line 13 "unit/generics/embed.sp"
int Maybe__int__get_val(Maybe__int * this);

#line 12 "unit/generics/embed.sp"
_Bool Maybe__float__has_val(Maybe__float * this);

#line 14 "unit/generics/embed.sp"
void Maybe__float__set_val(Maybe__float * this, float v);

#line 13 "unit/generics/embed.sp"
float Maybe__float__get_val(Maybe__float * this);

#line 28 "unit/generics/embed.sp"
void Labeled__int__set_label(Labeled__int * this, int val);

#line 29 "unit/generics/embed.sp"
void Labeled__int__set_xy(Labeled__int * this, int x, int y);

#line 27 "unit/generics/embed.sp"
int Labeled__int__get_label(Labeled__int * this);

#line 30 "unit/generics/embed.sp"
int Labeled__int__pos_x(Labeled__int * this);

#line 28 "unit/generics/embed.sp"
void Labeled__float__set_label(Labeled__float * this, float val);

#line 27 "unit/generics/embed.sp"
float Labeled__float__get_label(Labeled__float * this);

#line 12 "unit/generics/embed.sp"

#line 13 "unit/generics/embed.sp"

#line 14 "unit/generics/embed.sp"

#line 27 "unit/generics/embed.sp"

#line 28 "unit/generics/embed.sp"

#line 29 "unit/generics/embed.sp"

#line 30 "unit/generics/embed.sp"

#line 32 "unit/generics/embed.sp"
int main() {

#line 34 "unit/generics/embed.sp"
    Maybe__int mi = { 0 };

#line 35 "unit/generics/embed.sp"
    if (Maybe__int__has_val(&mi)) 
        return 1;

#line 37 "unit/generics/embed.sp"
    Maybe__int__set_val(&mi, 42);

#line 38 "unit/generics/embed.sp"
    if (!Maybe__int__has_val(&mi)) 
        return 2;

#line 39 "unit/generics/embed.sp"
    if (Maybe__int__get_val(&mi) != 42) 
        return 3;

#line 41 "unit/generics/embed.sp"
    Maybe__float mf = { 0 };

#line 42 "unit/generics/embed.sp"
    if (Maybe__float__has_val(&mf)) 
        return 4;

#line 44 "unit/generics/embed.sp"
    Maybe__float__set_val(&mf, 1.5f);

#line 45 "unit/generics/embed.sp"
    if (Maybe__float__get_val(&mf) < 1.49f || Maybe__float__get_val(&mf) > 1.51f) 
        return 5;

#line 48 "unit/generics/embed.sp"
    Labeled__int a = { 0 };

#line 49 "unit/generics/embed.sp"
    Labeled__int__set_label(&a, 42);

#line 50 "unit/generics/embed.sp"
    Labeled__int__set_xy(&a, 10, 20);

#line 52 "unit/generics/embed.sp"
    if (Labeled__int__get_label(&a) != 42) 
        return 6;

#line 53 "unit/generics/embed.sp"
    if (Labeled__int__pos_x(&a) != 10) 
        return 7;

#line 55 "unit/generics/embed.sp"
    Labeled__float b = { 0 };

#line 56 "unit/generics/embed.sp"
    Labeled__float__set_label(&b, 3.14f);

#line 57 "unit/generics/embed.sp"
    if (Labeled__float__get_label(&b) < 3.13f || Labeled__float__get_label(&b) > 3.15f) 
        return 8;

#line 59 "unit/generics/embed.sp"
    return 0;
}

#line 12 "unit/generics/embed.sp"
__attribute__((weak)) _Bool Maybe__int__has_val(Maybe__int * this) {
    return this->has;
}


#line 14 "unit/generics/embed.sp"
__attribute__((weak)) void Maybe__int__set_val(Maybe__int * this, int v) {
    this->has = 1;
    this->val = v;
}


#line 13 "unit/generics/embed.sp"
__attribute__((weak)) int Maybe__int__get_val(Maybe__int * this) {
    return this->val;
}


#line 12 "unit/generics/embed.sp"
__attribute__((weak)) _Bool Maybe__float__has_val(Maybe__float * this) {
    return this->has;
}


#line 14 "unit/generics/embed.sp"
__attribute__((weak)) void Maybe__float__set_val(Maybe__float * this, float v) {
    this->has = 1;
    this->val = v;
}


#line 13 "unit/generics/embed.sp"
__attribute__((weak)) float Maybe__float__get_val(Maybe__float * this) {
    return this->val;
}


#line 28 "unit/generics/embed.sp"
__attribute__((weak)) void Labeled__int__set_label(Labeled__int * this, int val) {
    this->label = val;
}


#line 29 "unit/generics/embed.sp"
__attribute__((weak)) void Labeled__int__set_xy(Labeled__int * this, int x, int y) {
    this->pos.x = x;
    this->pos.y = y;
}


#line 27 "unit/generics/embed.sp"
__attribute__((weak)) int Labeled__int__get_label(Labeled__int * this) {
    return this->label;
}


#line 30 "unit/generics/embed.sp"
__attribute__((weak)) int Labeled__int__pos_x(Labeled__int * this) {
    return this->pos.x;
}


#line 28 "unit/generics/embed.sp"
__attribute__((weak)) void Labeled__float__set_label(Labeled__float * this, float val) {
    this->label = val;
}


#line 27 "unit/generics/embed.sp"
__attribute__((weak)) float Labeled__float__get_label(Labeled__float * this) {
    return this->label;
}


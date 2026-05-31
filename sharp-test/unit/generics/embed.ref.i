
#line 17 "sharp-test/unit/generics/embed.ce"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 22 "sharp-test/unit/generics/embed.ce"

#line 7 "sharp-test/unit/generics/embed.ce"
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



#line 22 "sharp-test/unit/generics/embed.ce"
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



#line 7 "sharp-test/unit/generics/embed.ce"
typedef struct Maybe__Maybe__int Maybe__Maybe__int;
struct Maybe__Maybe__int {
    _Bool has;
    Maybe__int val;
};



#line 12 "sharp-test/unit/generics/embed.ce"
_Bool Maybe__int__has_val(Maybe__int * this);

#line 14 "sharp-test/unit/generics/embed.ce"
void Maybe__int__set_val(Maybe__int * this, int v);

#line 13 "sharp-test/unit/generics/embed.ce"
int Maybe__int__get_val(Maybe__int * this);

#line 12 "sharp-test/unit/generics/embed.ce"
_Bool Maybe__float__has_val(Maybe__float * this);

#line 14 "sharp-test/unit/generics/embed.ce"
void Maybe__float__set_val(Maybe__float * this, float v);

#line 13 "sharp-test/unit/generics/embed.ce"
float Maybe__float__get_val(Maybe__float * this);

#line 28 "sharp-test/unit/generics/embed.ce"
void Labeled__int__set_label(Labeled__int * this, int val);

#line 29 "sharp-test/unit/generics/embed.ce"
void Labeled__int__set_xy(Labeled__int * this, int x, int y);

#line 27 "sharp-test/unit/generics/embed.ce"
int Labeled__int__get_label(Labeled__int * this);

#line 30 "sharp-test/unit/generics/embed.ce"
int Labeled__int__pos_x(Labeled__int * this);

#line 28 "sharp-test/unit/generics/embed.ce"
void Labeled__float__set_label(Labeled__float * this, float val);

#line 27 "sharp-test/unit/generics/embed.ce"
float Labeled__float__get_label(Labeled__float * this);

#line 12 "sharp-test/unit/generics/embed.ce"
_Bool Maybe__Maybe__int__has_val(Maybe__Maybe__int * this);

#line 14 "sharp-test/unit/generics/embed.ce"
void Maybe__Maybe__int__set_val(Maybe__Maybe__int * this, Maybe__int v);

#line 13 "sharp-test/unit/generics/embed.ce"
Maybe__int Maybe__Maybe__int__get_val(Maybe__Maybe__int * this);

#line 31 "sharp-test/unit/generics/embed.ce"
int Labeled__int__move_x(Labeled__int * this, int dx);

#line 12 "sharp-test/unit/generics/embed.ce"

#line 13 "sharp-test/unit/generics/embed.ce"

#line 14 "sharp-test/unit/generics/embed.ce"

#line 27 "sharp-test/unit/generics/embed.ce"

#line 28 "sharp-test/unit/generics/embed.ce"

#line 29 "sharp-test/unit/generics/embed.ce"

#line 30 "sharp-test/unit/generics/embed.ce"

#line 31 "sharp-test/unit/generics/embed.ce"

#line 33 "sharp-test/unit/generics/embed.ce"
int main() {

#line 35 "sharp-test/unit/generics/embed.ce"
    Maybe__int mi = { 0 };

#line 36 "sharp-test/unit/generics/embed.ce"
    if (Maybe__int__has_val(&mi)) 
        return 1;

#line 38 "sharp-test/unit/generics/embed.ce"
    Maybe__int__set_val(&mi, 42);

#line 39 "sharp-test/unit/generics/embed.ce"
    if (!Maybe__int__has_val(&mi)) 
        return 2;

#line 40 "sharp-test/unit/generics/embed.ce"
    if (Maybe__int__get_val(&mi) != 42) 
        return 3;

#line 42 "sharp-test/unit/generics/embed.ce"
    Maybe__float mf = { 0 };

#line 43 "sharp-test/unit/generics/embed.ce"
    if (Maybe__float__has_val(&mf)) 
        return 4;

#line 45 "sharp-test/unit/generics/embed.ce"
    Maybe__float__set_val(&mf, 1.5f);

#line 46 "sharp-test/unit/generics/embed.ce"
    if (Maybe__float__get_val(&mf) < 1.49f || Maybe__float__get_val(&mf) > 1.51f) 
        return 5;

#line 49 "sharp-test/unit/generics/embed.ce"
    Labeled__int a = { 0 };

#line 50 "sharp-test/unit/generics/embed.ce"
    Labeled__int__set_label(&a, 42);

#line 51 "sharp-test/unit/generics/embed.ce"
    Labeled__int__set_xy(&a, 10, 20);

#line 53 "sharp-test/unit/generics/embed.ce"
    if (Labeled__int__get_label(&a) != 42) 
        return 6;

#line 54 "sharp-test/unit/generics/embed.ce"
    if (Labeled__int__pos_x(&a) != 10) 
        return 7;

#line 56 "sharp-test/unit/generics/embed.ce"
    Labeled__float b = { 0 };

#line 57 "sharp-test/unit/generics/embed.ce"
    Labeled__float__set_label(&b, 3.14f);

#line 58 "sharp-test/unit/generics/embed.ce"
    if (Labeled__float__get_label(&b) < 3.13f || Labeled__float__get_label(&b) > 3.15f) 
        return 8;

#line 61 "sharp-test/unit/generics/embed.ce"
    Maybe__Maybe__int outer = { 0 };

#line 62 "sharp-test/unit/generics/embed.ce"
    if (Maybe__Maybe__int__has_val(&outer)) 
        return 9;

#line 63 "sharp-test/unit/generics/embed.ce"
    Maybe__int inner = { 0 };

#line 64 "sharp-test/unit/generics/embed.ce"
    Maybe__int__set_val(&inner, 42);

#line 65 "sharp-test/unit/generics/embed.ce"
    Maybe__Maybe__int__set_val(&outer, inner);

#line 66 "sharp-test/unit/generics/embed.ce"
    if (!Maybe__Maybe__int__has_val(&outer)) 
        return 10;

#line 67 "sharp-test/unit/generics/embed.ce"
    Maybe__int retrieved = Maybe__Maybe__int__get_val(&outer);

#line 68 "sharp-test/unit/generics/embed.ce"
    if (!Maybe__int__has_val(&retrieved)) 
        return 11;

#line 69 "sharp-test/unit/generics/embed.ce"
    if (Maybe__int__get_val(&retrieved) != 42) 
        return 12;

#line 72 "sharp-test/unit/generics/embed.ce"
    Labeled__int c = { 0 };

#line 73 "sharp-test/unit/generics/embed.ce"
    Labeled__int__set_label(&c, 99);

#line 74 "sharp-test/unit/generics/embed.ce"
    Labeled__int__set_xy(&c, 5, 10);

#line 75 "sharp-test/unit/generics/embed.ce"
    if (Labeled__int__move_x(&c, 3) != 8) 
        return 13;

#line 76 "sharp-test/unit/generics/embed.ce"
    if (Labeled__int__pos_x(&c) != 8) 
        return 14;

#line 78 "sharp-test/unit/generics/embed.ce"
    return 0;
}

#line 12 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) _Bool Maybe__int__has_val(Maybe__int * this) {
    return this->has;
}


#line 14 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) void Maybe__int__set_val(Maybe__int * this, int v) {
    this->has = 1;
    this->val = v;
}


#line 13 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) int Maybe__int__get_val(Maybe__int * this) {
    return this->val;
}


#line 12 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) _Bool Maybe__float__has_val(Maybe__float * this) {
    return this->has;
}


#line 14 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) void Maybe__float__set_val(Maybe__float * this, float v) {
    this->has = 1;
    this->val = v;
}


#line 13 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) float Maybe__float__get_val(Maybe__float * this) {
    return this->val;
}


#line 28 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) void Labeled__int__set_label(Labeled__int * this, int val) {
    this->label = val;
}


#line 29 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) void Labeled__int__set_xy(Labeled__int * this, int x, int y) {
    this->pos.x = x;
    this->pos.y = y;
}


#line 27 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) int Labeled__int__get_label(Labeled__int * this) {
    return this->label;
}


#line 30 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) int Labeled__int__pos_x(Labeled__int * this) {
    return this->pos.x;
}


#line 28 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) void Labeled__float__set_label(Labeled__float * this, float val) {
    this->label = val;
}


#line 27 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) float Labeled__float__get_label(Labeled__float * this) {
    return this->label;
}


#line 12 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) _Bool Maybe__Maybe__int__has_val(Maybe__Maybe__int * this) {
    return this->has;
}


#line 14 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) void Maybe__Maybe__int__set_val(Maybe__Maybe__int * this, Maybe__int v) {
    this->has = 1;
    this->val = v;
}


#line 13 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) Maybe__int Maybe__Maybe__int__get_val(Maybe__Maybe__int * this) {
    return this->val;
}


#line 31 "sharp-test/unit/generics/embed.ce"
__attribute__((weak)) int Labeled__int__move_x(Labeled__int * this, int dx) {
    this->pos.x = this->pos.x + dx;
    return this->pos.x;
}


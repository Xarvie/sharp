
#line 2 "sharp-test/unit/extensions/ext_const.ce"
typedef struct Vec2 Vec2;
struct Vec2 {
    int x;
    int y;
};

#line 7 "sharp-test/unit/extensions/ext_const.ce"
int Vec2__length_sq(Vec2 * this);

#line 8 "sharp-test/unit/extensions/ext_const.ce"
void Vec2__scale(Vec2 * this, int s);

#line 10 "sharp-test/unit/extensions/ext_const.ce"
typedef struct Rect Rect;
struct Rect {
    int w;
    int h;
};

#line 15 "sharp-test/unit/extensions/ext_const.ce"
int Rect__area(Rect * this);

#line 16 "sharp-test/unit/extensions/ext_const.ce"
int Rect__perimeter(Rect * this);

#line 7 "sharp-test/unit/extensions/ext_const.ce"
int Vec2__length_sq(Vec2 * this) {
    return this->x * this->x + this->y * this->y;
}

#line 8 "sharp-test/unit/extensions/ext_const.ce"
void Vec2__scale(Vec2 * this, int s) {
    this->x = this->x * s;
    this->y = this->y * s;
}

#line 15 "sharp-test/unit/extensions/ext_const.ce"
int Rect__area(Rect * this) {
    return this->w * this->h;
}

#line 16 "sharp-test/unit/extensions/ext_const.ce"
int Rect__perimeter(Rect * this) {
    return 2 * (this->w + this->h);
}

#line 18 "sharp-test/unit/extensions/ext_const.ce"
int main() {

#line 20 "sharp-test/unit/extensions/ext_const.ce"
    Vec2 v;
    v.x = 3;
    v.y = 4;

#line 21 "sharp-test/unit/extensions/ext_const.ce"
    if (Vec2__length_sq(&v) != 25) 
        return 1;

#line 24 "sharp-test/unit/extensions/ext_const.ce"
    Vec2__scale(&v, 2);

#line 25 "sharp-test/unit/extensions/ext_const.ce"
    if (v.x != 6) 
        return 2;

#line 26 "sharp-test/unit/extensions/ext_const.ce"
    if (v.y != 8) 
        return 3;

#line 27 "sharp-test/unit/extensions/ext_const.ce"
    if (Vec2__length_sq(&v) != 100) 
        return 4;

#line 30 "sharp-test/unit/extensions/ext_const.ce"
    Rect r;
    r.w = 5;
    r.h = 3;

#line 31 "sharp-test/unit/extensions/ext_const.ce"
    int a = Rect__area(&r);

#line 32 "sharp-test/unit/extensions/ext_const.ce"
    int p = Rect__perimeter(&r);

#line 33 "sharp-test/unit/extensions/ext_const.ce"
    if (a != 15) 
        return 5;

#line 34 "sharp-test/unit/extensions/ext_const.ce"
    if (p != 16) 
        return 6;

#line 37 "sharp-test/unit/extensions/ext_const.ce"
    const Rect * pr = &r;

#line 38 "sharp-test/unit/extensions/ext_const.ce"
    if (Rect__area(pr) != 15) 
        return 7;

#line 39 "sharp-test/unit/extensions/ext_const.ce"
    if (Rect__perimeter(pr) != 16) 
        return 8;

#line 41 "sharp-test/unit/extensions/ext_const.ce"
    return 0;
}

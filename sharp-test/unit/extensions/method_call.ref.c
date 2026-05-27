#include <stdbool.h>

#include <stdlib.h>

#line 6 "unit/extensions/method_call.sp"
typedef struct Rect Rect;
struct Rect {
    int w;
    int h;
};

#line 11 "unit/extensions/method_call.sp"
int Rect__area(Rect * this);

#line 12 "unit/extensions/method_call.sp"
int Rect__perimeter(Rect * this);

#line 15 "unit/extensions/method_call.sp"
_Bool Rect__is_square(Rect * this);

#line 11 "unit/extensions/method_call.sp"
int Rect__area(Rect * this) {
    return this->w * this->h;
}

#line 12 "unit/extensions/method_call.sp"
int Rect__perimeter(Rect * this) {
    return 2 * (this->w + this->h);
}

#line 15 "unit/extensions/method_call.sp"
_Bool Rect__is_square(Rect * this) {

#line 16 "unit/extensions/method_call.sp"
    return this->w == this->h;
}

#line 19 "unit/extensions/method_call.sp"
int main() {

#line 20 "unit/extensions/method_call.sp"
    Rect r;

#line 21 "unit/extensions/method_call.sp"
    r.w = 5;
    r.h = 7;

#line 22 "unit/extensions/method_call.sp"
    int a = Rect__area(&r);

#line 23 "unit/extensions/method_call.sp"
    if (a != 35) 
        return 1;

#line 24 "unit/extensions/method_call.sp"
    int p = Rect__perimeter(&r);

#line 25 "unit/extensions/method_call.sp"
    if (p != 24) 
        return 2;

#line 26 "unit/extensions/method_call.sp"
    if (Rect__is_square(&r)) 
        return 3;

#line 28 "unit/extensions/method_call.sp"
    Rect s2;

#line 29 "unit/extensions/method_call.sp"
    s2.w = 10;
    s2.h = 10;

#line 30 "unit/extensions/method_call.sp"
    if (!Rect__is_square(&s2)) 
        return 4;

#line 31 "unit/extensions/method_call.sp"
    int a2 = Rect__area(&s2);

#line 32 "unit/extensions/method_call.sp"
    if (a2 != 100) 
        return 5;

#line 34 "unit/extensions/method_call.sp"
    return 0;
}

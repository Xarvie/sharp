
#include <stdlib.h>

#line 5 "unit/const/method.sp"
typedef struct Vec Vec;
struct Vec {
    int * data;
    long len;
    long cap;
};

#line 10 "unit/const/method.sp"
long Vec__size(Vec * this);

#line 11 "unit/const/method.sp"
void Vec__push(Vec * this, int item);

#line 14 "unit/const/method.sp"
typedef struct Counter Counter;
struct Counter {
    long count;
};

#line 17 "unit/const/method.sp"
void Counter__inc(Counter * this);

#line 18 "unit/const/method.sp"
long Counter__get(Counter * this);

#line 21 "unit/const/method.sp"
typedef struct BBox BBox;
struct BBox {
    long x;
    long y;
    long w;
    long h;
};

#line 24 "unit/const/method.sp"
long BBox__area(BBox * this);

#line 25 "unit/const/method.sp"
void BBox__move(BBox * this, long dx, long dy);

#line 26 "unit/const/method.sp"
long BBox__at_x(BBox * this);

#line 27 "unit/const/method.sp"
long BBox__at_y(BBox * this);

#line 10 "unit/const/method.sp"
long Vec__size(Vec * this) {
    return this->len;
}

#line 11 "unit/const/method.sp"
void Vec__push(Vec * this, int item) {
    this->data[this->len] = item;
    this->len++;
}

#line 17 "unit/const/method.sp"
void Counter__inc(Counter * this) {
    this->count = this->count + 1;
}

#line 18 "unit/const/method.sp"
long Counter__get(Counter * this) {
    return this->count;
}

#line 24 "unit/const/method.sp"
long BBox__area(BBox * this) {
    return this->w * this->h;
}

#line 25 "unit/const/method.sp"
void BBox__move(BBox * this, long dx, long dy) {
    this->x = this->x + dx;
    this->y = this->y + dy;
}

#line 26 "unit/const/method.sp"
long BBox__at_x(BBox * this) {
    return this->x;
}

#line 27 "unit/const/method.sp"
long BBox__at_y(BBox * this) {
    return this->y;
}

#line 29 "unit/const/method.sp"
int main(void) {

#line 31 "unit/const/method.sp"
    int arr[10];

#line 32 "unit/const/method.sp"
    Vec v = { arr, 0, 10 };

#line 33 "unit/const/method.sp"
    Vec__push(&v, 42);

#line 34 "unit/const/method.sp"
    Vec__push(&v, 99);

#line 35 "unit/const/method.sp"
    long s = Vec__size(&v);

#line 36 "unit/const/method.sp"
    if (s != 2) 
        return 1;

#line 39 "unit/const/method.sp"
    Counter c = { 0 };

#line 40 "unit/const/method.sp"
    Counter__inc(&c);

#line 41 "unit/const/method.sp"
    Counter__inc(&c);

#line 42 "unit/const/method.sp"
    long cv = Counter__get(&c);

#line 43 "unit/const/method.sp"
    if (cv != 2) 
        return 2;

#line 46 "unit/const/method.sp"
    BBox b = { 1, 2, 10, 20 };

#line 47 "unit/const/method.sp"
    long a = BBox__area(&b);

#line 48 "unit/const/method.sp"
    BBox__move(&b, 3, 4);

#line 49 "unit/const/method.sp"
    long sx = BBox__at_x(&b);

#line 50 "unit/const/method.sp"
    long sy = BBox__at_y(&b);

#line 51 "unit/const/method.sp"
    if (a + sx + sy != 210) 
        return 3;

#line 53 "unit/const/method.sp"
    return 0;
}

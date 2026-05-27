
#include <stdlib.h>

#line 5 "unit/extensions/basic.sp"
typedef struct Str Str;
struct Str {
    const char * ptr;
    long len;
};

#line 10 "unit/extensions/basic.sp"
long Str__length(Str * this);

#line 12 "unit/extensions/basic.sp"
int Str__starts_with(Str * this, const char * prefix);

#line 22 "unit/extensions/basic.sp"
long Str__find(Str * this, char c);

#line 38 "unit/extensions/basic.sp"
struct Box {
    int x;
    int y;
};

#line 10 "unit/extensions/basic.sp"
long Str__length(Str * this) {
    return this->len;
}

#line 12 "unit/extensions/basic.sp"
int Str__starts_with(Str * this, const char * prefix) {

#line 13 "unit/extensions/basic.sp"
    long i = 0;

#line 14 "unit/extensions/basic.sp"
    while (prefix[i] != 0) {

#line 15 "unit/extensions/basic.sp"
        if (i >= this->len) 
            return 0;

#line 16 "unit/extensions/basic.sp"
        if (this->ptr[i] != prefix[i]) 
            return 0;

#line 17 "unit/extensions/basic.sp"
        i = i + 1;
    }

#line 19 "unit/extensions/basic.sp"
    return 1;
}

#line 22 "unit/extensions/basic.sp"
long Str__find(Str * this, char c) {

#line 23 "unit/extensions/basic.sp"
    for (long i = 0; i < this->len; i = i + 1) 

#line 24 "unit/extensions/basic.sp"
        if (this->ptr[i] == c) 
            return i;

#line 25 "unit/extensions/basic.sp"
    return (long)-1;
}

#line 29 "unit/extensions/basic.sp"
long array_sum(const int * a, long n) {

#line 30 "unit/extensions/basic.sp"
    long s = 0;

#line 31 "unit/extensions/basic.sp"
    for (long i = 0; i < n; i = i + 1) 
        s = s + (long)a[i];

#line 32 "unit/extensions/basic.sp"
    return s;
}

#line 34 "unit/extensions/basic.sp"
float array_avg(const int * a, long n) {

#line 35 "unit/extensions/basic.sp"
    return (float)array_sum(a, n) / (float)n;
}

#line 39 "unit/extensions/basic.sp"
long box_hypot(struct Box b) {

#line 40 "unit/extensions/basic.sp"
    return (long)b.x * (long)b.x + (long)b.y * (long)b.y;
}

#line 43 "unit/extensions/basic.sp"
int main() {

#line 45 "unit/extensions/basic.sp"
    Str s;
    s.ptr = "hello world";
    s.len = 11;

#line 46 "unit/extensions/basic.sp"
    if (Str__length(&s) != 11) 
        return 1;

#line 47 "unit/extensions/basic.sp"
    if (Str__starts_with(&s, "hello") != 1) 
        return 2;

#line 48 "unit/extensions/basic.sp"
    if (Str__starts_with(&s, "world") != 0) 
        return 3;

#line 49 "unit/extensions/basic.sp"
    long pos_w = Str__find(&s, 'w');

#line 50 "unit/extensions/basic.sp"
    long pos_z = Str__find(&s, 'z');

#line 51 "unit/extensions/basic.sp"
    if (pos_w != 6) 
        return 4;

#line 52 "unit/extensions/basic.sp"
    if (pos_z != -1) 
        return 5;

#line 55 "unit/extensions/basic.sp"
    int arr[5] = { 1, 2, 3, 4, 5 };

#line 56 "unit/extensions/basic.sp"
    long sum = array_sum(arr, 5);

#line 57 "unit/extensions/basic.sp"
    float avg = array_avg(arr, 5);

#line 58 "unit/extensions/basic.sp"
    if (sum != 15) 
        return 6;

#line 59 "unit/extensions/basic.sp"
    if (avg < 2.9f || avg > 3.1f) 
        return 7;

#line 61 "unit/extensions/basic.sp"
    struct Box b = { 3, 4 };

#line 62 "unit/extensions/basic.sp"
    long h = box_hypot(b);

#line 63 "unit/extensions/basic.sp"
    if (h != 25) 
        return 8;

#line 65 "unit/extensions/basic.sp"
    return 0;
}

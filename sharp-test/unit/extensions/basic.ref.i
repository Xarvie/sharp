
#line 5 "sharp-test/unit/extensions/basic.ce"
typedef struct Str Str;
struct Str {
    const char * ptr;
    long len;
};

#line 10 "sharp-test/unit/extensions/basic.ce"
long Str__length(Str * this);

#line 12 "sharp-test/unit/extensions/basic.ce"
int Str__starts_with(Str * this, const char * prefix);

#line 22 "sharp-test/unit/extensions/basic.ce"
long Str__find(Str * this, char c);

#line 28 "sharp-test/unit/extensions/basic.ce"
int Str__count_char(Str * this, char c);

#line 35 "sharp-test/unit/extensions/basic.ce"
typedef struct Counter Counter;
struct Counter {
    int n;
};

#line 38 "sharp-test/unit/extensions/basic.ce"
int Counter__increment(Counter * this);

#line 53 "sharp-test/unit/extensions/basic.ce"
struct Box {
    int x;
    int y;
};

#line 10 "sharp-test/unit/extensions/basic.ce"
long Str__length(Str * this) {
    return this->len;
}

#line 12 "sharp-test/unit/extensions/basic.ce"
int Str__starts_with(Str * this, const char * prefix) {

#line 13 "sharp-test/unit/extensions/basic.ce"
    long i = 0;

#line 14 "sharp-test/unit/extensions/basic.ce"
    while (prefix[i] != 0) {

#line 15 "sharp-test/unit/extensions/basic.ce"
        if (i >= this->len) 
            return 0;

#line 16 "sharp-test/unit/extensions/basic.ce"
        if (this->ptr[i] != prefix[i]) 
            return 0;

#line 17 "sharp-test/unit/extensions/basic.ce"
        i = i + 1;
    }

#line 19 "sharp-test/unit/extensions/basic.ce"
    return 1;
}

#line 22 "sharp-test/unit/extensions/basic.ce"
long Str__find(Str * this, char c) {

#line 23 "sharp-test/unit/extensions/basic.ce"
    for (long i = 0; i < this->len; i = i + 1) 

#line 24 "sharp-test/unit/extensions/basic.ce"
        if (this->ptr[i] == c) 
            return i;

#line 25 "sharp-test/unit/extensions/basic.ce"
    return (long)-1;
}

#line 28 "sharp-test/unit/extensions/basic.ce"
int Str__count_char(Str * this, char c) {

#line 29 "sharp-test/unit/extensions/basic.ce"
    int count = 0;

#line 30 "sharp-test/unit/extensions/basic.ce"
    for (long i = 0; i < this->len; i = i + 1) 

#line 31 "sharp-test/unit/extensions/basic.ce"
        if (this->ptr[i] == c) 
            count = count + 1;

#line 32 "sharp-test/unit/extensions/basic.ce"
    return count;
}

#line 38 "sharp-test/unit/extensions/basic.ce"
int Counter__increment(Counter * this) {

#line 39 "sharp-test/unit/extensions/basic.ce"
    this->n = this->n + 1;

#line 40 "sharp-test/unit/extensions/basic.ce"
    return this->n;
}

#line 44 "sharp-test/unit/extensions/basic.ce"
long array_sum(const int * a, long n) {

#line 45 "sharp-test/unit/extensions/basic.ce"
    long s = 0;

#line 46 "sharp-test/unit/extensions/basic.ce"
    for (long i = 0; i < n; i = i + 1) 
        s = s + (long)a[i];

#line 47 "sharp-test/unit/extensions/basic.ce"
    return s;
}

#line 49 "sharp-test/unit/extensions/basic.ce"
float array_avg(const int * a, long n) {

#line 50 "sharp-test/unit/extensions/basic.ce"
    return (float)array_sum(a, n) / (float)n;
}

#line 54 "sharp-test/unit/extensions/basic.ce"
long box_hypot(struct Box b) {

#line 55 "sharp-test/unit/extensions/basic.ce"
    return (long)b.x * (long)b.x + (long)b.y * (long)b.y;
}

#line 58 "sharp-test/unit/extensions/basic.ce"
int main() {

#line 60 "sharp-test/unit/extensions/basic.ce"
    Str s;
    s.ptr = "hello world";
    s.len = 11;

#line 61 "sharp-test/unit/extensions/basic.ce"
    if (Str__length(&s) != 11) 
        return 1;

#line 62 "sharp-test/unit/extensions/basic.ce"
    if (Str__starts_with(&s, "hello") != 1) 
        return 2;

#line 63 "sharp-test/unit/extensions/basic.ce"
    if (Str__starts_with(&s, "world") != 0) 
        return 3;

#line 64 "sharp-test/unit/extensions/basic.ce"
    long pos_w = Str__find(&s, 'w');

#line 65 "sharp-test/unit/extensions/basic.ce"
    long pos_z = Str__find(&s, 'z');

#line 66 "sharp-test/unit/extensions/basic.ce"
    if (pos_w != 6) 
        return 4;

#line 67 "sharp-test/unit/extensions/basic.ce"
    if (pos_z != -1) 
        return 5;

#line 70 "sharp-test/unit/extensions/basic.ce"
    int arr[5] = { 1, 2, 3, 4, 5 };

#line 71 "sharp-test/unit/extensions/basic.ce"
    long sum = array_sum(arr, 5);

#line 72 "sharp-test/unit/extensions/basic.ce"
    float avg = array_avg(arr, 5);

#line 73 "sharp-test/unit/extensions/basic.ce"
    if (sum != 15) 
        return 6;

#line 74 "sharp-test/unit/extensions/basic.ce"
    if (avg < 2.9f || avg > 3.1f) 
        return 7;

#line 76 "sharp-test/unit/extensions/basic.ce"
    struct Box b = { 3, 4 };

#line 77 "sharp-test/unit/extensions/basic.ce"
    long h = box_hypot(b);

#line 78 "sharp-test/unit/extensions/basic.ce"
    if (h != 25) 
        return 8;

#line 81 "sharp-test/unit/extensions/basic.ce"
    Counter cnt;
    cnt.n = 0;

#line 82 "sharp-test/unit/extensions/basic.ce"
    if (Counter__increment(&cnt) != 1) 
        return 9;

#line 83 "sharp-test/unit/extensions/basic.ce"
    if (Counter__increment(&cnt) != 2) 
        return 10;

#line 84 "sharp-test/unit/extensions/basic.ce"
    if (cnt.n != 2) 
        return 11;

#line 87 "sharp-test/unit/extensions/basic.ce"
    Str s2;
    s2.ptr = "abracadabra";
    s2.len = 11;

#line 88 "sharp-test/unit/extensions/basic.ce"
    if (Str__count_char(&s2, 'a') != 5) 
        return 12;

#line 89 "sharp-test/unit/extensions/basic.ce"
    if (Str__count_char(&s2, 'b') != 2) 
        return 13;

#line 91 "sharp-test/unit/extensions/basic.ce"
    return 0;
}

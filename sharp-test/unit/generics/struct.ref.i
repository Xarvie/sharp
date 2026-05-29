
#line 6 "sharp-test/unit/generics/struct.ce"

#line 12 "sharp-test/unit/generics/struct.ce"

#line 18 "sharp-test/unit/generics/struct.ce"

#line 23 "sharp-test/unit/generics/struct.ce"

#line 39 "sharp-test/unit/generics/struct.ce"

#line 6 "sharp-test/unit/generics/struct.ce"
typedef struct Vec__int Vec__int;
struct Vec__int {
    int data;
    int len;
};


typedef struct Vec__float Vec__float;
struct Vec__float {
    float data;
    int len;
};



#line 12 "sharp-test/unit/generics/struct.ce"
typedef struct Pair__int__int Pair__int__int;
struct Pair__int__int {
    int first;
    int second;
};


typedef struct Pair__int__float Pair__int__float;
struct Pair__int__float {
    int first;
    float second;
};



#line 18 "sharp-test/unit/generics/struct.ce"
typedef struct Box__int Box__int;
struct Box__int {
    int * value;
};



#line 23 "sharp-test/unit/generics/struct.ce"
typedef struct Stack__int Stack__int;
struct Stack__int {
    int top;
    int size;
};



#line 39 "sharp-test/unit/generics/struct.ce"
typedef struct Map__int__float Map__int__float;
struct Map__int__float {
    int * keys;
    float * values;
    int count;
};



#line 28 "sharp-test/unit/generics/struct.ce"
int Stack__int__push(Stack__int * this, int val);

#line 34 "sharp-test/unit/generics/struct.ce"
int Stack__int__peek(Stack__int * this);

#line 28 "sharp-test/unit/generics/struct.ce"

#line 34 "sharp-test/unit/generics/struct.ce"

#line 45 "sharp-test/unit/generics/struct.ce"
int main() {

#line 47 "sharp-test/unit/generics/struct.ce"
    Vec__int vi;

#line 48 "sharp-test/unit/generics/struct.ce"
    vi.data = 42;

#line 49 "sharp-test/unit/generics/struct.ce"
    vi.len = 1;

#line 50 "sharp-test/unit/generics/struct.ce"
    if (vi.data != 42) 
        return 1;

#line 51 "sharp-test/unit/generics/struct.ce"
    if (vi.len != 1) 
        return 2;

#line 53 "sharp-test/unit/generics/struct.ce"
    Vec__float vf;

#line 54 "sharp-test/unit/generics/struct.ce"
    vf.data = 3.14f;

#line 55 "sharp-test/unit/generics/struct.ce"
    vf.len = 1;

#line 56 "sharp-test/unit/generics/struct.ce"
    if (vf.len != 1) 
        return 3;

#line 59 "sharp-test/unit/generics/struct.ce"
    Pair__int__int p;

#line 60 "sharp-test/unit/generics/struct.ce"
    p.first = 10;

#line 61 "sharp-test/unit/generics/struct.ce"
    p.second = 20;

#line 62 "sharp-test/unit/generics/struct.ce"
    if (p.first != 10) 
        return 4;

#line 63 "sharp-test/unit/generics/struct.ce"
    if (p.second != 20) 
        return 5;

#line 65 "sharp-test/unit/generics/struct.ce"
    Pair__int__float pif;

#line 66 "sharp-test/unit/generics/struct.ce"
    pif.first = 7;

#line 67 "sharp-test/unit/generics/struct.ce"
    pif.second = 1.5f;

#line 68 "sharp-test/unit/generics/struct.ce"
    if (pif.first != 7) 
        return 6;

#line 71 "sharp-test/unit/generics/struct.ce"
    int x = 100;

#line 72 "sharp-test/unit/generics/struct.ce"
    Box__int box;

#line 73 "sharp-test/unit/generics/struct.ce"
    box.value = &x;

#line 74 "sharp-test/unit/generics/struct.ce"
    if (box.value[0] != 100) 
        return 7;

#line 77 "sharp-test/unit/generics/struct.ce"
    Stack__int s;

#line 78 "sharp-test/unit/generics/struct.ce"
    s.size = 0;

#line 79 "sharp-test/unit/generics/struct.ce"
    Stack__int__push(&s, 42);

#line 80 "sharp-test/unit/generics/struct.ce"
    if (Stack__int__peek(&s) != 42) 
        return 8;

#line 81 "sharp-test/unit/generics/struct.ce"
    if (s.size != 1) 
        return 9;

#line 82 "sharp-test/unit/generics/struct.ce"
    Stack__int__push(&s, 99);

#line 83 "sharp-test/unit/generics/struct.ce"
    if (Stack__int__peek(&s) != 99) 
        return 10;

#line 84 "sharp-test/unit/generics/struct.ce"
    if (s.size != 2) 
        return 11;

#line 87 "sharp-test/unit/generics/struct.ce"
    Pair__int__float lit = { .first = 7, .second = 3.14f };

#line 88 "sharp-test/unit/generics/struct.ce"
    if (lit.first != 7) 
        return 12;

#line 89 "sharp-test/unit/generics/struct.ce"
    if (lit.second < 3.13f || lit.second > 3.15f) 
        return 13;

#line 92 "sharp-test/unit/generics/struct.ce"
    Map__int__float m;

#line 93 "sharp-test/unit/generics/struct.ce"
    m.count = 5;

#line 94 "sharp-test/unit/generics/struct.ce"
    if (m.count != 5) 
        return 14;

#line 96 "sharp-test/unit/generics/struct.ce"
    return 0;
}

#line 28 "sharp-test/unit/generics/struct.ce"
__attribute__((weak)) int Stack__int__push(Stack__int * this, int val) {

#line 29 "sharp-test/unit/generics/struct.ce"
    this->top = val;

#line 30 "sharp-test/unit/generics/struct.ce"
    this->size = this->size + 1;

#line 31 "sharp-test/unit/generics/struct.ce"
    return this->size;
}


#line 34 "sharp-test/unit/generics/struct.ce"
__attribute__((weak)) int Stack__int__peek(Stack__int * this) {

#line 35 "sharp-test/unit/generics/struct.ce"
    return this->top;
}



#line 2 "sharp-test/unit/extensions/ext_generic.ce"

#line 12 "sharp-test/unit/extensions/ext_generic.ce"

#line 2 "sharp-test/unit/extensions/ext_generic.ce"
typedef struct Wrapper__int Wrapper__int;
struct Wrapper__int {
    int value;
};



#line 12 "sharp-test/unit/extensions/ext_generic.ce"
typedef struct Pair__int Pair__int;
struct Pair__int {
    int first;
    int second;
};



#line 7 "sharp-test/unit/extensions/ext_generic.ce"
void Wrapper__int__set(Wrapper__int * this, int v);

#line 6 "sharp-test/unit/extensions/ext_generic.ce"
int Wrapper__int__get(Wrapper__int * this);

#line 8 "sharp-test/unit/extensions/ext_generic.ce"
int Wrapper__int__transform(Wrapper__int * this, int (*fn)(int));

#line 17 "sharp-test/unit/extensions/ext_generic.ce"
int Pair__int__max_elem(Pair__int * this);

#line 21 "sharp-test/unit/extensions/ext_generic.ce"
void Pair__int__swap(Pair__int * this);

#line 6 "sharp-test/unit/extensions/ext_generic.ce"

#line 7 "sharp-test/unit/extensions/ext_generic.ce"

#line 8 "sharp-test/unit/extensions/ext_generic.ce"

#line 10 "sharp-test/unit/extensions/ext_generic.ce"
int double_it(int x) {
    return x * 2;
}

#line 17 "sharp-test/unit/extensions/ext_generic.ce"

#line 21 "sharp-test/unit/extensions/ext_generic.ce"

#line 27 "sharp-test/unit/extensions/ext_generic.ce"
int main() {

#line 29 "sharp-test/unit/extensions/ext_generic.ce"
    Wrapper__int w;

#line 30 "sharp-test/unit/extensions/ext_generic.ce"
    Wrapper__int__set(&w, 42);

#line 31 "sharp-test/unit/extensions/ext_generic.ce"
    if (Wrapper__int__get(&w) != 42) 
        return 1;

#line 32 "sharp-test/unit/extensions/ext_generic.ce"
    Wrapper__int__set(&w, 10);

#line 33 "sharp-test/unit/extensions/ext_generic.ce"
    if (Wrapper__int__get(&w) != 10) 
        return 2;

#line 36 "sharp-test/unit/extensions/ext_generic.ce"
    Wrapper__int w2;

#line 37 "sharp-test/unit/extensions/ext_generic.ce"
    Wrapper__int__set(&w2, 5);

#line 38 "sharp-test/unit/extensions/ext_generic.ce"
    int r = Wrapper__int__transform(&w2, double_it);

#line 39 "sharp-test/unit/extensions/ext_generic.ce"
    if (r != 10) 
        return 3;

#line 40 "sharp-test/unit/extensions/ext_generic.ce"
    if (Wrapper__int__get(&w2) != 10) 
        return 4;

#line 43 "sharp-test/unit/extensions/ext_generic.ce"
    Pair__int p;

#line 44 "sharp-test/unit/extensions/ext_generic.ce"
    p.first = 10;
    p.second = 20;

#line 45 "sharp-test/unit/extensions/ext_generic.ce"
    if (Pair__int__max_elem(&p) != 20) 
        return 5;

#line 46 "sharp-test/unit/extensions/ext_generic.ce"
    Pair__int__swap(&p);

#line 47 "sharp-test/unit/extensions/ext_generic.ce"
    if (p.first != 20) 
        return 6;

#line 48 "sharp-test/unit/extensions/ext_generic.ce"
    if (p.second != 10) 
        return 7;

#line 50 "sharp-test/unit/extensions/ext_generic.ce"
    return 0;
}

#line 7 "sharp-test/unit/extensions/ext_generic.ce"
__attribute__((weak)) void Wrapper__int__set(Wrapper__int * this, int v) {
    this->value = v;
}


#line 6 "sharp-test/unit/extensions/ext_generic.ce"
__attribute__((weak)) int Wrapper__int__get(Wrapper__int * this) {
    return this->value;
}


#line 8 "sharp-test/unit/extensions/ext_generic.ce"
__attribute__((weak)) int Wrapper__int__transform(Wrapper__int * this, int (*fn)(int)) {
    this->value = fn(this->value);
    return this->value;
}


#line 17 "sharp-test/unit/extensions/ext_generic.ce"
__attribute__((weak)) int Pair__int__max_elem(Pair__int * this) {

#line 18 "sharp-test/unit/extensions/ext_generic.ce"
    return this->first > this->second ? this->first : this->second;
}


#line 21 "sharp-test/unit/extensions/ext_generic.ce"
__attribute__((weak)) void Pair__int__swap(Pair__int * this) {

#line 22 "sharp-test/unit/extensions/ext_generic.ce"
    int tmp = this->first;

#line 23 "sharp-test/unit/extensions/ext_generic.ce"
    this->first = this->second;

#line 24 "sharp-test/unit/extensions/ext_generic.ce"
    this->second = tmp;
}


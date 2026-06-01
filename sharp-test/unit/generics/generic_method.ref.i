
#line 2 "sharp-test/unit/generics/generic_method.ce"
typedef struct Stack__int Stack__int;
struct Stack__int {
    int data[16];
    int top;
};


typedef struct Stack__float Stack__float;
struct Stack__float {
    float data[16];
    int top;
};



#line 7 "sharp-test/unit/generics/generic_method.ce"
void Stack__int__push(Stack__int * this, int val);

#line 17 "sharp-test/unit/generics/generic_method.ce"
int Stack__int__size(Stack__int * this);

#line 12 "sharp-test/unit/generics/generic_method.ce"
int Stack__int__pop(Stack__int * this);

#line 7 "sharp-test/unit/generics/generic_method.ce"
void Stack__float__push(Stack__float * this, float val);

#line 17 "sharp-test/unit/generics/generic_method.ce"
int Stack__float__size(Stack__float * this);

#line 12 "sharp-test/unit/generics/generic_method.ce"
float Stack__float__pop(Stack__float * this);

#line 7 "sharp-test/unit/generics/generic_method.ce"

#line 12 "sharp-test/unit/generics/generic_method.ce"

#line 17 "sharp-test/unit/generics/generic_method.ce"

#line 19 "sharp-test/unit/generics/generic_method.ce"
int main() {

#line 21 "sharp-test/unit/generics/generic_method.ce"
    Stack__int si;

#line 22 "sharp-test/unit/generics/generic_method.ce"
    si.top = 0;

#line 23 "sharp-test/unit/generics/generic_method.ce"
    Stack__int__push(&si, 10);

#line 24 "sharp-test/unit/generics/generic_method.ce"
    Stack__int__push(&si, 20);

#line 25 "sharp-test/unit/generics/generic_method.ce"
    Stack__int__push(&si, 30);

#line 26 "sharp-test/unit/generics/generic_method.ce"
    if (Stack__int__size(&si) != 3) 
        return 1;

#line 27 "sharp-test/unit/generics/generic_method.ce"
    if (Stack__int__pop(&si) != 30) 
        return 2;

#line 28 "sharp-test/unit/generics/generic_method.ce"
    if (Stack__int__pop(&si) != 20) 
        return 3;

#line 29 "sharp-test/unit/generics/generic_method.ce"
    if (Stack__int__size(&si) != 1) 
        return 4;

#line 32 "sharp-test/unit/generics/generic_method.ce"
    Stack__float sf;

#line 33 "sharp-test/unit/generics/generic_method.ce"
    sf.top = 0;

#line 34 "sharp-test/unit/generics/generic_method.ce"
    Stack__float__push(&sf, 1.5f);

#line 35 "sharp-test/unit/generics/generic_method.ce"
    Stack__float__push(&sf, 2.5f);

#line 36 "sharp-test/unit/generics/generic_method.ce"
    if (Stack__float__size(&sf) != 2) 
        return 5;

#line 37 "sharp-test/unit/generics/generic_method.ce"
    float fv = Stack__float__pop(&sf);

#line 38 "sharp-test/unit/generics/generic_method.ce"
    if (fv < 2.49f || fv > 2.51f) 
        return 6;

#line 41 "sharp-test/unit/generics/generic_method.ce"
    Stack__int s1;
    s1.top = 0;

#line 42 "sharp-test/unit/generics/generic_method.ce"
    Stack__int s2;
    s2.top = 0;

#line 43 "sharp-test/unit/generics/generic_method.ce"
    Stack__int__push(&s1, 100);

#line 44 "sharp-test/unit/generics/generic_method.ce"
    Stack__int__push(&s2, 200);

#line 45 "sharp-test/unit/generics/generic_method.ce"
    if (Stack__int__pop(&s1) != 100) 
        return 7;

#line 46 "sharp-test/unit/generics/generic_method.ce"
    if (Stack__int__pop(&s2) != 200) 
        return 8;

#line 48 "sharp-test/unit/generics/generic_method.ce"
    return 0;
}

#line 7 "sharp-test/unit/generics/generic_method.ce"
__attribute__((weak)) void Stack__int__push(Stack__int * this, int val) {

#line 8 "sharp-test/unit/generics/generic_method.ce"
    this->data[this->top] = val;

#line 9 "sharp-test/unit/generics/generic_method.ce"
    this->top = this->top + 1;
}


#line 17 "sharp-test/unit/generics/generic_method.ce"
__attribute__((weak)) int Stack__int__size(Stack__int * this) {
    return this->top;
}


#line 12 "sharp-test/unit/generics/generic_method.ce"
__attribute__((weak)) int Stack__int__pop(Stack__int * this) {

#line 13 "sharp-test/unit/generics/generic_method.ce"
    this->top = this->top - 1;

#line 14 "sharp-test/unit/generics/generic_method.ce"
    return this->data[this->top];
}


#line 7 "sharp-test/unit/generics/generic_method.ce"
__attribute__((weak)) void Stack__float__push(Stack__float * this, float val) {

#line 8 "sharp-test/unit/generics/generic_method.ce"
    this->data[this->top] = val;

#line 9 "sharp-test/unit/generics/generic_method.ce"
    this->top = this->top + 1;
}


#line 17 "sharp-test/unit/generics/generic_method.ce"
__attribute__((weak)) int Stack__float__size(Stack__float * this) {
    return this->top;
}


#line 12 "sharp-test/unit/generics/generic_method.ce"
__attribute__((weak)) float Stack__float__pop(Stack__float * this) {

#line 13 "sharp-test/unit/generics/generic_method.ce"
    this->top = this->top - 1;

#line 14 "sharp-test/unit/generics/generic_method.ce"
    return this->data[this->top];
}


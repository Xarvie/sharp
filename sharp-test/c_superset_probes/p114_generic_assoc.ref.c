

#line 2 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
typedef struct Stack__int Stack__int;
struct Stack__int {
    int data;
    int n;
};



#line 6 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
Stack__int Stack__int__new(void);

#line 9 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
void Stack__int__push(Stack__int * this, int v);

#line 10 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
int Stack__int__top(Stack__int * this);

#line 12 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
int main(void) {

#line 13 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
    Stack__int s = Stack__int__new();

#line 14 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
    Stack__int__push(&s, 42);

#line 15 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
    if (Stack__int__top(&s) != 42) 
        return 1;

#line 16 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
    if (s.n != 1) 
        return 2;

#line 17 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
    return 0;
}

#line 6 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
__attribute__((weak)) Stack__int Stack__int__new(void) {

#line 7 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
    Stack__int s;
    s.n = 0;
    return s;
}


#line 9 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
__attribute__((weak)) void Stack__int__push(Stack__int * this, int v) {
    this->data = v;
    this->n = this->n + 1;
}


#line 10 "sharp-test/c_superset_probes/p114_generic_assoc.sp"
__attribute__((weak)) int Stack__int__top(Stack__int * this) {
    return this->data;
}


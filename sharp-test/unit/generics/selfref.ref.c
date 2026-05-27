
#include <stdlib.h>

#line 5 "unit/generics/selfref.sp"
typedef struct Node__int Node__int;
struct Node__int {
    int val;
    Node__int * next;
};



#line 10 "unit/generics/selfref.sp"
void Node__int__set_val(Node__int * this, int v);

#line 14 "unit/generics/selfref.sp"
void Node__int__set_next(Node__int * this, Node__int * n);

#line 18 "unit/generics/selfref.sp"
int Node__int__get_val(Node__int * this);

#line 10 "unit/generics/selfref.sp"

#line 14 "unit/generics/selfref.sp"

#line 18 "unit/generics/selfref.sp"

#line 22 "unit/generics/selfref.sp"
int main() {

#line 24 "unit/generics/selfref.sp"
    Node__int a;

#line 25 "unit/generics/selfref.sp"
    Node__int b;

#line 26 "unit/generics/selfref.sp"
    a.val = 1;

#line 27 "unit/generics/selfref.sp"
    b.val = 2;

#line 28 "unit/generics/selfref.sp"
    a.next = &b;

#line 29 "unit/generics/selfref.sp"
    b.next = (Node__int *)0;

#line 31 "unit/generics/selfref.sp"
    if (a.val != 1) 
        return 1;

#line 32 "unit/generics/selfref.sp"
    if (a.next->val != 2) 
        return 2;

#line 33 "unit/generics/selfref.sp"
    if (a.next->next != (Node__int *)0) 
        return 3;

#line 36 "unit/generics/selfref.sp"
    Node__int x = { 0 };

#line 37 "unit/generics/selfref.sp"
    Node__int__set_val(&x, 10);

#line 39 "unit/generics/selfref.sp"
    Node__int y = { 0 };

#line 40 "unit/generics/selfref.sp"
    Node__int__set_val(&y, 20);

#line 42 "unit/generics/selfref.sp"
    Node__int__set_next(&x, &y);

#line 44 "unit/generics/selfref.sp"
    if (Node__int__get_val(&x) != 10) 
        return 4;

#line 45 "unit/generics/selfref.sp"
    if (x.next->val != 20) 
        return 5;

#line 47 "unit/generics/selfref.sp"
    x.next->val = 30;

#line 48 "unit/generics/selfref.sp"
    if (y.val != 30) 
        return 6;

#line 51 "unit/generics/selfref.sp"
    Node__int head = { 0 };

#line 52 "unit/generics/selfref.sp"
    Node__int__set_val(&head, 100);

#line 53 "unit/generics/selfref.sp"
    Node__int tail = { 0 };

#line 54 "unit/generics/selfref.sp"
    Node__int__set_val(&tail, 200);

#line 55 "unit/generics/selfref.sp"
    Node__int__set_next(&head, &tail);

#line 56 "unit/generics/selfref.sp"
    if (Node__int__get_val(&head) != 100) 
        return 7;

#line 57 "unit/generics/selfref.sp"
    if (head.next->val != 200) 
        return 8;

#line 59 "unit/generics/selfref.sp"
    return 0;
}

#line 10 "unit/generics/selfref.sp"
__attribute__((weak)) void Node__int__set_val(Node__int * this, int v) {

#line 11 "unit/generics/selfref.sp"
    this->val = v;
}


#line 14 "unit/generics/selfref.sp"
__attribute__((weak)) void Node__int__set_next(Node__int * this, Node__int * n) {

#line 15 "unit/generics/selfref.sp"
    this->next = n;
}


#line 18 "unit/generics/selfref.sp"
__attribute__((weak)) int Node__int__get_val(Node__int * this) {

#line 19 "unit/generics/selfref.sp"
    return this->val;
}


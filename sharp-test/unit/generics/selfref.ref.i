
#line 5 "sharp-test/unit/generics/selfref.ce"
typedef struct Node__int Node__int;
struct Node__int {
    int val;
    Node__int * next;
};



#line 10 "sharp-test/unit/generics/selfref.ce"
void Node__int__set_val(Node__int * this, int v);

#line 14 "sharp-test/unit/generics/selfref.ce"
void Node__int__set_next(Node__int * this, Node__int * n);

#line 18 "sharp-test/unit/generics/selfref.ce"
int Node__int__get_val(Node__int * this);

#line 10 "sharp-test/unit/generics/selfref.ce"

#line 14 "sharp-test/unit/generics/selfref.ce"

#line 18 "sharp-test/unit/generics/selfref.ce"

#line 22 "sharp-test/unit/generics/selfref.ce"
int main() {

#line 24 "sharp-test/unit/generics/selfref.ce"
    Node__int a;

#line 25 "sharp-test/unit/generics/selfref.ce"
    Node__int b;

#line 26 "sharp-test/unit/generics/selfref.ce"
    a.val = 1;

#line 27 "sharp-test/unit/generics/selfref.ce"
    b.val = 2;

#line 28 "sharp-test/unit/generics/selfref.ce"
    a.next = &b;

#line 29 "sharp-test/unit/generics/selfref.ce"
    b.next = (Node__int *)0;

#line 31 "sharp-test/unit/generics/selfref.ce"
    if (a.val != 1) 
        return 1;

#line 32 "sharp-test/unit/generics/selfref.ce"
    if (a.next->val != 2) 
        return 2;

#line 33 "sharp-test/unit/generics/selfref.ce"
    if (a.next->next != (Node__int *)0) 
        return 3;

#line 36 "sharp-test/unit/generics/selfref.ce"
    Node__int x = { 0 };

#line 37 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_val(&x, 10);

#line 39 "sharp-test/unit/generics/selfref.ce"
    Node__int y = { 0 };

#line 40 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_val(&y, 20);

#line 42 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_next(&x, &y);

#line 44 "sharp-test/unit/generics/selfref.ce"
    if (Node__int__get_val(&x) != 10) 
        return 4;

#line 45 "sharp-test/unit/generics/selfref.ce"
    if (x.next->val != 20) 
        return 5;

#line 47 "sharp-test/unit/generics/selfref.ce"
    x.next->val = 30;

#line 48 "sharp-test/unit/generics/selfref.ce"
    if (y.val != 30) 
        return 6;

#line 51 "sharp-test/unit/generics/selfref.ce"
    Node__int head = { 0 };

#line 52 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_val(&head, 100);

#line 53 "sharp-test/unit/generics/selfref.ce"
    Node__int tail = { 0 };

#line 54 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_val(&tail, 200);

#line 55 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_next(&head, &tail);

#line 56 "sharp-test/unit/generics/selfref.ce"
    if (Node__int__get_val(&head) != 100) 
        return 7;

#line 57 "sharp-test/unit/generics/selfref.ce"
    if (head.next->val != 200) 
        return 8;

#line 60 "sharp-test/unit/generics/selfref.ce"
    Node__int n1 = { 0 };

#line 61 "sharp-test/unit/generics/selfref.ce"
    Node__int n2 = { 0 };

#line 62 "sharp-test/unit/generics/selfref.ce"
    Node__int n3 = { 0 };

#line 63 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_val(&n1, 10);

#line 64 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_val(&n2, 20);

#line 65 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_val(&n3, 30);

#line 66 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_next(&n1, &n2);

#line 67 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_next(&n2, &n3);

#line 68 "sharp-test/unit/generics/selfref.ce"
    Node__int__set_next(&n3, (Node__int *)0);

#line 70 "sharp-test/unit/generics/selfref.ce"
    int total = 0;

#line 71 "sharp-test/unit/generics/selfref.ce"
    Node__int * cur = &n1;

#line 72 "sharp-test/unit/generics/selfref.ce"
    while (cur != (Node__int *)0) {

#line 73 "sharp-test/unit/generics/selfref.ce"
        total = total + Node__int__get_val(cur);

#line 74 "sharp-test/unit/generics/selfref.ce"
        cur = cur->next;
    }

#line 76 "sharp-test/unit/generics/selfref.ce"
    if (total != 60) 
        return 9;

#line 78 "sharp-test/unit/generics/selfref.ce"
    return 0;
}

#line 10 "sharp-test/unit/generics/selfref.ce"
__attribute__((weak)) void Node__int__set_val(Node__int * this, int v) {

#line 11 "sharp-test/unit/generics/selfref.ce"
    this->val = v;
}


#line 14 "sharp-test/unit/generics/selfref.ce"
__attribute__((weak)) void Node__int__set_next(Node__int * this, Node__int * n) {

#line 15 "sharp-test/unit/generics/selfref.ce"
    this->next = n;
}


#line 18 "sharp-test/unit/generics/selfref.ce"
__attribute__((weak)) int Node__int__get_val(Node__int * this) {

#line 19 "sharp-test/unit/generics/selfref.ce"
    return this->val;
}


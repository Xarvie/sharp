
#include <stdlib.h>

#line 8 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
typedef struct Node__int Node__int;
struct Node__int {
    int val;
    Node__int * next;
};



#line 14 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
void Node__int__set_val(Node__int * this, int v);

#line 15 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
void Node__int__set_next(Node__int * this, Node__int * n);

#line 13 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
int Node__int__get_val(Node__int * this);

#line 17 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
int main() {

#line 18 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    Node__int a = { 0 };

#line 19 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    Node__int__set_val(&a, 10);

#line 21 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    Node__int b = { 0 };

#line 22 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    Node__int__set_val(&b, 20);

#line 24 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    Node__int__set_next(&a, &b);

#line 26 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    if (Node__int__get_val(&a) != 10) 
        return 1;

#line 27 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    if (Node__int__get_val(a.next) != 20) 
        return 2;

#line 30 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    Node__int__set_val(a.next, 30);

#line 31 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    if (Node__int__get_val(&b) != 30) 
        return 3;

#line 33 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
    return 0;
}

#line 14 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
__attribute__((weak)) void Node__int__set_val(Node__int * this, int v) {
    this->val = v;
}


#line 15 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
__attribute__((weak)) void Node__int__set_next(Node__int * this, Node__int * n) {
    this->next = n;
}


#line 13 "sharp-test/c_superset_probes/p238_generic_selfref.sp"
__attribute__((weak)) int Node__int__get_val(Node__int * this) {
    return this->val;
}


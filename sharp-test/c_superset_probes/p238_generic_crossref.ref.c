
#include <stdlib.h>

#line 10 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
typedef struct Node__int Node__int;
struct Node__int {
    int val;
    Node__int * next;
};



#line 16 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
void Node__int__set_val(Node__int * this, int v);

#line 17 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
void Node__int__link(Node__int * this, Node__int * n);

#line 15 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
int Node__int__get_val(Node__int * this);

#line 19 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
int main() {

#line 20 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    Node__int a = { 0 };

#line 21 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    Node__int__set_val(&a, 10);

#line 23 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    Node__int b = { 0 };

#line 24 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    Node__int__set_val(&b, 20);

#line 26 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    Node__int__link(&a, &b);

#line 28 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    if (Node__int__get_val(&a) != 10) 
        return 1;

#line 29 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    if (Node__int__get_val(a.next) != 20) 
        return 2;

#line 32 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    Node__int__set_val(a.next, 30);

#line 33 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    if (Node__int__get_val(&b) != 30) 
        return 3;

#line 35 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
    return 0;
}

#line 16 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
__attribute__((weak)) void Node__int__set_val(Node__int * this, int v) {
    this->val = v;
}


#line 17 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
__attribute__((weak)) void Node__int__link(Node__int * this, Node__int * n) {
    this->next = n;
}


#line 15 "sharp-test/c_superset_probes/p238_generic_crossref.sp"
__attribute__((weak)) int Node__int__get_val(Node__int * this) {
    return this->val;
}


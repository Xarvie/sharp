/* pc00220 — C struct via #include, Sharp uses bare struct name (C++ style) */

#include <stdlib.h>

/* C struct defined via #include */
struct CIface {
    void *userdata;
    int refcount;
    struct CIface *next;  /* self-ref with struct keyword */
};

/* Sharp code uses bare CIface name (C++ style, like Sharp semantics) */
void reset_refcount(CIface *c) {
    c->refcount = 0;
    c->next = (CIface *)0;
}

CIface *create_c_iface(void) {
    CIface *p = (CIface *)malloc(sizeof(CIface));
    p->userdata = 0;
    p->refcount = 0;
    p->next = 0;
    return p;
}

int main(void) {
    CIface *c = create_c_iface();
    reset_refcount(c);
    free(c);
    return 0;
}

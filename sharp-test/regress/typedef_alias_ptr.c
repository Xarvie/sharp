/*
 * Regression test: typedef alias with pointer types must NOT expand.
 * Bug: cg_typedef_c was incorrectly expanding the underlying type.
 *
 * Expected: typedef Base *PBase;
 * Not:      typedef struct Base { ... } *PBase;
 */
struct Base {
    int x;
    double y;
};

typedef struct Base Base;
typedef Base *PBase;
typedef PBase *PPBase;

static void use_pbase(PBase p) { (void)p; }
static void use_ppbase(PPBase p) { (void)p; }

int main(void) {
    Base b;
    PBase pb = &b;
    PPBase ppb = &pb;
    use_pbase(pb);
    use_ppbase(ppb);
    return 0;
}

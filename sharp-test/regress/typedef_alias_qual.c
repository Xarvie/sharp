/*
 * Regression test: typedef alias with const/volatile qualifiers.
 * Expected: typedef const Base ConstBase;
 * Not:      typedef const struct Base { ... } ConstBase;
 */
struct Base {
    int x;
    double y;
};

typedef struct Base Base;
typedef const Base ConstBase;
typedef volatile Base VolBase;

static void use_const(ConstBase *p) { (void)p; }
static void use_vol(VolBase *p) { (void)p; }

int main(void) {
    ConstBase cb;
    VolBase vb;
    use_const(&cb);
    use_vol(&vb);
    return 0;
}

/*
 * sharpc bug: struct tag redefinition inside anonymous struct
 *
 * Expected: compiles without error (gcc/clang accept this)
 * Actual:   sharpc reports "redefinition of 'inner_tag'"
 *
 * Pattern from mbedtls2/include/mbedtls/ssl_internal.h:
 *   struct { struct inner_tag { ... }; } anonymous;
 *   typedef struct inner_tag inner_tag;
 *
 * Extensions cover:
 *  1. union variant (nested union tag)
 *  2. __attribute__((packed)) on nested struct
 *  3. struct tag used as pointer in other struct fields
 *  4. multiple levels of nesting
 *  5. typedef before struct body definition
 *  6. struct tag used in cast expression
 *  7. union with attribute on tag definition
 */

/* --- Original test case --- */

struct outer {
    struct {
        int x;
        struct inner_tag {
            int a;
            int b;
        } items[4];
    } buffering;
};

typedef struct inner_tag inner_tag;

/* --- Extension 1: union variant --- */

struct union_outer {
    struct {
        int x;
        union inner_utag {
            int i;
            float f;
        } u;
    } buffering;
};

typedef union inner_utag inner_utag;

/* --- Extension 2: __attribute__((packed)) on nested struct --- */

struct packed_outer {
    struct {
        int x;
        struct __attribute__((packed)) packed_tag {
            char a;
            int b;
        } items[2];
    } buffering;
};

typedef struct packed_tag packed_tag;

/* --- Extension 3: struct tag used as pointer in another field --- */

struct ptr_outer {
    struct {
        struct node_tag {
            struct node_tag *next;
            int value;
        } head;
        struct node_tag *cursor;
    } list;
};

typedef struct node_tag node_tag;

/* --- Extension 4: multiple levels of nesting --- */

struct deep_outer {
    struct {
        struct {
            struct deep_tag {
                int depth;
            } deep;
        } middle;
    } inner;
};

typedef struct deep_tag deep_tag;

/* --- Extension 5: typedef before struct body definition --- */

typedef struct forward_tag forward_tag;

struct uses_forward {
    struct {
        struct forward_tag {
            int val;
        } fwd;
    } data;
};

/* --- Extension 6: struct tag in cast expression --- */

struct cast_outer {
    struct {
        struct cast_tag {
            int x;
        } c;
    } buffering;
};

typedef struct cast_tag cast_tag;

/* --- Extension 7: union with attribute --- */

struct attr_union_outer {
    struct {
        union __attribute__((packed)) attr_utag {
            int i;
            short s;
        } u;
    } data;
};

typedef union attr_utag attr_utag;

/* --- main: exercise all cases --- */

int main(void) {
    /* Original */
    struct outer o;
    o.buffering.x = 1;
    o.buffering.items[0].a = 10;
    o.buffering.items[0].b = 20;
    inner_tag t = { .a = 3, .b = 4 };

    /* Extension 1: union variant */
    struct union_outer uo;
    uo.buffering.x = 2;
    uo.buffering.u.i = 42;
    inner_utag ut = { .f = 3.14f };

    /* Extension 2: packed nested struct */
    struct packed_outer po;
    po.buffering.x = 3;
    po.buffering.items[0].a = 'A';
    po.buffering.items[0].b = 12345;
    packed_tag pt = { .a = 'B', .b = 67890 };

    /* Extension 3: pointer usage */
    struct ptr_outer pro;
    pro.list.head.value = 100;
    pro.list.head.next = 0;
    pro.list.cursor = 0;
    node_tag nt;
    nt.value = 200;
    nt.next = 0;

    /* Extension 4: multiple nesting levels */
    struct deep_outer d;
    d.inner.middle.deep.depth = 3;
    deep_tag dt = { .depth = 4 };

    /* Extension 5: forward typedef */
    struct uses_forward uf;
    uf.data.fwd.val = 42;
    forward_tag ft = { .val = 99 };

    /* Extension 6: cast expression */
    struct cast_outer co;
    co.buffering.c.x = 77;
    void *p = (struct cast_tag *)0;
    void *p2 = (cast_tag *)0;
    cast_tag ct = { .x = 88 };

    /* Extension 7: union with attribute */
    struct attr_union_outer auo;
    auo.data.u.i = 12345;
    attr_utag aut = { .s = 42 };

    /* Suppress unused warnings */
    (void)o; (void)t;
    (void)uo; (void)ut;
    (void)po; (void)pt;
    (void)pro; (void)nt;
    (void)d; (void)dt;
    (void)uf; (void)ft;
    (void)co; (void)p; (void)p2; (void)ct;
    (void)auo; (void)aut;

    return 0;
}

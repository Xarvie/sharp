/*
 * Test: trailing __attribute__ on struct/union definitions
 * These patterns appear throughout glibc headers.
 * We test ALL variants to find which ones sharpc handles correctly.
 */

/* ===== packed variants ===== */

/* 1a. } __attribute__((packed)) */
struct p1a { char a; int b; } __attribute__((packed));

/* 1b. } __attribute__((__packed__)) */
struct p1b { char a; int b; } __attribute__((__packed__));

/* ===== aligned variants ===== */

/* 2a. } __attribute__((aligned(N))) */
struct a2a { char a; int b; } __attribute__((aligned(8)));

/* 2b. } __attribute__((__aligned__(N))) */
struct a2b { char a; int b; } __attribute__((__aligned__(16)));

/* ===== transparent_union variants ===== */

/* 3a. typedef union ... } __attribute__((transparent_union)) */
typedef union { int a; long b; } __attribute__((transparent_union)) tu3a;

/* 3b. typedef union ... } __attribute__((__transparent_union__)) */
typedef union { int a; long b; } __attribute__((__transparent_union__)) tu3b;

/* ===== may_alias variants ===== */

/* 4a. } __attribute__((may_alias)) */
struct ma4a { char data[4]; } __attribute__((may_alias));

/* 4b. } __attribute__((__may_alias__)) */
struct ma4b { char data[4]; } __attribute__((__may_alias__));

/* ===== nested trailing attrs (tftp.h style) ===== */

struct nested {
    short op;
    union {
        struct {
            unsigned short block;
        } __attribute__((packed)) inner;
    } __attribute__((packed)) outer;
} __attribute__((packed));

/* ===== epoll_event style (the reported bug) ===== */

typedef union {
    void *ptr;
    int fd;
    unsigned int u32;
    unsigned long long u64;
} epoll_data_t;

struct epoll_event_test {
    unsigned int events;
    epoll_data_t data;
} __attribute__((packed));

/* ===== __EPOLL_PACKED macro wrapper ===== */

#ifndef __EPOLL_PACKED
# define __EPOLL_PACKED __attribute__((packed))
#endif

struct epoll_macro_test {
    unsigned int events;
    epoll_data_t data;
} __EPOLL_PACKED;

/* ===== combined attrs ===== */

/* 5. packed + aligned together */
struct pa5 {
    char a;
    int b;
} __attribute__((packed, aligned(4)));

int main(void) {
    struct p1a v1a = {0, 0};
    struct a2a v2a = {0, 0};
    tu3a v3a = {0};
    struct ma4a v4a = {{0}};
    struct nested vn = {0, {{0}}};
    struct epoll_event_test ve = {0, {0}};
    struct epoll_macro_test vm = {0, {0}};
    struct pa5 v5 = {0, 0};
    (void)v1a; (void)v2a; (void)v3a; (void)v4a;
    (void)vn; (void)ve; (void)vm; (void)v5;
    return 0;
}

// 测试: typedef 匿名 struct + __attribute__((packed))
// Bug: sharpc 预处理丢失了匿名 struct 体

#include <stdint.h>

#if defined(__linux__) && defined(__x86_64__)
#define SHARP_PACKED __attribute__((__packed__))
#else
#define SHARP_PACKED
#endif

typedef struct {
    uint32_t events;     /* POLLER_* bitmask */
    uint64_t user_data;  /* opaque user data (pointer/index/id) */
} SHARP_PACKED poller_event_t;

int main(void) {
    poller_event_t ev;
    ev.events = 0xDEADBEEF;
    ev.user_data = 0xCAFEBABE12345678ULL;

    if (ev.events != 0xDEADBEEF) return 1;
    if (ev.user_data != 0xCAFEBABE12345678ULL) return 2;

    // verify struct size: 4 + 4(padding) + 8 = 16 without packed, 12 with packed
    if (sizeof(poller_event_t) != 12) return 3;

    return 0;
}

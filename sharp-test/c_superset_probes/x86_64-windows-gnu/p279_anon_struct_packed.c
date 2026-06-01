// 测试: typedef 匿名 struct (Windows — no packed attribute)
// On x86_64-windows-gnu, SHARP_PACKED is empty, so struct has padding.

#include <stdint.h>

#define SHARP_PACKED

typedef struct {
    uint32_t events;
    uint64_t user_data;
} SHARP_PACKED poller_event_t;

int main(void) {
    poller_event_t ev;
    ev.events = 0xDEADBEEF;
    ev.user_data = 0xCAFEBABE12345678ULL;

    if (ev.events != 0xDEADBEEF) return 1;
    if (ev.user_data != 0xCAFEBABE12345678ULL) return 2;

    if (sizeof(poller_event_t) != 16) return 3;

    return 0;
}

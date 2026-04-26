/* compiler: any */
/* Feature 9: #pragma pack(push/pop) with nested struct alignment */

#pragma pack(push, 1)

struct packed {
    char a;
    i32 b;
    char c;
}

#pragma pack(pop)

i32 main() {
    struct packed p;
    return sizeof(p);
}

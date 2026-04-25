/* compiler: any */
/* §10 #pragma pack */
#pragma pack(push, 1)

struct Packed {
    u8 a;
    i32 b;
};

#pragma pack(pop)

i32 main() {
    return 0;
}

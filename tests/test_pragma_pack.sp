/* compiler: any */
/* §10 #pragma pack */
#pragma pack(push, 1)

struct Packed {
    char a;
    int b;
};

#pragma pack(pop)

int main() {
    return 0;
}
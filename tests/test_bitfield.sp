/* compiler: any */
/* C Feature: Bit fields in structs */
/* Expected: Parser should handle unsigned int field : width syntax */

struct Flags {
    u32 a : 1;
    u32 b : 2;
    u32 c : 4;
    u32 d : 25;
}

i32 main() {
    return 0;
}

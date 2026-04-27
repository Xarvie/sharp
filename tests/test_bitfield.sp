/* compiler: any */
/* C Feature: Bit fields in structs */
/* Expected: Parser should handle unsigned int field : width syntax */

struct Flags {
    int a : 1;
    int b : 2;
    int c : 4;
    int d : 25;
}

int main() {
    return 0;
}

#line 3 "sharp-test/unit/cinterop/bitops.ce"
int main() {

#line 5 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int a = 0xFF00;

#line 6 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int b = 0x0FF0;

#line 7 "sharp-test/unit/cinterop/bitops.ce"
    if ((a & b) != 0x0F00) 
        return 1;

#line 8 "sharp-test/unit/cinterop/bitops.ce"
    if ((a | b) != 0xFFF0) 
        return 2;

#line 9 "sharp-test/unit/cinterop/bitops.ce"
    if ((a ^ b) != 0xF0F0) 
        return 3;

#line 12 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int c = 1;

#line 13 "sharp-test/unit/cinterop/bitops.ce"
    if ((c << 8) != 256) 
        return 4;

#line 14 "sharp-test/unit/cinterop/bitops.ce"
    if ((256 >> 4) != 16) 
        return 5;

#line 17 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int flags = 0;

#line 18 "sharp-test/unit/cinterop/bitops.ce"
    flags = flags | (1u << 3);

#line 19 "sharp-test/unit/cinterop/bitops.ce"
    if (!(flags & (1u << 3))) 
        return 6;

#line 20 "sharp-test/unit/cinterop/bitops.ce"
    flags = flags & ~(1u << 3);

#line 21 "sharp-test/unit/cinterop/bitops.ce"
    if (flags & (1u << 3)) 
        return 7;

#line 24 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int mask1 = 0x0F;

#line 25 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int mask2 = 0xF0;

#line 26 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int combined = mask1 | mask2;

#line 27 "sharp-test/unit/cinterop/bitops.ce"
    if (combined != 0xFF) 
        return 8;

#line 28 "sharp-test/unit/cinterop/bitops.ce"
    if ((combined & mask1) != mask1) 
        return 9;

#line 29 "sharp-test/unit/cinterop/bitops.ce"
    if ((combined & mask2) != mask2) 
        return 10;

#line 32 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int d = 0x0000FFFF;

#line 33 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int e = ~d;

#line 34 "sharp-test/unit/cinterop/bitops.ce"
    unsigned int top16 = e >> 16;

#line 35 "sharp-test/unit/cinterop/bitops.ce"
    if (top16 != 0xFFFF) 
        return 11;

#line 37 "sharp-test/unit/cinterop/bitops.ce"
    return 0;
}

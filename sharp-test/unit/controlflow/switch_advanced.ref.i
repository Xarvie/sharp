
#line 5 "sharp-test/unit/controlflow/switch_advanced.ce"
enum Direction {
    DIR_NORTH,
    DIR_SOUTH,
    DIR_EAST,
    DIR_WEST
};

#line 12 "sharp-test/unit/controlflow/switch_advanced.ce"
enum Priority {
    PRIO_LOW = 1,
    PRIO_MED = 5,
    PRIO_HIGH = 10,
    PRIO_CRIT = 99
};

#line 19 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_fallthrough_accumulate(int mode) {

#line 20 "sharp-test/unit/controlflow/switch_advanced.ce"
    int acc = 0;

#line 21 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (mode) {

#line 22 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 3:
        acc = acc + 100;

#line 23 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:
        acc = acc + 10;

#line 24 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:
        acc = acc + 1;

#line 25 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;

#line 26 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        acc = -1;
    }

#line 28 "sharp-test/unit/controlflow/switch_advanced.ce"
    return acc;
}

#line 31 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_default_first(int val) {

#line 32 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 33 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 34 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = -1;
        break;

#line 35 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:
        result = 10;
        break;

#line 36 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:
        result = 20;
        break;
    }

#line 38 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 41 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_default_middle(int val) {

#line 42 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 43 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 44 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:
        result = 10;
        break;

#line 45 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = -1;
        break;

#line 46 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:
        result = 20;
        break;
    }

#line 48 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 51 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_nested_switch(int outer, int inner) {

#line 52 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 53 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (outer) {

#line 54 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:

#line 55 "sharp-test/unit/controlflow/switch_advanced.ce"
        switch (inner) {

#line 56 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 1:
            result = 11;
            break;

#line 57 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 2:
            result = 12;
            break;

#line 58 "sharp-test/unit/controlflow/switch_advanced.ce"
            default:
            result = 19;
            break;
        }

#line 60 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;

#line 61 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:

#line 62 "sharp-test/unit/controlflow/switch_advanced.ce"
        switch (inner) {

#line 63 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 1:
            result = 21;
            break;

#line 64 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 2:
            result = 22;
            break;

#line 65 "sharp-test/unit/controlflow/switch_advanced.ce"
            default:
            result = 29;
            break;
        }

#line 67 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;

#line 68 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:

#line 69 "sharp-test/unit/controlflow/switch_advanced.ce"
        result = 99;

#line 70 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;
    }

#line 72 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 75 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_triple_nested(int a, int b, int c) {

#line 76 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 77 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (a) {

#line 78 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:

#line 79 "sharp-test/unit/controlflow/switch_advanced.ce"
        switch (b) {

#line 80 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 1:

#line 81 "sharp-test/unit/controlflow/switch_advanced.ce"
            switch (c) {

#line 82 "sharp-test/unit/controlflow/switch_advanced.ce"
                case 1:
                result = 111;
                break;

#line 83 "sharp-test/unit/controlflow/switch_advanced.ce"
                case 2:
                result = 112;
                break;

#line 84 "sharp-test/unit/controlflow/switch_advanced.ce"
                default:
                result = 119;
                break;
            }

#line 86 "sharp-test/unit/controlflow/switch_advanced.ce"
            break;

#line 87 "sharp-test/unit/controlflow/switch_advanced.ce"
            default:
            result = 190;
            break;
        }

#line 89 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;

#line 90 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = 900;
        break;
    }

#line 92 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 95 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_break_mid_case(int val) {

#line 96 "sharp-test/unit/controlflow/switch_advanced.ce"
    int x = 0;

#line 97 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 98 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:

#line 99 "sharp-test/unit/controlflow/switch_advanced.ce"
        x = x + 1;

#line 100 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;

#line 101 "sharp-test/unit/controlflow/switch_advanced.ce"
        x = x + 100;

#line 102 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:

#line 103 "sharp-test/unit/controlflow/switch_advanced.ce"
        x = x + 2;

#line 104 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;

#line 105 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:

#line 106 "sharp-test/unit/controlflow/switch_advanced.ce"
        x = x + 9;

#line 107 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;
    }

#line 109 "sharp-test/unit/controlflow/switch_advanced.ce"
    return x;
}

#line 112 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_duff_device(int count) {

#line 113 "sharp-test/unit/controlflow/switch_advanced.ce"
    int sum = 0;

#line 114 "sharp-test/unit/controlflow/switch_advanced.ce"
    int n = count;

#line 115 "sharp-test/unit/controlflow/switch_advanced.ce"
    int i = 0;

#line 116 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (n % 4) {

#line 117 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 0:
        sum = sum + i;
        i = i + 1;

#line 118 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 3:
        sum = sum + i;
        i = i + 1;

#line 119 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:
        sum = sum + i;
        i = i + 1;

#line 120 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:
        sum = sum + i;
        i = i + 1;
    }

#line 122 "sharp-test/unit/controlflow/switch_advanced.ce"
    return sum;
}

#line 125 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_duff_device_loop(int total) {

#line 126 "sharp-test/unit/controlflow/switch_advanced.ce"
    int sum = 0;

#line 127 "sharp-test/unit/controlflow/switch_advanced.ce"
    int n = total;

#line 128 "sharp-test/unit/controlflow/switch_advanced.ce"
    int vals[16];

#line 129 "sharp-test/unit/controlflow/switch_advanced.ce"
    for (int k = 0; k < 16; k = k + 1) 
        vals[k] = k + 1;

#line 131 "sharp-test/unit/controlflow/switch_advanced.ce"
    int idx = 0;

#line 132 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (n % 4) {

#line 133 "sharp-test/unit/controlflow/switch_advanced.ce"
        do {

#line 134 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 0:
            sum = sum + vals[idx];
            idx = idx + 1;

#line 135 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 3:
            sum = sum + vals[idx];
            idx = idx + 1;

#line 136 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 2:
            sum = sum + vals[idx];
            idx = idx + 1;

#line 137 "sharp-test/unit/controlflow/switch_advanced.ce"
            case 1:
            sum = sum + vals[idx];
            idx = idx + 1;
        }
        while ((n = n - 4) > 0);
    }

#line 140 "sharp-test/unit/controlflow/switch_advanced.ce"
    return sum;
}

#line 143 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_large_case_values(int val) {

#line 144 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 145 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 146 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 0:
        result = 1;
        break;

#line 147 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1000:
        result = 2;
        break;

#line 148 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 100000:
        result = 3;
        break;

#line 149 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2147483647:
        result = 4;
        break;

#line 150 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = -1;
        break;
    }

#line 152 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 155 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_large_sparse_case(int val) {

#line 156 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 157 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 158 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:
        result = 10;
        break;

#line 159 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 500:
        result = 20;
        break;

#line 160 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 99999:
        result = 30;
        break;

#line 161 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = 0;
        break;
    }

#line 163 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 166 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_char_switch(char c) {

#line 167 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 168 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (c) {

#line 169 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 'A':
        result = 1;
        break;

#line 170 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 'B':
        result = 2;
        break;

#line 171 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 'Z':
        result = 26;
        break;

#line 172 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 'a':
        result = 27;
        break;

#line 173 "sharp-test/unit/controlflow/switch_advanced.ce"
        case '0':
        result = 48;
        break;

#line 174 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = -1;
        break;
    }

#line 176 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 179 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_char_fallthrough(char c) {

#line 180 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 181 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (c) {

#line 182 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 'A':
        case 'a':
        result = 1;
        break;

#line 183 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 'B':
        case 'b':
        result = 2;
        break;

#line 184 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = -1;
        break;
    }

#line 186 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 189 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_enum_switch(enum Direction d) {

#line 190 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 191 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (d) {

#line 192 "sharp-test/unit/controlflow/switch_advanced.ce"
        case DIR_NORTH:
        result = 1;
        break;

#line 193 "sharp-test/unit/controlflow/switch_advanced.ce"
        case DIR_SOUTH:
        result = 2;
        break;

#line 194 "sharp-test/unit/controlflow/switch_advanced.ce"
        case DIR_EAST:
        result = 3;
        break;

#line 195 "sharp-test/unit/controlflow/switch_advanced.ce"
        case DIR_WEST:
        result = 4;
        break;
    }

#line 197 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 200 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_enum_with_default(enum Priority p) {

#line 201 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 202 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (p) {

#line 203 "sharp-test/unit/controlflow/switch_advanced.ce"
        case PRIO_LOW:
        result = 1;
        break;

#line 204 "sharp-test/unit/controlflow/switch_advanced.ce"
        case PRIO_MED:
        result = 5;
        break;

#line 205 "sharp-test/unit/controlflow/switch_advanced.ce"
        case PRIO_HIGH:
        result = 10;
        break;

#line 206 "sharp-test/unit/controlflow/switch_advanced.ce"
        case PRIO_CRIT:
        result = 99;
        break;

#line 207 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = -1;
        break;
    }

#line 209 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 212 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_enum_fallthrough(enum Direction d) {

#line 213 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 214 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (d) {

#line 215 "sharp-test/unit/controlflow/switch_advanced.ce"
        case DIR_NORTH:
        result = result + 1;

#line 216 "sharp-test/unit/controlflow/switch_advanced.ce"
        case DIR_SOUTH:
        result = result + 2;

#line 217 "sharp-test/unit/controlflow/switch_advanced.ce"
        case DIR_EAST:
        result = result + 4;

#line 218 "sharp-test/unit/controlflow/switch_advanced.ce"
        case DIR_WEST:
        result = result + 8;

#line 219 "sharp-test/unit/controlflow/switch_advanced.ce"
        break;
    }

#line 221 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 224 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_switch_no_default(int val) {

#line 225 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 42;

#line 226 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 227 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:
        result = 10;
        break;

#line 228 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:
        result = 20;
        break;
    }

#line 230 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 233 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_switch_only_default(int val) {

#line 234 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 235 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 236 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = 99;
        break;
    }

#line 238 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 241 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_switch_empty_case(int val) {

#line 242 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 243 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 244 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:

#line 245 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:

#line 246 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 3:
        result = 30;
        break;

#line 247 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 4:

#line 248 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 5:
        result = 50;
        break;

#line 249 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = 0;
        break;
    }

#line 251 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 254 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_switch_return_from_case(int val) {

#line 255 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 256 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:
        return 10;

#line 257 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 2:
        return 20;

#line 258 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 3:
        return 30;

#line 259 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        return 0;
    }
}

#line 263 "sharp-test/unit/controlflow/switch_advanced.ce"
int test_switch_negative_case(int val) {

#line 264 "sharp-test/unit/controlflow/switch_advanced.ce"
    int result = 0;

#line 265 "sharp-test/unit/controlflow/switch_advanced.ce"
    switch (val) {

#line 266 "sharp-test/unit/controlflow/switch_advanced.ce"
        case -3:
        result = -3;
        break;

#line 267 "sharp-test/unit/controlflow/switch_advanced.ce"
        case -2:
        result = -2;
        break;

#line 268 "sharp-test/unit/controlflow/switch_advanced.ce"
        case -1:
        result = -1;
        break;

#line 269 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 0:
        result = 0;
        break;

#line 270 "sharp-test/unit/controlflow/switch_advanced.ce"
        case 1:
        result = 1;
        break;

#line 271 "sharp-test/unit/controlflow/switch_advanced.ce"
        default:
        result = 99;
        break;
    }

#line 273 "sharp-test/unit/controlflow/switch_advanced.ce"
    return result;
}

#line 276 "sharp-test/unit/controlflow/switch_advanced.ce"
int main(void) {

#line 278 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_fallthrough_accumulate(1) != 1) 
        return 1;

#line 279 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_fallthrough_accumulate(2) != 11) 
        return 2;

#line 280 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_fallthrough_accumulate(3) != 111) 
        return 3;

#line 281 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_fallthrough_accumulate(0) != -1) 
        return 4;

#line 282 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_fallthrough_accumulate(4) != -1) 
        return 5;

#line 285 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_default_first(1) != 10) 
        return 10;

#line 286 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_default_first(2) != 20) 
        return 11;

#line 287 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_default_first(3) != -1) 
        return 12;

#line 288 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_default_first(0) != -1) 
        return 13;

#line 291 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_default_middle(1) != 10) 
        return 20;

#line 292 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_default_middle(2) != 20) 
        return 21;

#line 293 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_default_middle(3) != -1) 
        return 22;

#line 296 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_nested_switch(1, 1) != 11) 
        return 30;

#line 297 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_nested_switch(1, 2) != 12) 
        return 31;

#line 298 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_nested_switch(1, 3) != 19) 
        return 32;

#line 299 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_nested_switch(2, 1) != 21) 
        return 33;

#line 300 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_nested_switch(2, 2) != 22) 
        return 34;

#line 301 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_nested_switch(3, 1) != 99) 
        return 35;

#line 304 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_triple_nested(1, 1, 1) != 111) 
        return 40;

#line 305 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_triple_nested(1, 1, 2) != 112) 
        return 41;

#line 306 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_triple_nested(1, 1, 3) != 119) 
        return 42;

#line 307 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_triple_nested(1, 2, 1) != 190) 
        return 43;

#line 308 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_triple_nested(2, 1, 1) != 900) 
        return 44;

#line 311 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_break_mid_case(1) != 1) 
        return 50;

#line 312 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_break_mid_case(2) != 2) 
        return 51;

#line 313 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_break_mid_case(3) != 9) 
        return 52;

#line 316 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_duff_device(1) != 0) 
        return 60;

#line 317 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_duff_device(2) != 1) 
        return 61;

#line 318 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_duff_device(3) != 3) 
        return 62;

#line 319 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_duff_device(4) != 6) 
        return 63;

#line 322 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_duff_device_loop(4) != 10) 
        return 70;

#line 323 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_duff_device_loop(8) != 36) 
        return 71;

#line 324 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_duff_device_loop(12) != 78) 
        return 72;

#line 327 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_case_values(0) != 1) 
        return 80;

#line 328 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_case_values(1000) != 2) 
        return 81;

#line 329 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_case_values(100000) != 3) 
        return 82;

#line 330 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_case_values(2147483647) != 4) 
        return 83;

#line 331 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_case_values(999) != -1) 
        return 84;

#line 334 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_sparse_case(1) != 10) 
        return 90;

#line 335 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_sparse_case(500) != 20) 
        return 91;

#line 336 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_sparse_case(99999) != 30) 
        return 92;

#line 337 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_large_sparse_case(50) != 0) 
        return 93;

#line 340 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_switch('A') != 1) 
        return 100;

#line 341 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_switch('B') != 2) 
        return 101;

#line 342 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_switch('Z') != 26) 
        return 102;

#line 343 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_switch('a') != 27) 
        return 103;

#line 344 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_switch('0') != 48) 
        return 104;

#line 345 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_switch('x') != -1) 
        return 105;

#line 348 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_fallthrough('A') != 1) 
        return 110;

#line 349 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_fallthrough('a') != 1) 
        return 111;

#line 350 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_fallthrough('B') != 2) 
        return 112;

#line 351 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_fallthrough('b') != 2) 
        return 113;

#line 352 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_char_fallthrough('c') != -1) 
        return 114;

#line 355 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_switch(DIR_NORTH) != 1) 
        return 120;

#line 356 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_switch(DIR_SOUTH) != 2) 
        return 121;

#line 357 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_switch(DIR_EAST) != 3) 
        return 122;

#line 358 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_switch(DIR_WEST) != 4) 
        return 123;

#line 361 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_with_default(PRIO_LOW) != 1) 
        return 130;

#line 362 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_with_default(PRIO_MED) != 5) 
        return 131;

#line 363 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_with_default(PRIO_HIGH) != 10) 
        return 132;

#line 364 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_with_default(PRIO_CRIT) != 99) 
        return 133;

#line 365 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_with_default(50) != -1) 
        return 134;

#line 368 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_fallthrough(DIR_NORTH) != 15) 
        return 140;

#line 369 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_fallthrough(DIR_SOUTH) != 14) 
        return 141;

#line 370 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_fallthrough(DIR_EAST) != 12) 
        return 142;

#line 371 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_enum_fallthrough(DIR_WEST) != 8) 
        return 143;

#line 374 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_no_default(1) != 10) 
        return 150;

#line 375 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_no_default(2) != 20) 
        return 151;

#line 376 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_no_default(3) != 42) 
        return 152;

#line 379 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_only_default(0) != 99) 
        return 160;

#line 380 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_only_default(1) != 99) 
        return 161;

#line 381 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_only_default(-1) != 99) 
        return 162;

#line 384 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_empty_case(1) != 30) 
        return 170;

#line 385 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_empty_case(2) != 30) 
        return 171;

#line 386 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_empty_case(3) != 30) 
        return 172;

#line 387 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_empty_case(4) != 50) 
        return 173;

#line 388 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_empty_case(5) != 50) 
        return 174;

#line 389 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_empty_case(6) != 0) 
        return 175;

#line 392 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_return_from_case(1) != 10) 
        return 180;

#line 393 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_return_from_case(2) != 20) 
        return 181;

#line 394 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_return_from_case(3) != 30) 
        return 182;

#line 395 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_return_from_case(4) != 0) 
        return 183;

#line 398 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_negative_case(-3) != -3) 
        return 190;

#line 399 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_negative_case(-2) != -2) 
        return 191;

#line 400 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_negative_case(-1) != -1) 
        return 192;

#line 401 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_negative_case(0) != 0) 
        return 193;

#line 402 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_negative_case(1) != 1) 
        return 194;

#line 403 "sharp-test/unit/controlflow/switch_advanced.ce"
    if (test_switch_negative_case(2) != 99) 
        return 195;

#line 405 "sharp-test/unit/controlflow/switch_advanced.ce"
    return 0;
}

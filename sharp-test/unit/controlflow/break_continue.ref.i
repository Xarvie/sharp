
#line 3 "sharp-test/unit/controlflow/break_continue.ce"
int main() {

#line 5 "sharp-test/unit/controlflow/break_continue.ce"
    int sum1 = 0;

#line 6 "sharp-test/unit/controlflow/break_continue.ce"
    for (int i = 0; i < 100; i = i + 1) {

#line 7 "sharp-test/unit/controlflow/break_continue.ce"
        if (i > 5) 
            break;

#line 8 "sharp-test/unit/controlflow/break_continue.ce"
        sum1 = sum1 + i;
    }

#line 10 "sharp-test/unit/controlflow/break_continue.ce"
    if (sum1 != 15) 
        return 1;

#line 13 "sharp-test/unit/controlflow/break_continue.ce"
    int sum2 = 0;

#line 14 "sharp-test/unit/controlflow/break_continue.ce"
    int i2 = 0;

#line 15 "sharp-test/unit/controlflow/break_continue.ce"
    while (1) {

#line 16 "sharp-test/unit/controlflow/break_continue.ce"
        if (i2 >= 5) 
            break;

#line 17 "sharp-test/unit/controlflow/break_continue.ce"
        sum2 = sum2 + i2;

#line 18 "sharp-test/unit/controlflow/break_continue.ce"
        i2 = i2 + 1;
    }

#line 20 "sharp-test/unit/controlflow/break_continue.ce"
    if (sum2 != 10) 
        return 2;

#line 23 "sharp-test/unit/controlflow/break_continue.ce"
    int sum3 = 0;

#line 24 "sharp-test/unit/controlflow/break_continue.ce"
    int i3 = 0;

#line 25 "sharp-test/unit/controlflow/break_continue.ce"
    do {

#line 26 "sharp-test/unit/controlflow/break_continue.ce"
        if (i3 > 3) 
            break;

#line 27 "sharp-test/unit/controlflow/break_continue.ce"
        sum3 = sum3 + i3;

#line 28 "sharp-test/unit/controlflow/break_continue.ce"
        i3 = i3 + 1;
    }
    while (1);

#line 30 "sharp-test/unit/controlflow/break_continue.ce"
    if (sum3 != 6) 
        return 3;

#line 33 "sharp-test/unit/controlflow/break_continue.ce"
    int sum4 = 0;

#line 34 "sharp-test/unit/controlflow/break_continue.ce"
    for (int i = 0; i < 10; i = i + 1) {

#line 35 "sharp-test/unit/controlflow/break_continue.ce"
        if (i % 3 == 0) 
            continue;

#line 36 "sharp-test/unit/controlflow/break_continue.ce"
        sum4 = sum4 + i;
    }

#line 38 "sharp-test/unit/controlflow/break_continue.ce"
    if (sum4 != 27) 
        return 4;

#line 41 "sharp-test/unit/controlflow/break_continue.ce"
    int outer_broke = 0;

#line 42 "sharp-test/unit/controlflow/break_continue.ce"
    for (int i = 0; i < 5; i = i + 1) {

#line 43 "sharp-test/unit/controlflow/break_continue.ce"
        for (int j = 0; j < 5; j = j + 1) {

#line 44 "sharp-test/unit/controlflow/break_continue.ce"
            if (i + j > 3) {

#line 45 "sharp-test/unit/controlflow/break_continue.ce"
                outer_broke = 1;

#line 46 "sharp-test/unit/controlflow/break_continue.ce"
                break;
            }
        }

#line 49 "sharp-test/unit/controlflow/break_continue.ce"
        if (outer_broke) 
            break;
    }

#line 51 "sharp-test/unit/controlflow/break_continue.ce"
    if (!outer_broke) 
        return 5;

#line 53 "sharp-test/unit/controlflow/break_continue.ce"
    return 0;
}


#line 4 "sharp-test/unit/controlflow/goto_jmp.ce"
int log_buf[32];

#line 5 "sharp-test/unit/controlflow/goto_jmp.ce"
int logn = 0;

#line 6 "sharp-test/unit/controlflow/goto_jmp.ce"
void mark(int v) {
    log_buf[logn++] = v;
}

#line 8 "sharp-test/unit/controlflow/goto_jmp.ce"
int main() {

#line 10 "sharp-test/unit/controlflow/goto_jmp.ce"
    int x = 0;

#line 11 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_set;

#line 12 "sharp-test/unit/controlflow/goto_jmp.ce"
    x = 99;

#line 13 "sharp-test/unit/controlflow/goto_jmp.ce"
L_set:

#line 14 "sharp-test/unit/controlflow/goto_jmp.ce"
    x = 42;

#line 15 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (x != 42) 
        return 1;
    int count = 0, i = 0;

#line 19 "sharp-test/unit/controlflow/goto_jmp.ce"
L_loop:

#line 20 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (i >= 5) 
        goto L_done;

#line 21 "sharp-test/unit/controlflow/goto_jmp.ce"
    count = count + i;

#line 22 "sharp-test/unit/controlflow/goto_jmp.ce"
    i = i + 1;

#line 23 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_loop;

#line 24 "sharp-test/unit/controlflow/goto_jmp.ce"
L_done:

#line 25 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (count != 10) 
        return 2;

#line 28 "sharp-test/unit/controlflow/goto_jmp.ce"
    int y = 5;

#line 29 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_skip_decl;

#line 30 "sharp-test/unit/controlflow/goto_jmp.ce"
{

#line 31 "sharp-test/unit/controlflow/goto_jmp.ce"
        int local = 99;

#line 32 "sharp-test/unit/controlflow/goto_jmp.ce"
        (void)local;
    }

#line 34 "sharp-test/unit/controlflow/goto_jmp.ce"
L_skip_decl:

#line 35 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (y != 5) 
        return 3;

#line 38 "sharp-test/unit/controlflow/goto_jmp.ce"
    int val = 0;

#line 39 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (val == 0) 
        goto L_A;

#line 40 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_B;

#line 41 "sharp-test/unit/controlflow/goto_jmp.ce"
L_A:

#line 42 "sharp-test/unit/controlflow/goto_jmp.ce"
    val = 1;

#line 43 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_C;

#line 44 "sharp-test/unit/controlflow/goto_jmp.ce"
L_B:

#line 45 "sharp-test/unit/controlflow/goto_jmp.ce"
    val = 2;

#line 46 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_C;

#line 47 "sharp-test/unit/controlflow/goto_jmp.ce"
L_C:

#line 48 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (val != 1) 
        return 4;

#line 51 "sharp-test/unit/controlflow/goto_jmp.ce"
{

#line 52 "sharp-test/unit/controlflow/goto_jmp.ce"
        int nested = 10;

#line 53 "sharp-test/unit/controlflow/goto_jmp.ce"
{

#line 54 "sharp-test/unit/controlflow/goto_jmp.ce"
            int deep = 20;

#line 55 "sharp-test/unit/controlflow/goto_jmp.ce"
            if (deep == 20) 
                goto L_out_nested;

#line 56 "sharp-test/unit/controlflow/goto_jmp.ce"
            nested = 99;
        }
    }

#line 59 "sharp-test/unit/controlflow/goto_jmp.ce"
L_out_nested:

#line 60 "sharp-test/unit/controlflow/goto_jmp.ce"
    val = 55;

#line 61 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (val != 55) 
        return 5;

#line 64 "sharp-test/unit/controlflow/goto_jmp.ce"
{

#line 65 "sharp-test/unit/controlflow/goto_jmp.ce"
        int nested = 10;

#line 66 "sharp-test/unit/controlflow/goto_jmp.ce"
{

#line 67 "sharp-test/unit/controlflow/goto_jmp.ce"
            int deep = 20;

#line 68 "sharp-test/unit/controlflow/goto_jmp.ce"
            if (deep == 20) 
                goto L_out_nested2;

#line 69 "sharp-test/unit/controlflow/goto_jmp.ce"
            nested = 99;
        }
    }

#line 72 "sharp-test/unit/controlflow/goto_jmp.ce"
L_out_nested2:

#line 73 "sharp-test/unit/controlflow/goto_jmp.ce"
    val = 66;

#line 74 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (val != 66) 
        return 6;

#line 77 "sharp-test/unit/controlflow/goto_jmp.ce"
    int sw_val = 0;

#line 78 "sharp-test/unit/controlflow/goto_jmp.ce"
    switch (2) {

#line 79 "sharp-test/unit/controlflow/goto_jmp.ce"
        case 1:
        sw_val = 10;
        break;

#line 80 "sharp-test/unit/controlflow/goto_jmp.ce"
        case 2:
        sw_val = 20;
        goto L_after_switch;

#line 81 "sharp-test/unit/controlflow/goto_jmp.ce"
        case 3:
        sw_val = 30;
        break;
    }

#line 83 "sharp-test/unit/controlflow/goto_jmp.ce"
    sw_val = sw_val + 100;

#line 84 "sharp-test/unit/controlflow/goto_jmp.ce"
L_after_switch:

#line 85 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (sw_val != 20) 
        return 8;

#line 88 "sharp-test/unit/controlflow/goto_jmp.ce"
    int state = 0;

#line 89 "sharp-test/unit/controlflow/goto_jmp.ce"
    int result = 0;

#line 90 "sharp-test/unit/controlflow/goto_jmp.ce"
L_state_0:

#line 91 "sharp-test/unit/controlflow/goto_jmp.ce"
    result = result + 1;

#line 92 "sharp-test/unit/controlflow/goto_jmp.ce"
    state = 1;

#line 93 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_state_1;

#line 94 "sharp-test/unit/controlflow/goto_jmp.ce"
L_state_1:

#line 95 "sharp-test/unit/controlflow/goto_jmp.ce"
    result = result + 10;

#line 96 "sharp-test/unit/controlflow/goto_jmp.ce"
    state = 2;

#line 97 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_state_2;

#line 98 "sharp-test/unit/controlflow/goto_jmp.ce"
L_state_2:

#line 99 "sharp-test/unit/controlflow/goto_jmp.ce"
    result = result + 100;

#line 100 "sharp-test/unit/controlflow/goto_jmp.ce"
    goto L_state_done;

#line 101 "sharp-test/unit/controlflow/goto_jmp.ce"
L_state_done:

#line 102 "sharp-test/unit/controlflow/goto_jmp.ce"
    if (result != 111) 
        return 9;

#line 104 "sharp-test/unit/controlflow/goto_jmp.ce"
    return 0;
}

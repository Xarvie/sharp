
#include <stdlib.h>

#line 4 "unit/controlflow/goto_jmp.sp"
int main() {

#line 6 "unit/controlflow/goto_jmp.sp"
    int x = 0;

#line 7 "unit/controlflow/goto_jmp.sp"
    goto L_set;

#line 8 "unit/controlflow/goto_jmp.sp"
    x = 99;

#line 9 "unit/controlflow/goto_jmp.sp"
L_set:

#line 10 "unit/controlflow/goto_jmp.sp"
    x = 42;

#line 11 "unit/controlflow/goto_jmp.sp"
    if (x != 42) 
        return 1;
    int count = 0, i = 0;

#line 15 "unit/controlflow/goto_jmp.sp"
L_loop:

#line 16 "unit/controlflow/goto_jmp.sp"
    if (i >= 5) 
        goto L_done;

#line 17 "unit/controlflow/goto_jmp.sp"
    count = count + i;

#line 18 "unit/controlflow/goto_jmp.sp"
    i = i + 1;

#line 19 "unit/controlflow/goto_jmp.sp"
    goto L_loop;

#line 20 "unit/controlflow/goto_jmp.sp"
L_done:

#line 21 "unit/controlflow/goto_jmp.sp"
    if (count != 10) 
        return 2;

#line 24 "unit/controlflow/goto_jmp.sp"
    int y = 5;

#line 25 "unit/controlflow/goto_jmp.sp"
    goto L_skip_decl;

#line 26 "unit/controlflow/goto_jmp.sp"
{

#line 27 "unit/controlflow/goto_jmp.sp"
        int local = 99;

#line 28 "unit/controlflow/goto_jmp.sp"
        (void)local;
    }

#line 30 "unit/controlflow/goto_jmp.sp"
L_skip_decl:

#line 31 "unit/controlflow/goto_jmp.sp"
    if (y != 5) 
        return 3;

#line 34 "unit/controlflow/goto_jmp.sp"
    int val = 0;

#line 35 "unit/controlflow/goto_jmp.sp"
    if (val == 0) 
        goto L_A;

#line 36 "unit/controlflow/goto_jmp.sp"
    goto L_B;

#line 37 "unit/controlflow/goto_jmp.sp"
L_A:

#line 38 "unit/controlflow/goto_jmp.sp"
    val = 1;

#line 39 "unit/controlflow/goto_jmp.sp"
    goto L_C;

#line 40 "unit/controlflow/goto_jmp.sp"
L_B:

#line 41 "unit/controlflow/goto_jmp.sp"
    val = 2;

#line 42 "unit/controlflow/goto_jmp.sp"
    goto L_C;

#line 43 "unit/controlflow/goto_jmp.sp"
L_C:

#line 44 "unit/controlflow/goto_jmp.sp"
    if (val != 1) 
        return 4;

#line 46 "unit/controlflow/goto_jmp.sp"
    return 0;
}

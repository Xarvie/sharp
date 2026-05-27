// 来源: p259_goto.sp
// goto 和 label 控制流

int main() {
    // T1: forward goto
    int x = 0;
    goto L_set;
    x = 99;
L_set:
    x = 42;
    if (x != 42) return 1;

    // T2: backward goto (loop-like)
    int count = 0, i = 0;
L_loop:
    if (i >= 5) goto L_done;
    count = count + i;
    i = i + 1;
    goto L_loop;
L_done:
    if (count != 10) return 2;  /* 0+1+2+3+4 */

    // T3: goto bypassing declaration
    int y = 5;
    goto L_skip_decl;
    {
        int local = 99;
        (void)local;
    }
L_skip_decl:
    if (y != 5) return 3;

    // T4: multiple labels
    int val = 0;
    if (val == 0) goto L_A;
    goto L_B;
L_A:
    val = 1;
    goto L_C;
L_B:
    val = 2;
    goto L_C;
L_C:
    if (val != 1) return 4;

    return 0;
}
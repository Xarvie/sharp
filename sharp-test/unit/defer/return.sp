// Merged defer return-path tests
// Sources: p108_defer_return.sp, p257_multi_return_defer.sp, p266_runtime_defer.sp(return部分)

int seq[10];
int seqn = 0;
void mark(int v) { seq[seqn++] = v; }

/* T1: return value captured before defer side effect (p108) */
int counter = 0;
void bump(void) { counter = counter + 1; }
int snapshot(void) {
    defer bump();
    return counter;
}

/* T2: LIFO order (p108/p266) */
void lifo(void) {
    defer mark(1);
    defer mark(2);
    defer mark(3);
}

/* T3: void function defer (p108) */
int void_ran = 0;
void set_void(void) { void_ran = 1; }
void vfunc(void) {
    defer set_void();
}

/* T4: defer fires on every return path (p257) */
int test_multi_return(int v) {
    defer mark(10);
    if (v > 0) return 10;
    return 20;
}

/* T5: early return with defer (p257) */
int test_early_return(int v) {
    defer mark(20);
    if (v == 1) return 100;
    if (v == 2) return 200;
    return 300;
}

/* T6: defer fires on return with value (p266) */
int test_defer_ret(int v) {
    defer mark(100);
    if (v > 0) return v;
    return 0;
}

int main(void) {
    /* T1: return-temp captured (p108) */
    counter = 0;
    int v = snapshot();
    if (v != 0)       return 1;
    if (counter != 1) return 2;

    /* T2: LIFO order (p108/p266) */
    seqn = 0;
    lifo();
    if (seqn != 3)    return 3;
    if (seq[0] != 3)  return 4;
    if (seq[1] != 2)  return 5;
    if (seq[2] != 1)  return 6;

    /* T3: void function defer (p108) */
    void_ran = 0;
    vfunc();
    if (void_ran != 1) return 7;

    /* T4: multi return paths (p257) */
    seqn = 0;
    int r1 = test_multi_return(1);
    if (r1 != 10) return 8;
    if (seqn != 1) return 9;
    if (seq[0] != 10) return 10;

    seqn = 0;
    int r2 = test_multi_return(0);
    if (r2 != 20) return 11;
    if (seqn != 1) return 12;

    /* T5: early returns (p257) */
    seqn = 0;
    int r3 = test_early_return(1);
    if (r3 != 100) return 13;
    if (seqn != 1) return 14;
    if (seq[0] != 20) return 15;

    seqn = 0;
    int r4 = test_early_return(2);
    if (r4 != 200) return 16;
    if (seqn != 1) return 17;

    seqn = 0;
    int r5 = test_early_return(3);
    if (r5 != 300) return 18;
    if (seqn != 1) return 19;

    /* T6: defer with return value (p266) */
    seqn = 0;
    int r6 = test_defer_ret(42);
    if (r6 != 42) return 20;
    if (seqn != 1) return 21;
    if (seq[0] != 100) return 22;

    return 0;
}

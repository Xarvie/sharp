/* p108 — defer: return-temp and LIFO order
 *
 * Verifies three things:
 * (1) return value is captured before defers run (return-temp)
 * (2) multiple defers run LIFO
 * (3) void function defers run on return
 */

int seq[8];
int seqn = 0;
void mark(int v) { seq[seqn++] = v; }

/* T1: return value captured before defer side effect */
int counter = 0;
void bump(void) { counter = counter + 1; }
int snapshot(void) {
    defer bump();
    return counter;   /* should return 0; bump runs after */
}

/* T2: LIFO order: registered 1,2,3 — fires 3,2,1 */
void lifo(void) {
    defer mark(1);
    defer mark(2);
    defer mark(3);
}

/* T3: void function defer */
int void_ran = 0;
void set_void(void) { void_ran = 1; }
void vfunc(void) {
    defer set_void();
}

int main(void) {
    /* T1 */
    counter = 0;
    int v = snapshot();
    if (v != 0)       return 1;   /* return-temp captured 0 */
    if (counter != 1) return 2;   /* bump ran after return */

    /* T2 */
    seqn = 0;
    lifo();
    if (seqn != 3)    return 3;
    if (seq[0] != 3)  return 4;
    if (seq[1] != 2)  return 5;
    if (seq[2] != 1)  return 6;

    /* T3 */
    void_ran = 0;
    vfunc();
    if (void_ran != 1) return 7;

    return 0;
}

/* p83_operator_ident_name.c
 * Phase R10: C functions whose names BEGIN with "operator" but continue
 * with identifier chars (e.g. operatorMask, operatorSet) must NOT be
 * mangled as Sharp operator overloads.  Only true operator overloads
 * like operator+, operator== have symbol chars after "operator". */
static int operatorMask(int x) { return x & 0xFF; }
static int operatorSet(int a, int b)  { return a | b; }
int main(void) {
    if (operatorMask(0x1234) != 0x34) return 1;
    if (operatorSet(0x10, 0x01) != 0x11) return 2;
    return 0;
}

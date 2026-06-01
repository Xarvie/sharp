unsigned long long test(unsigned long long a, unsigned long long b, unsigned long long *hi) {
    __extension__ union { unsigned __int128 v; unsigned long long sv[2]; } var;
    var.v = a;
    var.v *= b;
    if (hi) *hi = var.sv[1];
    return var.sv[0];
}
int main(void) { return 0; }

long long test(void) {
    __extension__ union { long long v; int sv[2]; } var;
    var.v = 1;
    return var.v;
}
int main(void) { return (int)test(); }

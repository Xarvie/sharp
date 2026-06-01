__extension__ union { unsigned long long v; unsigned int sv[2]; } ext_var;
__extension__ struct { int x; int y; } ext_s;

int main(void) {
    ext_s.x = 1;
    ext_s.y = 2;
    return ext_s.x + ext_s.y - 3;
}

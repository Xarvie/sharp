void inc(int* p) { *p = *p + 1; }
int main() { int x = 0; inc(&x); return x - 1; }

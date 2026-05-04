int main() { int a[3]; int* p = a; *p = 5; *(p+1) = 10; return *p + *(p+1) - 15; }

long long unsigned int test_ullong_variants(void) {
    long long unsigned int a = 1;
    unsigned long long b = 2;
    long long unsigned c = 3;
    return a + b + c;
}

int main(void) {
    long long unsigned int r = test_ullong_variants();
    return (int)(r - 6);
}

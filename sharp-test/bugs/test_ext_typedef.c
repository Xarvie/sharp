__extension__ typedef unsigned long long size_t;
size_t test_fn(size_t x) { return x + 1; }
int main(void) { return (int)test_fn(1); }

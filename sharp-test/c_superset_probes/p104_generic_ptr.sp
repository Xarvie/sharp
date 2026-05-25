/* p104 — generic with pointer field */
struct Box<T> {
    T* ptr;
    int valid;
}

int main(void) {
    int x = 99;
    Box<int> b;
    b.ptr   = &x;
    b.valid = 1;
    if (!b.valid)   return 1;
    if (*b.ptr != 99) return 2;
    return 0;
}

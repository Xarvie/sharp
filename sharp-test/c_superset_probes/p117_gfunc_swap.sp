void swap<T>(T* a, T* b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

int main(void) {
    int x = 10, y = 20;
    swap<int>(&x, &y);
    if (x != 20) return 1;
    if (y != 10) return 2;
    return 0;
}

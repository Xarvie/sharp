void swap<T>(T* a, T* b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

extern swap<int>;

int main(void) {
    int x = 1, y = 2;
    swap<int>(&x, &y);
    return x - 2 + y - 1;  /* x=2, y=1, result = 0 */
}

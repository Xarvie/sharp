void swap<T>(T* a, T* b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

extern swap<int>;

int main(void) {
    int x = 5, y = 10;
    swap<int>(&x, &y);
    return x - 10 + y - 5;  /* x=10, y=5, result = 0 */
}

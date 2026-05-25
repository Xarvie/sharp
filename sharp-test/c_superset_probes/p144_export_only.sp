void swap<T>(T* a, T* b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

extern swap<int>;

int main(void) {
    float a = 1.0f, b = 2.0f;
    swap<float>(&a, &b);
    return (int)(a + b - 3.0f);  /* a=2, b=1, result = 0 */
}

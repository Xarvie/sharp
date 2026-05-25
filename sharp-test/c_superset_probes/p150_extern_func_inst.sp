void swap<T>(T* a, T* b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

/* extern tells compiler "instantiated elsewhere, don't generate here" */
extern swap<int>;
extern swap<float>;

int main(void) {
    float fa = 1.0f, fb = 2.0f;
    swap<float>(&fa, &fb);
    return (int)(fa + fb - 3.0f);
}

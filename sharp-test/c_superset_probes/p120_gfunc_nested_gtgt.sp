typedef struct Vec<T> {
    T* data;
    int len;
} Vec;

Vec<Vec<int>> flatten(Vec<Vec<int>> v) {
    return v;
}

int main(void) {
    Vec<Vec<int>> x;
    flatten(x);
    return 0;
}

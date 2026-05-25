int max<T>(T a, T b) {
    if (a > b) return a;
    return b;
}

int main(void) {
    if (max(3, 7) != 7) return 1;
    if (max(10, 5) != 10) return 2;
    return 0;
}

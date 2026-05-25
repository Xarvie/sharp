int pass<T,U>(T a, U b) {
    return 0;
}

int main(void) {
    pass<int, float>(1, 2.5f);
    return 0;
}

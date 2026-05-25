/* p102 — generic function: max<T> */
int imax(int a, int b) {
    if (a > b) { return a; } else { return b; }
}
float fmax2(float a, float b) {
    if (a > b) { return a; } else { return b; }
}

int main(void) {
    if (imax(3, 5) != 5) return 1;
    if (imax(7, 2) != 7) return 2;
    return 0;
}

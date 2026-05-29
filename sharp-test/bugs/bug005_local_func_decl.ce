/* Bug-005: 局部函数声明，函数名不应丢失 */
int f1(char *p) {
    return *p + 1;
}

int main() {
    char s = 1;
    int f1(char *);

    if (f1(&s) != 2)
        return 1;
    return 0;
}

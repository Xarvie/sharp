#define CALL(MAC, ...) MAC ## __VA_ARGS__

/* Empty VA_ARGS */
int main() {
    int A = 1;
    return CALL(A, );
}

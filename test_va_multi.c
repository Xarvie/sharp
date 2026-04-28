#define CALL(MAC, ...) MAC ## __VA_ARGS__

int main() {
    int AB = 1;
    int CD = 2;
    return CALL(A, B) + CALL(C, D);
}

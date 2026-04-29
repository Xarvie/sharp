#define PASTE(_1, _2) _1 ## _2
#define MAKE_ABC(A, B, C) A ## B ## C

int ABC = 1;

int main() {
    return PASTE(MAKE_ABC(A, B, C), D);
}

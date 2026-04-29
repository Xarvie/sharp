#define PASTE2(a, b) a ## b
#define PASTE3(a, b, c) a ## b ## c

int ABC = 1;
int ABCD = 2;

int main() {
    return PASTE2(ABC, D);
}

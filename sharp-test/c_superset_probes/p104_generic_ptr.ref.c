typedef struct Box__int Box__int;
struct Box__int {
    int * ptr;
    int valid;
};

int main(void) {
    int x = 99;
    Box__int b;
    b.ptr = &x;
    b.valid = 1;
    if (!b.valid) 
        return 1;
    if (*b.ptr != 99) 
        return 2;
    return 0;
}

typedef struct Vec__int Vec__int;
struct Vec__int {
    int data;
    int len;
};

typedef struct Vec__float Vec__float;
struct Vec__float {
    float data;
    int len;
};

int main(void) {
    Vec__int vi;
    vi.data = 42;
    vi.len = 1;
    if (vi.data != 42) 
        return 1;
    if (vi.len != 1) 
        return 2;
    Vec__float vf;
    vf.data = 3.14f;
    vf.len = 1;
    if (vf.len != 1) 
        return 3;
    return 0;
}

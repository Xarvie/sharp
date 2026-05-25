/* p101 — generic struct: Vec<T> basic specialization and field access */
struct Vec<T> {
    T data;
    int len;
}

int main(void) {
    Vec<int> vi;
    vi.data = 42;
    vi.len = 1;
    if (vi.data != 42) return 1;
    if (vi.len  != 1)  return 2;

    Vec<float> vf;
    vf.data = 3.14f;
    vf.len = 1;
    if (vf.len != 1) return 3;
    return 0;
}

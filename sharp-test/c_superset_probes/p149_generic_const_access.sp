/* FIXED: generic + const method field access */
class Arr<T> {
    T* data;
    long len;
}

long Arr<T>.size(this) const { return len; }

int main(void) {
    long buf[3] = { 10, 20, 30 };
    Arr<long> a = { buf, 3 };
    return a.size() - 3;
}
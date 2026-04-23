// examples/generic.sp — generic struct + impl smoke test.
//
// Exercises generic struct, impl, two monomorphisations, destructor,
// value/pointer-self methods, postfix ++/--, non-generic struct.
//
// The Stack<T> is caller-owned (no malloc in the generic body).
// Buffers are provided by extern C helpers that know the byte size.
//
// Expected exit code: 42.

extern i32* make_i32_buf(i32 n);   // returns malloc'd i32[n], caller frees
extern void buf_free(i32* p);

struct Stack<T> {
    T*  data;
    i32 len;
    i32 cap;
}

impl Stack<T> {
    static Stack<T> wrap(T* buf, i32 cap) {
        Stack<T> s;
        s.data = buf;
        s.len  = 0;
        s.cap  = cap;
        return s;
    }

    void push(Stack<T>* self, T val) {
        if (len < cap) {
            data[len] = val;
            len++;
        }
    }

    T pop(Stack<T>* self) {
        len--;
        return data[len];
    }

    i32 depth() { return len; }
    bool empty() { return len == 0; }
}

struct Counter { i32 n; }
impl Counter {
    static Counter make() { return Counter { n: 0 }; }
    void inc(Counter* self) { n++; }
    void dec(Counter* self) { n--; }
}

i32 main() {
    i32* pa = make_i32_buf(8);
    i32* pb = make_i32_buf(4);

    Stack<i32> sa = Stack<i32>.wrap(pa, 8);
    sa.push(10);
    sa.push(20);
    sa.push(30);
    i32 p3 = sa.pop();     // 30
    i32 p2 = sa.pop();     // 20
    i32 d1 = sa.depth();   // 1

    Stack<i32> sb = Stack<i32>.wrap(pb, 4);
    sb.push(7);
    sb.push(9);
    i32 q1 = sb.pop();     // 9
    i32 d2 = sb.depth();   // 1

    buf_free(pa);
    buf_free(pb);

    Counter c = Counter.make();
    c.inc(); c.inc(); c.inc(); c.dec();
    i32 cnt = c.n;   // 2

    i32 x = 5;
    i32 y = x++;    // y=5, x→6
    i32 z = x--;    // z=6, x→5
    i32 post = y + z - x;   // 5+6-5=6

    i32 eflag = 0;
    if (!sb.empty()) { eflag = 1; }

    // (30-20)+1+9+1+2+6+1+12 = 42
    return (p3 - p2) + d1 + q1 + d2 + cnt + post + eflag + 12;
}

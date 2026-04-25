/* compiler: any */
/* TDD-Generic-1: struct + impl with generic parameter */

struct Box<T> {
    T value;
};

impl Box<T> {
    T get() {
        return self.value;
    }
}

i32 main() {
    Box<i32> b;
    b.value = 42;
    return b.get(); /* 42 */
}

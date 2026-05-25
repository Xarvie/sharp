/* p105 — generic struct with method */
class Stack<T> {
    T top;
    int size;
}
int Stack.push(this, T val) {
    this->top  = val;
    this->size = this->size + 1;
    return this->size;
}
T Stack.peek(this) {
    return this->top;
}

int main(void) {
    Stack<int> s;
    s.size = 0;
    s.push(42);
    if (s.peek() != 42) return 1;
    if (s.size   != 1)  return 2;
    s.push(99);
    if (s.peek() != 99) return 3;
    if (s.size   != 2)  return 4;
    return 0;
}
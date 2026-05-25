/* p114 — generic struct associated function */
class Stack<T> {
    T data;
    int n;
}
Stack<T> Stack.new() {
    Stack<T> s; s.n = 0; return s;
}
void Stack.push(this, T v) { this->data = v; this->n = this->n + 1; }
T Stack.top(this) { return this->data; }

int main(void) {
    Stack<int> s = Stack<int>.new();
    s.push(42);
    if (s.top() != 42) return 1;
    if (s.n != 1) return 2;
    return 0;
}
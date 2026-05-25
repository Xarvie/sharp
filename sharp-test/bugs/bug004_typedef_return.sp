struct Foo {
    int x;
}

Foo make_foo() {
    Foo f;
    f.x = 42;
    return f;
}

int main() {
    Foo f = make_foo();
    return f.x;
}

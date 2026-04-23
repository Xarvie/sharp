// EXPECT: E2012
struct Foo { int x; }
impl Foo {
    static Foo make() { return Foo { x: 0 }; }
}
int main() {
    Foo f = Foo.make();
    f.nonexistent();
    return 0;
}

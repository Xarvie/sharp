// EXPECT: E3003
int add(int a, int b) { return a + b; }
int main() {
    return add(1, "two");
}

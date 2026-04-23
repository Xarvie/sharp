// EXPECT: E3004
int get() {
    return "not an int";
}
int main() { return get(); }

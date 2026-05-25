/* Phase R18: parenthesized field name `struct { T (name); }` round-trip */
struct box {
    struct inner { int val; } (item);
};
int use_it(struct box *b) { return b->item.val; }
int main(void) {
    struct box b;
    b.item.val = 42;
    return use_it(&b) == 42 ? 0 : 1;
}

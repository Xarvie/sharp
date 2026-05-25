/* p130 — taking address of compound literal: &(T){...}
 * KNOWN-ERROR: sema reports "return type mismatch: expected ptr, got ptr"
 * Compound literals are lvalues in C99/C11 so &(T){...} is valid.
 * Expected: sharpc compiles and generates correct C
 * Actual:   sema error even though both sides have the same pointer type
 */
struct node { int val; struct node *next; };
struct node *make_node(int v) {
    return &(struct node){ .val = v, .next = 0 };
}
int main(void) { return make_node(42)->val - 42; }

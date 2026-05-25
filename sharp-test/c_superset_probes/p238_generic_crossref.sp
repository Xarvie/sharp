/* p238 — cross-referencing generic structs
 *
 * Tests two generic structs that reference each other through pointers.
 * This is the classic linked-list / tree node pattern.
 */
/* FIXED(e2e): generic self-referential type — use Type* path with ty_subst for field types */
#include <stdbool.h>

/* Forward declared — will be defined */
class Node<T> {
    T       val;
    Node*   next;      /* pointer to same type */
};

T Node.get_val(this) const { return this->val; }
void Node.set_val(this, T v) { this->val = v; }
void Node.link(this, Node* n) { this->next = n; }

int main() {
    Node<int> a = {0};
    a.set_val(10);

    Node<int> b = {0};
    b.set_val(20);

    a.link(&b);

    if (a.get_val() != 10) return 1;
    if (a.next->get_val() != 20) return 2;

    /* verify modifications through link */
    a.next->set_val(30);
    if (b.get_val() != 30) return 3;

    return 0;
}
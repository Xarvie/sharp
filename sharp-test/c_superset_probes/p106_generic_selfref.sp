/* p106 — self-referential generic: linked list node */
struct Node<T> {
    T val;
    Node<T>* next;
}

int main(void) {
    Node<int> a;
    Node<int> b;
    a.val  = 1;
    b.val  = 2;
    a.next = &b;
    b.next = (Node<int>*)0;
    if (a.val        != 1) return 1;
    if (a.next->val  != 2) return 2;
    if (a.next->next != (Node<int>*)0) return 3;
    return 0;
}

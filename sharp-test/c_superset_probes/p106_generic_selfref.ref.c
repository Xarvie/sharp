typedef struct Node__int Node__int;
struct Node__int {
    int val;
    Node__int * next;
};

int main(void) {
    Node__int a;
    Node__int b;
    a.val = 1;
    b.val = 2;
    a.next = &b;
    b.next = (Node__int *)0;
    if (a.val != 1) 
        return 1;
    if (a.next->val != 2) 
        return 2;
    if (a.next->next != (Node__int *)0) 
        return 3;
    return 0;
}

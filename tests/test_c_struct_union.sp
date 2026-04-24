/* Phase 3: C struct/union tests */

/* Union type */
union Value {
    int i;
    double d;
    char* s;
};

/* Forward struct reference */
struct Node;  /* forward declaration */

struct Node {
    int data;
    struct Node* next;
};

/* Anonymous struct in typedef */
typedef struct {
    int x;
    int y;
} Point;

/* Usage */
i32 main() {
    union Value v;
    struct Node n;
    Point p;
    return 0;
}

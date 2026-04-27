/* compiler: any */
/* Feature 1: typedef struct Tag Name — reuse existing struct tag */

struct _iobuf {
    int _cnt;
    char* _ptr;
}

typedef struct _iobuf FILE;

int main() {
    FILE* f;
    return 0;
}
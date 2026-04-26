/* compiler: any */
/* Feature 1: typedef struct Tag Name — reuse existing struct tag */

struct _iobuf {
    i32 _cnt;
    char* _ptr;
}

typedef struct _iobuf FILE;

i32 main() {
    FILE* f;
    return 0;
}

/* compiler: any */
/* C Feature: __declspec on struct fields */
/* Expected: Parser should handle __declspec after type in struct fields */

struct _iobuf {
    char* _ptr;
    int _cnt;
    char* __declspec(nothrow) _base;
    int _flag;
}

int main() {
    return 0;
}
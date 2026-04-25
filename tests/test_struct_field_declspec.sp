/* compiler: any */
/* C Feature: __declspec on struct fields */
/* Expected: Parser should handle __declspec after type in struct fields */

struct _iobuf {
    char* _ptr;
    i32 _cnt;
    char* __declspec(nothrow) _base;
    i32 _flag;
}

i32 main() {
    return 0;
}

#include <_mingw.h>
__extension__ __int64 test_abs64(__int64 x) { return x < 0 ? -x : x; }
int main(void) { return (int)test_abs64(-1); }

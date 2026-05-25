/* p275 — sizeof on complex types
 *
 * Tests sizeof on structs, pointers, arrays, and typedef'd types.
 */

class Vec4 {
    int x;
    int y;
    int z;
    int w;
};

typedef int IntArr[10];

int main() {
    if (sizeof(int) != 4)  return 1;
    if (sizeof(Vec4) != 16) return 2;  /* 4 ints = 16 bytes */
    if (sizeof(Vec4*) != 8) return 3;  /* pointer = 8 bytes on 64-bit */

    /* sizeof array */
    int arr[5] = {0};
    if (sizeof(arr) != 20) return 4;  /* 5*4=20 */

    /* sizeof typedef */
    if (sizeof(IntArr) != 40) return 5;  /* 10*4=40 */

    /* sizeof expression-like */
    if (sizeof(Vec4) / sizeof(int) != 4) return 6;

    return 0;
}
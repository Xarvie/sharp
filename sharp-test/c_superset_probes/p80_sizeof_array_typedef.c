/* p80_sizeof_array_typedef.c
 * Phase R9: sizeof(T) where T is a typedef for an array type.
 * sizeof(QuadArr) must yield the full array byte count, not sizeof(pointer).
 * Previously emitted sizeof(__jmp_buf_tag*) instead of sizeof array. */
typedef struct { int x; int y; int z; int w; } Quad;
typedef Quad QuadArr[4];   /* array typedef — 4 Quads = 4*sizeof(Quad) bytes */

int main(void) {
    /* sizeof(QuadArr) must equal 4 * sizeof(Quad), not sizeof(Quad*) */
    if (sizeof(QuadArr) != 4 * sizeof(Quad)) return 1;
    if (sizeof(QuadArr) == sizeof(Quad *))   return 2;
    return 0;
}

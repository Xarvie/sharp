/* compiler: any */
/* Test: Multi-dimensional array typedef — verifies that parse_declarator
 * correctly chains multiple array suffixes: int m[3][4] should produce
 * TY_ARRAY(TY_ARRAY(TY_INT, 4), 3). */

/* 2D array */
typedef int matrix2d[3][4];

/* 3D array */
typedef char tensor[2][3][4];

/* Array of const int arrays */
typedef const int const_rows[5][6];

int main() {
    matrix2d m;
    tensor t;
    return 0;
}

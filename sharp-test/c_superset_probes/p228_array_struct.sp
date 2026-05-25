/* p228 — array types in structs and extension methods
 *
 * Verifies array fields, array parameters, and array access
 * through extension methods.
 */

#include <stdbool.h>
#include "../../sharp/std/types.sph"

/* ===== struct with fixed-size array ===== */
class Matrix {
    int data[4][4];   /* 4x4 matrix */
};

/* Extension method: read element */
int Matrix.at(this, isize row, isize col) const {
    return this->data[row][col];
}

/* Extension method: write element */
void Matrix.set(this, isize row, isize col, int val) {
    this->data[row][col] = val;
}

/* Extension method: fill row with value */
void Matrix.fill_row(this, isize row, int val) {
    isize c = 0;
    while (c < 4) {
        this->data[row][c] = val;
        c = c + 1;
    }
}

/* Extension method: sum all elements */
int Matrix.sum_all(this) const {
    int s = 0;
    isize r = 0;
    while (r < 4) {
        isize c = 0;
        while (c < 4) {
            s = s + this->data[r][c];
            c = c + 1;
        }
        r = r + 1;
    }
    return s;
}

/* ===== free function with array parameter ===== */
int sum_array(int* arr, isize n) {
    /* arr is int* (array decay) */
    int s = 0;
    isize i = 0;
    while (i < n) {
        s = s + arr[i];
        i = i + 1;
    }
    return s;
}

int main() {
    Matrix m = {0};

    /* --- set individual elements --- */
    m.set(0, 0, 1);
    m.set(0, 1, 2);
    m.set(0, 2, 3);
    m.set(0, 3, 4);

    if (m.at(0, 0) != 1) return 1;
    if (m.at(0, 3) != 4) return 2;

    /* zero-init → all other elements are 0 */
    if (m.at(1, 0) != 0) return 3;

    /* --- fill a row --- */
    m.fill_row(2, 99);
    if (m.at(2, 0) != 99) return 4;
    if (m.at(2, 3) != 99) return 5;

    /* --- sum all --- */
    /* row0: 1,2,3,4=10  row2: 99x4=396  total: 406 */
    int total = m.sum_all();
    if (total != 406) return 6;

    /* --- free function array parameter --- */
    int vals[5];
    vals[0] = 10;
    vals[1] = 20;
    vals[2] = 30;
    vals[3] = 40;
    vals[4] = 50;
    int sv = sum_array(vals, 5);
    if (sv != 150) return 7;

    return 0;
}
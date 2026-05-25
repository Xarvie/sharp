

#line 3 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 11 "sharp-test/c_superset_probes/p228_array_struct.sp"
typedef struct Matrix Matrix;
struct Matrix {
    int data[4][4];
};

#line 16 "sharp-test/c_superset_probes/p228_array_struct.sp"
int Matrix__at(Matrix * this, isize row, isize col);

#line 21 "sharp-test/c_superset_probes/p228_array_struct.sp"
void Matrix__set(Matrix * this, isize row, isize col, int val);

#line 26 "sharp-test/c_superset_probes/p228_array_struct.sp"
void Matrix__fill_row(Matrix * this, isize row, int val);

#line 35 "sharp-test/c_superset_probes/p228_array_struct.sp"
int Matrix__sum_all(Matrix * this);

#line 16 "sharp-test/c_superset_probes/p228_array_struct.sp"
int Matrix__at(Matrix * this, isize row, isize col) {

#line 17 "sharp-test/c_superset_probes/p228_array_struct.sp"
    return this->data[row][col];
}

#line 21 "sharp-test/c_superset_probes/p228_array_struct.sp"
void Matrix__set(Matrix * this, isize row, isize col, int val) {

#line 22 "sharp-test/c_superset_probes/p228_array_struct.sp"
    this->data[row][col] = val;
}

#line 26 "sharp-test/c_superset_probes/p228_array_struct.sp"
void Matrix__fill_row(Matrix * this, isize row, int val) {

#line 27 "sharp-test/c_superset_probes/p228_array_struct.sp"
    isize c = 0;

#line 28 "sharp-test/c_superset_probes/p228_array_struct.sp"
    while (c < 4) {

#line 29 "sharp-test/c_superset_probes/p228_array_struct.sp"
        this->data[row][c] = val;

#line 30 "sharp-test/c_superset_probes/p228_array_struct.sp"
        c = c + 1;
    }
}

#line 35 "sharp-test/c_superset_probes/p228_array_struct.sp"
int Matrix__sum_all(Matrix * this) {

#line 36 "sharp-test/c_superset_probes/p228_array_struct.sp"
    int s = 0;

#line 37 "sharp-test/c_superset_probes/p228_array_struct.sp"
    isize r = 0;

#line 38 "sharp-test/c_superset_probes/p228_array_struct.sp"
    while (r < 4) {

#line 39 "sharp-test/c_superset_probes/p228_array_struct.sp"
        isize c = 0;

#line 40 "sharp-test/c_superset_probes/p228_array_struct.sp"
        while (c < 4) {

#line 41 "sharp-test/c_superset_probes/p228_array_struct.sp"
            s = s + this->data[r][c];

#line 42 "sharp-test/c_superset_probes/p228_array_struct.sp"
            c = c + 1;
        }

#line 44 "sharp-test/c_superset_probes/p228_array_struct.sp"
        r = r + 1;
    }

#line 46 "sharp-test/c_superset_probes/p228_array_struct.sp"
    return s;
}

#line 50 "sharp-test/c_superset_probes/p228_array_struct.sp"
int sum_array(int * arr, isize n) {

#line 52 "sharp-test/c_superset_probes/p228_array_struct.sp"
    int s = 0;

#line 53 "sharp-test/c_superset_probes/p228_array_struct.sp"
    isize i = 0;

#line 54 "sharp-test/c_superset_probes/p228_array_struct.sp"
    while (i < n) {

#line 55 "sharp-test/c_superset_probes/p228_array_struct.sp"
        s = s + arr[i];

#line 56 "sharp-test/c_superset_probes/p228_array_struct.sp"
        i = i + 1;
    }

#line 58 "sharp-test/c_superset_probes/p228_array_struct.sp"
    return s;
}

#line 61 "sharp-test/c_superset_probes/p228_array_struct.sp"
int main() {

#line 62 "sharp-test/c_superset_probes/p228_array_struct.sp"
    Matrix m = { 0 };

#line 65 "sharp-test/c_superset_probes/p228_array_struct.sp"
    Matrix__set(&m, 0, 0, 1);

#line 66 "sharp-test/c_superset_probes/p228_array_struct.sp"
    Matrix__set(&m, 0, 1, 2);

#line 67 "sharp-test/c_superset_probes/p228_array_struct.sp"
    Matrix__set(&m, 0, 2, 3);

#line 68 "sharp-test/c_superset_probes/p228_array_struct.sp"
    Matrix__set(&m, 0, 3, 4);

#line 70 "sharp-test/c_superset_probes/p228_array_struct.sp"
    if (Matrix__at(&m, 0, 0) != 1) 
        return 1;

#line 71 "sharp-test/c_superset_probes/p228_array_struct.sp"
    if (Matrix__at(&m, 0, 3) != 4) 
        return 2;

#line 74 "sharp-test/c_superset_probes/p228_array_struct.sp"
    if (Matrix__at(&m, 1, 0) != 0) 
        return 3;

#line 77 "sharp-test/c_superset_probes/p228_array_struct.sp"
    Matrix__fill_row(&m, 2, 99);

#line 78 "sharp-test/c_superset_probes/p228_array_struct.sp"
    if (Matrix__at(&m, 2, 0) != 99) 
        return 4;

#line 79 "sharp-test/c_superset_probes/p228_array_struct.sp"
    if (Matrix__at(&m, 2, 3) != 99) 
        return 5;

#line 83 "sharp-test/c_superset_probes/p228_array_struct.sp"
    int total = Matrix__sum_all(&m);

#line 84 "sharp-test/c_superset_probes/p228_array_struct.sp"
    if (total != 406) 
        return 6;

#line 87 "sharp-test/c_superset_probes/p228_array_struct.sp"
    int vals[5];

#line 88 "sharp-test/c_superset_probes/p228_array_struct.sp"
    vals[0] = 10;

#line 89 "sharp-test/c_superset_probes/p228_array_struct.sp"
    vals[1] = 20;

#line 90 "sharp-test/c_superset_probes/p228_array_struct.sp"
    vals[2] = 30;

#line 91 "sharp-test/c_superset_probes/p228_array_struct.sp"
    vals[3] = 40;

#line 92 "sharp-test/c_superset_probes/p228_array_struct.sp"
    vals[4] = 50;

#line 93 "sharp-test/c_superset_probes/p228_array_struct.sp"
    int sv = sum_array(vals, 5);

#line 94 "sharp-test/c_superset_probes/p228_array_struct.sp"
    if (sv != 150) 
        return 7;

#line 96 "sharp-test/c_superset_probes/p228_array_struct.sp"
    return 0;
}

/* p264 — Runtime: struct layout and field access
 *
 * Verifies that generated struct layout matches expected behavior at runtime.
 */

class Vec3D {
    float x;
    float y;
    float z;
};

float Vec3D.length(this) const {
    float xx = this->x * this->x;
    float yy = this->y * this->y;
    float zz = this->z * this->z;
    return xx + yy + zz;
}

int main() {
    Vec3D v;
    v.x = 3.0f;
    v.y = 4.0f;
    v.z = 0.0f;

    float len_sq = v.length();
    /* 3*3 + 4*4 + 0*0 = 25 */
    if (len_sq != 25.0f) return 1;

    /* Test field modification */
    v.z = 12.0f;
    float len_sq2 = v.length();
    /* 9 + 16 + 144 = 169 */
    if (len_sq2 != 169.0f) return 2;

    /* Copy struct */
    Vec3D w = v;
    if (w.x != 3.0f) return 3;
    if (w.y != 4.0f) return 4;
    if (w.z != 12.0f) return 5;

    return 0;
}
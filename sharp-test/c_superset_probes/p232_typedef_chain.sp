/* p232 — type alias chains (typedef of typedef of typedef)
 *
 * Verifies that chained typedefs work correctly for
 * variables, parameters, and method calls.
 */

#include <stdbool.h>
#include "../../std/types.sph"

/* Chain: isize → MySize → MyLen → MyCount */
typedef isize MySize;
typedef MySize MyLen;
typedef MyLen MyCount;

/* Chain for struct type */
class Vec3 {
    float x; float y; float z;
};
typedef Vec3 Point3D;
typedef Point3D Position;

float Vec3.dot(this, Vec3 other) const {
    return this->x * other.x + this->y * other.y + this->z * other.z;
}

/* free function using chained typedefs */
MyCount add_counts(MyCount a, MyCount b) {
    return a + b;
}

int main() {
    /* --- scalar typedef chain --- */
    MyCount n = 10;
    isize m = n;
    if (m != 10) return 1;

    MyLen l1 = 3;
    MyLen l2 = 5;
    MyCount r = add_counts(l1, l2);
    if (r != 8) return 2;

    /* --- struct typedef chain --- */
    Position p;
    p.x = 1.0f;
    p.y = 2.0f;
    p.z = 3.0f;

    Position q;
    q.x = 4.0f;
    q.y = 5.0f;
    q.z = 6.0f;

    float d = p.dot(q);
    if (d < 31.99f || d > 32.01f) return 3;  /* 4+10+18=32 */

    /* --- use intermediate typedef --- */
    Point3D r3;
    r3.x = 0.0f;
    r3.y = 0.0f;
    r3.z = 1.0f;
    if (r3.dot(p) < 2.99f || r3.dot(p) > 3.01f) return 4;  /* 0+0+3=3 */

    return 0;
}
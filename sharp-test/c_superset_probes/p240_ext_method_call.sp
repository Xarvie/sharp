/* p240 — extension method calling another extension method
 *
 * Tests that extension methods can call other extension methods
 * on the same type via this->other_method().
 */

#include <stdbool.h>
#include "../../std/types.sph"

class Stats {
    int   count;
    float sum;
};

void Stats.record(this, float v) {
    this->count = this->count + 1;
    this->sum   = this->sum + v;
}

void Stats.record_many(this, float* vals, isize n) {
    isize i = 0;
    while (i < n) {
        this->record(vals[i]);  /* extension method calling extension method */
        i = i + 1;
    }
}

int   Stats.count_val(this) const { return this->count; }
float Stats.avg(this) const {
    if (this->count == 0) return 0.0f;
    return this->sum / (float)this->count;
}

int main() {
    Stats s = {0};

    float vals[4];
    vals[0] = 10.0f;
    vals[1] = 20.0f;
    vals[2] = 30.0f;
    vals[3] = 40.0f;

    s.record_many(vals, 4);

    if (s.count_val() != 4) return 1;
    float a = s.avg();
    if (a < 24.9f || a > 25.1f) return 2;   /* (10+20+30+40)/4 = 25 */

    return 0;
}
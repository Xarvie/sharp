/* p248 — comparison operators with extension methods
 *
 * Tests ==, !=, <, >, <=, >= as extension method operators.
 */

#include <stdbool.h>

class Score {
    int pts;
};

bool Score.operator==(this, Score other) const {
    return this->pts == other.pts;
}
bool Score.operator<(this, Score other) const {
    return this->pts < other.pts;
}
bool Score.operator>(this, Score other) const {
    return this->pts > other.pts;
}
bool Score.operator<=(this, Score other) const {
    return this->pts <= other.pts;
}
bool Score.operator>=(this, Score other) const {
    return this->pts >= other.pts;
}

int main() {
    Score s10; s10.pts = 10;
    Score s20; s20.pts = 20;
    Score s10b; s10b.pts = 10;

    if (!(s10 == s10b)) return 1;
    if (s10 == s20) return 2;

    /* != derived */
    if (s10 == s10b) { } else { return 3; }

    if (!(s10 < s20)) return 4;
    if (s20 < s10) return 5;

    if (!(s20 > s10)) return 6;
    if (s10 > s20) return 7;

    if (!(s10 <= s20)) return 8;
    if (!(s10 <= s10b)) return 9;
    if (s20 <= s10) return 10;

    if (!(s20 >= s10)) return 11;
    if (!(s10 >= s10b)) return 12;

    return 0;
}
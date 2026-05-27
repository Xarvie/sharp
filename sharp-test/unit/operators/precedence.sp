// 来源: p247_operator_precedence.sp, p248_comparison_ops.sp
// 运算符优先级 + 比较运算符测试

#include <stdbool.h>

// === p247: 运算符优先级 (+, -, *, /) ===
class Num {
    int val;
};
Num Num.operator+(this, Num other) const {
    Num r; r.val = this->val + other.val; return r;
}
Num Num.operator-(this, Num other) const {
    Num r; r.val = this->val - other.val; return r;
}
Num Num.operator*(this, Num other) const {
    Num r; r.val = this->val * other.val; return r;
}
Num Num.operator/(this, Num other) const {
    Num r; r.val = this->val / other.val; return r;
}
bool Num.operator==(this, Num other) const {
    return this->val == other.val;
}

// === p248: 比较运算符 (==, <, >, <=, >=) ===
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
    // --- p247 测试: 优先级 ---
    Num two;   two.val   = 2;
    Num three; three.val = 3;
    Num four;  four.val  = 4;
    Num ten;   ten.val   = 10;

    // a + b * c = 2 + 3*4 = 14
    Num r1 = two + three * four;
    if (r1.val != 14) return 1;
    // a * b + c = 2*3 + 4 = 10
    Num r2 = two * three + four;
    if (r2.val != 10) return 2;
    // a + b - c*d = 10 + 2 - 12 = 0
    Num r3 = ten + two - three * four;
    if (r3.val != 0) return 3;
    // a * b / c = 3*4 / 2 = 6
    Num r4 = three * four / two;
    if (r4.val != 6) return 4;

    // --- p248 测试: 比较运算符 ---
    Score s10; s10.pts = 10;
    Score s20; s20.pts = 20;
    Score s10b; s10b.pts = 10;

    if (!(s10 == s10b)) return 5;
    if (s10 == s20) return 6;
    if (s10 == s10b) { } else { return 7; }

    if (!(s10 < s20)) return 8;
    if (s20 < s10) return 9;
    if (!(s20 > s10)) return 10;
    if (s10 > s20) return 11;
    if (!(s10 <= s20)) return 12;
    if (!(s10 <= s10b)) return 13;
    if (s20 <= s10) return 14;
    if (!(s20 >= s10)) return 15;
    if (!(s10 >= s10b)) return 16;

    return 0;
}
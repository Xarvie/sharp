// 来源: p273_struct_array.sp, p276_compound_literal.sp
// C代码生成: struct数组 + 复合字面量(返回struct)

#include <stdbool.h>

// === p273: struct array ===
class Vec2 {
    int x;
    int y;
};

// === p276: compound literal (return struct) ===
class Point {
    int x;
    int y;
};
Point make_point(int xx, int yy) {
    Point p; p.x = xx; p.y = yy;
    return p;
}
Point Point.add(this, Point rhs) {
    Point r;
    r.x = this->x + rhs.x;
    r.y = this->y + rhs.y;
    return r;
}

int main() {
    // --- p273: struct array ---
    Vec2 points[4] = {0};
    points[0].x = 1; points[0].y = 2;
    points[1].x = 3; points[1].y = 4;
    points[2].x = 5; points[2].y = 6;
    points[3].x = 7; points[3].y = 8;

    int sum_x = 0, sum_y = 0;
    for (int i = 0; i < 4; i = i + 1) {
        sum_x = sum_x + points[i].x;
        sum_y = sum_y + points[i].y;
    }
    if (sum_x != 16) return 1;
    if (sum_y != 20) return 2;

    Vec2 partial[3] = {0};
    partial[1].x = 99;
    if (partial[0].x != 0) return 3;
    if (partial[1].x != 99) return 4;
    if (partial[2].x != 0) return 5;

    // --- p276: compound literal ---
    Point p1 = make_point(10, 20);
    if (p1.x != 10) return 6;
    if (p1.y != 20) return 7;

    Point p2 = make_point(5, 7);
    Point p3 = p1.add(p2);
    if (p3.x != 15) return 8;
    if (p3.y != 27) return 9;

    Point p4 = make_point(1, 2).add(make_point(3, 4));
    if (p4.x != 4) return 10;
    if (p4.y != 6) return 11;

    return 0;
}
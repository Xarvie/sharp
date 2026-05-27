// 来源: p261_module_cross.sp
// 模块交叉引用: 导入带有类和扩展方法的模块

#include "cross_ref.sph"

int distance_sq(class Point2D a, class Point2D b) {
    int dx = a.get_x() - b.get_x();
    int dy = a.get_y() - b.get_y();
    return dx * dx + dy * dy;
}

int main() {
    class Point2D p1;
    p1.move_to(0, 0);

    class Point2D p2;
    p2.move_to(3, 4);

    int d = distance_sq(p1, p2);
    if (d != 25) return 1;  /* 3*3 + 4*4 = 25 */

    if (p1.get_x() != 0) return 2;
    if (p1.get_y() != 0) return 3;
    if (p2.get_x() != 3) return 4;
    if (p2.get_y() != 4) return 5;

    return 0;
}
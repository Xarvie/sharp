// 来源: p219_v013_intrinsics.sp
// v0.13 编译期内省原语回归: @has_method, @has_field, @is_arithmetic, @is_pointer, @type_name

#include <stdbool.h>

class Point {
    int x;
    int y;
}
int Point.hash(this) const { return this->x * 31 + this->y; }
Point Point.operator+(this, Point o) const {
    Point r; r.x = this->x + o.x; r.y = this->y + o.y; return r;
}

struct NoHash {
    int v;
}

int main() {
    // @has_method
    if (!@has_method(Point, hash))   return 1;
    if (@has_method(NoHash, hash))   return 2;

    // @has_field
    if (!@has_field(Point, x))       return 10;
    if (!@has_field(Point, y))       return 11;
    if (@has_field(Point, missing))  return 12;
    if (@has_field(Point, hash))     return 13;

    // @is_arithmetic
    if (!@is_arithmetic(int))            return 20;
    if (!@is_arithmetic(long))           return 21;
    if (!@is_arithmetic(unsigned long))  return 22;
    if (!@is_arithmetic(char))           return 23;
    if (!@is_arithmetic(bool))           return 24;
    if (!@is_arithmetic(float))          return 25;
    if (!@is_arithmetic(double))         return 26;
    if (@is_arithmetic(int*))            return 27;
    if (@is_arithmetic(Point))           return 28;
    if (@is_arithmetic(void))            return 29;

    // @is_pointer
    if (!@is_pointer(int*))               return 30;
    if (!@is_pointer(const char*))        return 31;
    if (!@is_pointer(const char* const))  return 32;
    if (!@is_pointer(void*))              return 33;
    if (@is_pointer(int))                 return 34;
    if (@is_pointer(Point))               return 35;

    // @type_name
    const char* n_int = @type_name(int);
    const char* n_pt  = @type_name(Point);
    const char* n_pp  = @type_name(int*);
    if (n_int[0] != 'i') return 40;
    if (n_pt[0]  != 'P') return 41;
    if (n_pp[0]  != 'P') return 42;

    // Dead-branch pruning
    if (@has_method(NoHash, hash)) {
        NoHash n;
        int x = n.hash();   /* dead code */
    }
    if (@has_field(Point, missing)) {
        Point p;
        int x = p.missing;  /* dead code */
    }

    return 0;
}
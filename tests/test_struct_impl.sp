/* compiler: any */
/* TDD-Struct-4: struct + impl + method call */

struct Vec2 {
    i32 x;
    i32 y;
};

impl Vec2 {
    i32 sum() {
        return self.x + self.y;
    }
}

i32 main() {
    Vec2 v;
    v.x = 3;
    v.y = 4;
    return v.sum(); /* 7 */
}

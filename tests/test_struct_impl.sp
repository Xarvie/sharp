/* compiler: any */
/* TDD-Struct-4: struct + impl + method call */

struct Vec2 {
    int x;
    int y;
};

impl Vec2 {
    int sum() {
        return self.x + self.y;
    }
}

int main() {
    Vec2 v;
    v.x = 3;
    v.y = 4;
    return v.sum(); /* 7 */
}
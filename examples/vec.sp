// examples/vec.sp - Phase 2 smoke test.
//
// Exercises every Phase-2 feature:
//   - struct data decl
//   - impl block with static / value-self / pointer-self methods
//   - expression-body method (=> expr ;)
//   - struct literal (named, positional, shorthand)
//   - implicit self field rewrite (bare x, y, z in method body -> self.x, ...)
//   - implicit self method call (bare dot(self) in a method body)
//   - method call routing at the use site (Vec3.new, v.dot, p.mul_ip)

struct Vec3 {
    i32 x;
    i32 y;
    i32 z;
}

impl Vec3 {
    // static, named struct literal
    static Vec3 new(i32 x, i32 y, i32 z)
        { return Vec3 { x: x, y: y, z: z }; }

    // static, positional struct literal
    static Vec3 axis_x() { return Vec3 { 1, 0, 0 }; }

    // value self: x, y, z rewrite to self.x, self.y, self.z
    i32 dot(Vec3 other)
        { return x*other.x + y*other.y + z*other.z; }

    // implicit-self call: dot(self) -> Vec3_dot(self, self)
    i32 sum_sq() {
        return dot(self);
    }

    // pointer self (T* self): mutates in place
    void add_ip(Vec3* self, Vec3 rhs) {
        x = x + rhs.x;
        y = y + rhs.y;
        z = z + rhs.z;
    }

    void mul_ip(Vec3* self, i32 k) {
        x = x * k;
        y = y * k;
        z = z * k;
    }
}

struct Counter {
    i32 n;
}

impl Counter {
    // pointer self with shorthand literal below
    void inc(Counter* self) {
        n = n + 1;
    }
}

i32 main() {
    Vec3 a = Vec3.new(1, 2, 3);
    Vec3 b = Vec3.new(4, 5, 6);

    // value-self call: a passed by value
    i32 d = a.dot(b);                        // 1*4+2*5+3*6 = 32

    // chained static -> value-self
    i32 s = Vec3.axis_x().dot(Vec3.new(7, 8, 9));  // 7

    // implicit-self inner call: Vec3.new(2,3,4).sum_sq() = 4+9+16 = 29
    i32 q = Vec3.new(2, 3, 4).sum_sq();

    // ref-self method call: passes &a
    a.mul_ip(2);                              // a -> (2, 4, 6)
    a.add_ip(b);                              // a -> (6, 9, 12)

    // bare field access on a local
    i32 after = a.x + a.y + a.z;              // 27

    // shorthand struct literal: Counter { n } where n is in scope
    i32 n = 0;
    Counter c = Counter { n };                // (Counter){ .n = n }
    c.inc(); c.inc(); c.inc();                // c.n = 3

    // 32 + 7 + 29 + 27 + 3 = 98
    return d + s + q + after + c.n;
}

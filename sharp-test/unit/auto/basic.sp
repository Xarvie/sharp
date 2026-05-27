// 来源: p111_auto_type.sp, p157_auto_deduction.sp (基础部分)
// auto 类型推导基础测试

// === p111: auto 基本推导 + for-loop ===
int identity(int x) { return x; }

struct Point { int x; int y; }
struct Point make_point(int x, int y) { struct Point p; p.x = x; p.y = y; return p; }

// === p157: auto 基本类型推导 ===
struct Val { int x; }

int main(void) {
    // --- p111: auto int / func / struct ---
    auto a = 42;
    if (a != 42) return 1;
    auto b = identity(7);
    if (b != 7) return 2;
    auto p = make_point(3, 4);
    if (p.x != 3) return 3;
    if (p.y != 4) return 4;
    int sum = 0;
    for (auto i = 0; i < 5; i = i + 1)
        sum = sum + i;
    if (sum != 10) return 5;

    // --- p157: auto float / char / string ---
    auto f = 3.14f;
    if ((int)f != 3) return 6;
    auto d = 7.0;
    if ((int)d != 7) return 7;
    auto c = 'A';
    if (c != 65) return 8;
    auto s = "hello";
    if (s[0] != 'h') return 9;

    // --- p157: auto from expression ---
    int m = 10;
    int n = 20;
    auto sum2 = m + n;
    if (sum2 != 30) return 10;
    float fa = 3.0f;
    float fb = 4.0f;
    auto prod = fa * fb;
    if ((int)prod != 12) return 11;

    // --- p157: auto pointer / const / if ---
    int z = 42;
    auto ptr = &z;
    if (*ptr != 42) return 12;
    auto cc = 99;
    if (cc != 99) return 13;
    auto cond = 1;
    if (cond) {
        auto inside = 77;
        if (inside != 77) return 14;
    }

    return 0;
}
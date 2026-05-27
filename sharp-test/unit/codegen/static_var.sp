// 来源: p277_static_var.sp
// C代码生成: static变量与初始化

int g_counter = 0;

int get_next() {
    static int local = 100;
    local = local + 1;
    return local;
}

void increment_global() {
    g_counter = g_counter + 1;
}

int main() {
    // Global variable
    g_counter = 0;
    increment_global();
    increment_global();
    increment_global();
    if (g_counter != 3) return 1;

    // Static local
    int a = get_next();  /* 101 */
    int b = get_next();  /* 102 */
    int c = get_next();  /* 103 */
    if (a != 101) return 2;
    if (b != 102) return 3;
    if (c != 103) return 4;

    return 0;
}
// 来源: p269_codegen_typedef.sp, p272_codegen_enum.sp
// C代码生成: typedef链 + enum

// === p269: typedef chain ===
typedef int MyInt;
typedef MyInt YourInt;
typedef YourInt OurInt;

int get_max() { return 2147483647; }

// === p272: enum ===
enum Status {
    OK,
    WARN,
    ERR,
    FATAL = 99
};

int get_status_val(enum Status s) {
    if (s == OK)    return 0;
    if (s == WARN)  return 1;
    if (s == ERR)   return 2;
    if (s == FATAL) return 99;
    return -1;
}

int main() {
    // --- p269 ---
    OurInt a = 100;
    MyInt  b = 200;
    int    c = a + b;
    if (c != 300) return 1;
    if (get_max() != 2147483647) return 2;

    // --- p272 ---
    if (get_status_val(OK) != 0)    return 10;
    if (get_status_val(WARN) != 1)  return 11;
    if (get_status_val(ERR) != 2)   return 12;
    if (get_status_val(FATAL) != 99) return 13;

    enum Status s = WARN;
    if (s != WARN) return 14;

    return 0;
}
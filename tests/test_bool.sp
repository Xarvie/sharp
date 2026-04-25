/* compiler: any */
/* TDD-Expr-4: bool literals and logic operators */

i32 main() {
    bool t = true;
    bool f = false;
    if (t && !f) {
        return 1;
    }
    return 0;
}

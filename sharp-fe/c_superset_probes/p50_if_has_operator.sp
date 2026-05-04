/* p50 — `if (@has_operator(T, op))` compile-time branch selection
 *
 * Verifies three things introduced in Sharp v0.12:
 *
 * (1) @has_operator(T, op) parses correctly when op is a bare operator
 *     token (e.g. `+`, `==`, `<`) — previously parse_expr rejected it.
 *
 * (2) `if (@has_operator(T, op))` selects the live branch at compile
 *     time (like C++ `if constexpr`) and does NOT emit the dead branch,
 *     so the dead branch can reference operations T doesn't support
 *     without causing a type error.
 *
 * (3) @static_assert(@has_operator(T, op), "msg") works correctly.
 *
 * Test layout:
 *   - HasPlus: struct with operator+, has_operator → true
 *   - NoPlus:  struct without operator+, has_operator → false
 *   - HasEq:   struct with operator==, check == and < separately
 */
struct HasPlus {
    int v;
    int operator+(HasPlus rhs) { return this->v + rhs.v; }
}

struct NoPlus { int v; }

struct HasEq {
    int v;
    int operator==(HasEq rhs) { return this->v == rhs.v; }
    int operator<(HasEq rhs)  { return this->v <  rhs.v; }
}

/* @static_assert with @has_operator must not fire for HasPlus */
void static_assert_ok(HasPlus a, HasPlus b) {
    @static_assert(@has_operator(HasPlus, +), "HasPlus must support +");
    (void)a; (void)b;
}

/* branch_plus: live=then for HasPlus, live=else for NoPlus */
int branch_plus_has() {
    if (@has_operator(HasPlus, +)) { return 1; } else { return 0; }
}
int branch_plus_no() {
    if (@has_operator(NoPlus, +)) { return 1; } else { return 0; }
}

/* branch_eq: live=then for HasEq, live=else for NoPlus */
int branch_eq_has() {
    if (@has_operator(HasEq, ==)) { return 1; } else { return 0; }
}
int branch_eq_no() {
    if (@has_operator(NoPlus, ==)) { return 1; } else { return 0; }
}

/* multi-operator checks on one struct */
int multi_op_check() {
    int r = 0;
    if (@has_operator(HasEq, ==)) { r = r + 1; }  /* +1: HasEq has == */
    if (@has_operator(HasEq, <))  { r = r + 2; }  /* +2: HasEq has <  */
    if (@has_operator(HasEq, +))  { r = r + 4; }  /* +0: HasEq no +   */
    if (@has_operator(HasEq, >))  { r = r + 8; }  /* +0: HasEq no >   */
    return r;  /* expected: 3 */
}

int main(void) {
    HasPlus a; a.v = 0;
    HasPlus b; b.v = 0;
    static_assert_ok(a, b);

    if (branch_plus_has() != 1) return 1;
    if (branch_plus_no()  != 0) return 2;
    if (branch_eq_has()   != 1) return 3;
    if (branch_eq_no()    != 0) return 4;
    if (multi_op_check()  != 3) return 5;
    return 0;
}

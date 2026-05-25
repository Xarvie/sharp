/* p66_abstract_fnptr_cast.c -- abstract function-pointer cast type.
 *
 * sqlite3.c calls into a syscall override table by casting a
 * void-typed pointer to the right function-pointer signature on
 * each invocation:
 *
 *     ((int (*)(int, uid_t, gid_t)) aSyscall[N].pCurrent)(fd, u, g);
 *
 * The cast type is an abstract declarator with the shape
 * `RetT (*)(args)` -- the `*` lives in parens with no name,
 * followed by the parameter list.  Pre-R3 parse_type didn't
 * recognise the parenthesised abstract `(*)(args)` form, and
 * even after parse, cg_type for TY_PTR(TY_FUNC) emitted only
 * "ret *" (losing the parameter list).  Both paths were fixed.
 */
typedef int uid_t;
typedef long gid_t;

static int counter = 0;

static int my_call(int a, uid_t b, gid_t c) {
    counter += a + (int)b + (int)c;
    return 0;
}

int main(void) {
    void *fp = (void *)my_call;
    /* the abstract function-pointer cast pattern */
    int r = ((int(*)(int, uid_t, gid_t))fp)(7, 11, 22);
    return r + counter - 40;   /* 0 + (7+11+22) - 40 == 0 */
}

/* p70_builtin_va_arg.c — __builtin_va_arg(ap, TYPE) where TYPE is a type.
 *
 * GCC/clang implement stdarg as compiler builtins: __builtin_va_list,
 * __builtin_va_start, __builtin_va_arg, __builtin_va_end.
 * After cpp preprocessing, all va_arg/va_start uses may be rewritten to
 * these builtins.  The key difficulty: __builtin_va_arg(ap, int) has
 * 'int' as a TYPE argument (not an expression), which cannot be parsed
 * by parse_call_args via parse_expr.
 *
 * Probe: manually inline a varargs sum without stdio (exit code encodes
 * result: 0 = correct, 1 = wrong).
 */

int builtin_va_sum(int count, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, count);
    int s = 0;
    for (int i = 0; i < count; i++) {
        s += __builtin_va_arg(ap, int);
    }
    __builtin_va_end(ap);
    return s;
}

int main(void) {
    int result = builtin_va_sum(3, 10, 20, 30);
    return result == 60 ? 0 : 1;
}

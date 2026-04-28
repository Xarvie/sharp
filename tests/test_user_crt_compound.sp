/* compiler: any */
/* Test: User-defined typedef with a CRT-reserved name and a COMPOUND type.
 * The old heuristic (simple type name check) would have skipped this because
 * "unsigned long" is not a simple primitive like "int" or "long".
 * The new source-based filter correctly emits it because it comes from user code. */

typedef unsigned long wint_t;

int main() {
    wint_t wc = 0;
    return 0;
}

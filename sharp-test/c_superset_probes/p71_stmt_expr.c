/* p71_stmt_expr.c — GCC statement-expression ({ stmts; expr; }).
 *
 * Statement-expressions allow a block of statements to produce a value.
 * They are written as ({ stmt1; stmt2; expr; }) and the value is the
 * last expression evaluated.  They're used in many GCC-style macros,
 * most notably glibc's assert() expansion and type-safe MAX/MIN macros.
 *
 * This construct requires:
 *  - parse: recognise '(' '{' as the start of a statement-expression
 *  - AST: a new node kind (AST_STMT_EXPR) holding the block
 *  - sema: type = type of the last expression-statement
 *  - cg: emit ({ ... }) verbatim
 *
 * Probe: compute max(a,b) via a statement-expression macro.
 */
#define MAX(a, b) ({ int _a = (a), _b = (b); _a > _b ? _a : _b; })

int main(void) {
    int x = MAX(3, 7);
    int y = MAX(10, 2);
    /* x == 7, y == 10 → exit 0 */
    return (x == 7 && y == 10) ? 0 : 1;
}

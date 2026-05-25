/* p59_block_scope_enum.c -- anonymous block-scope enum.
 *
 * In ISO C, an anonymous enum at function scope injects each
 * enumerator into the enclosing scope as a constant of type int.
 * stb_driver.c uses this idiom to keep magic numbers close to their
 * usage:
 *
 *     int main(void) {
 *         enum { W = 16, H = 16, C = 3 };
 *         unsigned char src[W * H * C];
 *         ...
 *     }
 *
 * Before R2, three separate bugs made this fail:
 *
 *   1. scope.c build_stmt had no AST_ENUM_DEF case, so enumerators
 *      were never visible to sema -- "undefined name 'W'".
 *
 *   2. cg.c cg_stmt had no AST_ENUM_DEF case, so emit produced
 *      "unsupported stmt kind 6" instead of the enum body, and cc
 *      never saw the constants.
 *
 *   3. parse_enum_def consumed the trailing semicolon itself, which
 *      made the surrounding parse_decl_specifiers swallow the next
 *      statement's int-x-equals-... as a continuation of the same
 *      declaration, reordering the AST so the enum appeared AFTER
 *      its use site in the regenerated C.
 *
 * Test exercises both presence (W is defined) and ordering
 * (computation uses W after the enum is in scope).
 */
int main(void) {
    enum { W = 16, H = 16, SUM = W + H };
    int x = W;
    int y = H;
    int z = SUM;
    return (x + y + z) - 64;
}

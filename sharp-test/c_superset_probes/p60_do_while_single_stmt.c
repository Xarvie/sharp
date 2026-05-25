/* p60_do_while_single_stmt.c -- do-while with an unbraced single-stmt body.
 *
 * ISO C admits "do STATEMENT while (COND);" where STATEMENT may be
 * any single statement, not necessarily a compound statement:
 *
 *     do *p++ = v; while (--len);
 *     do { *p++ = v; } while (--len);    -- equivalent
 *
 * Both forms are valid and semantically identical.  cg.c had a
 * pre-R2 asymmetry: if/while/for/switch all dispatched on
 * body-kind == AST_BLOCK and emitted a synthetic brace wrapper for
 * non-block bodies, but AST_DO_WHILE unconditionally called
 * cg_block.  When body was actually an AST_EXPR_STMT (the unbraced
 * form), cg_block dereferenced the wrong union arm and emitted
 *
 *     do {
 *     } while ((--len));
 *
 * dropping the body silently.  stb_image.h zlib decoder
 *
 *     do *zout++ = v; while (--len);
 *
 * triggered exactly this and decoded gibberish ("bad dist" runtime).
 *
 * Test runs the loop 5 times and asserts the side effect happened.
 */
int main(void) {
    int x = 0;
    int i = 5;
    do x++; while (--i);
    return x - 5;
}

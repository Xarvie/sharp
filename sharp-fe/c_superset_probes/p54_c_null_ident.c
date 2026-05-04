/* p54_c_null_ident.c — `null` as a plain C identifier.
 *
 * Sharp reserves `null` as the null-pointer-constant keyword, but
 * standard C does not.  Real C code (cJSON 1.7.18 line 6360:
 *   `cJSON *null = cJSON_CreateNull();`) uses `null` as a variable name.
 *
 * sharpc must run lex in C mode for `.c` inputs so this parses as
 * `IDENT IDENT = ...` rather than as the Sharp null literal.
 */
int main(void) {
    int *null = (int *)0;
    int *p    = null;
    if (p != null) return 1;
    return 0;
}

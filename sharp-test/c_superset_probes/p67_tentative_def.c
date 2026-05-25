/* p67_tentative_def.c -- ISO C99 tentative definitions.
 *
 * Per C99 6.9.2 paragraph 2, a file-scope declaration with no
 * initialiser and no extern is a "tentative definition".  Multiple
 * tentative defs of the same name combine, and a later definition
 * with an initialiser supplants them all without diagnostic.
 *
 *     static int trace_flags;          // tentative
 *     // ... later in the same TU ...
 *     static int trace_flags = 0;      // real def, supplants above
 *
 * Pre-R3 sharp-fe rejected the second occurrence as a redefinition.
 * sqlite3.c uses this idiom across its trace / config globals.
 */

/* Tentative: no initialiser, no extern */
static int trace_flags;

/* Real definition with initialiser, same name -- ISO C lets these
 * combine into a single var initialised to 7. */
static int trace_flags = 7;

int main(void) {
    return trace_flags - 7;
}

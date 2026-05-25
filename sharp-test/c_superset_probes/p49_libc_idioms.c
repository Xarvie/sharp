/* p49 — Four small idioms that block real-world libc/Lua code if Sharp
 * doesn't accept them, all latent through S2–S4:
 *
 *   (1) Forward struct decl + later complete definition:
 *           struct _IO_FILE;
 *           struct _IO_FILE { int flags; };
 *       glibc's `<bits/types/struct_FILE.h>` does this exact dance.
 *       Sharp now merges the two AST_STRUCT_DEFs in scope_define.
 *
 *   (2) Pointer initialised from integer 0 (the C null-pointer constant):
 *           T *p = 0;
 *           T *q = (void *)0;   // already worked
 *       assign_compat now accepts integer→pointer in this position.
 *
 *   (3) `__builtin_*` callable without explicit declaration.  Lua's
 *       `luai_likely(x)` macro expands to `__builtin_expect(...)`;
 *       sema treats any name with the `__builtin_` prefix as an
 *       undeclared external returning int.
 *
 *   (4) Sibling block scopes don't share declarations.  Lua's `setobj`
 *       macro expands to `{ TValue *io1 = ...; ... }`, called many
 *       times in the same function.  Until S5, build_block reused the
 *       SCOPE_FUNC for the function body's outer block, so all sibling
 *       inner blocks collapsed into the function scope and `io1` was
 *       reported as a redefinition the second time the macro fired.
 *
 *   (5) `typedef enum Tag { ... } Tag;` with the tag name reused as
 *       the typedef alias — Lua's `lcode.h` defines `BinOpr` this way.
 *       scope_define now recognises the enum-tag-equals-typedef-alias
 *       case and merges the two SYM_TYPE entries.
 */

struct fwd_then_body;          /* forward                  (1) */
struct fwd_then_body {         /* body — same name         (1) */
    int flags;
};

typedef enum BinOpr {          /* tag and alias coincide   (5) */
    OPR_ADD, OPR_SUB, OPR_MUL
} BinOpr;

static int probe_takes_ptr(int *p) { return p ? *p : -1; }

int main(int argc, char **argv) {
    (void)argv;

    int                       *p1 = 0;        /* (2) integer 0 */
    struct fwd_then_body      *p2 = 0;        /* (2) ditto, w/ struct ptr */
    BinOpr                     op = OPR_MUL;  /* (5) enum-as-int */

    /* (3) __builtin_expect: sema must NOT report it as undefined.
     * The C compiler's lowering will resolve the real signature; we
     * just need Sharp to type-check the call. */
    int hot = __builtin_expect((argc != 0), 1);

    /* (4) sibling blocks: each defines `io1` independently.  Without
     * the build_block fix, the second declaration would diagnose. */
    if (argc > 1) { int io1 = 0; (void)io1; }
    if (argc > 2) { int io1 = 1; (void)io1; }

    return (p1 != 0)
         + (p2 != 0)
         + (op - OPR_MUL)
         + (hot != 1)
         + (probe_takes_ptr(0) + 1);   /* (2) integer 0 to function param */
}

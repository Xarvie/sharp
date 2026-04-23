/* tests/test_ast_assert.c — negative test proving that accessor macros
 * catch kind mismatches at runtime.
 *
 * We build a dummy ND_IF node and try to read it through an ND_FOR
 * accessor. In debug mode this should assert() and abort (exit != 0).
 * With -DNDEBUG the assert is compiled out — we still check that the
 * program did not crash in some OTHER way, but it won't detect the bug.
 *
 *   cc -std=c11 -I.. -o test_ast_assert test_ast_assert.c ../util.c ../types.c
 *   ./test_ast_assert           # should abort with assert failure
 */
#include "sharp.h"
#include "ast.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/* Force a clean exit code when assert() fires so the harness can detect it. */
static void on_abort(int sig) { (void)sig; exit(42); }

int main(void) {
    signal(SIGABRT, on_abort);

    /* Construct an ND_IF node by hand (no parser needed). */
    Arena* arena = NULL;
    Node* if_node = ARENA_NEW(&arena, Node);
    memset(if_node, 0, sizeof *if_node);
    if_node->kind = ND_IF;

    /* Valid access: if_cond on an if node. */
    Node* c = if_cond(if_node);
    printf("if_cond succeeded (got %p) — this is correct.\n", (void*)c);

    /* Invalid access: for_init on an if node. Expected to fire assert. */
    Node* x = for_init(if_node);
    printf("for_init on ND_IF DID NOT assert — got %p. That's a regression.\n",
           (void*)x);
    arena_free_all(&arena);
    return 0;  /* wrong path: assertion mechanism is broken */
}

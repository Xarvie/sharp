/* tests/test_types.c — unit tests for the types interning module.
 *
 * Runs independently of sharpc. Proves that structurally equal types
 * return the same pointer, that substitution preserves identity on
 * no-op, and that render/mangle are deterministic.
 *
 *   cc -std=c11 -I.. -o test_types test_types.c ../types.c ../util.c
 *   ./test_types
 */
#include "types.h"
#include <stdio.h>
#include <string.h>

static int s_pass = 0, s_fail = 0;
#define CHECK(cond, msg) do {                                                  \
    if (cond) { s_pass++; printf("  ok    %s\n", msg); }                       \
    else      { s_fail++; printf("  FAIL  %s\n", msg); }                       \
} while (0)

int main(void) {
    ty_init();

    /* ─────────────────────────── primitives ─────────────────────────── */
    CHECK(ty_prim(TY_I32) == ty_prim(TY_I32),
          "same primitive kind → same pointer");
    CHECK(ty_prim(TY_I32) != ty_prim(TY_I64),
          "different primitives differ");
    CHECK(ty_is_integer(ty_prim(TY_I32)),
          "i32 is integer");
    CHECK(!ty_is_integer(ty_prim(TY_F64)),
          "f64 is NOT integer");
    CHECK(ty_is_float(ty_prim(TY_F64)),
          "f64 is float");
    CHECK(ty_is_signed_integer(ty_prim(TY_I64)),
          "i64 is signed");
    CHECK(!ty_is_signed_integer(ty_prim(TY_U64)),
          "u64 is unsigned");

    /* ─────────────────────────── pointers ───────────────────────────── */
    Type* pi32_a = ty_ptr(ty_prim(TY_I32));
    Type* pi32_b = ty_ptr(ty_prim(TY_I32));
    CHECK(pi32_a == pi32_b,           "i32* interned identically");
    CHECK(ty_is_pointer_like(pi32_a), "i32* is pointer-like");
    CHECK(ty_is_ptr(pi32_a),          "i32* is ptr");
    CHECK(ty_base(pi32_a) == ty_prim(TY_I32), "base of i32* is i32");

    /* ─────────────────────────── named types ────────────────────────── */
    Type* point_a = ty_named("Point");
    Type* point_b = ty_named("Point");
    CHECK(point_a == point_b, "Point interned");
    CHECK(ty_named("Point") != ty_named("Vec"), "different names differ");

    /* ─────────────────────────── generics ───────────────────────────── */
    Type* targs_a[] = { ty_prim(TY_I32) };
    Type* targs_b[] = { ty_prim(TY_I32) };
    Type* span_i32_a = ty_generic("Span", targs_a, 1);
    Type* span_i32_b = ty_generic("Span", targs_b, 1);
    CHECK(span_i32_a == span_i32_b, "Span<i32> interned");

    Type* targs_u8[] = { ty_prim(TY_U8) };
    Type* span_u8 = ty_generic("Span", targs_u8, 1);
    CHECK(span_i32_a != span_u8, "Span<i32> != Span<u8>");

    /* ─────────────────────────── substitution ───────────────────────── */
    /* subst with empty env → identity */
    CHECK(ty_subst(span_i32_a, NULL, NULL, 0) == span_i32_a,
          "empty subst is identity");

    /* subst with non-matching name → identity */
    const char* names_nope[] = { "U" };
    Type*       repls_nope[] = { ty_prim(TY_I64) };
    CHECK(ty_subst(span_i32_a, names_nope, repls_nope, 1) == span_i32_a,
          "subst with no matching name is identity");

    /* T → i32 applied to Span<T> yields Span<i32>; applied again returns SAME ptr */
    Type* tpl_targs[] = { ty_named("T") };
    Type* span_T      = ty_generic("Span", tpl_targs, 1);
    const char* names_T[] = { "T" };
    Type*       repls_I[] = { ty_prim(TY_I32) };
    Type* subst1 = ty_subst(span_T, names_T, repls_I, 1);
    Type* subst2 = ty_subst(span_T, names_T, repls_I, 1);
    CHECK(subst1 == subst2,        "subst is deterministic");
    CHECK(subst1 == span_i32_a,    "Span<T>[T→i32] == Span<i32> (interned)");

    /* ─────────────────────────── assignability ──────────────────────── */
    CHECK(ty_assignable(ty_prim(TY_I32), ty_prim(TY_I32)), "i32 ← i32");
    CHECK(ty_assignable(ty_prim(TY_I64), ty_prim(TY_I32)), "i64 ← i32");
    CHECK(ty_assignable(ty_prim(TY_F32), ty_prim(TY_I32)), "f32 ← i32 (numeric)");
    CHECK(!ty_assignable(ty_prim(TY_I32),
                         ty_ptr(ty_prim(TY_U8))),
          "i32 ← u8* forbidden");
    CHECK(ty_assignable(ty_ptr(ty_prim(TY_I32)),
                        ty_ptr(ty_prim(TY_VOID))),
          "T* ← void* (NULL literal)");

    /* ─────────────────────────── render ─────────────────────────────── */
    CHECK(strcmp(ty_render(ty_prim(TY_I32)), "i32") == 0,
          "render i32");
    CHECK(strcmp(ty_render(pi32_a), "i32*") == 0,
          "render i32*");
    CHECK(strcmp(ty_render(span_i32_a), "Span<i32>") == 0,
          "render Span<i32>");
    /* cache: second call returns same pointer */
    CHECK(ty_render(span_i32_a) == ty_render(span_i32_a),
          "render result cached");

    /* ─────────────────────────── mangle ─────────────────────────────── */
    CHECK(strcmp(ty_mangle(ty_prim(TY_I32)), "i32") == 0,
          "mangle i32");
    CHECK(strcmp(ty_mangle(pi32_a), "p_i32") == 0,
          "mangle i32* → p_i32");
    CHECK(strcmp(ty_mangle(span_i32_a), "Span_i32") == 0,
          "mangle Span<i32>");
    Type* box_span_targs[] = { span_i32_a };
    Type* box_span = ty_generic("Box", box_span_targs, 1);
    CHECK(strcmp(ty_mangle(box_span), "Box_Span_i32") == 0,
          "mangle Box<Span<i32>>");

    /* ─────────────────────────── summary ────────────────────────────── */
    printf("\n%d passed, %d failed\n", s_pass, s_fail);
    ty_shutdown();
    return s_fail ? 1 : 0;
}

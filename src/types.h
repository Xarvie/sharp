/* types.h — central authority for Sharp's type system.
 *
 * Design:
 *   - All Type* values returned here are INTERNED: two structurally equal
 *     types have the same address. `ty_eq(a, b)` is therefore identity
 *     comparison — O(1), no recursion.
 *   - The module owns its own arena; callers no longer need to thread an
 *     Arena** through every construction site. Call `ty_shutdown()` at
 *     process exit (optional; the OS reclaims).
 *   - Rendering to human-readable strings is cached per Type*.
 *   - Substitution `ty_subst(t, names, types, n)` is pure — the Type's
 *     own contents are never mutated.
 *
 * Migration note: the older `type_prim` / `type_ptr` / … entry points in
 * util.c are still exposed via sharp.h and now delegate here, so existing
 * callers keep working during the cutover.
 */
#ifndef SHARP_TYPES_H
#define SHARP_TYPES_H

#include "sharp.h"      /* brings Type, TypeKind, enum values, Arena */
#include <stdbool.h>
#include <stddef.h>

/* ===================================================================== *
 *   Module lifecycle
 * ===================================================================== */

/* Initialise the intern table + internal arena. Idempotent. */
void  ty_init(void);

/* Release everything allocated by this module. After this, every Type*
 * previously returned is invalid. Only meaningful at process shutdown. */
void  ty_shutdown(void);

/* ===================================================================== *
 *   Construction (all interned)
 * ===================================================================== */

Type* ty_prim   (TypeKind kind);
Type* ty_ptr    (Type* base);
Type* ty_named  (const char* name);
Type* ty_generic(const char* name, Type** targs, int ntargs);
Type* ty_const  (Type* base);   /* const-qualified type */

/* ===================================================================== *
 *   Queries
 * ===================================================================== */

TypeKind     ty_kind  (const Type* t);
Type*        ty_base  (const Type* t);     /* NULL for non-ptr/ref */
const char*  ty_name  (const Type* t);     /* NULL for primitives */
int          ty_ntargs(const Type* t);
Type*        ty_targ  (const Type* t, int i);
bool         ty_is_const(const Type* t);   /* true if const-qualified */

/* ===================================================================== *
 *   Comparison, hashing
 * ===================================================================== */

/* O(1) identity comparison — interning guarantees structural equivalence
 * iff pointer equality. Handles NULLs (two NULLs equal, one NULL unequal). */
static inline bool ty_eq(const Type* a, const Type* b) { return a == b; }

/* Hash a Type* — just cast-to-uintptr-and-mix is enough given interning,
 * but exposed as a function so callers don't special-case. */
size_t ty_hash(const Type* t);

/* ===================================================================== *
 *   Predicates
 * ===================================================================== */

bool ty_is_primitive     (const Type* t);
bool ty_is_integer       (const Type* t);
bool ty_is_signed_integer(const Type* t);
bool ty_is_float         (const Type* t);
bool ty_is_numeric       (const Type* t);
bool ty_is_bool          (const Type* t);
bool ty_is_void          (const Type* t);

bool ty_is_pointer_like  (const Type* t);   /* ptr or ref */
bool ty_is_ptr           (const Type* t);   /* ptr only */
bool ty_is_named         (const Type* t);

/* ===================================================================== *
 *   Substitution
 * ===================================================================== */

/* Apply a substitution environment: wherever a bare TY_NAMED's name
 * matches one of `names[i]`, return `repls[i]`. Traverses into ptr/ref
 * bases and generic targs. Returns the interned result; if nothing
 * changed, returns the input pointer verbatim. */
Type* ty_subst(Type* t, const char** names, Type** repls, int n);

/* ===================================================================== *
 *   Assignability
 *
 *   Is a value of type `from` assignable to a slot of type `to` without
 *   an explicit cast? Encodes Sharp's conversion rules:
 *     - Interned equality → yes.
 *     - NULL (void*) → any ptr-like.
 *     - Numeric ↔ numeric (C's usual arithmetic conversions follow).
 *     - bool/char ↔ integer.
 *     - ref T ↔ T* when bases match.
 * ===================================================================== */
bool ty_assignable(const Type* to, const Type* from);

/* Resolve a C primitive type name to its corresponding primitive Type*.
 * If the name is not a known C primitive alias, returns NULL.
 * Handles all common C spellings: "int", "long long", "unsigned int", etc. */
Type* ty_resolve_c_named(const char* name);

/* ===================================================================== *
 *   Rendering
 * ===================================================================== */

/* Return a human-readable rendering ("i32", "Span<u8>", "ref Point").
 * The returned pointer is owned by this module and stable for the
 * lifetime of the Type* (i.e. essentially forever). Never returns NULL —
 * for a NULL Type*, returns "<unknown>". */
const char* ty_render(const Type* t);

/* Emit a C-safe mangled suffix for monomorphization: `Span_i32`,
 * Cached per Type*, same lifetime as ty_render. */
const char* ty_mangle(const Type* t);

#endif /* SHARP_TYPES_H */

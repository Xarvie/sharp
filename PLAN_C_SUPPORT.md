# Sharp Compiler — C Language Full Support Implementation Plan

## Status: PLAN MODE — awaiting user confirmation before any code changes

**Date:** 2026-04-26
**Scope:** Fix all 13 failing C-compatibility test cases with production-grade architecture
**Current Pass Rate:** 60/73 (82.2%)
**Target Pass Rate:** 73/73 (100%)

---

## 1. Executive Summary

This plan addresses the 13 remaining C-language compatibility gaps in the Sharp compiler. The fixes are organized by architectural dependency — each phase builds upon the previous, ensuring no shortcuts or workarounds. Every change is designed to be a **permanent, correct addition** to the compiler's grammar, AST, semantic analysis, and code generation.

---

## 2. Root Cause Analysis (per failing test)

### P0 — Blocking Standard Library Header Support

| # | Test | Root Cause | Location |
|---|------|-----------|----------|
| 1 | `test_anonymous_struct.sp` | `parse_struct()` hardcodes: `Type IDENT ;`. Anonymous fields (`struct { ... } ;`) have no identifier. | `parser.c:1203` |
| 2 | `test_struct_field_declspec.sp` | `parse_struct()` calls `parse_type()` then expects `IDENT`. `__declspec` between type and name breaks this. | `parser.c:1202-1203` |
| 3 | `test_complex_typedef.sp` | `parse_type()` returns a single `Type*`. `typedef T A, *B, **C;` requires multiple declarators per type. | `parser.c:1553-1571` |
| 4 | `test_func_pointer_param.sp` | `parse_type()` has no concept of function-pointer types. `void (*fp)(int)` is not parsed. | `parser.c:276-412` |

### P1 — Important Language Features

| # | Test | Root Cause | Location |
|---|------|-----------|----------|
| 5 | `test_designated_init.sp` | `parse_struct_lit()` only handles `IDENT ':' expr` and positional. `.field = value` in initializer lists not supported. | `parser.c:688-734` |
| 6 | `test_compound_literal.sp` | `parse_primary()` treats `(Type)` as cast. `(Type){ ... }` (compound literal) has no AST node. | `parser.c:820-852` |
| 7 | `test_bitfield.sp` | `parse_struct()` expects `;` after field, encounters `:`. No bitfield width parsing. | `parser.c:1212` |
| 8 | `test_thread_local.sp` | `__thread` / `__declspec(thread)` not recognized as storage-class keywords. | `parser.c:276` |

### P2 — Lower Priority

| # | Test | Root Cause | Location |
|---|------|-----------|----------|
| 9 | `test_complex_attribute.sp` | `skip_attribute()` discards attributes. `aligned(16)`, `packed`, etc. need to carry through to codegen. | `parser.c:230-248` |
| 10 | `test_nested_struct.sp` | `parse_struct()` delegates to `parse_type()` which handles `struct Tag` but not `struct Tag { ... }` inline definition. | `parser.c:374-381` |
| 11 | `test_restrict_param.sp` | `__restrict__` is skipped in `parse_type()` but not attached to parameter nodes. | `parser.c:405-408` |
| 12 | `test_func_pointer.sp` | Same as #4 — function pointer variable declaration. | `parser.c:276-412` |
| 13 | `test_extension.sp` | `__extension__` keyword not recognized/discarded. | `parser.c:276` |

---

## 3. Architectural Principles

1. **No Workarounds**: Every feature gets proper AST representation, not token discarding.
2. **C11 Grammar Compliance**: The parser must accept all C11 declaration and expression forms.
3. **Type System Integrity**: New type forms (function pointer, bitfield) extend `Type`, not bypass it.
4. **Codegen Transparency**: Every AST node has a deterministic C emission path.
5. **Backward Compatibility**: All 60 passing tests must continue to pass.

---

## 4. Implementation Phases

### Phase 1: Declarator System Refactor (Foundation)

**Goal:** Replace the current "type then name" parsing with a proper C-style declarator system that handles pointers, arrays, function types, and multiple declarators per declaration.

**Why first:** Phases 2-5 all depend on being able to parse `void (*fp)(int)`, `T a, *b;`, and `int x:1`.

**Changes:**

#### 1.1 Extend `Type` system (`src/types.h` + `src/types.c`)

Add new type kinds:
```c
TY_FUNC,      /* function type: ret(args...) */
TY_BITFIELD,  /* bitfield: base : width */
```

Add type constructors:
```c
Type* type_func(Arena** a, Type* ret, Type** params, int nparams, bool variadic);
Type* type_bitfield(Arena** a, Type* base, int width);
```

#### 1.2 Create `parse_declarator()` function (`src/parser.c`)

This is the **core architectural addition**. A declarator parser implements the classic C grammar:

```
declaration     = decl_specifiers init_declarator_list ';'
init_declarator = declarator ('=' initializer)?
declarator      = pointer? direct_declarator
direct_declarator = IDENT
                  | '(' declarator ')'
                  | direct_declarator '[' expr? ']'
                  | direct_declarator '(' param_list? ')'
pointer         = '*' type_qualifier* pointer?
```

Implementation strategy:
- `parse_declaration()` — parses declaration-specifiers (type + qualifiers + storage class)
- `parse_declarator()` — parses the declarator, returns `(Type* full_type, const char* name)`
- `parse_abstract_declarator()` — parses type-only declarator (for casts, sizeof, etc.)

The key insight: `parse_declarator()` builds the type **inside-out**. For `int (*fp)(int)`:
1. Start with base type `int`
2. See `(*fp)` → wrap in pointer: `int *`
3. See `(int)` → wrap in function: `int (*)(int)`
4. Extract name `fp`

#### 1.3 Refactor `parse_struct()` to use declarator system

Current:
```c
Type* ty = parse_type(p);
Tok fid = expect(p, TK_IDENT, "expected field name");
```

New:
```c
Type* base = parse_decl_specifiers(p);
Type* full_ty;
const char* name;
bool has_name = parse_field_declarator(p, base, &full_ty, &name);
if (!has_name) {
    /* Anonymous struct/union field */
    /* Create anonymous field with type base */
}
```

#### 1.4 Refactor top-level declarations to use declarator system

`parse_program()` currently has ad-hoc logic for distinguishing functions from variables. Replace with unified `parse_declaration()` that handles:
- `extern int foo(int);`
- `int x = 1, *y;`
- `typedef void (*handler_t)(int);`
- `__thread int tls_var;`

#### 1.5 Refactor `parse_type()` to use abstract declarator

For contexts that need a type without a name (casts, sizeof, generic type args):
```c
Type* parse_type(P* p) {
    Type* base = parse_decl_specifiers(p);
    Type* full = parse_abstract_declarator(p, base);
    return full;
}
```

**Tests unlocked by Phase 1:**
- `test_func_pointer_param.sp` (#4)
- `test_func_pointer.sp` (#12)
- `test_complex_typedef.sp` (#3)
- `test_restrict_param.sp` (#11)
- `test_thread_local.sp` (#8)
- `test_extension.sp` (#13)

---

### Phase 2: Anonymous & Nested Struct/Union Fields

**Goal:** Support anonymous struct/union members and nested struct/union definitions.

**Depends on:** Phase 1 (declarator system for field parsing)

**Changes:**

#### 2.1 Extend `NodeKind`
```c
ND_ANON_STRUCT,  /* anonymous struct field */
ND_ANON_UNION,   /* anonymous union field */
```

#### 2.2 Extend `parse_struct()` field loop

When `parse_field_declarator()` returns `has_name == false`:
- If `base` is a `struct` or `union` type → create `ND_ANON_STRUCT` or `ND_ANON_UNION`
- Otherwise → error (only struct/union can be anonymous)

#### 2.3 Support nested struct/union definitions

In `parse_decl_specifiers()`, when encountering `struct` or `union`:
- If followed by `{` → parse inline definition, generate internal name, return named type
- If followed by `IDENT` then `{` → parse named definition, return named type
- If followed by `IDENT` then not `{` → forward reference, return named type

#### 2.4 Extend `cgen.c` struct emission

For anonymous fields:
```c
case ND_ANON_STRUCT:
    emit_type(g, f->declared_type);  /* emits "struct __anon_N" */
    sb_puts(&g->out, ";\n");         /* no field name */
    break;
```

**Tests unlocked by Phase 2:**
- `test_anonymous_struct.sp` (#1)
- `test_nested_struct.sp` (#10)

---

### Phase 3: Bitfields

**Goal:** Support C bitfield syntax: `unsigned int flags : 3;`

**Depends on:** Phase 1 (declarator system)

**Changes:**

#### 3.1 Extend `Node` for bitfields
```c
/* In struct Node: */
int bit_width;  /* for ND_FIELD with bitfield */
```

#### 3.2 Extend `parse_field_declarator()`

After parsing the declarator, check for `:` followed by constant expression:
```c
if (accept(p, TK_COLON)) {
    Node* width_expr = parse_const_expr(p);
    full_ty = type_bitfield(p->arena, full_ty, eval_const_int(width_expr));
}
```

#### 3.3 Extend `cgen.c` struct emission

```c
emit_type(g, f->declared_type);  /* for bitfield, emits base type */
sb_printf(&g->out, " %s : %d;\n", f->name, f->bit_width);
```

**Tests unlocked by Phase 3:**
- `test_bitfield.sp` (#7)

---

### Phase 4: Designated Initializers & Compound Literals

**Goal:** Support `.field = value` in initializers and `(Type){ ... }` compound literals.

**Depends on:** Phase 1 (for type parsing in compound literals)

**Changes:**

#### 4.1 Extend `parse_struct_lit()`

Current code already handles `IDENT ':' expr` (Sharp named init). Extend to also handle:
- `.IDENT '=' expr` — C designated initializer
- `.IDENT` — shorthand (same as Sharp)

#### 4.2 Add `ND_COMPOUND_LIT` AST node

```c
ND_COMPOUND_LIT,  /* (Type){ init_list } */
```

#### 4.3 Extend `parse_primary()` for compound literals

In the `TK_LPAREN` branch, after confirming it's a cast:
- Lookahead: if next token after `)` is `{`, it's a compound literal, not a cast.
- Parse the type, consume `)`, parse `{ init_list }`.

```c
if (is_cast) {
    Tok after_paren = lex_peek(p->lex);
    if (after_paren.kind == TK_LBRACE) {
        /* Compound literal */
        lex_restore(...);  /* re-parse properly */
        return parse_compound_literal(p);
    }
    /* Regular cast */
    ...
}
```

#### 4.4 Extend `cgen.c`

Compound literals emit as C99 compound literals:
```c
(Type){ .field = value, ... }
```

**Tests unlocked by Phase 4:**
- `test_designated_init.sp` (#5)
- `test_compound_literal.sp` (#6)

---

### Phase 5: __declspec on Struct Fields & Complex Attributes

**Goal:** Carry `__declspec` and `__attribute__` through to codegen for struct fields and declarations.

**Depends on:** Phase 1 (declarator system has proper specifiers)

**Changes:**

#### 5.1 Extend `Node` for field attributes
```c
const char* declspec;     /* __declspec content for fields */
const char* attribute;    /* __attribute__ content */
```

#### 5.2 Capture __declspec in field parsing

In `parse_decl_specifiers()`, capture `__declspec(...)` content and store it.

#### 5.3 Capture __attribute__ in field parsing

Replace `skip_attribute()` with `parse_attribute()` that returns the attribute string.

#### 5.4 Extend `cgen.c` field emission

```c
if (f->declspec) sb_printf(&g->out, "__declspec(%s) ", f->declspec);
if (f->attribute) sb_printf(&g->out, "__attribute__((%s)) ", f->attribute);
emit_type(g, f->declared_type);
sb_printf(&g->out, " %s", f->name);
```

**Tests unlocked by Phase 5:**
- `test_struct_field_declspec.sp` (#2)
- `test_complex_attribute.sp` (#9)

---

## 5. File-by-File Change Summary

| File | Changes |
|------|---------|
| `src/sharp.h` | Add `ND_ANON_STRUCT`, `ND_ANON_UNION`, `ND_COMPOUND_LIT`, `TY_FUNC`, `TY_BITFIELD`, extend `Node` with `bit_width`, `declspec`, `attribute` |
| `src/types.h` | Add `type_func()`, `type_bitfield()` declarations |
| `src/types.c` | Implement `type_func()`, `type_bitfield()`, extend `ty_eq()`, `ty_render()` |
| `src/parser.c` | **Major refactor**: implement `parse_decl_specifiers()`, `parse_declarator()`, `parse_abstract_declarator()`, refactor `parse_struct()`, `parse_union()`, `parse_program()`, `parse_type()` |
| `src/cgen.c` | Extend `emit_struct_body()`, `emit_union_body()`, `emit_expr()` for new node kinds; extend `emit_type()` for `TY_FUNC` and `TY_BITFIELD` |
| `src/sema.c` | Handle new node kinds in semantic analysis (minimal changes needed since C pass-through is the goal) |

---

## 6. Testing Strategy

1. After each phase, run the full test suite to ensure no regressions.
2. After Phase 1, verify the 6 unlocked tests pass.
3. After Phase 2, verify the 2 unlocked tests pass.
4. After Phase 3, verify the 1 unlocked test passes.
5. After Phase 4, verify the 2 unlocked tests pass.
6. After Phase 5, verify the 2 unlocked tests pass.
7. Final validation: all 73 tests pass.

---

## 7. Risk Assessment

| Risk | Mitigation |
|------|-----------|
| Parser refactor breaks existing Sharp syntax | Comprehensive regression testing after each phase; speculative parsing with lexer save/restore already in place |
| Function pointer types complicate type system | `TY_FUNC` is purely for codegen; semantic analysis treats it as opaque |
| Bitfield width needs compile-time evaluation | For now, only accept integer literals; full const-eval can be added later |
| Anonymous structs require name generation | Use `__sharp_anon_struct_N` naming scheme, consistent with existing `__anon_union_N` |

---

## 8. Estimated Scope

| Phase | Files Modified | Lines Added/Modified | Complexity |
|-------|---------------|---------------------|------------|
| Phase 1 | 4 | ~600 | High |
| Phase 2 | 2 | ~150 | Medium |
| Phase 3 | 3 | ~100 | Low |
| Phase 4 | 2 | ~120 | Medium |
| Phase 5 | 2 | ~80 | Low |
| **Total** | **4-5** | **~1050** | **High** |

---

## 9. Success Criteria

- [ ] All 73 `.sp` test files compile successfully with `sharpc -no-link`
- [ ] All 60 previously passing tests continue to produce identical C output
- [ ] All 13 previously failing tests now produce valid C code that compiles with `gcc -std=c11`
- [ ] No compiler warnings at `/W4` level (MSVC)
- [ ] Memory leak check passes (no arena or malloc leaks in test runs)

---

## Appendix A: Template/Generic Compatibility Analysis

### A.1 The Core Conflict

Sharp's template syntax uses `< >` for generic parameters and instantiations:
```sharp
struct Box<T> { T value; }        /* template parameter */
Box<i32> b;                        /* instantiation in expression */
```

C's declarator system also uses `< >` in... nowhere actually. But the **problem** is that `parse_type()` currently has this logic:

```c
if (accept(p, TK_LT)) {
    /* Parse generic type arguments: Name<T, U> */
    Type** targs = ...;
    base = type_named_generic(p->arena, nm, targs, ntargs);
}
```

This is called from **type context** where `<` is unambiguously a generic bracket. But a full C declarator parser would call `parse_type()` from **many more contexts** (inside declarators, after `*`, etc.), and the `<` vs comparison ambiguity becomes critical.

### A.2 Where the Conflict Manifests

| Context | Current Behavior | With C Declarator System |
|---------|-----------------|-------------------------|
| `Box<i32> b;` | `parse_type()` sees `Box`, then `<`, parses `i32` as type arg | Same — **type context**, `<` is generic bracket |
| `a < b > c` (expr) | `parse_expr()` handles comparison | Same — **expression context**, `<` is comparison |
| `void (*fp)(int)` | Not supported | `parse_type()` called from `parse_declarator()` — no `<` involved, safe |
| `T* p` where `T` is generic | `parse_type()` sees `T`, no `<`, returns named type | Same — safe |
| `Name<T> *p` | Not currently supported | `parse_type()` sees `Name`, then `<`, parses `T` — **type context**, safe |

**Conclusion**: The `<` ambiguity is already solved in the current codebase via **speculative parsing** (`lex_save`/`lex_restore` in `parse_primary()`). The declarator system does not introduce new ambiguity because:
1. `parse_type()` is only called in **type contexts** where `<` is always a generic bracket
2. Expression contexts use `parse_expr()` which handles `<` as comparison
3. The only bridge is `parse_primary()` which already has speculation logic

### A.3 Template-Specific Test Cases to Protect

The following tests **must** continue to pass and will be regression-tested after each phase:

| Test | Template Feature | Risk Level |
|------|-----------------|------------|
| `test_generic.sp` | `struct Box<T>`, `impl Box<T>`, `Box<i32>` instantiation | **Critical** |
| `test_struct_impl.sp` | Non-generic struct + impl (no `<` involved) | Low |
| `test_builtin_macro_types.sp` | `__SIZE_TYPE__` typedef (no generics) | Low |
| `test_sizeof_dispatch.sp` | Function overloading by type (no generics) | Low |

### A.4 Design Decision: Conservative Integration

To ensure template compatibility, the declarator system will be implemented with these constraints:

1. **`parse_type()` behavior is PRESERVED exactly** for all existing call sites
   - Generic instantiation `Name<T>` continues to work identically
   - The `<` in type context is always a generic bracket

2. **New `parse_declarator()` delegates to `parse_type()` for the base type**
   ```c
   Type* base = parse_type(p);  /* unchanged behavior */
   Type* full = parse_declarator_suffix(p, base, &name);  /* new: handles *, [], () */
   ```

3. **Function pointer types use a SEPARATE parser path**
   ```c
   /* In parse_declarator_suffix, when we see '(' declarator ')' */
   if (accept(p, TK_LPAREN)) {
       /* This is the function-pointer syntax: (*fp) */
       /* NOT a generic instantiation because we're in declarator context */
       Type* inner_type;
       const char* inner_name;
       parse_declarator(p, &inner_type, &inner_name);  /* recursive */
       expect(p, TK_RPAREN, "expected ')'");
       /* Continue with function parameter list... */
   }
   ```

4. **No changes to `parse_type_params()` or generic instantiation logic**
   - `struct Box<T>` parsing is untouched
   - `Box<i32>` in expressions uses existing speculation logic

### A.5 Mitigation: Incremental Rollout with Regression Gates

After each sub-phase of Phase 1, the following tests will be run:
- `test_generic.sp` — must pass
- `test_struct_impl.sp` — must pass
- All 60 currently passing tests — must pass

If any template test fails, that sub-phase is incomplete and will be fixed before proceeding.

---

*This plan is ready for review. Upon user confirmation, implementation will proceed phase by phase with continuous verification.*

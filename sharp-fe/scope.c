/*
 * scope.c — Phase 4: scope chain and symbol table implementation.
 */

#include "scope.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * djb2 hash for symbol names
 * ====================================================================== */
static unsigned sym_hash(const char *name) {
    unsigned h = 5381;
    for (const unsigned char *p = (const unsigned char *)name; *p; p++)
        h = h * 33 ^ *p;
    return h % SCOPE_NBUCKETS;
}

/* =========================================================================
 * Construction / destruction
 * ====================================================================== */

Scope *scope_new(ScopeKind kind, Scope *parent, AstNode *owner) {
    Scope *s = calloc(1, sizeof *s);
    if (!s) { perror("sharp-fe scope"); abort(); }
    s->kind   = kind;
    s->parent = parent;
    s->owner  = owner;
    /* Phase 4: register with parent so scope_free() can reach all children. */
    if (parent) {
        if (parent->nchildren == parent->children_cap) {
            size_t nc = parent->children_cap ? parent->children_cap * 2 : 4;
            parent->children = realloc(parent->children, nc * sizeof *parent->children);
            if (!parent->children) { perror("sharp-fe scope"); abort(); }
            parent->children_cap = nc;
        }
        parent->children[parent->nchildren++] = s;
    }
    return s;
}

void scope_free(Scope *s) {
    if (!s) return;
    /* Phase 4: free children depth-first before freeing this scope. */
    for (size_t i = 0; i < s->nchildren; i++)
        scope_free(s->children[i]);
    free(s->children);
    for (int i = 0; i < SCOPE_NBUCKETS; i++) {
        Symbol *sym = s->buckets[i];
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }
    free(s);
}

void scope_free_chain(Scope *s) {
    if (!s) return;
    /* Phase 4 fix: walk up to the root first, then call scope_free() which
     * descends depth-first through the children lists.  This prevents the
     * double-free that would occur if we freed each node on the way up and
     * then scope_free() tried to free the same nodes again as children.   */
    while (s->parent) s = s->parent;
    scope_free(s);
}

/* =========================================================================
 * Symbol operations
 * ====================================================================== */

Symbol *scope_lookup_local(Scope *s, const char *name) {
    if (!s) return NULL;
    unsigned h = sym_hash(name);
    for (Symbol *sym = s->buckets[h]; sym; sym = sym->next)
        if (strcmp(sym->name, name) == 0) return sym;
    return NULL;
}

/* Type-aware lookup: walk the chain for a same-named symbol whose kind
 * is in the type namespace (SYM_TYPE / SYM_BUILTIN / SYM_GENERIC_PARAM).
 * Used by sema/cg when interpreting AST_TYPE_NAME to disambiguate the
 * C-tag-vs-ordinary-identifier overlap (e.g. `struct sigaction` and
 * `int sigaction(...)` legally coexist; with both registered in the
 * same Sharp scope, plain scope_lookup_local would return whichever
 * was inserted last). */
static bool sym_kind_is_type(SymKind k) {
    return k == SYM_TYPE || k == SYM_BUILTIN || k == SYM_GENERIC_PARAM;
}
Symbol *scope_lookup_local_type(Scope *s, const char *name) {
    if (!s) return NULL;
    unsigned h = sym_hash(name);
    for (Symbol *sym = s->buckets[h]; sym; sym = sym->next)
        if (strcmp(sym->name, name) == 0 && sym_kind_is_type(sym->kind))
            return sym;
    return NULL;
}
Symbol *scope_lookup_type(Scope *s, const char *name) {
    for (; s; s = s->parent) {
        Symbol *sym = scope_lookup_local_type(s, name);
        if (sym) return sym;
    }
    return NULL;
}

Symbol *scope_lookup(Scope *s, const char *name) {
    for (; s; s = s->parent) {
        Symbol *sym = scope_lookup_local(s, name);
        if (sym) return sym;
    }
    return NULL;
}

Symbol *scope_define(Scope *s, SymKind kind, const char *name,
                     AstNode *decl, FeDiagArr *diags) {
    /* Check for redefinition in the same scope. */
    Symbol *existing = scope_lookup_local(s, name);
    if (existing) {
        /* S1: ISO C allows a function prototype to be followed by the
         * actual definition (and vice-versa: definition + later
         * compatible declarations).  Both have the same SYM_FUNC kind in
         * Sharp's symbol table; we distinguish "prototype" (body == NULL)
         * from "definition" (body != NULL).  Allowed transitions:
         *   prototype  + prototype   → keep first  (both bodyless)
         *   prototype  + definition  → adopt definition
         *   definition + prototype   → keep definition (later proto OK)
         *   definition + definition  → real redefinition error
         * We do not check parameter compatibility here — the C compiler
         * catches mismatches in the generated code. */
        if (kind == SYM_FUNC && existing->kind == SYM_FUNC &&
            decl && decl->kind == AST_FUNC_DEF &&
            existing->decl && existing->decl->kind == AST_FUNC_DEF) {
            bool old_is_def = existing->decl->u.func_def.body != NULL;
            bool new_is_def = decl->u.func_def.body != NULL;
            if (!new_is_def) {
                /* Late prototype after anything — keep existing. */
                return existing;
            }
            if (!old_is_def) {
                /* Promote prototype to full definition. */
                existing->decl = decl;
                return existing;
            }
            /* Both definitions — fall through to error. */
        }
        /* S1: similar for variables — `extern T x; T x = init;` is the
         * canonical "tentative declaration completed by definition"
         * pattern.  Adopt the definition (the one with init) when it
         * comes after an extern-marked declaration; tolerate `extern`
         * after the definition. */
        if (kind == SYM_VAR && existing->kind == SYM_VAR &&
            decl && decl->kind == AST_VAR_DECL &&
            existing->decl && existing->decl->kind == AST_VAR_DECL) {
            bool old_is_extern = existing->decl->u.var_decl.storage == SC_EXTERN;
            bool new_is_extern = decl->u.var_decl.storage == SC_EXTERN;
            bool old_has_init  = existing->decl->u.var_decl.init != NULL;
            bool new_has_init  = decl->u.var_decl.init != NULL;
            /* `extern T x;` then `T x = init;` — promote to definition. */
            if (old_is_extern && !new_is_extern) {
                existing->decl = decl;
                return existing;
            }
            /* `T x = init;` then `extern T x;` — keep definition. */
            if (!old_is_extern && new_is_extern) {
                return existing;
            }
            /* Both extern (no init), or pure tentative defs — keep first. */
            if ((old_is_extern && new_is_extern) ||
                (!old_has_init && !new_has_init && !old_is_extern && !new_is_extern)) {
                return existing;
            }
            /* Phase R3: ISO C99 §6.9.2 tentative definition followed by
             * a real definition with initialiser.  `static T x;` /
             * `T x;` is a tentative def; the later `T x = init;` is
             * the canonical real def, and the two refer to one
             * variable.  Promote the new (initialised) decl over the
             * tentative one regardless of static/extern; sqlite3.c
             * uses this idiom for trace flags and config globals. */
            if (!old_has_init && new_has_init) {
                existing->decl = decl;
                return existing;
            }
            if (old_has_init && !new_has_init) {
                /* Real def already seen; later tentative is a no-op. */
                return existing;
            }
            /* Otherwise (two real definitions with init) → fall through. */
        }
        /* S5: `typedef struct Tag Tag;` followed by `struct Tag { ... };`
         * — the canonical C idiom for a forward-declared typedef whose
         * struct body comes later.  Both register under the same name in
         * Sharp's combined SYM_TYPE namespace.  When the existing symbol
         * is a self-referential typedef (target = AST_TYPE_NAME naming
         * itself, see ty_from_ast self-ref guard) and the new decl is
         * AST_STRUCT_DEF with the same name, promote: replace the
         * typedef decl with the struct decl so subsequent type lookups
         * see the struct body directly.  The reverse order (struct
         * first, then typedef) is also accepted — keep the struct decl. */
        if (kind == SYM_TYPE && existing->kind == SYM_TYPE && decl) {
            AstNode *od = existing->decl;
            AstNode *nd = decl;
            bool od_is_self_typedef =
                od && od->kind == AST_TYPEDEF_DECL &&
                od->u.typedef_decl.target &&
                od->u.typedef_decl.target->kind == AST_TYPE_NAME &&
                od->u.typedef_decl.target->u.type_name.name &&
                strcmp(od->u.typedef_decl.target->u.type_name.name, name) == 0;
            bool nd_is_self_typedef =
                nd && nd->kind == AST_TYPEDEF_DECL &&
                nd->u.typedef_decl.target &&
                nd->u.typedef_decl.target->kind == AST_TYPE_NAME &&
                nd->u.typedef_decl.target->u.type_name.name &&
                strcmp(nd->u.typedef_decl.target->u.type_name.name, name) == 0;
            if (od_is_self_typedef && nd && nd->kind == AST_STRUCT_DEF) {
                existing->decl = nd;  /* promote typedef → struct def */
                return existing;
            }
            if (od && od->kind == AST_STRUCT_DEF && nd_is_self_typedef) {
                return existing;       /* keep struct, ignore late typedef */
            }
            /* S5: `typedef enum Tag { ... } Tag;` — the enum's tag and
             * the typedef alias share the same name.  parse_enum_def
             * registers the AST_ENUM_DEF in pending_decls (named tag)
             * and the surrounding declaration registers an
             * AST_TYPEDEF_DECL whose target points at `Tag`.  Both end
             * up in this scope under the same name.  Accept the
             * collision: promote / keep whichever side carries the
             * enum body so subsequent uses of `Tag` resolve to the
             * enum (which Sharp collapses to int anyway). */
            if (od && od->kind == AST_ENUM_DEF && nd_is_self_typedef) {
                return existing;       /* keep enum, ignore late typedef */
            }
            if (od_is_self_typedef && nd && nd->kind == AST_ENUM_DEF) {
                existing->decl = nd;  /* promote typedef → enum def */
                return existing;
            }
            /* `typedef enum { ... } Tag;` (anonymous enum) followed by
             * later `enum Tag` references — sharp's parse_enum_def has
             * already given the anon enum a synth tag in its name slot
             * (or NULL when truly anonymous).  When the typedef alias
             * matches the enum tag explicitly, scope sees the same name
             * twice; same merge logic. */
            if (od && nd && od->kind == AST_ENUM_DEF &&
                nd->kind == AST_TYPEDEF_DECL) {
                /* Plain typedef whose target is the enum-tag name.  The
                 * typedef just adds an alias in C (in Sharp the enum
                 * already provides the alias).  Drop the typedef. */
                return existing;
            }
            if (od && nd && od->kind == AST_TYPEDEF_DECL &&
                nd->kind == AST_ENUM_DEF) {
                existing->decl = nd;
                return existing;
            }
            /* S5: two AST_STRUCT_DEF entries for the same tag.  C
             * permits any number of `struct Tag;` forward declarations
             * plus at most one full definition — they all refer to the
             * same incomplete-or-completed type.  This pattern shows
             * up in libc's <bits/types/struct_FILE.h>:
             *
             *   struct _IO_FILE;            // forward
             *   struct _IO_FILE { ... };    // body
             *
             * Resolution depends on which side has a body:
             *   forward + forward  → keep first (still incomplete)
             *   forward + body     → adopt body
             *   body    + forward  → keep body
             *   body    + body     → real redefinition (fall through). */
            if (od && od->kind == AST_STRUCT_DEF &&
                nd && nd->kind == AST_STRUCT_DEF &&
                od->u.struct_def.is_union == nd->u.struct_def.is_union) {
                bool od_has_body = od->u.struct_def.fields.len > 0 ||
                                   od->u.struct_def.methods.len > 0;
                bool nd_has_body = nd->u.struct_def.fields.len > 0 ||
                                   nd->u.struct_def.methods.len > 0;
                if (!nd_has_body) return existing;            /* late forward */
                if (!od_has_body) {
                    existing->decl = nd;                       /* promote to body */
                    return existing;
                }
                /* both have bodies → fall through to redefinition error */
            }
        }
        /* S5: ISO C tag namespace is separate from ordinary identifiers.
         * `struct sigaction { ... };` and `int sigaction(int);` legally
         * coexist (glibc <signal.h> uses exactly this).  Sharp folds
         * tags into SYM_TYPE; we recognize the cross-kind collision and
         * register the new symbol alongside instead of erroring.  This
         * fall-through to the bottom of the function will create a
         * second Symbol in the chain — scope_lookup_local stops at the
         * first match by name, so SYM_TYPE wins for type contexts and
         * SYM_FUNC/SYM_VAR for value contexts.  We chain them so that
         * scope_lookup can find either; sema disambiguates by context. */
        if ((kind == SYM_FUNC && existing->kind == SYM_TYPE) ||
            (kind == SYM_TYPE && existing->kind == SYM_FUNC) ||
            (kind == SYM_VAR  && existing->kind == SYM_TYPE) ||
            (kind == SYM_TYPE && existing->kind == SYM_VAR)) {
            /* Fall through into the chain-allocation path below.  Mark
             * this by skipping the redefinition error; control resumes
             * at the malloc just past this block. */
            goto register_new;
        }
        if (diags) {
            CppLoc loc = decl ? decl->loc : (CppLoc){"<unknown>", 0, 0};
            FE_ERROR(diags, loc,
                "redefinition of '%s' (previously defined as %s)",
                name, sym_kind_name(existing->kind));
        }
        return existing;
    }

register_new:
    {
    Symbol *sym = malloc(sizeof *sym);
    if (!sym) { perror("sharp-fe scope"); abort(); }
    sym->kind = kind;
    sym->name = cpp_xstrdup(name);
    sym->decl = decl;
    sym->next = NULL;

    unsigned h       = sym_hash(name);
    sym->next        = s->buckets[h];
    s->buckets[h]    = sym;
    s->nsymbols++;
    return sym;
    }
}

/* =========================================================================
 * Built-in type registration
 * ====================================================================== */

static void register_builtins(Scope *g, FeDiagArr *diags) {
    static const char *builtins[] = {
        "void", "bool", "char", "short", "int", "long",
        "float", "double", "signed", "unsigned",
        "_Bool", "__int128",
        NULL
    };
    for (int i = 0; builtins[i]; i++)
        scope_define(g, SYM_BUILTIN, builtins[i], NULL, diags);
}

/* =========================================================================
 * Recursive scope building
 * ====================================================================== */

/* Forward declarations */
static void build_func(Scope *parent, AstNode *fn, FeDiagArr *diags);
static void build_block(Scope *func_scope, AstNode *block, FeDiagArr *diags);

static void build_struct(Scope *file_scope, AstNode *sd, FeDiagArr *diags) {
    Scope *ss = scope_new(SCOPE_STRUCT, file_scope, sd);
    /* Attach the struct scope to the decl node for later retrieval. */
    sd->type_ref = ss;

    /* Pass 1: register generic params. */
    for (size_t i = 0; i < sd->u.struct_def.generic_params.len; i++) {
        AstNode *gp = sd->u.struct_def.generic_params.data[i];
        scope_define(ss, SYM_GENERIC_PARAM, gp->u.generic_param.name, gp, diags);
    }

    /* Pass 2: register fields.
     * Phase R8: anonymous union/struct member injection.
     * When a field is named `__anon_field_N` (synthesised by parse.c for
     * C anonymous aggregates like `union { T a; U b; };`), also inject
     * the nested aggregate's own fields into the outer struct scope so
     * that `outer->a` and `outer->b` work without naming the union.
     * ISO C §6.7.2.1¶15: "Each member of an anonymous structure or
     * union is considered to be a member of the containing structure
     * or union."
     *
     * Implementation: look up the anonymous type by name in file_scope,
     * find its struct scope, and register each sub-field into ss. */
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        AstNode *fd = sd->u.struct_def.fields.data[i];
        /* Phase R3: skip empty-named bit-field padding. */
        if (!fd->u.field_decl.name || !fd->u.field_decl.name[0])
            continue;
        scope_define(ss, SYM_FIELD, fd->u.field_decl.name, fd, diags);

        /* Anonymous aggregate injection: field name starts with __anon_ */
        if (strncmp(fd->u.field_decl.name, "__anon_", 7) == 0 &&
            fd->u.field_decl.type &&
            fd->u.field_decl.type->kind == AST_TYPE_NAME) {
            const char *inner_name = fd->u.field_decl.type->u.type_name.name;
            Symbol *inner_sym = inner_name
                ? scope_lookup_local(file_scope, inner_name) : NULL;
            AstNode *inner_sd = inner_sym ? inner_sym->decl : NULL;
            if (inner_sd && inner_sd->kind == AST_STRUCT_DEF) {
                /* The inner struct scope may not be built yet (we process
                 * pending_decls in file-scope order, but the anonymous
                 * struct was emitted before the outer struct in pending_decls
                 * so its scope should already exist). */
                Scope *inner_ss = inner_sd->type_ref
                                    ? (Scope *)inner_sd->type_ref : NULL;
                if (inner_ss) {
                    /* Inject each field of the anonymous aggregate. */
                    for (size_t j = 0; j < inner_sd->u.struct_def.fields.len; j++) {
                        AstNode *sub = inner_sd->u.struct_def.fields.data[j];
                        if (!sub->u.field_decl.name || !sub->u.field_decl.name[0])
                            continue;
                        /* Only inject if name not already defined (avoid
                         * overwriting an explicit field with the same name). */
                        if (!scope_lookup_local(ss, sub->u.field_decl.name))
                            scope_define(ss, SYM_FIELD,
                                         sub->u.field_decl.name, sub, diags);
                    }
                }
            }
        }
    }

    /* Pass 3: register methods (two-pass: names first, bodies second). */
    for (size_t i = 0; i < sd->u.struct_def.methods.len; i++) {
        AstNode *fn = sd->u.struct_def.methods.data[i];
        scope_define(ss, SYM_METHOD, fn->u.func_def.name, fn, diags);
    }
    for (size_t i = 0; i < sd->u.struct_def.methods.len; i++) {
        AstNode *fn = sd->u.struct_def.methods.data[i];
        if (fn->u.func_def.body)
            build_func(ss, fn, diags);
    }
}

/* Recursive scope building forward decls */
static void build_stmt(Scope *cur, AstNode *stmt, FeDiagArr *diags);
static void build_block(Scope *parent, AstNode *block, FeDiagArr *diags);

static void build_func(Scope *parent, AstNode *fn, FeDiagArr *diags) {
    Scope *fs = scope_new(SCOPE_FUNC, parent, fn);
    fn->type_ref = fs;  /* attach for retrieval */

    /* Register generic params. */
    for (size_t i = 0; i < fn->u.func_def.generic_params.len; i++) {
        AstNode *gp = fn->u.func_def.generic_params.data[i];
        scope_define(fs, SYM_GENERIC_PARAM, gp->u.generic_param.name, gp, diags);
    }

    /* Register parameters. */
    for (size_t i = 0; i < fn->u.func_def.params.len; i++) {
        AstNode *p = fn->u.func_def.params.data[i];
        if (p->u.param_decl.name)
            scope_define(fs, SYM_PARAM, p->u.param_decl.name, p, diags);
    }

    /* Implicit 'this' for methods (functions inside a SCOPE_STRUCT). */
    if (parent->kind == SCOPE_STRUCT)
        scope_define(fs, SYM_PARAM, "this", fn, diags);

    /* Recurse into body.  We do NOT call build_block(fs, body) because
     * build_block always wants to create its own SCOPE_BLOCK; the
     * function body's outermost `{...}` shares fs (where parameters
     * live), so we walk its statements directly with fs as the
     * current scope.  Without this split, sibling inner blocks like
     *   if (a) { int x; }
     *   if (b) { int x; }
     * would both end up trying to define `x` in fs and produce a
     * spurious "redefinition" diagnostic. */
    if (fn->u.func_def.body) {
        AstNode *body = fn->u.func_def.body;
        body->type_ref = fs;
        for (size_t i = 0; i < body->u.block.stmts.len; i++)
            build_stmt(fs, body->u.block.stmts.data[i], diags);
    }
}

static void build_block(Scope *parent, AstNode *block, FeDiagArr *diags) {
    /* Always create a fresh SCOPE_BLOCK.  Function-body blocks reuse
     * the SCOPE_FUNC at their declaration site (see build_func above);
     * by the time we land here, the block is a true inner block and
     * deserves its own scope so that sibling blocks don't collide on
     * declarations. */
    Scope *bs = scope_new(SCOPE_BLOCK, parent, block);
    block->type_ref = bs;

    for (size_t i = 0; i < block->u.block.stmts.len; i++)
        build_stmt(bs, block->u.block.stmts.data[i], diags);
}

static void build_stmt(Scope *cur, AstNode *stmt, FeDiagArr *diags) {
    if (!stmt) return;
    switch (stmt->kind) {
    case AST_DECL_STMT: {
        AstNode *vd = stmt->u.decl_stmt.decl;
        if (vd && vd->kind == AST_VAR_DECL && vd->u.var_decl.name)
            scope_define(cur, SYM_VAR, vd->u.var_decl.name, vd, diags);
        /* Also handle typedef and struct declarations wrapped in
         * DECL_STMT (as produced when a local typedef appears in a
         * block-scope declaration). */
        if (vd && vd->kind == AST_TYPEDEF_DECL && vd->u.typedef_decl.alias)
            scope_define(cur, SYM_TYPE, vd->u.typedef_decl.alias, vd, diags);
        if (vd && vd->kind == AST_STRUCT_DEF && vd->u.struct_def.name)
            scope_define(cur, SYM_TYPE, vd->u.struct_def.name, vd, diags);
        break;
    }
    /* Bare struct/union/enum definition at block scope — produced when
     * tspec_try_consume parses an inline type like `struct cD { ... }`
     * inside a local variable declaration and queues the definition to
     * pending_decls.  parse_block drains pending_decls as bare AST
     * nodes (not wrapped in DECL_STMT) so we register them here. */
    case AST_STRUCT_DEF:
        if (stmt->u.struct_def.name) {
            scope_define(cur, SYM_TYPE, stmt->u.struct_def.name, stmt, diags);
            /* Build the struct's internal field scope so field-access
             * sema can look up members.  Use the current block scope
             * as the "file" scope parent so generic-param resolution
             * (if any) and field type lookup works correctly. */
            build_struct(cur, stmt, diags);
        }
        break;
    case AST_TYPEDEF_DECL:
        if (stmt->u.typedef_decl.alias)
            scope_define(cur, SYM_TYPE, stmt->u.typedef_decl.alias, stmt, diags);
        break;
    /* Phase R2: block-scope enum.  Mirrors the top-level handling in
     * scope_build (lines 528-540): register the optional tag and each
     * enumerator name in the surrounding scope.  Without this, code
     * like `enum { W = 16, H = 16 }; int x = W + H;` inside a
     * function body fails with "undefined name 'W'".  stb_driver.c's
     * `enum { W = 16, H = 16, C = 3 };` in main is the canonical
     * real-world case. */
    case AST_ENUM_DEF:
        if (stmt->u.enum_def.name)
            scope_define(cur, SYM_TYPE, stmt->u.enum_def.name, stmt, diags);
        for (size_t j = 0; j < stmt->u.enum_def.items.len; j++) {
            AstNode *en = stmt->u.enum_def.items.data[j];
            if (en && en->kind == AST_ENUMERATOR && en->u.enumerator.name)
                scope_define(cur, SYM_VAR, en->u.enumerator.name, en, diags);
        }
        break;
    case AST_BLOCK:
        build_block(cur, stmt, diags);
        break;
    case AST_IF:
        build_stmt(cur, stmt->u.if_.then_, diags);
        build_stmt(cur, stmt->u.if_.else_, diags);
        break;
    case AST_WHILE:
        build_stmt(cur, stmt->u.while_.body, diags);
        break;
    case AST_FOR:
        /* 'for' init may introduce a new scope if it's a decl */
        if (stmt->u.for_.init) {
            Scope *for_scope = scope_new(SCOPE_BLOCK, cur, stmt);
            stmt->type_ref = for_scope;
            build_stmt(for_scope, stmt->u.for_.init, diags);
            build_stmt(for_scope, stmt->u.for_.body, diags);
        } else {
            build_stmt(cur, stmt->u.for_.body, diags);
        }
        break;
    case AST_DO_WHILE:
        build_stmt(cur, stmt->u.do_while.body, diags);
        break;
    case AST_DEFER:
        build_stmt(cur, stmt->u.defer_.body, diags);
        break;
    case AST_LABEL:
        /* labels live in the function scope — look up by traversing parents */
        break;
    default:
        break;
    }
}

/* =========================================================================
 * scope_build — top-level entry point
 * ====================================================================== */

Scope *scope_build(AstNode *file, FeDiagArr *diags) {
    /* Create global scope with built-in types. */
    Scope *global = scope_new(SCOPE_GLOBAL, NULL, NULL);
    register_builtins(global, diags);

    /* Create file scope. */
    Scope *fs = scope_new(SCOPE_FILE, global, file);
    if (file) file->type_ref = fs;

    if (!file) return fs;

    /* Pass 1: register all top-level names (forward-ref support). */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        switch (d->kind) {
        case AST_STRUCT_DEF:
            scope_define(fs, SYM_TYPE, d->u.struct_def.name, d, diags);
            break;
        case AST_FUNC_DEF:
            scope_define(fs, SYM_FUNC, d->u.func_def.name, d, diags);
            break;
        case AST_VAR_DECL:
            if (d->u.var_decl.name)
                scope_define(fs, SYM_VAR, d->u.var_decl.name, d, diags);
            break;
        case AST_TYPEDEF_DECL:
            if (d->u.typedef_decl.alias)
                scope_define(fs, SYM_TYPE, d->u.typedef_decl.alias, d, diags);
            /* Phase R8: `typedef struct Foo Bar;` — opaque forward typedef.
             * The struct tag `Foo` may never appear as a standalone
             * AST_STRUCT_DEF in file.decls (there's no body).  Register `Foo`
             * as a forward-declared SYM_TYPE so that `ty_from_ast` can resolve
             * `Foo` in pointer contexts (`Bar *ptr;`) without "unknown type".
             * We synthesise a no-body AST_STRUCT_DEF node and register it.
             *
             * Guard: skip when the target name is a C built-in primitive
             * (int, long, char, …).  `typedef long X;` must not emit a
             * synthetic `struct long;`. */
            if (d->u.typedef_decl.target &&
                d->u.typedef_decl.target->kind == AST_TYPE_NAME) {
                const char *tag = d->u.typedef_decl.target->u.type_name.name;
                static const char * const BUILTINS[] = {
                    /* single-word primitives */
                    "void","bool","_Bool","char","short","int","long","float",
                    "double","signed","unsigned","__int128",
                    /* two-word combinations that ty_from_name recognises */
                    "long long","long double","signed char","unsigned char",
                    "signed short","unsigned short","signed int","unsigned int",
                    "signed long","unsigned long","long int","short int",
                    "long long int","unsigned long int","signed long long",
                    "unsigned long long","unsigned long long int",
                    "long long unsigned int",
                    /* GCC/clang internal primitives */
                    "__builtin_va_list","_Float64","_Float32",
                    NULL
                };
                bool is_builtin = false;
                if (tag) {
                    for (int bi = 0; BUILTINS[bi]; bi++)
                        if (!strcmp(tag, BUILTINS[bi]))
                            { is_builtin = true; break; }
                }
                if (tag && tag[0] && !is_builtin &&
                    !scope_lookup_local(fs, tag)) {
                    AstNode *fwd = ast_node_new(AST_STRUCT_DEF, d->loc);
                    fwd->u.struct_def.name = cpp_xstrdup(tag);
                    fwd->u.struct_def.is_union = false;
                    scope_define(fs, SYM_TYPE, tag, fwd, diags);
                }
            }
            break;
        case AST_ENUM_DEF:
            /* S2: register the optional tag (if any) as SYM_TYPE so
             * `enum Tag x;` resolves; the type resolver in type.c
             * collapses enum tags to int.  Each enumerator is registered
             * as SYM_VAR so that bare references (`return RED;`) bind. */
            if (d->u.enum_def.name)
                scope_define(fs, SYM_TYPE, d->u.enum_def.name, d, diags);
            for (size_t j = 0; j < d->u.enum_def.items.len; j++) {
                AstNode *en = d->u.enum_def.items.data[j];
                if (en && en->kind == AST_ENUMERATOR && en->u.enumerator.name)
                    scope_define(fs, SYM_VAR, en->u.enumerator.name, en, diags);
            }
            break;
        default:
            break;
        }
    }

    /* Pass 2: build inner scopes (struct bodies, function bodies). */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        if (d->kind == AST_STRUCT_DEF)
            build_struct(fs, d, diags);
        else if (d->kind == AST_FUNC_DEF && d->u.func_def.body)
            build_func(fs, d, diags);
    }

    return fs;
}

/* =========================================================================
 * Name helpers
 * ====================================================================== */

const char *sym_kind_name(SymKind k) {
    static const char *names[] = {
        "var", "func", "type", "param", "field",
        "method", "generic_param", "builtin"
    };
    if ((unsigned)k < sizeof names / sizeof names[0]) return names[k];
    return "?";
}

const char *scope_kind_name(ScopeKind k) {
    static const char *names[] = {
        "global", "file", "struct", "func", "block"
    };
    if ((unsigned)k < sizeof names / sizeof names[0]) return names[k];
    return "?";
}

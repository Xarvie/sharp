/*
 * scope.c — Phase 4: scope chain and symbol table implementation.
 */

#include "scope.h"
#include "type.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * djb2 hash for symbol names
 * ====================================================================== */
/* djb2 variant — returns raw hash; callers mask with (nbuckets - 1). */
static unsigned sym_hash(const char *name) {
    unsigned h = 5381;
    for (const unsigned char *p = (const unsigned char *)name; *p; p++)
        h = h * 33 ^ *p;
    return h;
}

/* =========================================================================
 * Internal: rehash scope when load factor > 0.75
 * ======================================================================== */
static void scope_rehash(Scope *s) {
    size_t new_cap = s->nbuckets ? s->nbuckets * 2 : SCOPE_INIT_BUCKETS;
    Symbol **nb = calloc(new_cap, sizeof *nb);
    if (!nb) { perror("sharp-fe scope rehash"); abort(); }
    size_t mask = new_cap - 1;
    for (size_t i = 0; i < s->nbuckets; i++) {
        Symbol *sym = s->buckets[i];
        while (sym) {
            Symbol *next = sym->next;
            unsigned h = sym_hash(sym->name) & mask;
            sym->next = nb[h];
            nb[h] = sym;
            sym = next;
        }
    }
    free(s->buckets);
    s->buckets  = nb;
    s->nbuckets = new_cap;
}

/* =========================================================================
 * Construction / destruction
 * ====================================================================== */

Scope *scope_new(ScopeKind kind, Scope *parent, AstNode *owner) {
    Scope *s = calloc(1, sizeof *s);
    if (!s) { perror("sharp-fe scope"); abort(); }
    s->kind     = kind;
    s->parent   = parent;
    s->owner    = owner;
    s->nbuckets = SCOPE_INIT_BUCKETS;
    s->buckets  = calloc(SCOPE_INIT_BUCKETS, sizeof *s->buckets);
    if (!s->buckets) { perror("sharp-fe scope_new"); abort(); }
    /* Phase 4: register with parent so scope_free() can reach all children. */
    if (parent) {
        if (parent->nchildren == parent->children_cap) {
            size_t nc = parent->children_cap ? parent->children_cap * 2 : 4;
            Scope **tmp = realloc(parent->children, nc * sizeof *parent->children);
            if (!tmp) { perror("sharp-fe scope"); abort(); }
            parent->children = tmp;
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
    for (size_t i = 0; i < s->nbuckets; i++) {
        Symbol *sym = s->buckets[i];
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }
    free(s->buckets);
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
    unsigned h = sym_hash(name) & (s->nbuckets - 1);
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
/* scope_lookup_local_type — current scope, type-namespace only.
 * Kept as a non-inline function for use from scope.c itself.            */
Symbol *scope_lookup_local_type(Scope *s, const char *name) {
    if (!s) return NULL;
    unsigned h = sym_hash(name) & (s->nbuckets - 1);
    for (Symbol *sym = s->buckets[h]; sym; sym = sym->next)
        if (strcmp(sym->name, name) == 0 && sym_kind_is_type(sym->kind))
            return sym;
    return NULL;
}

/* =========================================================================
 * scope_lookup_ex — the single unified lookup implementation.
 *
 * Replaces the old scope_lookup / scope_lookup_type / scope_lookup_struct_tag
 * trio.  The legacy wrappers in scope.h inline down to this.
 * ======================================================================== */
/* scope_lookup_next_local — given a symbol already found via
 * scope_lookup_local(), return the next symbol with the same name in
 * the same hash bucket, or NULL when the chain is exhausted.
 *
 * Primary use case: iterating operator overloads.  Multiple free-
 * function operators share the same name (e.g. "operator+") but have
 * different parameter types.  scope_lookup / scope_lookup_local return
 * only the first match; this function advances to the next one.
 *
 * Note: only searches the given scope (not parents).  For file-scope
 * operators this is always correct because all overloads are defined
 * at the same level. */
Symbol *scope_lookup_next_local(Scope *s, Symbol *prev, const char *name) {
    (void)s;  /* bucket chain is embedded in prev->next */
    if (!prev || !name) return NULL;
    for (Symbol *sym = prev->next; sym; sym = sym->next)
        if (strcmp(sym->name, name) == 0) return sym;
    return NULL;
}

Symbol *scope_lookup_ex(Scope *s, const char *name, ScopeLookupFlags flags) {
    for (; s; s = s->parent) {
        unsigned h = sym_hash(name) & (s->nbuckets - 1);
        for (Symbol *sym = s->buckets[h]; sym; sym = sym->next) {
            if (strcmp(sym->name, name) != 0) continue;
            switch (flags) {
            case SLOOK_ANY:
                return sym;
            case SLOOK_TYPE_ONLY:
                if (sym_kind_is_type(sym->kind)) return sym;
                break;
            case SLOOK_STRUCT_TAG:
                if (sym->kind == SYM_TYPE && sym->decl &&
                    sym->decl->kind == AST_STRUCT_DEF) return sym;
                break;
            }
        }
    }
    return NULL;
}

/* scope_lookup_value — expression-context lookup that prefers values over
 * types when both share the same name, mirroring C's separate tag / ordinary
 * identifier namespaces.  Returns the first SYM_VAR, SYM_FUNC, SYM_PARAM,
 * or SYM_FIELD found; if none, falls back to the first type symbol. */
Symbol *scope_lookup_value(Scope *s, const char *name) {
    Symbol *type_sym = NULL;
    for (; s; s = s->parent) {
        unsigned h = sym_hash(name) & (s->nbuckets - 1);
        for (Symbol *sym = s->buckets[h]; sym; sym = sym->next) {
            if (strcmp(sym->name, name) != 0) continue;
            if (sym->kind == SYM_VAR || sym->kind == SYM_FUNC ||
                sym->kind == SYM_PARAM || sym->kind == SYM_FIELD)
                return sym;
            if (!type_sym && sym_kind_is_type(sym->kind))
                type_sym = sym;
        }
    }
    return type_sym;
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
            /* Both definitions — fall through to error, unless this
             * is an operator overload with a different parameter
             * signature.  Operator overloads share the same name
             * (e.g. "operator+") but differ in their parameter types
             * (e.g. operator+(VecC, VecS) vs operator+(VecS, VecC)).
             * Chain the new overload in the hash bucket so callers
             * can iterate all overloads via scope_lookup_next_local(). */
            if (decl->u.func_def.is_operator &&
                existing->decl->u.func_def.is_operator) {
                goto register_new;
            }
        }
        /* `FuncTypedef funcName;` declares a function via its
         * function-type typedef (e.g. `mbedtls_ssl_ticket_write_t write;`).
         * Our scope registers it as SYM_VAR (type resolution happens later).
         * When the actual function definition arrives as SYM_FUNC, promote
         * instead of erroring.  The C standard treats this as a compatible
         * forward declaration. */
        if (kind == SYM_FUNC && existing->kind == SYM_VAR &&
            decl && decl->kind == AST_FUNC_DEF) {
            existing->kind = SYM_FUNC;
            existing->decl = decl;
            return existing;
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
            /* ISO C99 §6.9.2 tentative definition followed by
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
                existing->was_typedef = true; /* remember: also typedef'd */
                return existing;
            }
            if (od && od->kind == AST_STRUCT_DEF && nd_is_self_typedef) {
                existing->was_typedef = true; /* typedef arrived late */
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
            /* `enum Tag;` (forward, no items) followed by
             * `enum Tag { A, B };` (full body).  Promote forward to body,
             * just like the S5 struct forward→body rule. */
            if (od && od->kind == AST_ENUM_DEF &&
                nd && nd->kind == AST_ENUM_DEF) {
                bool od_has_body = od->u.enum_def.items.len > 0;
                bool nd_has_body = nd->u.enum_def.items.len > 0;
                if (!od_has_body && nd_has_body) {
                    existing->decl = nd;   /* promote forward → body */
                    return existing;
                }
                if (!nd_has_body) return existing; /* late forward, keep */
                /* both have bodies → fall through to redefinition */
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
                nd && nd->kind == AST_STRUCT_DEF) {
                bool od_has_body = od->u.struct_def.fields.len > 0 ||
                                   od->u.struct_def.methods.len > 0;
                bool nd_has_body = nd->u.struct_def.fields.len > 0 ||
                                   nd->u.struct_def.methods.len > 0;
                /* relax is_union check for forward→body promotion.
                 * R8 creates forward stubs with is_union=false even for
                 * `typedef union Foo T;` because we don't track the keyword.
                 * When the existing entry has NO body, it was synthesised as
                 * a placeholder and must always yield to a real definition,
                 * regardless of struct vs union.  Only block the two-bodies
                 * case (true redefinition) when is_union mismatches. */
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
        /* ISO C allows a struct/union tag to share a name with
         * a non-struct typedef (e.g. `typedef enum {...} Foo` + `struct Foo
         * {...}` — separate tag and ordinary namespaces).  When the NEW
         * definition is an AST_STRUCT_DEF but the existing one is NOT, OR
         * vice versa, let them coexist by registering the new symbol alongside
         * the existing one (same chain mechanism as the SYM_FUNC/SYM_TYPE
         * case above).  The newest entry wins for type resolution but the
         * old typedef is still accessible through the symbol chain. */
        {
            AstNode *od2 = existing->decl;
            AstNode *nd2 = decl;
            bool new_is_struct = nd2 && nd2->kind == AST_STRUCT_DEF;
            bool old_is_struct = od2 && od2->kind == AST_STRUCT_DEF;
            if (new_is_struct != old_is_struct &&
                kind == SYM_TYPE && existing->kind == SYM_TYPE) {
                goto register_new;
            }
        }
        if (kind == SYM_TYPE && existing->kind == SYM_TYPE) {
            return existing;
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
    sym->kind          = kind;
    sym->name          = cpp_xstrdup(name);
    sym->decl          = decl;
    sym->next          = NULL;
    sym->was_typedef   = false;
    sym->is_enum_const = false;  /* V3: set true for injected enum constants */

    /* Resize before insert if load factor would exceed 0.75 */
    if (s->nsymbols + 1 > s->nbuckets * 3 / 4)
        scope_rehash(s);
    unsigned h = sym_hash(name) & (s->nbuckets - 1);
    /* when a struct/union definition coexists with a non-struct
     * typedef of the same name (e.g. `typedef enum{...} Foo` + `struct Foo
     * {...}`), append the struct AFTER the existing symbol so that bare-name
     * lookups (which stop at the first match) find the typedef first.  In C,
     * bare `Foo` refers to the ordinary namespace (typedef), while `struct Foo`
     * refers to the tag namespace (struct).  Appending preserves this priority.
     * For all other cases, prepend (standard LIFO shadowing). */
    if (decl && decl->kind == AST_STRUCT_DEF) {
        /* Check if there's an existing non-struct SYM_TYPE with this name
         * (e.g. typedef) — in that case append so the typedef wins for bare
         * name lookup.  Also check for existing SYM_VAR/SYM_FUNC: in C the
         * tag namespace is separate from the ordinary namespace, so
         * `const int a` and `struct a` coexist; the variable should win for
         * value-context lookups, so append the struct after the variable. */
        Symbol *cur = s->buckets[h];
        bool found_ordinary = false;
        while (cur) {
            if (strcmp(cur->name, name) == 0 &&
                (cur->kind == SYM_VAR || cur->kind == SYM_FUNC ||
                 (cur->kind == SYM_TYPE && cur->decl &&
                  cur->decl->kind != AST_STRUCT_DEF))) {
                found_ordinary = true;
                break;
            }
            cur = cur->next;
        }
        if (found_ordinary) {
            /* Append after the last symbol with this name, or at end of chain */
            Symbol *tail = s->buckets[h]; Symbol *after = NULL;
            while (tail) { after = tail; tail = tail->next; }
            if (after) { sym->next = after->next; after->next = sym; }
            else { sym->next = s->buckets[h]; s->buckets[h] = sym; }
            s->nsymbols++;
            return sym;
        }
    }
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
        "void", "char", "short", "int", "long",
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

static void build_struct(Scope *file_scope, AstNode *sd, FeDiagArr *diags);

/* Recursive helper: inject all visible fields from an anonymous
 * struct/union into the enclosing struct scope.  Handles arbitrary
 * nesting depth by recursing when an injected field is itself an
 * anonymous aggregate. */
static void inject_anon_fields_recursive(Scope *file_scope, Scope *ss,
                                          AstNode *sd, AstNode *inner_sd,
                                          FeDiagArr *diags) {
    for (size_t j = 0; j < inner_sd->u.struct_def.fields.len; j++) {
        AstNode *sub = inner_sd->u.struct_def.fields.data[j];
        if (!sub->u.field_decl.name || !sub->u.field_decl.name[0])
            continue;
        /* Only inject if name not already defined. */
        if (!scope_lookup_local(ss, sub->u.field_decl.name))
            scope_define(ss, SYM_FIELD,
                         sub->u.field_decl.name, sub, diags);

        /* If the injected field is itself an anonymous aggregate,
         * recurse to inject its fields too. */
        if (strncmp(sub->u.field_decl.name, "__anon_", 7) == 0 &&
            sub->u.field_decl.type &&
            sub->u.field_decl.type->kind == AST_TYPE_NAME) {
            const char *deep_name = sub->u.field_decl.type->u.type_name.name;
            Symbol *deep_sym = deep_name
                ? scope_lookup_local(file_scope, deep_name) : NULL;
            AstNode *deep_sd = deep_sym ? deep_sym->decl : NULL;
            if (deep_sd && deep_sd->kind == AST_STRUCT_DEF) {
                Scope *deep_ss = deep_sd->sem_scope
                                   ? (Scope *)deep_sd->sem_scope : NULL;
                if (deep_ss)
                    inject_anon_fields_recursive(file_scope, ss, sd, deep_sd, diags);
            }
        }
    }
}

static void build_struct(Scope *file_scope, AstNode *sd, FeDiagArr *diags) {
    Scope *ss = scope_new(SCOPE_STRUCT, file_scope, sd);
    /* Attach the struct scope to the decl node for later retrieval. */
    sd->sem_scope = ss;

    /* Pass 1: register generic params. */
    for (size_t i = 0; i < sd->u.struct_def.generic_params.len; i++) {
        AstNode *gp = sd->u.struct_def.generic_params.data[i];
        scope_define(ss, SYM_GENERIC_PARAM, gp->u.generic_param.name, gp, diags);
    }

    /* Pass 2: register fields.
     * anonymous union/struct member injection.
     * When a field is named `__anon_field_N` (synthesised by parse.c for
     * C anonymous aggregates like `union { T a; U b; };`), also inject
     * the nested aggregate's own fields into the outer struct scope so
     * that `outer->a` and `outer->b` work without naming the union.
     * ISO C §6.7.2.1¶15: "Each member of an anonymous structure or
     * union is considered to be a member of the containing structure
     * or union."
     *
     * Implementation: look up the anonymous type by name in file_scope,
     * find its struct scope, and register each sub-field into ss.
     * Uses inject_anon_fields_recursive for arbitrary nesting depth. */
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        AstNode *fd = sd->u.struct_def.fields.data[i];
        /* skip empty-named bit-field padding and static members. */
        if (!fd->u.field_decl.name || !fd->u.field_decl.name[0])
            continue;
        if (fd->u.field_decl.is_static)
            continue;  /* static members live at file scope */
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
                Scope *inner_ss = inner_sd->sem_scope
                                    ? (Scope *)inner_sd->sem_scope : NULL;
                if (inner_ss) {
                    inject_anon_fields_recursive(file_scope, ss, sd, inner_sd, diags);
                }
            }
        }
    }

    /* Pass 3: register methods as SYM_METHOD in struct scope.
     * This is required for @has_method(T, name) to find class methods
     * at both template-definition time and specialization time. */
    for (size_t i = 0; i < sd->u.struct_def.methods.len; i++) {
        AstNode *md = sd->u.struct_def.methods.data[i];
        if (!md || !md->u.func_def.name) continue;
        scope_define(ss, SYM_METHOD, md->u.func_def.name, md, diags);
    }
}

/* Recursive scope building forward decls */
static void build_stmt(Scope *cur, AstNode *stmt, FeDiagArr *diags);
static void build_block(Scope *parent, AstNode *block, FeDiagArr *diags);

static void build_func(Scope *parent, AstNode *fn, FeDiagArr *diags) {
    Scope *fs = scope_new(SCOPE_FUNC, parent, fn);
    fn->sem_scope = fs;  /* attach for retrieval */

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

    /* Implicit 'this' for methods (functions inside a SCOPE_STRUCT).
     * Skip if 'this' is already registered (extension methods declare
     * 'this' explicitly in the parameter list). */
    if (parent->kind == SCOPE_STRUCT && !scope_lookup_local(fs, "this"))
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
        body->sem_scope = fs;
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
    block->sem_scope = bs;

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
        /* BUG-005: local function prototype `int f1(char *);` is parsed
         * as a FUNC_DEF with no body inside a DECL_STMT.  Register it
         * so that calls to f1 in the same block are resolved. */
        if (vd && vd->kind == AST_FUNC_DEF && vd->u.func_def.name &&
            !vd->u.func_def.body)
            scope_define(cur, SYM_FUNC, vd->u.func_def.name, vd, diags);
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
    /* block-scope enum.  Mirrors the top-level handling in
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
            stmt->sem_scope = for_scope;
            /* Compound init (AST_BLOCK wrapping multiple decl-stmts
             * from comma-separated multi-variable for-init like
             * `for (int i=0, j=1; …)`): process each stmt directly
             * in for_scope so the variables are visible to the body. */
            if (stmt->u.for_.init->kind == AST_BLOCK) {
                AstNode *ci = stmt->u.for_.init;
                for (size_t i = 0; i < ci->u.block.stmts.len; i++)
                    build_stmt(for_scope, ci->u.block.stmts.data[i], diags);
            } else {
                build_stmt(for_scope, stmt->u.for_.init, diags);
            }
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
    return scope_build_with_prelude(file, diags, NULL);
}

Scope *scope_build_with_prelude(AstNode *file, FeDiagArr *diags,
                                 Scope *prelude) {
    /* Create global scope with built-in types.
     * If a prelude scope is supplied (pass NULL for default global scope),
     * use it as the global scope instead of creating a fresh one. */
    Scope *global = prelude ? prelude : scope_new(SCOPE_GLOBAL, NULL, NULL);
    /* Always register builtins — they're idempotent (scope_define skips
     * duplicates), so calling twice is safe. */
    register_builtins(global, diags);

    /* Create file scope. */
    Scope *fs = scope_new(SCOPE_FILE, global, file);
    if (file) file->sem_scope = fs;

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
            if (d->u.func_def.struct_name) {
                /* Extension method: register with mangled name to avoid
                 * collisions (e.g. both Str.size and HashMap.size would
                 * otherwise register as plain "size"). */
                size_t nlen = strlen(d->u.func_def.struct_name) + 2 + strlen(d->u.func_def.name) + 1;
                char *mangled = malloc(nlen);
                if (mangled) {
                    snprintf(mangled, nlen, "%s__%s",
                             d->u.func_def.struct_name, d->u.func_def.name);
                    scope_define(fs, SYM_FUNC, mangled, d, diags);
                    free(mangled);
                }
            } else {
                scope_define(fs, SYM_FUNC, d->u.func_def.name, d, diags);
            }
            break;
        case AST_VAR_DECL:
            if (d->u.var_decl.name)
                scope_define(fs, SYM_VAR, d->u.var_decl.name, d, diags);
            /* `extern struct Tag var;` where the struct body is
             * defined in another header (e.g. curl's Curl_cftype, Curl_cfilter).
             * If the variable's leaf type is an unrecognised AST_TYPE_NAME that
             * is not yet in scope, synthesise a no-body forward struct so that
             * ty_from_ast can resolve it in pointer/extern contexts.
             * Guard: only for simple identifiers, not primitive types. */
            {
                const AstNode *ty = d->u.var_decl.type;
                while (ty) {
                    if (ty->kind == AST_TYPE_PTR)     { ty = ty->u.type_ptr.base;   continue; }
                    if (ty->kind == AST_TYPE_CONST)   { ty = ty->u.type_const.base; continue; }
                    if (ty->kind == AST_TYPE_ARRAY)   { ty = ty->u.type_array.base; continue; }
                    break;
                }
                if (ty && ty->kind == AST_TYPE_NAME && ty->u.type_name.name) {
                    const char *tname = ty->u.type_name.name;
                    bool has_space = false;
                    for (const char *p = tname; *p; p++) if (*p == ' ') { has_space = true; break; }
                    if (!has_space && !ty_is_known_name(tname) &&
                        !scope_lookup_local(fs, tname) &&
                        (tname[0] >= 'A' && tname[0] <= 'Z') /* struct tag heuristic: starts uppercase */) {
                        AstNode *fwd = ast_node_new(AST_STRUCT_DEF, d->loc);
                        fwd->u.struct_def.name         = cpp_xstrdup(tname);
                        fwd->u.struct_def.is_union     = false;
                        fwd->u.struct_def.is_synthetic = true; /* C8 */
                        astvec_push(&file->u.file.decls, fwd);
                        scope_define(fs, SYM_TYPE, tname, fwd, diags);
                    }
                }
            }
            break;
        case AST_TYPEDEF_DECL:
            if (d->u.typedef_decl.alias)
                scope_define(fs, SYM_TYPE, d->u.typedef_decl.alias, d, diags);
            /* `typedef struct Foo Bar;` — opaque forward typedef.
             * The struct tag `Foo` may never appear as a standalone
             * AST_STRUCT_DEF in file.decls (there's no body).  Register `Foo`
             * as a forward-declared SYM_TYPE so that `ty_from_ast` can resolve
             * `Foo` in pointer contexts (`Bar *ptr;`) without "unknown type".
             * We synthesise a no-body AST_STRUCT_DEF node and register it.
             *
             * Guard: skip when the target name is a C built-in primitive
             * (int, long, char, …).  `typedef long X;` must not emit a
             * synthetic `struct long;`. */
            /* target may also be AST_STRUCT_DEF (when struct
             * keyword was used in the typedef — e.g. typedef struct Foo Bar). */
            if (d->u.typedef_decl.target) {
                const char *tag = NULL;
                /* Unwrap AST_TYPE_PTR layers to reach the inner type,
                 * e.g. typedef struct __pthread * pthread_t;
                 * The target is AST_TYPE_PTR -> AST_TYPE_NAME("__pthread") */
                AstNode *inner = d->u.typedef_decl.target;
                while (inner->kind == AST_TYPE_PTR)
                    inner = inner->u.type_ptr.base;
                if (inner->kind == AST_TYPE_NAME)
                    tag = inner->u.type_name.name;
                else if (inner->kind == AST_STRUCT_DEF)
                    tag = inner->u.struct_def.name;
            if (tag) {
                if (tag[0] && !ty_is_known_name(tag) &&
                    !scope_lookup_local(fs, tag)) {
                    AstNode *fwd = ast_node_new(AST_STRUCT_DEF, d->loc);
                    fwd->u.struct_def.name         = cpp_xstrdup(tag);
                    fwd->u.struct_def.is_union     = false;
                    fwd->u.struct_def.is_synthetic = true; /* C8 */
                    scope_define(fs, SYM_TYPE, tag, fwd, diags);
                    /* push fwd into file->u.file.decls so that
                     * ast_node_free(file) reclaims it; without this the
                     * synthesised node and its name string leak under ASan. */
                    astvec_push(&file->u.file.decls, fwd);
                }
            } /* if (tag) */
            } /* if (d->u.typedef_decl.target) */
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


    /* Build inner scopes for structs, free functions, and extension methods. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        if (d->kind == AST_STRUCT_DEF) {
            build_struct(fs, d, diags);
        } else if (d->kind == AST_FUNC_DEF && d->u.func_def.body) {
            if (d->u.func_def.struct_name) {
                /* Extension method: validate struct_name resolves to a struct/class. */
                const char *sname = d->u.func_def.struct_name;
                Symbol *tsym = scope_lookup_local(fs, sname);
                if (!tsym) {
                    FE_ERROR(diags, d->loc,
                        "extension method target '%s' not found in scope", sname);
                    continue;
                }
                if (!tsym->decl || tsym->decl->kind != AST_STRUCT_DEF) {
                    FE_ERROR(diags, d->loc,
                        "'%s' is not a struct or class", sname);
                    continue;
                }
                /* Check for ambiguous: if there are multiple symbols with same name
                 * (e.g. struct and function), check next in bucket for another struct. */
                {
                    const char *check_name = sname;
                    unsigned h = 5381;
                    for (const unsigned char *p = (const unsigned char *)check_name; *p; p++)
                        h = h * 33 ^ *p;
                    h &= (fs->nbuckets - 1);
                    for (Symbol *nxt = tsym->next; nxt; nxt = nxt->next) {
                        if (nxt == (Symbol*)1) continue;
                        if (strcmp(nxt->name, sname) == 0 &&
                            nxt->decl && nxt->decl->kind == AST_STRUCT_DEF) {
                            FE_ERROR(diags, d->loc,
                                "ambiguous extension method target '%s': multiple matches found", sname);
                            continue;
                        }
                    }
                }

                /* Inherit generic params from the target struct so that
                 * `T Vec<T>.get(...)` can resolve `T` in return/param types. */
                {
                    AstNode *sd = tsym->decl;
                    for (size_t gi = 0; gi < sd->u.struct_def.generic_params.len; gi++) {
                        AstNode *gp_node = (AstNode*)sd->u.struct_def.generic_params.data[gi];
                        if (!gp_node || gp_node->kind != AST_GENERIC_PARAM) continue;
                        const char *gp = gp_node->u.generic_param.name;
                        if (!gp) continue;
                        bool already = false;
                        for (size_t fj = 0; fj < d->u.func_def.generic_params.len; fj++) {
                            AstNode *fn_gp = (AstNode*)d->u.func_def.generic_params.data[fj];
                            if (fn_gp && fn_gp->kind == AST_GENERIC_PARAM &&
                                fn_gp->u.generic_param.name &&
                                strcmp(fn_gp->u.generic_param.name, gp) == 0) {
                                already = true; break;
                            }
                        }
                        if (!already) {
                            AstNode *copy = ast_node_new(AST_GENERIC_PARAM, gp_node->loc);
                            copy->u.generic_param.name = cpp_xstrdup(gp);
                            astvec_push(&d->u.func_def.generic_params, copy);
                        }
                    }
                }

                /* Inject/replace 'this' param for instance methods.
                 * When 'this' was explicitly declared in the param list (parsed),
                 * we replace its type with the proper struct pointer type.
                 * Otherwise (legacy path) we inject a new param. */
                if (d->u.func_def.has_receiver) {
                    if (d->u.func_def.params.len > 0) {
                        AstNode *p0 = d->u.func_def.params.data[0];
                        if (p0 && p0->u.param_decl.name &&
                            strcmp(p0->u.param_decl.name, "this") == 0) {
                            /* Already has a 'this' param from parsing; replace its type */
                            if (p0->u.param_decl.type) ast_node_free(p0->u.param_decl.type);
                            AstNode *pt = ast_node_new(AST_TYPE_PTR, d->loc);
                            AstNode *tn = ast_node_new(AST_TYPE_NAME, d->loc);
                            tn->u.type_name.name = cpp_xstrndup(sname, strlen(sname));
                            pt->u.type_ptr.base = tn;
                            p0->u.param_decl.type = pt;
                            goto after_this_inject;
                        }
                    }
                    /* Inject `StructType* this` as first param (legacy/fallback). */
                    AstNode *tp2 = ast_node_new(AST_PARAM_DECL, d->loc);
                    AstNode *pt2 = ast_node_new(AST_TYPE_PTR, d->loc);
                    AstNode *tn2 = ast_node_new(AST_TYPE_NAME, d->loc);
                    tn2->u.type_name.name = cpp_xstrndup(sname, strlen(sname));
                    pt2->u.type_ptr.base = tn2;
                    tp2->u.param_decl.type = pt2;
                    tp2->u.param_decl.name = cpp_xstrndup("this", 4);
                    astvec_push(&d->u.func_def.params, NULL);
                    for (size_t k = d->u.func_def.params.len - 1; k > 0; k--)
                        d->u.func_def.params.data[k] = d->u.func_def.params.data[k - 1];
                    d->u.func_def.params.data[0] = tp2;
                after_this_inject:;
                }
                /* Extension method: use the struct's scope as parent so that
                 * unqualified field names (e.g. `count` in `this->count`)
                 * resolve correctly via scope traversal: func → struct → file. */
                Scope *struct_scope = tsym->decl->sem_scope;
                build_func(struct_scope ? struct_scope : fs, d, diags);
            } else {
                /* Free function: build with file scope as parent */
                build_func(fs, d, diags);
            }
        }
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

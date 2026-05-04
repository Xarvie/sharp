/*
 * macro.c — Macro definition table and expansion engine.
 *
 * Implements §6.10.3 in full:
 *   - Object-like macros
 *   - Function-like macros (including variadic via __VA_ARGS__)
 *   - Stringification operator  #param
 *   - Token-pasting operator    lhs ## rhs
 *   - Blue-painting / hide-set to prevent recursive expansion (§6.10.3.4)
 *   - __VA_OPT__(tokens)  (C23-style, accepted in Sharp mode)
 */

#include "cpp_internal.h"
#include "macro.h"
#include "pptok.h"

/* =========================================================================
 * Macro parameter table entry
 * ====================================================================== */

typedef struct {
    char   *name;
    int     index;    /* 0-based position in parameter list       */
    bool    is_va;    /* true if this is the __VA_ARGS__ pseudo-param */
} MacroParam;

/* =========================================================================
 * Macro definition
 * ====================================================================== */

typedef struct MacroDef {
    const char   *name;          /* interned                                 */
    bool          is_func;       /* function-like?                           */
    bool          is_variadic;   /* accepts ...?                             */
    int           nparams;       /* number of named parameters               */
    char        **param_names;   /* [nparams] (heap-allocated strings)       */
    TokList       body;          /* replacement list                         */
    CppLoc        def_loc;       /* location of the #define                 */
    struct MacroDef *next;       /* hash-chain                               */
} MacroDef;

/* =========================================================================
 * MacroTable
 * ====================================================================== */

#define MACRO_BUCKETS 512

/* Hide-set: a small unordered array of interned macro-name pointers.
 * Forward-declared here so MacroTable can hold one. */
typedef struct {
    const char **names;
    int          len;
    int          cap;
} HideSet;

/* Hide-set operations are defined further down (after macro definition
 * helpers) but macro_table_free below needs hs_free, so forward-declare. */
static void hs_free(HideSet *hs);

struct MacroTable {
    MacroDef    *buckets[MACRO_BUCKETS];
    InternTable *interns;
    /* Per-run expansion limits (replaces unsafe file-static globals) */
    int          expand_depth;
    int          expand_tokens;
    bool         limits_breached;

    /* Phase 2: stack of macro names currently in the middle of being
     * expanded, used to break mutual-recursion cycles such as
     *      #define A B
     *      #define B A
     *
     * Per ISO C11 §6.10.3.4, an identifier whose name appears in this set
     * must NOT be re-expanded — it passes through as a plain identifier.
     *
     * The set is maintained as a balanced push/pop stack by expand_list:
     *   push name  before recursing into a macro's expansion
     *   pop  name  after the recursion (and any combined rescan) finishes
     * Each top-level macro_expand() call observes the set empty on entry
     * and leaves it empty on exit.                                       */
    HideSet      active_expansions;
};

MacroTable *macro_table_new(InternTable *interns) {
    MacroTable *t = cpp_xmalloc(sizeof *t);
    memset(t, 0, sizeof *t);
    t->interns = interns;
    return t;
}

static uint32_t macro_hash(const char *name) {
    uint32_t h = 2166136261u;
    for (const char *p = name; *p; p++)
        h = (h ^ (uint8_t)*p) * 16777619u;
    return h & (MACRO_BUCKETS - 1);
}

static MacroDef **macro_slot(MacroTable *t, const char *name) {
    uint32_t h = macro_hash(name);
    for (MacroDef **pp = &t->buckets[h]; *pp; pp = &(*pp)->next)
        if (strcmp((*pp)->name, name) == 0)
            return pp;
    return NULL;
}

bool macro_lookup_is_func(MacroTable *t, const char *name) {
    MacroDef *m = macro_lookup(t, name);
    return m && m->is_func;
}

/* Phase 10: see macro.h for rationale.  Trivial accessor; -O2 LTO
 * collapses calls to a single load when callers are in another TU.   */
bool macro_def_is_func(const MacroDef *def) {
    return def && def->is_func;
}

MacroDef *macro_lookup(MacroTable *t, const char *name) {
    MacroDef **pp = macro_slot(t, name);
    return pp ? *pp : NULL;
}

/* Insert or replace a macro definition.  Old definition is freed. */
static bool macro_bodies_equal(const MacroDef *a, const MacroDef *b) {
    if (a->is_func != b->is_func) return false;
    if (a->is_variadic != b->is_variadic) return false;
    if (a->nparams != b->nparams) return false;
    for (int i = 0; i < a->nparams; i++)
        if (strcmp(a->param_names[i], b->param_names[i]) != 0) return false;
    /* Compare body token by token, skipping whitespace.  C99 §6.10.3
     * says two macro definitions are identical if their replacement
     * lists are identical "except for white-space separations" — i.e.
     * any sequence of whitespace counts the same as any other (single
     * space, multiple spaces, line continuations, comments-replaced-
     * by-spaces).  mcpp's n_19.c relies on this: bodies that look like
     * `(1-1)` and `[ws-comment-ws](1-1)[ws-comment-ws]` are valid
     * identical redefinitions per the standard.                     */
    TokNode *an = a->body.head, *bn = b->body.head;
    for (;;) {
        while (an && (an->tok.kind == CPPT_SPACE ||
                      an->tok.kind == CPPT_NEWLINE ||
                      an->tok.kind == CPPT_COMMENT)) an = an->next;
        while (bn && (bn->tok.kind == CPPT_SPACE ||
                      bn->tok.kind == CPPT_NEWLINE ||
                      bn->tok.kind == CPPT_COMMENT)) bn = bn->next;
        if (!an && !bn) return true;
        if (!an || !bn) return false;
        if (an->tok.kind != bn->tok.kind) return false;
        if (strcmp(pptok_spell(&an->tok), pptok_spell(&bn->tok)) != 0) return false;
        /* Phase R4: whitespace requirements between non-ws tokens.
         * Spec says "delimited by white space" must agree; we approximate
         * by requiring that if two tokens were adjacent (no ws between)
         * in one, they are also adjacent in the other.  But this is
         * brittle — tracked differently across macros — so for now we
         * accept any whitespace pattern.  Lua/SQLite/Redis don't trip
         * on this loose check.                                         */
        an = an->next; bn = bn->next;
    }
}

void macro_define(MacroTable *t, MacroDef *def, CppDiagArr *diags, CppLoc loc) {
    def->def_loc = loc;

    /* Phase R4 (e_vargs): C99 §6.10.8/4 — `defined` and `__VA_ARGS__`
     * are not macro names that user code may #define.  GCC and Clang
     * both reject these in pedantic mode; mcpp rejects unconditionally.
     * `__VA_ARGS__` is reserved for the variadic-macro parameter form,
     * and `defined` is a #if operator that must not be hidden by a
     * macro of the same name.                                          */
    if (strcmp(def->name, "__VA_ARGS__") == 0 ||
        strcmp(def->name, "defined") == 0) {
        char msg[160];
        snprintf(msg, sizeof msg,
                 "'%s' shouldn't be defined as a macro name (reserved)",
                 def->name);
        CppDiag d = { CPP_DIAG_ERROR, loc, cpp_xstrdup(msg) };
        da_push(diags, d);
        /* Don't install — free it. */
        for (int i = 0; i < def->nparams; i++) free(def->param_names[i]);
        free(def->param_names);
        tl_free(&def->body);
        free(def);
        return;
    }

    uint32_t h = macro_hash(def->name);
    for (MacroDef **pp = &t->buckets[h]; *pp; pp = &(*pp)->next) {
        if (strcmp((*pp)->name, def->name) == 0) {
            MacroDef *old = *pp;
            /* Check if bodies are identical */
            if (!macro_bodies_equal(old, def)) {
                CppDiag d = { CPP_DIAG_WARNING, def->def_loc,
                              cpp_xstrdup("macro redefined with different body") };
                da_push(diags, d);
            }
            def->next = old->next;
            *pp = def;
            /* Free old */
            for (int i = 0; i < old->nparams; i++) free(old->param_names[i]);
            free(old->param_names);
            tl_free(&old->body);
            free(old);
            return;
        }
    }
    def->next = t->buckets[h];
    t->buckets[h] = def;
}

void macro_undef(MacroTable *t, const char *name) {
    uint32_t h = macro_hash(name);
    for (MacroDef **pp = &t->buckets[h]; *pp; pp = &(*pp)->next) {
        if (strcmp((*pp)->name, name) == 0) {
            MacroDef *m = *pp;
            *pp = m->next;
            for (int i = 0; i < m->nparams; i++) free(m->param_names[i]);
            free(m->param_names);
            tl_free(&m->body);
            free(m);
            return;
        }
    }
}

void macro_table_free(MacroTable *t) {
    for (int i = 0; i < MACRO_BUCKETS; i++) {
        MacroDef *m = t->buckets[i];
        while (m) {
            MacroDef *nx = m->next;
            for (int j = 0; j < m->nparams; j++) free(m->param_names[j]);
            free(m->param_names);
            tl_free(&m->body);
            free(m);
            m = nx;
        }
    }
    /* Phase 2: free the active-expansion stack.  By the time we reach
     * here the stack should already be empty (each macro_expand is
     * balanced); but free defensively in case of an aborted expansion. */
    hs_free(&t->active_expansions);
    free(t);
}

/* =========================================================================
 * Parse a #define directive
 *
 * On entry the cursor is just after "#define" (with whitespace consumed).
 * `toks` is the remaining token list for the directive line.
 * ====================================================================== */

MacroDef *macro_parse_define(const TokList *line_toks,
                              InternTable *interns,
                              CppDiagArr *diags) {
    TokNode *cur = line_toks->head;

    /* Skip leading whitespace */
    while (cur && cur->tok.kind == CPPT_SPACE) cur = cur->next;

    if (!cur || cur->tok.kind != CPPT_IDENT) {
        CppDiag d = { CPP_DIAG_ERROR,
                      cur ? cur->tok.loc : (CppLoc){NULL,0,0},
                      cpp_xstrdup("expected macro name after #define") };
        da_push(diags, d);
        return NULL;
    }

    MacroDef *def = cpp_xmalloc(sizeof *def);
    memset(def, 0, sizeof *def);
    def->name    = intern_cstr(interns, pptok_spell(&cur->tok));
    def->def_loc = cur->tok.loc;
    cur = cur->next; /* advance past name */

    /* If the very next character (no whitespace!) is '(' this is a
     * function-like macro.  has_leading_space distinguishes:
     *   #define FOO(x)   — function-like
     *   #define FOO (x)  — object-like, body starts with '('            */
    if (cur && cur->tok.kind == CPPT_PUNCT &&
        strcmp(pptok_spell(&cur->tok), "(") == 0 &&
        !cur->tok.has_leading_space) {
        def->is_func = true;
        cur = cur->next; /* consume '(' */

        /* Parse parameter list */
        DA(char*) params = {0};
        bool first = true;
        while (cur && !(cur->tok.kind == CPPT_PUNCT &&
                        strcmp(pptok_spell(&cur->tok), ")") == 0)) {
            /* Skip whitespace */
            while (cur && cur->tok.kind == CPPT_SPACE) cur = cur->next;
            if (!cur) break;

            if (cur->tok.kind == CPPT_PUNCT &&
                strcmp(pptok_spell(&cur->tok), "...") == 0) {
                def->is_variadic = true;
                cur = cur->next;
                /* Skip whitespace before ')' */
                while (cur && cur->tok.kind == CPPT_SPACE) cur = cur->next;
                break;
            }

            if (!first) {
                if (cur->tok.kind != CPPT_PUNCT ||
                    strcmp(pptok_spell(&cur->tok), ",") != 0) {
                    CppDiag d = { CPP_DIAG_ERROR, cur->tok.loc,
                                  cpp_xstrdup("expected ',' between macro parameters") };
                    da_push(diags, d);
                    break;
                }
                cur = cur->next;
                while (cur && cur->tok.kind == CPPT_SPACE) cur = cur->next;
                if (!cur) break;
                /* Check for trailing ellipsis after ',' */
                if (cur->tok.kind == CPPT_PUNCT &&
                    strcmp(pptok_spell(&cur->tok), "...") == 0) {
                    def->is_variadic = true;
                    cur = cur->next;
                    while (cur && cur->tok.kind == CPPT_SPACE) cur = cur->next;
                    break;
                }
            }

            if (cur->tok.kind != CPPT_IDENT) {
                CppDiag d = { CPP_DIAG_ERROR, cur->tok.loc,
                              cpp_xstrdup("expected parameter name in macro definition") };
                da_push(diags, d);
                cur = cur->next;
                continue;
            }
            /* Check for duplicate parameter name */
            const char *pname = pptok_spell(&cur->tok);
            for (int pi = 0; pi < (int)params.len; pi++) {
                if (strcmp(params.data[pi], pname) == 0) {
                    CppDiag d = { CPP_DIAG_ERROR, cur->tok.loc,
                                  cpp_xstrdup("duplicate parameter name in macro definition") };
                    da_push(diags, d);
                    break;
                }
            }
            da_push(&params, cpp_xstrdup(pname));
            cur = cur->next;
            first = false;
        }

        /* Consume ')' */
        if (cur && cur->tok.kind == CPPT_PUNCT &&
            strcmp(pptok_spell(&cur->tok), ")") == 0)
            cur = cur->next;

        def->nparams     = (int)params.len;
        def->param_names = params.data; /* transfer ownership */
    }

    /* Replacement list: skip at most ONE leading space (ISO C11 §6.10.3.1:
     * "After the macro name ... the preprocessing tokens that make up the
     *  replacement list ...").  But we must preserve subsequent spaces. */
    while (cur && cur->tok.kind == CPPT_SPACE) cur = cur->next;

    /* Copy remaining tokens into body, stripping trailing whitespace/newline */
    TokNode *last_real = NULL;
    for (TokNode *n = cur; n; n = n->next) {
        if (n->tok.kind == CPPT_NEWLINE) break;
        if (n->tok.kind != CPPT_SPACE)   last_real = n;
    }
    for (TokNode *n = cur; n; n = n->next) {
        if (n->tok.kind == CPPT_NEWLINE) break;

        /* Deep-copy the token into the body list. */
        PPTok copy = n->tok;
        copy.spell = (StrBuf){0};
        sb_push_cstr(&copy.spell, pptok_spell(&n->tok));
        tl_append(&def->body, copy);

        if (n == last_real) break;
    }

    /* Phase R4: validate function-like macro body invariants per
     * C99 §6.10.3.2 and §6.10.3.3.
     *
     *   1. The `#` operator must be followed (skipping whitespace) by
     *      a parameter name (or __VA_ARGS__ in variadic macros).
     *      mcpp's e_24_6.c exercises this: `#define FUNC(a) # b`.
     *   2. The `##` operator must not appear at the very beginning or
     *      end of the replacement list (after stripping whitespace).
     *      mcpp's e_23_3.c exercises both:
     *        `#define con  ## name`
     *        `#define con  name ##`                                    */
    if (def->is_func) {
        for (TokNode *n = def->body.head; n; n = n->next) {
            const char *sp = pptok_spell(&n->tok);
            if (n->tok.kind == CPPT_PUNCT && strcmp(sp, "#") == 0) {
                /* Skip whitespace to find the next real token */
                TokNode *nx = n->next;
                while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
                if (!nx || nx->tok.kind != CPPT_IDENT) {
                    CppDiag d = { CPP_DIAG_ERROR, n->tok.loc,
                        cpp_xstrdup("'#' is not followed by a macro parameter") };
                    da_push(diags, d);
                    continue;
                }
                const char *pname = pptok_spell(&nx->tok);
                bool found = false;
                for (int i = 0; i < def->nparams; i++)
                    if (strcmp(def->param_names[i], pname) == 0)
                        { found = true; break; }
                if (!found && def->is_variadic &&
                    strcmp(pname, "__VA_ARGS__") == 0)
                    found = true;
                if (!found) {
                    char msg[160];
                    snprintf(msg, sizeof msg,
                        "'#' is not followed by a macro parameter ('%s' is not a parameter)",
                        pname);
                    CppDiag d = { CPP_DIAG_ERROR, n->tok.loc, cpp_xstrdup(msg) };
                    da_push(diags, d);
                }
            }
        }
    }

    /* Phase R4: ## at start or end of body — applies to BOTH object-like
     * and function-like macros per C99 §6.10.3.3/1.  mcpp's e_23_3.c
     * exercises both of:
     *     #define con  ## name   (object-like, ## at start)
     *     #define con  name ##   (object-like, ## at end)               */
    {
        TokNode *first_real = NULL, *last_in_body = NULL;
        for (TokNode *n = def->body.head; n; n = n->next) {
            if (n->tok.kind == CPPT_SPACE) continue;
            if (!first_real) first_real = n;
            last_in_body = n;
        }
        if (first_real && first_real->tok.kind == CPPT_PUNCT &&
            strcmp(pptok_spell(&first_real->tok), "##") == 0) {
            CppDiag d = { CPP_DIAG_ERROR, first_real->tok.loc,
                cpp_xstrdup("'##' cannot appear at the start of a macro replacement list") };
            da_push(diags, d);
        }
        if (last_in_body && last_in_body != first_real &&
            last_in_body->tok.kind == CPPT_PUNCT &&
            strcmp(pptok_spell(&last_in_body->tok), "##") == 0) {
            CppDiag d = { CPP_DIAG_ERROR, last_in_body->tok.loc,
                cpp_xstrdup("'##' cannot appear at the end of a macro replacement list") };
            da_push(diags, d);
        }
    }

    return def;
}

/* =========================================================================
 * Hide-set helpers.
 *
 * The HideSet type is forward-declared near struct MacroTable (it appears
 * there as a member).  Below are the operations on it.  Phase 2 uses the
 * set as a push/pop stack of macro names that are currently in the middle
 * of being expanded; an identifier whose name is in the set must not be
 * re-expanded (ISO C11 §6.10.3.4).  The classic case this defeats:
 *
 *      #define A B
 *      #define B A
 *      A   →   B (because A is being expanded, B's body 'A' sees A in the
 *               active set, so A passes through unexpanded)
 *
 * For Phase 2 the set is consulted globally on the MacroTable; per-token
 * hide-sets (the more general Prosser model) remain a Phase 5 polish item.
 * ====================================================================== */

static bool hs_contains(const HideSet *hs, const char *name) {
    for (int i = 0; i < hs->len; i++)
        if (hs->names[i] == name) return true;
    return false;
}

static HideSet hs_union(const HideSet *a, const HideSet *b) {
    HideSet r = {0};
    for (int i = 0; i < a->len; i++) {
        if (r.len == r.cap) {
            r.cap = r.cap ? r.cap*2 : 4;
            r.names = cpp_xrealloc(r.names, r.cap * sizeof *r.names);
        }
        r.names[r.len++] = a->names[i];
    }
    for (int i = 0; i < b->len; i++) {
        if (!hs_contains(&r, b->names[i])) {
            if (r.len == r.cap) {
                r.cap = r.cap ? r.cap*2 : 4;
                r.names = cpp_xrealloc(r.names, r.cap * sizeof *r.names);
            }
            r.names[r.len++] = b->names[i];
        }
    }
    return r;
}

static HideSet hs_add(HideSet *hs, const char *name) {
    HideSet r = *hs;
    if (!hs_contains(&r, name)) {
        if (r.len == r.cap) {
            r.cap = r.cap ? r.cap*2 : 4;
            r.names = cpp_xrealloc(r.names, r.cap * sizeof *r.names);
        }
        r.names[r.len++] = name;
    }
    return r;
}

static void hs_free(HideSet *hs) { free(hs->names); hs->names = NULL; hs->len = hs->cap = 0; }

/* Push an interned name onto MacroTable's active-expansion stack.  The
 * caller is responsible for popping it via active_pop when the recursion
 * window for that macro closes.                                          */
static void active_push(MacroTable *mt, const char *name) {
    HideSet *hs = &mt->active_expansions;
    /* Allow duplicates so that push/pop pairing is straightforward.  The
     * lookup hs_contains will still return true while ANY copy is on
     * the stack — exactly what we want.                                   */
    if (hs->len == hs->cap) {
        hs->cap = hs->cap ? hs->cap * 2 : 8;
        hs->names = cpp_xrealloc(hs->names, hs->cap * sizeof *hs->names);
    }
    hs->names[hs->len++] = name;
}

/* Pop the topmost name (which the caller asserts is `name`) from the stack.
 * Defensive: if the top doesn't match, search downward for the latest
 * occurrence and remove it.  Either way, length decreases by one. */
static void active_pop(MacroTable *mt, const char *name) {
    HideSet *hs = &mt->active_expansions;
    if (hs->len == 0) return;
    if (hs->names[hs->len - 1] == name) { hs->len--; return; }
    /* Mismatched pair — search and remove the most recent matching entry. */
    for (int i = hs->len - 1; i >= 0; i--) {
        if (hs->names[i] == name) {
            for (int j = i; j < hs->len - 1; j++)
                hs->names[j] = hs->names[j + 1];
            hs->len--;
            return;
        }
    }
    /* Nothing matched — shouldn't happen, but stay defensive. */
}

static bool active_contains(MacroTable *mt, const char *name) {
    return hs_contains(&mt->active_expansions, name);
}

/* =========================================================================
 * Stringification (#param)
 * ====================================================================== */

static PPTok stringify(const TokList *arg, CppLoc loc) {
    StrBuf sb = {0};
    sb_push_ch(&sb, '"');
    bool first = true;
    for (TokNode *n = arg->head; n; n = n->next) {
        /* Phase 2 fix: pptok never emits CPPT_SPACE tokens for horizontal
         * whitespace — it folds runs into the next token's
         * has_leading_space flag.  The previous loop only emitted spaces
         * for CPPT_SPACE tokens, so it lost ALL inter-token spacing in
         * the stringified form: S(a   b   c) wrongly became "abc".
         *
         * Rule (ISO C11 §6.10.3.2p2): "Each occurrence of white space
         * between the argument's preprocessing tokens becomes a single
         * space character in the character string literal.  White space
         * before the first preprocessing token and after the last
         * preprocessing token composing the argument is deleted."
         *
         * We emit a single space whenever a non-first non-whitespace
         * token has has_leading_space set OR is itself a CPPT_SPACE
         * token (defensive — preserves correctness if a future tokenizer
         * change starts emitting them).                                  */
        if (n->tok.kind == CPPT_SPACE || n->tok.kind == CPPT_NEWLINE) {
            /* If we ever do see a SPACE token after some content, treat
             * it as inter-token whitespace; ignore leading runs.        */
            if (!first) {
                /* Coalesce multiple whitespace tokens into a single space:
                 * only push if the previous emission wasn't already one. */
                if (sb.len > 1 && sb.buf[sb.len - 1] != ' ')
                    sb_push_ch(&sb, ' ');
            }
            continue;
        }

        if (!first && n->tok.has_leading_space &&
            (sb.len <= 1 || sb.buf[sb.len - 1] != ' '))
            sb_push_ch(&sb, ' ');
        first = false;

        const char *sp = pptok_spell(&n->tok);
        for (const char *p = sp; *p; p++) {
            if (*p == '"' || *p == '\\') sb_push_ch(&sb, '\\');
            sb_push_ch(&sb, *p);
        }
    }
    /* Strip any trailing whitespace before the closing quote (handles the
     * case of a CPPT_SPACE/NEWLINE at the end of the argument list).     */
    while (sb.len > 1 && sb.buf[sb.len - 1] == ' ') sb.len--;
    sb_push_ch(&sb, '"');
    if (sb.buf) sb.buf[sb.len] = '\0';

    PPTok t = {0};
    t.kind  = CPPT_STRING_LIT;
    t.spell = sb;
    t.loc   = loc;
    return t;
}

/* =========================================================================
 * Token pasting (##)
 *
 * Sharp note (lang spec v0.10 §"`#` 与 `##`"):
 *   "预处理器的 # 和 ## 只在 #define 替换列表内生效，与 @ 无关。"
 *   The preprocessor's # and ## have nothing to do with the @ namespace.
 * Concretely: applying ## to an @-intrinsic operand is a programmer
 * mistake.  We detect it, emit a warning, and PRESERVE BOTH tokens as
 * separate output instead of silently dropping the RHS (which is what
 * the standard "invalid paste" path used to do — clang/gcc behavior is
 * fine for plain C but loses information for Sharp's @-tokens).
 *
 * The `out_keep_rhs` out-parameter, when non-NULL, is set to true if the
 * caller should APPEND the RHS to the output list separately (rather
 * than replacing LHS with the pasted result).  Set to false on a
 * normal paste.                                                         */
static PPTok token_paste(const PPTok *lhs, const PPTok *rhs,
                         CppReader *scratch_rd,
                         InternTable *interns,
                         CppDiagArr *diags,
                         bool *out_keep_rhs) {
    if (out_keep_rhs) *out_keep_rhs = false;

    /* Phase 5: Sharp guard — detect @-token operand on either side. */
    if (lhs->kind == CPPT_AT_INTRINSIC || rhs->kind == CPPT_AT_INTRINSIC) {
        if (diags) {
            CppDiag d = {0};
            d.level = CPP_DIAG_WARNING;
            d.loc   = lhs->loc;
            const char *msg = "## cannot paste to/from a Sharp '@' intrinsic; "
                              "tokens preserved as-is";
            d.msg = (char *)cpp_xmalloc(strlen(msg) + 1);
            strcpy(d.msg, msg);
            diag_push(diags, d);
        }
        /* Return LHS unchanged, signal caller to also append RHS. */
        PPTok result = *lhs;
        result.spell = (StrBuf){0};
        sb_push_cstr(&result.spell, pptok_spell(lhs));
        if (out_keep_rhs) *out_keep_rhs = true;
        return result;
    }

    /* Concatenate the spellings and retokenize. */
    StrBuf sb = {0};
    sb_push_cstr(&sb, pptok_spell(lhs));
    sb_push_cstr(&sb, pptok_spell(rhs));

    CppReader *rd = reader_new_from_buf(sb.buf, sb.len,
                                        lhs->loc.file ? lhs->loc.file : "<paste>",
                                        interns, diags);
    PPTok result = reader_next_tok(rd, false);
    /* Verify it was a single token.
     *
     * Phase R4: also reject when the result is whitespace.  This catches
     * `/ ## /` (the lhs+rhs spelling "//" retokenizes to a line comment,
     * which after phase-3 normalisation is CPPT_SPACE).  A pp-token paste
     * must produce one pp-token, and a comment/whitespace is not one.
     * mcpp's u_concat.c exercises exactly this:
     *     #define _VARIANT_BOOL  /##/                                   */
    PPTok check  = reader_next_tok(rd, false);
    bool valid = (check.kind == CPPT_EOF || check.kind == CPPT_NEWLINE ||
                  check.kind == CPPT_SPACE) &&
                 (result.kind != CPPT_SPACE && result.kind != CPPT_COMMENT &&
                  result.kind != CPPT_NEWLINE && result.kind != CPPT_EOF);
    if (!valid) {
        /* C99 6.10.3.3p3: invalid paste → undefined behavior.
         * Phase R4: emit a warning so the diagnostic surfaces (mcpp's
         * Validation Suite expects a diagnostic; previously sharp-cpp
         * silently fell back to LHS).                                  */
        if (diags) {
            CppDiag d = {0};
            d.level = CPP_DIAG_WARNING;
            d.loc   = lhs->loc;
            char buf[160];
            snprintf(buf, sizeof buf,
                "pasting '%s' and '%s' does not give a valid preprocessing token",
                pptok_spell(lhs), pptok_spell(rhs));
            d.msg = cpp_xstrdup(buf);
            diag_push(diags, d);
        }
        pptok_free(&result);
        result = *lhs;
        result.spell = (StrBuf){0};
        sb_push_cstr(&result.spell, pptok_spell(lhs));
    }
    pptok_free(&check);
    reader_free(rd);
    sb_free(&sb);
    result.loc = lhs->loc;
    /* Preserve leading space from LHS — the retokenization of the pasted
     * result (e.g. "HKEY__") starts fresh and loses the original token's
     * has_leading_space.  The paste result inherits the LHS spacing. */
    result.has_leading_space = lhs->has_leading_space;
    return result;
}

/* =========================================================================
 * Core expansion algorithm (§6.10.3.4)
 *
 * expand(token-list, macro-table, hide-set) → token-list
 *
 * We implement the algorithm described in Dave Prosser's original paper and
 * the GCC internals documentation, adapted to a TokList representation.
 * ====================================================================== */

/* Forward declaration */
static void expand_list(TokList *input, MacroTable *mt,
                        InternTable *interns, CppDiagArr *diags,
                        TokList *output);

/* Collect one function-like macro argument (up to the next ',' or ')' at
 * nesting level 0).  Returns the argument as a TokList.                  */
/* collect_arg — read one macro argument until `,` or `)` at depth 0.
 * On return, *cur_p points to the next token after the comma (if a comma
 * ended the arg) or directly at the `)` (if the paren ended it).
 * `ended_with_comma`, if non-NULL, tells the caller which case happened
 * — caller needs this to distinguish `(a)` (1 arg) from `(a,)` (2 args
 * the second being empty).  Phase R4 found this via mcpp's n_nularg.c. */
static TokList collect_arg(TokNode **cur_p, bool *ended_with_comma) {
    TokList arg = {0};
    int depth = 0;
    if (ended_with_comma) *ended_with_comma = false;
    TokNode *last = NULL;
    for (TokNode *n = *cur_p; n; n = n->next) {
        last = n;
        if (n->tok.kind == CPPT_PUNCT) {
            const char *sp = pptok_spell(&n->tok);
            if (strcmp(sp, "(") == 0) depth++;
            else if (strcmp(sp, ")") == 0) {
                if (depth == 0) { *cur_p = n; return arg; }
                depth--;
            } else if (strcmp(sp, ",") == 0 && depth == 0) {
                *cur_p = n->next; /* skip ',' */
                if (ended_with_comma) *ended_with_comma = true;
                return arg;
            }
        }
        /* Copy token into arg */
        PPTok copy = n->tok;
        copy.spell = (StrBuf){0};
        sb_push_cstr(&copy.spell, pptok_spell(&n->tok));
        tl_append(&arg, copy);
    }
    /* Ran off the end without hitting `,` or `)`.  Leave *cur_p
     * pointing at the last consumed token rather than NULL — this
     * lets cascaded macro expansions in expand_list distinguish
     * "incomplete in this list, args continue from source" from
     * "truly unterminated".  The directive layer (process_buf) is
     * the place that detects the latter and emits the diagnostic. */
    *cur_p = last;
    return arg;
}

/* Substitute one replacement list: replace param refs, apply # and ##,
 * then recursively expand.                                                */
static TokList substitute(const MacroDef *def,
                           TokList *args,    /* [nparams + is_variadic] */
                           int nargs,
                           HideSet *caller_hs,
                           MacroTable *mt,
                           InternTable *interns,
                           CppDiagArr *diags) {
    TokList result = {0};

    TokNode *n = def->body.head;
    while (n) {
        const PPTok *t = &n->tok;
        const char  *sp = pptok_spell(t);

        /* --- Stringification: # param --- */
        if (t->kind == CPPT_PUNCT && strcmp(sp, "#") == 0 &&
            def->is_func) {
            TokNode *nx = n->next;
            while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
            if (nx && nx->tok.kind == CPPT_IDENT) {
                /* Find parameter index */
                int idx = -1;
                for (int i = 0; i < def->nparams; i++)
                    if (strcmp(def->param_names[i], pptok_spell(&nx->tok)) == 0)
                        { idx = i; break; }
                if (idx < 0 && def->is_variadic &&
                    strcmp(pptok_spell(&nx->tok), "__VA_ARGS__") == 0)
                    idx = def->nparams;

                int max_idx = def->nparams + (def->is_variadic ? 1 : 0);
                if (idx >= 0 && idx < max_idx) {
                    PPTok s = stringify(&args[idx], t->loc);
                    tl_append(&result, s);
                    n = nx->next; /* skip past # AND the param name */
                    continue;
                }
            }
        }

        /* --- Parameter reference --- */
        if (t->kind == CPPT_IDENT && def->is_func) {
            int idx = -1;
            for (int i = 0; i < def->nparams; i++)
                if (strcmp(def->param_names[i], sp) == 0) { idx = i; break; }
            if (idx < 0 && def->is_variadic && strcmp(sp, "__VA_ARGS__") == 0)
                idx = def->nparams;

            int max_idx = def->nparams + (def->is_variadic ? 1 : 0);
            if (idx >= 0 && idx < max_idx) {
                /* Check for ## on either side */
                bool paste_right = false;
                TokNode *nx = n->next;
                while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
                if (nx && nx->tok.kind == CPPT_PUNCT &&
                    strcmp(pptok_spell(&nx->tok), "##") == 0) {
                    paste_right = true;
                }

                if (paste_right) {
                    /* Paste the unexpanded argument */
                    bool first_arg = true;
                    for (TokNode *an = args[idx].head; an; an = an->next) {
                        PPTok copy = an->tok;
                        copy.spell = (StrBuf){0};
                        sb_push_cstr(&copy.spell, pptok_spell(&an->tok));
                        if (first_arg) {
                            copy.has_leading_space = copy.has_leading_space || t->has_leading_space;
                            first_arg = false;
                        }
                        tl_append(&result, copy);
                    }
                } else {
                    /* Expand the argument before substitution */
                    TokList expanded = {0};
                    TokList arg_copy = {0};
                    for (TokNode *an = args[idx].head; an; an = an->next) {
                        PPTok copy = an->tok;
                        copy.spell = (StrBuf){0};
                        sb_push_cstr(&copy.spell, pptok_spell(&an->tok));
                        tl_append(&arg_copy, copy);
                    }
                    expand_list(&arg_copy, mt, interns, diags, &expanded);
                    tl_free(&arg_copy);

                    /* Phase R10: detect a cascaded macro call whose
                     * arguments themselves expand to an unbalanced
                     * paren sequence — mcpp's e_25_6.c.  Per C99
                     * §6.10.3.4, the rescanning of an argument should
                     * complete within that argument; if it leaves a
                     * dangling `(`, the resulting outer macro call
                     * has undefined behaviour.  GCC silently accepts
                     * (and emits the unexpanded ident); mcpp errors.
                     * sharp-cpp emits a warning — observable enough
                     * for the testsuite, lenient enough not to break
                     * real code (Lua/SQLite/Redis don't do this).   */
                    {
                        int depth = 0;
                        bool unbalanced = false;
                        for (TokNode *en = expanded.head; en; en = en->next) {
                            if (en->tok.kind != CPPT_PUNCT) continue;
                            const char *spx = pptok_spell(&en->tok);
                            if (strcmp(spx, "(") == 0) depth++;
                            else if (strcmp(spx, ")") == 0) {
                                if (depth == 0) { unbalanced = true; break; }
                                depth--;
                            }
                        }
                        if (depth > 0) unbalanced = true;
                        if (unbalanced && diags) {
                            CppDiag d = {0};
                            d.level = CPP_DIAG_WARNING;
                            d.loc   = t->loc;
                            d.msg   = cpp_xstrdup(
                                "macro argument's expansion has unbalanced "
                                "parentheses (cascaded call may be "
                                "unterminated)");
                            diag_push(diags, d);
                        }
                    }

                    /* Phase 2 fix: the first token of the expanded
                     * argument takes its leading-space flag PURELY from
                     * the position of the parameter reference in the
                     * body — not from the expanded argument's first
                     * token, which may carry an irrelevant leading-space
                     * inherited from how the argument's own macro
                     * happened to be defined.
                     *
                     *   #define A 1            // body's '1' has has_leading_space=true
                     *   #define F(x) [x]      // body's 'x' has has_leading_space=false
                     *   F(A)                  // expected: [1]
                     *
                     * Previously this code OR'd the two flags, so the
                     * stray leading-space from A's body leaked through
                     * and produced "[ 1]".  The fix overrides instead. */
                    bool first_arg = true;
                    for (TokNode *en = expanded.head; en; en = en->next) {
                        PPTok copy = en->tok;
                        copy.spell = (StrBuf){0};
                        sb_push_cstr(&copy.spell, pptok_spell(&en->tok));
                        if (first_arg) {
                            copy.has_leading_space = t->has_leading_space;
                            first_arg = false;
                        }
                        tl_append(&result, copy);
                    }
                    tl_free(&expanded);
                }
                n = n->next; continue;
            }
        }

        /* --- Token paste: lhs ## rhs --- */
        if (t->kind == CPPT_PUNCT && strcmp(sp, "##") == 0) {
            /* Remove trailing whitespace from result (C99 6.10.3.3p2).
             * Phase R7: also strip trailing NEWLINE tokens.  When a
             * macro call spans multiple source lines, an arg copied
             * into the body via parameter substitution can carry a
             * trailing NEWLINE; on the next paste round that NEWLINE
             * becomes the LHS and triggers a spurious "pasting '\n'
             * and 'X'" warning.  mcpp's n_37.c.                     */
            TokNode *ws_before_paste = NULL;
            while (result.tail &&
                   (result.tail->tok.kind == CPPT_SPACE ||
                    result.tail->tok.kind == CPPT_NEWLINE)) {
                ws_before_paste = result.tail;
                if (ws_before_paste == result.head) {
                    result.head = NULL;
                    result.tail = NULL;
                } else {
                    TokNode *prev = result.head;
                    while (prev && prev->next != ws_before_paste) prev = prev->next;
                    if (prev) { prev->next = NULL; result.tail = prev; }
                }
                result.len--;
                pptok_free(&ws_before_paste->tok); free(ws_before_paste);
                ws_before_paste = NULL;
            }

            TokNode *nx = n->next;
            while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
            if (nx && result.tail) {
                PPTok lhs = result.tail->tok;

                /* Check if RHS is a macro parameter */
                int param_idx = -1;
                if (nx->tok.kind == CPPT_IDENT && def->is_func) {
                    for (int i = 0; i < def->nparams; i++)
                        if (strcmp(def->param_names[i], pptok_spell(&nx->tok)) == 0)
                            { param_idx = i; break; }
                    if (param_idx < 0 && def->is_variadic &&
                        strcmp(pptok_spell(&nx->tok), "__VA_ARGS__") == 0) {
                        param_idx = def->nparams;
                    }
                }

                /* Determine whether the VA-args slot is empty (not supplied or
                 * supplied as zero tokens).  Used for GNU comma-deletion.    */
                bool va_empty = (param_idx == def->nparams) &&
                                (nargs <= def->nparams ||
                                 !args[def->nparams].head);

                if (va_empty) {
                    /* GNU extension: `lhs, ## __VA_ARGS__` with empty VA →
                     * delete ## and __VA_ARGS__, AND delete a preceding ','
                     * from lhs so MACRO(fmt) expands cleanly.               */
                    if (pptok_spell(&lhs)[0] == ',') {
                        /* Remove the trailing comma from result */
                        TokNode *comma_node = result.tail;
                        if (comma_node == result.head) {
                            result.head = result.tail = NULL;
                        } else {
                            TokNode *prev = result.head;
                            while (prev && prev->next != comma_node) prev = prev->next;
                            if (prev) { prev->next = NULL; result.tail = prev; }
                        }
                        result.len--;
                        pptok_free(&comma_node->tok);
                        free(comma_node);
                    }
                    n = nx->next;
                    continue;
                } else if (param_idx == def->nparams && def->is_variadic &&
                           pptok_spell(&lhs)[0] == ',') {
                    /* Phase R6: GNU extension `, ## __VA_ARGS__` with
                     * NON-empty VA — the `##` is purely a marker for
                     * the VA-empty comma-deletion behaviour above; in
                     * the non-empty case GCC does NOT actually paste
                     * the comma to the first VA token (which would
                     * produce an invalid pp-token like `,foo`).
                     * Instead it emits the comma as-is then the VA
                     * tokens.  This is the canonical idiom and must
                     * not raise a warning.                            */
                    /* Just append the VA arg tokens; lhs (comma) is
                     * already in result.tail.                         */
                    for (TokNode *an = args[param_idx].head; an; an = an->next) {
                        PPTok copy = an->tok;
                        copy.spell = (StrBuf){0};
                        sb_push_cstr(&copy.spell, pptok_spell(&an->tok));
                        tl_append(&result, copy);
                    }
                    n = nx->next;
                    continue;
                } else if (param_idx >= 0 && param_idx < nargs && args[param_idx].head) {
                    /* RHS is a parameter with tokens.
                     * Paste lhs with the FIRST token of the argument,
                     * then append remaining argument tokens to result.
                     * Phase R7: skip leading whitespace tokens (SPACE/
                     * NEWLINE) when picking the first token, otherwise
                     * a multi-line macro call like
                     *     glue31(A, B, C, ... R
                     *            , S, T, ...)
                     * causes paste to see a NEWLINE as RHS, producing
                     * spurious "pasting '\n' and 'S' does not give a
                     * valid pp-token" warnings.  mcpp's n_37.c.       */
                    TokNode *first_node = args[param_idx].head;
                    while (first_node &&
                           (first_node->tok.kind == CPPT_SPACE ||
                            first_node->tok.kind == CPPT_NEWLINE))
                        first_node = first_node->next;
                    if (!first_node) {
                        /* All-whitespace argument — fall through to the
                         * va_empty-style fallback: just remove the LHS
                         * and ## from result.                          */
                        pptok_free(&result.tail->tok);
                        if (result.tail == result.head) {
                            free(result.tail);
                            result.head = result.tail = NULL;
                            result.len = 0;
                        } else {
                            TokNode *prev = result.head;
                            while (prev && prev->next != result.tail) prev = prev->next;
                            free(result.tail);
                            if (prev) { prev->next = NULL; result.tail = prev; }
                            result.len--;
                        }
                        n = nx->next;
                        continue;
                    }
                    PPTok first = first_node->tok;
                    PPTok rhs_first = first;
                    rhs_first.spell = (StrBuf){0};
                    sb_push_cstr(&rhs_first.spell, pptok_spell(&first));

                    bool keep_rhs = false;
                    PPTok pasted = token_paste(&lhs, &rhs_first, NULL,
                                                interns, diags, &keep_rhs);
                    pptok_free(&result.tail->tok);
                    result.tail->tok = pasted;
                    /* Phase 5: if Sharp's @-token guard fired, also append
                     * the RHS verbatim so it survives to the output stream. */
                    if (keep_rhs) {
                        PPTok rhs_copy = rhs_first;
                        rhs_copy.spell = (StrBuf){0};
                        sb_push_cstr(&rhs_copy.spell, pptok_spell(&rhs_first));
                        tl_append(&result, rhs_copy);
                    }
                    pptok_free(&rhs_first);

                    /* Append remaining argument tokens (skip past the
                     * first non-whitespace one we used).               */
                    for (TokNode *an = first_node->next; an; an = an->next) {
                        PPTok copy = an->tok;
                        copy.spell = (StrBuf){0};
                        sb_push_cstr(&copy.spell, pptok_spell(&an->tok));
                        tl_append(&result, copy);
                    }
                    n = nx->next;
                    continue;
                } else if (param_idx >= 0 && param_idx < nargs) {
                    /* RHS is an empty named parameter (zero tokens): lhs wins. */
                    n = nx->next;
                    continue;
                } else {
                    /* RHS is not a parameter — paste tokens literally */
                    PPTok rhs = nx->tok;
                    rhs.spell = (StrBuf){0};
                    sb_push_cstr(&rhs.spell, pptok_spell(&nx->tok));

                    bool keep_rhs = false;
                    PPTok pasted = token_paste(&lhs, &rhs, NULL,
                                                interns, diags, &keep_rhs);
                    pptok_free(&result.tail->tok);
                    result.tail->tok = pasted;
                    if (keep_rhs) {
                        PPTok rhs_copy = rhs;
                        rhs_copy.spell = (StrBuf){0};
                        sb_push_cstr(&rhs_copy.spell, pptok_spell(&rhs));
                        tl_append(&result, rhs_copy);
                    }
                    pptok_free(&rhs);

                    n = nx->next;
                    continue;
                }
            }
            n = n->next; continue; /* ## with nothing on left or right: skip */
        }

        /* --- __VA_OPT__(tokens): expand to tokens iff __VA_ARGS__ non-empty --- */
        if (t->kind == CPPT_IDENT && def->is_variadic &&
            strcmp(sp, "__VA_OPT__") == 0) {
            /* peek for '(' */
            TokNode *nx = n->next;
            while (nx && nx->tok.kind == CPPT_SPACE) nx = nx->next;
            if (nx && nx->tok.kind == CPPT_PUNCT &&
                strcmp(pptok_spell(&nx->tok), "(") == 0) {
                /* Collect body tokens until matching ')' */
                TokList body = {0};
                int depth = 1;
                nx = nx->next;
                while (nx && depth > 0) {
                    if (nx->tok.kind == CPPT_PUNCT) {
                        if (strcmp(pptok_spell(&nx->tok), "(") == 0) depth++;
                        else if (strcmp(pptok_spell(&nx->tok), ")") == 0) {
                            depth--;
                            if (depth == 0) { nx = nx->next; break; }
                        }
                    }
                    tl_append_copy(&body, &nx->tok);
                    nx = nx->next;
                }
                /* Determine if __VA_ARGS__ is non-empty */
                bool va_nonempty = false;
                if (nargs > def->nparams && args[def->nparams].head) {
                    /* check that VA arg is not just whitespace */
                    for (TokNode *an = args[def->nparams].head; an; an = an->next) {
                        if (an->tok.kind != CPPT_SPACE) { va_nonempty = true; break; }
                    }
                }
                if (va_nonempty) {
                    /* Expand body and append to result */
                    TokList exp_body = {0};
                    expand_list(&body, mt, interns, diags, &exp_body);
                    for (TokNode *en = exp_body.head; en; en = en->next)
                        tl_append_copy(&result, &en->tok);
                    tl_free(&exp_body);
                }
                tl_free(&body);
                /* Consume the rest of the body token list from nx onwards.
                 * We can't walk back in a singly-linked list, so we append
                 * all remaining nodes from nx directly and break out.      */
                /* Advance n to nx so the while loop continues
                 * processing remaining body tokens (e.g. __VA_ARGS__)
                 * through normal parameter substitution.              */
                n = nx;
                continue;
            }
        }

        /* --- Regular token: copy through --- */
        PPTok copy = *t;
        copy.spell = (StrBuf){0};
        sb_push_cstr(&copy.spell, sp);
        tl_append(&result, copy);
        n = n->next;
    }

    return result;
}

/* =========================================================================
 * Expansion depth / token limits to prevent runaway macro expansion
 * (e.g. P99 library with deeply nested recursive macros)
 *
 * These limits are intentionally conservative:
 *   - Normal C code (even complex headers) rarely exceeds 2000 expansions
 *   - P99 metaprogramming can use 10K+ expansions but is not "normal C"
 *   - The goal is to prevent memory explosion, not to fully support P99
 * ====================================================================== */
/* Phase 2: lowered from 1,000,000 to 200.  The C call-stack budget is the
 * binding constraint here: each recursive expand_list frame is ~150 bytes,
 * so even with an 8 MB stack we exhaust before reaching 1 M frames.  GCC
 * uses ~200, Clang uses ~160; 200 is a safe, conventional choice.        */
#define MAX_EXPAND_DEPTH       200
#define MAX_EXPAND_TOKENS  5000000 /* max tokens produced per file             */

static void expand_limits_reset(MacroTable *mt) {
    mt->expand_depth    = 0;
    mt->expand_tokens   = 0;
    mt->limits_breached = false;
    /* Phase 2: ensure the active-expansion stack is empty at the start
     * of each top-level macro_expand call.  In normal balanced operation
     * push/pop pairs leave it empty; this is a defensive reset in case
     * an earlier call returned via a non-balanced path (e.g. limits
     * breach in the middle of a recursive expansion).                  */
    mt->active_expansions.len = 0;
}

static bool expand_limits_check_depth(MacroTable *mt, CppDiagArr *diags,
                                       const char *name, CppLoc loc) {
    /* Phase 2: use the active-expansions stack length as the actual
     * recursion depth.  The stack is push/pop-balanced inside expand_list,
     * so its size precisely tracks the depth of the C call chain we're
     * about to enter.  The previous design only incremented and never
     * decremented expand_depth, which made the check fire only after
     * cumulatively many expansions — useless against unbounded recursion.
     *
     * The +1 anticipates the push that's about to happen for `name`.    */
    if (mt->active_expansions.len + 1 > MAX_EXPAND_DEPTH) {
        mt->limits_breached = true;
        if (diags) {
            CppDiag d = {0};
            d.level = CPP_DIAG_WARNING;
            d.loc = loc;
            char buf[512];
            int n = snprintf(buf, sizeof(buf),
                "macro expansion depth limit reached (%d) — %s passed through unexpanded",
                MAX_EXPAND_DEPTH, name ? name : "(macro)");
            if (n > 0 && (size_t)n < sizeof(buf))
                d.msg = (char *)cpp_xmalloc(n + 1);
            else {
                d.msg = (char *)cpp_xmalloc(80);
                snprintf(d.msg, 80, "macro expansion depth limit reached");
            }
            if (d.msg) { strcpy(d.msg, buf); d.msg[n] = '\0'; }
            diag_push(diags, d);
        }
        return false;
    }
    return true;
}

static bool expand_limits_check_tokens(MacroTable *mt, CppDiagArr *diags, CppLoc loc) {
    if (mt->expand_tokens > MAX_EXPAND_TOKENS) {
        mt->limits_breached = true;
        if (diags) {
            CppDiag d = {0};
            d.level = CPP_DIAG_WARNING;
            d.loc = loc;
            char buf[256];
            int n = snprintf(buf, sizeof(buf),
                "macro expansion token limit reached (%d) — truncating output",
                MAX_EXPAND_TOKENS);
            if (n > 0 && (size_t)n < sizeof(buf))
                d.msg = (char *)cpp_xmalloc(n + 1);
            else
                d.msg = (char *)cpp_xmalloc(60);
            if (d.msg) { strcpy(d.msg, buf); d.msg[n] = '\0'; }
            diag_push(diags, d);
        }
        return false;
    }
    return true;
}

static void expand_count_token(MacroTable *mt) {
    mt->expand_tokens++;
}

/* Expand a token list recursively. */
static void expand_list(TokList *input, MacroTable *mt,
                        InternTable *interns, CppDiagArr *diags,
                        TokList *output) {
    for (TokNode *n = input->head; n; n = n->next) {
        const PPTok *t = &n->tok;

        /* Non-identifier tokens pass through unchanged. */
        if (t->kind != CPPT_IDENT) {
            PPTok copy = *t;
            copy.spell = (StrBuf){0};
            sb_push_cstr(&copy.spell, pptok_spell(t));
            tl_append(output, copy);
            continue;
        }

        const char *name = intern_cstr(interns, pptok_spell(t));

        /* Limits breached? Skip ALL further expansion, pass through as-is. */
        if (mt->limits_breached) {
            PPTok copy = *t;
            copy.spell = (StrBuf){0};
            sb_push_cstr(&copy.spell, pptok_spell(t));
            tl_append(output, copy);
            continue;
        }

        /* Blue-painted? Don't expand. */
        if (t->hide) {
            PPTok copy = *t;
            copy.spell = (StrBuf){0};
            sb_push_cstr(&copy.spell, pptok_spell(t));
            tl_append(output, copy);
            continue;
        }

        /* Phase 2: name currently in the active-expansion stack? Pass
         * through as-is and mark the token as hidden so that callers
         * higher up the rescan chain also skip it.  This is what stops
         * mutual recursion (#define A B / #define B A): when expanding
         * A's body we push 'A', recurse with body 'B', expand B which
         * pushes 'B' and rescans body 'A' — at this point A is in the
         * active set and we passthrough rather than re-recursing.       */
        if (active_contains(mt, name)) {
            PPTok copy = *t;
            copy.spell = (StrBuf){0};
            sb_push_cstr(&copy.spell, pptok_spell(t));
            copy.hide = true;  /* persist past this rescan window */
            tl_append(output, copy);
            continue;
        }

        MacroDef *def = macro_lookup(mt, name);
        if (!def) {
            /* Not a macro — pass through.
             * (Note: `_Pragma(...)` operator is handled in directive.c
             * process_buf before tokens reach macro_expand here.)      */
            PPTok copy = *t;
            copy.spell = (StrBuf){0};
            sb_push_cstr(&copy.spell, pptok_spell(t));
            tl_append(output, copy);
            continue;
        }

        if (def->is_func) {
            /* Scan ahead for '(' — skip whitespace. */
            TokNode *nx = n->next;
            while (nx && (nx->tok.kind == CPPT_SPACE ||
                          nx->tok.kind == CPPT_NEWLINE)) nx = nx->next;

            if (!nx || nx->tok.kind != CPPT_PUNCT ||
                strcmp(pptok_spell(&nx->tok), "(") != 0) {
                /* No argument list — pass through as-is. */
                PPTok copy = *t;
                copy.spell = (StrBuf){0};
                sb_push_cstr(&copy.spell, pptok_spell(t));
                tl_append(output, copy);
                continue;
            }
            nx = nx->next; /* skip '(' */

            /* Collect arguments */
            int max_args = def->nparams + (def->is_variadic ? 1 : 0) + 1;
            TokList *args = cpp_xmalloc(max_args * sizeof *args);
            memset(args, 0, max_args * sizeof *args);
            int nargs = 0;

            /* Handle zero-argument call: FOO() */
            TokNode *tmp = nx;
            while (tmp && tmp->tok.kind == CPPT_SPACE) tmp = tmp->next;
            bool empty_args = (tmp && tmp->tok.kind == CPPT_PUNCT &&
                               strcmp(pptok_spell(&tmp->tok), ")") == 0);

            if (!empty_args) {
                int named = def->nparams;
                bool more_args = true;
                /* Collect named parameters */
                while (more_args && nargs < named) {
                    bool ended_with_comma = false;
                    args[nargs++] = collect_arg(&nx, &ended_with_comma);
                    if (!nx) { more_args = false; break; }
                    if (!ended_with_comma) {
                        /* collect_arg returned because it hit `)` —
                         * this was the last arg.                     */
                        more_args = false;
                    }
                }
                /* If variadic, collect everything remaining as the VA arg */
                if (def->is_variadic && nargs == named && nx &&
                    !(nx->tok.kind == CPPT_PUNCT &&
                      strcmp(pptok_spell(&nx->tok), ")") == 0)) {
                    /* Collect all remaining tokens up to ')' as VA args.
                     * Phase R5: track paren depth so nested calls like
                     * VA_FUNC(INNER()) collect INNER's `(` and `)` as
                     * part of the variadic arg rather than letting the
                     * inner `)` terminate the outer call.            */
                    TokList va_arg = {0};
                    int depth = 0;
                    while (nx) {
                        if (nx->tok.kind == CPPT_PUNCT) {
                            const char *vs = pptok_spell(&nx->tok);
                            if (strcmp(vs, "(") == 0) depth++;
                            else if (strcmp(vs, ")") == 0) {
                                if (depth == 0) break;
                                depth--;
                            }
                        }
                        PPTok copy = nx->tok;
                        copy.spell = (StrBuf){0};
                        sb_push_cstr(&copy.spell, pptok_spell(&nx->tok));
                        tl_append(&va_arg, copy);
                        nx = nx->next;
                    }
                    args[nargs++] = va_arg;
                }
            } else if (def->nparams == 1) {
                /* Phase R4: `FOO()` for a 1-param macro is widely
                 * accepted as 1 empty argument (GCC, Clang, mcpp).
                 * Treat it as such instead of 0 args.  This is what
                 * mcpp's n_nularg.c expects via SHOWS(EMPTY) → ARG()
                 * (after EMPTY pre-expands to nothing).               */
                args[0] = (TokList){0};
                nargs = 1;
                empty_args = false;
            }

            /* Phase R4: argument-count diagnostics per C99 §6.10.3/4.
             * Without this, sharp-cpp silently truncated extra args
             * and silently substituted nothing for missing ones.
             *
             * Only report when the macro call is complete in this
             * input list — i.e. we see a closing ')'.  Cascaded macro
             * expansions like
             *   #define head  sub(
             *   head a, b );
             * leave sub's args in the *source* stream, not in head's
             * input list, and the directive layer (process_buf) is
             * what completes the call.  In that case nx is NULL here
             * and we should NOT report "too few".                     */
            bool input_ends_at_paren = false;
            {
                TokNode *peek = nx;
                while (peek && peek->tok.kind == CPPT_SPACE) peek = peek->next;
                if (peek && peek->tok.kind == CPPT_PUNCT &&
                    strcmp(pptok_spell(&peek->tok), ")") == 0)
                    input_ends_at_paren = true;
            }
            int got_args = nargs;
            int need = def->nparams;
            /* Phase R4 (n_nularg): C99 §6.10.3.1 + GCC/Clang convention
             * treats `f()` for a 1-parameter macro as supplying ONE
             * empty argument, not zero arguments.  Promote `empty_args`
             * to a single empty-arg list when need == 1 so that the
             * substitution produces an empty stringification rather
             * than raising "too few".  For 0-parameter macros `f()`
             * remains 0 args (correct).                                */
            if (empty_args && need == 1 && nargs == 0) {
                args[0]   = (TokList){0};
                nargs     = 1;
                got_args  = 1;
                empty_args = false;
            }
            bool too_few = input_ends_at_paren && (got_args < need) &&
                           !(empty_args && need == 0);
            bool too_many = false;
            if (!input_ends_at_paren && nx && !def->is_variadic) {
                /* nx points to a non-')' token still inside parens —
                 * extras follow.  Only report for non-variadic; variadic
                 * absorbs them.
                 *
                 * Phase R4: we DON'T report unterminated here (nx==NULL
                 * case).  nx==NULL inside expand_list can mean either
                 * (a) genuine unterminated arg list at top level, or
                 * (b) a cascaded macro expansion where the args live in
                 * the source stream after the current input list.  We
                 * can't distinguish these from inside expand_list, and
                 * the cascaded case (e.g. `#define head sub(` followed
                 * by `head a, b );`) is a legitimate C99 pattern.
                 * Top-level unterminated arg lists are caught by the
                 * directive layer (process_buf) which can see the EOF.  */
                too_many = true;
            }
            if (too_few) {
                char msg[160];
                snprintf(msg, sizeof msg,
                    "macro '%s' requires %d argument%s, but %d %s given",
                    name, need, need == 1 ? "" : "s",
                    got_args, got_args == 1 ? "was" : "were");
                CppDiag d = { CPP_DIAG_ERROR, t->loc, cpp_xstrdup(msg) };
                da_push(diags, d);
            } else if (too_many) {
                char msg[160];
                snprintf(msg, sizeof msg,
                    "macro '%s' passed too many arguments (takes just %d)",
                    name, need);
                CppDiag d = { CPP_DIAG_ERROR, t->loc, cpp_xstrdup(msg) };
                da_push(diags, d);
            }

            /* Skip to ')' */
            while (nx && !(nx->tok.kind == CPPT_PUNCT &&
                           strcmp(pptok_spell(&nx->tok), ")") == 0))
                nx = nx->next;
            if (nx) n = nx; /* advance past ')' */

            /* Substitute and expand */
            TokList subst = substitute(def, args, nargs, NULL, mt, interns, diags);
            /* Blue-paint the macro name in the substituted list. */
            for (TokNode *sn = subst.head; sn; sn = sn->next) {
                if (sn->tok.kind == CPPT_IDENT &&
                    strcmp(pptok_spell(&sn->tok), name) == 0)
                    sn->tok.hide = true;
            }

            /* Phase C2: Check expansion depth limit before recursing */
            CppLoc expand_loc = def->def_loc;
            if (!expand_limits_check_depth(mt, diags, name, expand_loc)) {
                /* Depth limit reached — pass macro name through unexpanded */
                tl_free(&subst);
                PPTok passthrough = *t;
                passthrough.spell = (StrBuf){0};
                sb_push_cstr(&passthrough.spell, name);
                tl_append(output, passthrough);
                for (int i = 0; i < nargs; i++) tl_free(&args[i]);
                free(args);
                continue;
            }

            /* Blue-paint: mark the macro's own name as hidden in the
             * substitution list to prevent recursive re-expansion during
             * rescan (C99 6.10.3.4p2). */
            for (TokNode *sn = subst.head; sn; sn = sn->next) {
                if (sn->tok.kind == CPPT_IDENT &&
                    strcmp(pptok_spell(&sn->tok), name) == 0)
                    sn->tok.hide = true;
            }

            TokList expanded = {0};
            /* Phase 2: push `name` onto the active-expansion stack so any
             * occurrence of this macro inside the recursive rescan is
             * passed through, not re-expanded.  Together with the
             * symmetrical handling for object-like macros below, this is
             * what makes mutual recursion (#define A B / #define B A)
             * terminate cleanly instead of infinite-looping.            */
            active_push(mt, name);
            expand_list(&subst, mt, interns, diags, &expanded);
            active_pop(mt, name);
            tl_free(&subst);

            /* Phase C2: Enforce token limit for expanded output */
            for (TokNode *en = expanded.head; en; en = en->next) {
                if (!expand_limits_check_tokens(mt, diags, expand_loc)) break;
                expand_count_token(mt);
                PPTok copy = en->tok;
                copy.spell = (StrBuf){0};
                sb_push_cstr(&copy.spell, pptok_spell(&en->tok));
                tl_append(output, copy);
            }
            tl_free(&expanded);

            for (int i = 0; i < nargs; i++) tl_free(&args[i]);
            free(args);
        } else {
            /* Object-like macro
             *
             * Phase R4: route through substitute() so the `##` paste
             * operator in the body is honoured.  Previously we copied
             * the body verbatim, which silently let `/##/` survive into
             * the output as `/##/` instead of either pasting (and
             * raising the "invalid pp-token" diagnostic) or producing
             * the legitimate paste result.  mcpp's u_concat.c uses
             *   #define _VARIANT_BOOL  /##/
             * to exercise this path.
             *
             * substitute() itself doesn't reference args/nargs unless
             * def->is_func is true, so passing NULL/0 is safe. */
            TokList body_copy = substitute(def, NULL, 0, NULL, mt, interns, diags);
            /* Blue-paint: mark ONLY the macro's own name as hidden
             * to prevent direct self-recursion. Do NOT hide other
             * macros that happen to appear in the body. */
            for (TokNode *bc = body_copy.head; bc; bc = bc->next) {
                if (bc->tok.kind == CPPT_IDENT &&
                    strcmp(pptok_spell(&bc->tok), name) == 0)
                    bc->tok.hide = true;
            }

            /* Phase C2: Check expansion depth limit before recursing */
            CppLoc expand_loc = def->def_loc;
            if (!expand_limits_check_depth(mt, diags, name, expand_loc)) {
                /* Depth limit reached — pass macro name through unexpanded */
                tl_free(&body_copy);
                PPTok passthrough = *t;
                passthrough.spell = (StrBuf){0};
                sb_push_cstr(&passthrough.spell, name);
                tl_append(output, passthrough);
                continue;
            }

            /* Expand the body first.  Push `name` onto the active stack so
             * that any recursive use of `name` (or a mutual peer) inside
             * its own expansion is short-circuited.  The pop must happen
             * AFTER any combined-rescan below, since that rescan is also
             * "expanding" this macro from the language standard's view.   */
            active_push(mt, name);
            TokList expanded = {0};
            expand_list(&body_copy, mt, interns, diags, &expanded);
            tl_free(&body_copy);

            /* Determine whether the expansion ends with a function-like macro
             * name that would consume the immediately-following '(' from the
             * remaining input (e.g. #define A B / #define B(x) ... / A(foo)).
             * Only in that case do we need to append the remaining input tokens
             * and rescan everything together.  For all other cases (e.g.
             * #define X "" — the expansion is a string literal, not a macro
             * call) we rescan only the expansion and let the outer loop
             * continue naturally with the remaining input tokens.           */
            TokNode *last_non_ws = NULL;
            for (TokNode *en = expanded.head; en; en = en->next)
                if (en->tok.kind != CPPT_SPACE && en->tok.kind != CPPT_NEWLINE)
                    last_non_ws = en;

            TokNode *next_non_ws = n->next;
            while (next_non_ws &&
                   (next_non_ws->tok.kind == CPPT_SPACE ||
                    next_non_ws->tok.kind == CPPT_NEWLINE))
                next_non_ws = next_non_ws->next;

            bool needs_combined_rescan =
                last_non_ws &&
                last_non_ws->tok.kind == CPPT_IDENT &&
                !last_non_ws->tok.hide &&
                /* Phase R18: use macro_def_is_func to avoid second hash lookup */
                macro_def_is_func(macro_lookup(mt, pptok_spell(&last_non_ws->tok))) &&
                next_non_ws &&
                next_non_ws->tok.kind == CPPT_PUNCT &&
                strcmp(pptok_spell(&next_non_ws->tok), "(") == 0;

            /* Phase R4: also need combined rescan when expansion contains
             * an unbalanced '(' — meaning a function-like macro call was
             * started but not finished within the expansion itself.  This
             * is the n_27.c case:
             *     #define sub( x, y)  (x - y)
             *     #define head        sub(
             *     head a, b );
             * `head` expands to `sub(`, which has `(` open but no `)`.
             * Without combined rescan, sub's args wouldn't be visible
             * (they're in the source after `head`) and we'd report
             * "unterminated argument list" wrongly.                       */
            if (!needs_combined_rescan) {
                int depth = 0;
                for (TokNode *en = expanded.head; en; en = en->next) {
                    if (en->tok.kind == CPPT_PUNCT) {
                        const char *spx = pptok_spell(&en->tok);
                        if (strcmp(spx, "(") == 0) depth++;
                        else if (strcmp(spx, ")") == 0 && depth > 0) depth--;
                    }
                }
                if (depth > 0) needs_combined_rescan = true;
            }

            if (needs_combined_rescan) {
                /* Append remaining input so the function-like macro at the end
                 * of the expansion can consume its argument list. */
                TokNode *rest = n->next;
                while (rest) {
                    PPTok rc = rest->tok;
                    rc.spell = (StrBuf){0};
                    sb_push_cstr(&rc.spell, pptok_spell(&rest->tok));
                    tl_append(&expanded, rc);
                    rest = rest->next;
                }
                TokList rescanned = {0};
                expand_list(&expanded, mt, interns, diags, &rescanned);
                tl_free(&expanded);
                for (TokNode *en = rescanned.head; en; en = en->next) {
                    if (!expand_limits_check_tokens(mt, diags, expand_loc)) break;
                    expand_count_token(mt);
                    PPTok copy = en->tok;
                    copy.spell = (StrBuf){0};
                    sb_push_cstr(&copy.spell, pptok_spell(&en->tok));
                    tl_append(output, copy);
                }
                tl_free(&rescanned);
                active_pop(mt, name);   /* paired with active_push above */
                break; /* all remaining input was consumed and rescanned */
            } else {
                /* Expansion result does not begin a new function-like macro
                 * call — `expanded` is already the fully-rescanned terminal
                 * form (expand_list is recursive). A second expand_list pass
                 * here is redundant: in well-formed cases it's a no-op, and
                 * in the presence of any hide-set propagation gap it can
                 * incorrectly re-expand tokens (or double-count token
                 * budget). Just transfer `expanded` directly to `output`.   */
                for (TokNode *en = expanded.head; en; en = en->next) {
                    if (!expand_limits_check_tokens(mt, diags, expand_loc)) break;
                    expand_count_token(mt);
                    PPTok copy = en->tok;
                    copy.spell = (StrBuf){0};
                    sb_push_cstr(&copy.spell, pptok_spell(&en->tok));
                    tl_append(output, copy);
                }
                tl_free(&expanded);
                active_pop(mt, name);   /* paired with active_push above */
                /* continue — outer for-loop advances to n->next */
            }
        }
    }
}

/* Public entry point */
void macro_expand(TokList *input, MacroTable *mt,
                  InternTable *interns, CppDiagArr *diags,
                  TokList *output) {
    /* Reset per-call limits so each top-level expansion starts fresh.
     * Limits are cumulative within a single macro_expand() call
     * because expand_list() is recursive and checks mt->limits_breached.
     * BUG FIX: previously `if (!mt->limits_breached)` guarded the reset,
     * meaning once limits were breached in ANY call, ALL future calls
     * would skip expansion entirely.  Always reset to restore per-call
     * semantics. */
    expand_limits_reset(mt);
    expand_list(input, mt, interns, diags, output);
}

bool macro_limits_breached(MacroTable *mt) {
    return mt->limits_breached;
}

/* =========================================================================
 * Convenience: define an object-like macro from a raw body string.
 * Used by directive.c to install built-in macros without touching MacroDef.
 * ====================================================================== */
void macro_define_object(MacroTable *t, InternTable *interns,
                         const char *name, const char *body_str,
                         CppLoc loc) {
    /* Tokenise body_str through a temporary in-memory reader. */
    /* We build a fake DA for diagnostics (errors in built-in bodies are
     * programming errors — abort rather than report gracefully). */
    CppDiagArr dummy = {0};

    /* Parse: name_str SP body_str => looks like a #define line */
    size_t nlen  = strlen(name);
    size_t blen  = body_str ? strlen(body_str) : 0;
    size_t total = nlen + 1 + blen + 1;
    char  *buf   = (char *)cpp_xmalloc(total);
    memcpy(buf, name, nlen);
    buf[nlen] = ' ';
    if (body_str && blen) memcpy(buf + nlen + 1, body_str, blen);
    buf[total - 1] = '\0';

    CppReader *rd = reader_new_from_buf(buf, total - 1, "<builtin>",
                                        interns, &dummy);
    TokList line  = {0};
    for (;;) {
        PPTok tk = reader_next_tok(rd, false);
        if (tk.kind == CPPT_EOF || tk.kind == CPPT_NEWLINE)
            { pptok_free(&tk); break; }
        tl_append(&line, tk);
    }
    reader_free(rd);
    free(buf);

    MacroDef *def = macro_parse_define(&line, interns, &dummy);
    tl_free(&line);
    if (def) { def->def_loc = loc; macro_define(t, def, &dummy, loc); }

    free(dummy.data);
}

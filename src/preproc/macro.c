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

struct MacroTable {
    MacroDef    *buckets[MACRO_BUCKETS];
    InternTable *interns;
    /* Per-run expansion limits (replaces unsafe file-static globals) */
    int          expand_depth;
    int          expand_tokens;
    bool         limits_breached;
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
    /* Compare body token by token */
    TokNode *an = a->body.head, *bn = b->body.head;
    while (an && bn) {
        if (an->tok.kind != bn->tok.kind) return false;
        if (strcmp(pptok_spell(&an->tok), pptok_spell(&bn->tok)) != 0) return false;
        an = an->next; bn = bn->next;
    }
    return !an && !bn;
}

void macro_define(MacroTable *t, MacroDef *def, CppDiagArr *diags, CppLoc loc) {
    def->def_loc = loc;
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

    return def;
}

/* =========================================================================
 * Hide-set: a simple sorted array of interned macro names.
 * A token is "blue-painted" with respect to a macro name if that name
 * appears in its hide-set (§6.10.3.4).
 * ====================================================================== */

typedef struct {
    const char **names;
    int          len;
    int          cap;
} HideSet;

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

/* =========================================================================
 * Stringification (#param)
 * ====================================================================== */

static PPTok stringify(const TokList *arg, CppLoc loc) {
    StrBuf sb = {0};
    sb_push_ch(&sb, '"');
    bool first = true;
    for (TokNode *n = arg->head; n; n = n->next) {
        if (n->tok.kind == CPPT_SPACE) {
            if (!first) sb_push_ch(&sb, ' ');
            continue;
        }
        first = false;
        const char *sp = pptok_spell(&n->tok);
        for (const char *p = sp; *p; p++) {
            if (*p == '"' || *p == '\\') sb_push_ch(&sb, '\\');
            sb_push_ch(&sb, *p);
        }
    }
    sb_push_ch(&sb, '"');

    PPTok t = {0};
    t.kind  = CPPT_STRING_LIT;
    t.spell = sb;
    t.loc   = loc;
    return t;
}

/* =========================================================================
 * Token pasting (##)
 * ====================================================================== */

static PPTok token_paste(const PPTok *lhs, const PPTok *rhs,
                         CppReader *scratch_rd,
                         InternTable *interns,
                         CppDiagArr *diags) {
    /* Concatenate the spellings and retokenize. */
    StrBuf sb = {0};
    sb_push_cstr(&sb, pptok_spell(lhs));
    sb_push_cstr(&sb, pptok_spell(rhs));

    CppReader *rd = reader_new_from_buf(sb.buf, sb.len,
                                        lhs->loc.file ? lhs->loc.file : "<paste>",
                                        interns, diags);
    PPTok result = reader_next_tok(rd, false);
    /* Verify it was a single token. */
    PPTok check  = reader_next_tok(rd, false);
    bool valid = (check.kind == CPPT_EOF || check.kind == CPPT_NEWLINE ||
                  check.kind == CPPT_SPACE);
    if (!valid) {
        /* C99 6.10.3.3p3: invalid paste → undefined behavior.
         * Match clang: return lhs token unchanged (paste fails silently),
         * allowing rescanning to expand nested macros. */
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
static TokList collect_arg(TokNode **cur_p) {
    TokList arg = {0};
    int depth = 0;
    for (TokNode *n = *cur_p; n; n = n->next) {
        if (n->tok.kind == CPPT_PUNCT) {
            const char *sp = pptok_spell(&n->tok);
            if (strcmp(sp, "(") == 0) depth++;
            else if (strcmp(sp, ")") == 0) {
                if (depth == 0) { *cur_p = n; return arg; }
                depth--;
            } else if (strcmp(sp, ",") == 0 && depth == 0) {
                *cur_p = n->next; /* skip ',' */
                return arg;
            }
        }
        /* Copy token into arg */
        PPTok copy = n->tok;
        copy.spell = (StrBuf){0};
        sb_push_cstr(&copy.spell, pptok_spell(&n->tok));
        tl_append(&arg, copy);
    }
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
                    /* C99 6.10.3.4: the first token of the expanded argument
                     * inherits the leading-space flag from the parameter
                     * reference in the macro body. This ensures that when a
                     * macro body like "extern x" (where x is a parameter) is
                     * expanded, the space before x is preserved as a leading
                     * space on the first substituted token.              */
                    bool first_arg = true;
                    for (TokNode *en = expanded.head; en; en = en->next) {
                        PPTok copy = en->tok;
                        copy.spell = (StrBuf){0};
                        sb_push_cstr(&copy.spell, pptok_spell(&en->tok));
                        if (first_arg) {
                            copy.has_leading_space = copy.has_leading_space || t->has_leading_space;
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
            /* Remove trailing whitespace from result (C99 6.10.3.3p2) */
            TokNode *ws_before_paste = NULL;
            if (result.tail && result.tail->tok.kind == CPPT_SPACE) {
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
                } else if (param_idx >= 0 && param_idx < nargs && args[param_idx].head) {
                    /* RHS is a parameter with tokens.
                     * Paste lhs with the FIRST token of the argument,
                     * then append remaining argument tokens to result. */
                    PPTok first = args[param_idx].head->tok;
                    PPTok rhs_first = first;
                    rhs_first.spell = (StrBuf){0};
                    sb_push_cstr(&rhs_first.spell, pptok_spell(&first));

                    PPTok pasted = token_paste(&lhs, &rhs_first, NULL, interns, diags);
                    pptok_free(&result.tail->tok);
                    result.tail->tok = pasted;
                    pptok_free(&rhs_first);

                    /* Append remaining argument tokens (skip the first one) */
                    for (TokNode *an = args[param_idx].head->next; an; an = an->next) {
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

                    PPTok pasted = token_paste(&lhs, &rhs, NULL, interns, diags);
                    pptok_free(&result.tail->tok);
                    result.tail->tok = pasted;
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
#define MAX_EXPAND_DEPTH   1000000 /* max recursive macro expansion calls      */
#define MAX_EXPAND_TOKENS  5000000 /* max tokens produced per file             */

static void expand_limits_reset(MacroTable *mt) {
    mt->expand_depth    = 0;
    mt->expand_tokens   = 0;
    mt->limits_breached = false;
}

static bool expand_limits_check_depth(MacroTable *mt, CppDiagArr *diags,
                                       const char *name, CppLoc loc) {
    if (++mt->expand_depth > MAX_EXPAND_DEPTH) {
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

        MacroDef *def = macro_lookup(mt, name);
        if (!def) {
            /* Not a macro — pass through. */
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
                /* Collect named parameters */
                while (nargs < named) {
                    args[nargs++] = collect_arg(&nx);
                    if (!nx) break;
                    if (nx->tok.kind == CPPT_PUNCT &&
                        strcmp(pptok_spell(&nx->tok), ")") == 0) break;
                }
                /* If variadic, collect everything remaining as the VA arg */
                if (def->is_variadic && nargs == named && nx &&
                    !(nx->tok.kind == CPPT_PUNCT &&
                      strcmp(pptok_spell(&nx->tok), ")") == 0)) {
                    /* Collect all remaining tokens up to ')' as VA args */
                    TokList va_arg = {0};
                    while (nx && !(nx->tok.kind == CPPT_PUNCT &&
                                   strcmp(pptok_spell(&nx->tok), ")") == 0)) {
                        PPTok copy = nx->tok;
                        copy.spell = (StrBuf){0};
                        sb_push_cstr(&copy.spell, pptok_spell(&nx->tok));
                        tl_append(&va_arg, copy);
                        nx = nx->next;
                    }
                    args[nargs++] = va_arg;
                }
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
            expand_list(&subst, mt, interns, diags, &expanded);
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
            /* Object-like macro */
            TokList body_copy = {0};
            for (TokNode *bn = def->body.head; bn; bn = bn->next) {
                PPTok copy = bn->tok;
                copy.spell = (StrBuf){0};
                sb_push_cstr(&copy.spell, pptok_spell(&bn->tok));
                tl_append(&body_copy, copy);
            }
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

            /* Expand the body first */
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
                macro_lookup_is_func(mt, pptok_spell(&last_non_ws->tok)) &&
                next_non_ws &&
                next_non_ws->tok.kind == CPPT_PUNCT &&
                strcmp(pptok_spell(&next_non_ws->tok), "(") == 0;

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
                break; /* all remaining input was consumed and rescanned */
            } else {
                /* Expansion result does not begin a new function-like macro
                 * call — rescan the expansion alone and emit it, then let the
                 * outer loop advance to n->next naturally.                   */
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

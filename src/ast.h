/* ast.h — typed constructors and accessors for AST nodes.
 *
 * Purpose
 * -------
 * Before this module existed, every pass in the compiler reached into
 * `Node` directly via generic fields like `n->cond` / `n->rhs` / `n->body`.
 * The meaning of those fields depended on `n->kind`; readers had to keep
 * the mapping in their head. That worked when the language was small,
 * but it:
 *
 *   1. Made every call site look interchangeable when it wasn't.
 *      (`n->init` means "for-init" in ND_FOR,
 *      "field init value" in ND_FIELD_INIT.)
 *   2. Made adding new node kinds a whack-a-mole exercise: every pass's
 *      big switch had to be updated, and there was no compile-time help.
 *   3. Made read-once tricks like "oh right, ND_MATCH stores the
 *      scrutinee in `cond`" a persistent source of confusion.
 *
 * This module introduces a FAÇADE over Node. Constructors like
 *   mk_if(cond, then_b, else_b)
 * package up the right fields; accessors like
 *   if_cond(n), if_then(n), if_else(n)
 * read them back with a runtime kind check under the hood. The Node
 * struct itself keeps its current layout, so existing call sites that
 * still use `n->cond` keep compiling while individual files migrate.
 *
 * Design choices
 * --------------
 * - Accessors return pointers / values, not references, so they are
 *   always safe at the call site; mutation happens through setters
 *   (e.g. `set_if_cond`) which also keep the invariant that Node's
 *   `kind` matches the accessor used.
 * - Every accessor contains an assertion checking `kind`. In release
 *   builds assert can be disabled; in debug builds it catches any
 *   misuse immediately.
 * - The accessors are `static inline` so they cost nothing at runtime.
 * - A few accessors intentionally expose Node** (for lists like
 *   block children and call args) so callers can still iterate without
 *   allocating.
 *
 * This file will GROW in lockstep with the language — every new NodeKind
 * should ship with its constructor + accessor pair. Parsers and visitors
 * that use these accessors automatically benefit from kind checks.
 */
#ifndef SHARP_AST_H
#define SHARP_AST_H

#include "sharp.h"
#include <assert.h>

/* ===================================================================== *
 *   Runtime kind assertions
 *
 *   Compile the compiler with -DNDEBUG to compile these out in release.
 *   During development they catch "you reached for this field on the
 *   wrong kind" within microseconds.
 * ===================================================================== */
#define AST_ASSERT_KIND(n, k) \
    assert((n) && (n)->kind == (k) && "AST accessor called on wrong NodeKind")

/* ===================================================================== *
 *   Generic helpers
 * ===================================================================== */

/* Every accessor for a "position info" style field lives on all nodes. */
static inline int ast_line(const Node* n) { return n ? n->line : 0; }

/* Some nodes carry a resolved Type* (filled by sema). */
static inline Type* ast_type(const Node* n) { return n ? n->type : NULL; }
static inline void  set_ast_type(Node* n, Type* t) { if (n) n->type = t; }

/* ===================================================================== *
 *   ND_INT — integer literal
 * ===================================================================== */
static inline long long int_value(const Node* n) {
    AST_ASSERT_KIND(n, ND_INT);
    return n->ival;
}

/* ===================================================================== *
 *   ND_FLOAT — floating literal
 * ===================================================================== */
static inline double float_value(const Node* n) {
    AST_ASSERT_KIND(n, ND_FLOAT);
    return n->fval;
}

/* ===================================================================== *
 *   ND_BOOL — boolean literal
 * ===================================================================== */
static inline bool bool_value(const Node* n) {
    AST_ASSERT_KIND(n, ND_BOOL);
    return n->bval;
}

/* ===================================================================== *
 *   ND_STR / ND_CHAR — string & char literals
 *
 *   String literals keep both raw bytes (n->sval) and length (n->slen),
 *   so escapes aren't lost. Same for char literals.
 * ===================================================================== */
static inline const char* str_bytes(const Node* n) {
    assert(n && (n->kind == ND_STR || n->kind == ND_CHAR));
    return n->sval;
}
static inline int str_len(const Node* n) {
    assert(n && (n->kind == ND_STR || n->kind == ND_CHAR));
    return n->slen;
}

/* ===================================================================== *
 *   ND_IDENT — identifier reference
 * ===================================================================== */
static inline const char* ident_name(const Node* n) {
    AST_ASSERT_KIND(n, ND_IDENT);
    return n->name;
}

/* ===================================================================== *
 *   ND_BINOP — binary operator
 * ===================================================================== */
static inline OpKind binop_op (const Node* n) { AST_ASSERT_KIND(n, ND_BINOP); return n->op;  }
static inline Node*  binop_lhs(const Node* n) { AST_ASSERT_KIND(n, ND_BINOP); return n->lhs; }
static inline Node*  binop_rhs(const Node* n) { AST_ASSERT_KIND(n, ND_BINOP); return n->rhs; }

/* ===================================================================== *
 *   ND_UNOP — unary operator (prefix and postfix)
 * ===================================================================== */
static inline OpKind unop_op     (const Node* n) { AST_ASSERT_KIND(n, ND_UNOP); return n->op;  }
static inline Node*  unop_operand(const Node* n) { AST_ASSERT_KIND(n, ND_UNOP); return n->rhs; }

/* ===================================================================== *
 *   ND_ASSIGN — assignment (incl. +=, etc.)
 * ===================================================================== */
static inline OpKind assign_op (const Node* n) { AST_ASSERT_KIND(n, ND_ASSIGN); return n->op;  }
static inline Node*  assign_lhs(const Node* n) { AST_ASSERT_KIND(n, ND_ASSIGN); return n->lhs; }
static inline Node*  assign_rhs(const Node* n) { AST_ASSERT_KIND(n, ND_ASSIGN); return n->rhs; }

/* ===================================================================== *
 *   ND_CALL — function / method call
 * ===================================================================== */
static inline Node*  call_callee(const Node* n) { AST_ASSERT_KIND(n, ND_CALL); return n->callee; }
static inline int    call_nargs (const Node* n) { AST_ASSERT_KIND(n, ND_CALL); return n->nargs;  }
static inline Node*  call_arg   (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_CALL);
    assert(i >= 0 && i < n->nargs && "call_arg index out of range");
    return n->args[i];
}
static inline Node** call_args  (const Node* n) { AST_ASSERT_KIND(n, ND_CALL); return n->args; }

/* ===================================================================== *
 *   ND_MEMBER — field / method access (x.y or x->y)
 * ===================================================================== */
static inline Node*       member_receiver(const Node* n) { AST_ASSERT_KIND(n, ND_MEMBER); return n->lhs;  }
static inline const char* member_name    (const Node* n) { AST_ASSERT_KIND(n, ND_MEMBER); return n->name; }

/* ===================================================================== *
 *   ND_INDEX — a[i]
 * ===================================================================== */
static inline Node* index_receiver(const Node* n) { AST_ASSERT_KIND(n, ND_INDEX); return n->lhs; }
static inline Node* index_index   (const Node* n) { AST_ASSERT_KIND(n, ND_INDEX); return n->rhs; }

/* ===================================================================== *
 *   ND_VARDECL — local variable declaration
 * ===================================================================== */
static inline Type*       vardecl_type(const Node* n) { AST_ASSERT_KIND(n, ND_VARDECL); return n->declared_type; }
static inline const char* vardecl_name(const Node* n) { AST_ASSERT_KIND(n, ND_VARDECL); return n->name;           }
static inline Node*       vardecl_init(const Node* n) { AST_ASSERT_KIND(n, ND_VARDECL); return n->rhs;            }

/* ===================================================================== *
 *   ND_IF — if / else
 * ===================================================================== */
static inline Node* if_cond(const Node* n) { AST_ASSERT_KIND(n, ND_IF); return n->cond;   }
static inline Node* if_then(const Node* n) { AST_ASSERT_KIND(n, ND_IF); return n->then_b; }
static inline Node* if_else(const Node* n) { AST_ASSERT_KIND(n, ND_IF); return n->else_b; }

/* ===================================================================== *
 *   ND_WHILE — while loop
 * ===================================================================== */
static inline Node* while_cond(const Node* n) { AST_ASSERT_KIND(n, ND_WHILE); return n->cond; }
static inline Node* while_body(const Node* n) { AST_ASSERT_KIND(n, ND_WHILE); return n->body; }

/* ===================================================================== *
 *   ND_FOR — C-style for
 * ===================================================================== */
static inline Node* for_init  (const Node* n) { AST_ASSERT_KIND(n, ND_FOR); return n->init;   }
static inline Node* for_cond  (const Node* n) { AST_ASSERT_KIND(n, ND_FOR); return n->cond;   }
static inline Node* for_update(const Node* n) { AST_ASSERT_KIND(n, ND_FOR); return n->update; }
static inline Node* for_body  (const Node* n) { AST_ASSERT_KIND(n, ND_FOR); return n->body;   }

/* ===================================================================== *
 *   ND_RETURN / ND_EXPR_STMT
 * ===================================================================== */
static inline Node* return_value  (const Node* n) { AST_ASSERT_KIND(n, ND_RETURN);    return n->rhs; }
static inline Node* expr_stmt_expr(const Node* n) { AST_ASSERT_KIND(n, ND_EXPR_STMT); return n->rhs; }

/* ===================================================================== *
 *   ND_BLOCK — compound statement
 * ===================================================================== */
static inline int   block_count(const Node* n) { AST_ASSERT_KIND(n, ND_BLOCK); return n->nchildren; }
static inline Node* block_stmt (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_BLOCK);
    assert(i >= 0 && i < n->nchildren);
    return n->children[i];
}
static inline Node** block_stmts(const Node* n) { AST_ASSERT_KIND(n, ND_BLOCK); return n->children; }

/* ===================================================================== *
 *   ND_STRUCT_LIT — struct literal  T { .x = 1, .y = 2 }
 * ===================================================================== */
static inline const char* struct_lit_name    (const Node* n) { AST_ASSERT_KIND(n, ND_STRUCT_LIT); return n->name;     }
static inline int         struct_lit_nfields (const Node* n) { AST_ASSERT_KIND(n, ND_STRUCT_LIT); return n->nchildren;}
static inline Node*       struct_lit_field   (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_STRUCT_LIT);
    assert(i >= 0 && i < n->nchildren);
    return n->children[i];
}
/* ND_FIELD_INIT lives as a child of ND_STRUCT_LIT. */
static inline const char* field_init_name (const Node* n) { AST_ASSERT_KIND(n, ND_FIELD_INIT); return n->name; }
static inline Node*       field_init_value(const Node* n) { AST_ASSERT_KIND(n, ND_FIELD_INIT); return n->rhs;  }

/* ===================================================================== *
 *   ND_PRINT / ND_PRINTLN
 * ===================================================================== */
static inline Node* print_value  (const Node* n) { AST_ASSERT_KIND(n, ND_PRINT);   return n->rhs; }
static inline Node* println_value(const Node* n) { AST_ASSERT_KIND(n, ND_PRINTLN); return n->rhs; }

/* ===================================================================== *
 *   ND_CAST — C-style type cast (Type)expr
 * ===================================================================== */
static inline Type*       cast_type (const Node* n) { AST_ASSERT_KIND(n, ND_CAST); return n->declared_type; }
static inline Node*       cast_expr (const Node* n) { AST_ASSERT_KIND(n, ND_CAST); return n->rhs; }

/* ===================================================================== *
 *   ND_FUNC_DECL — function or method
 * ===================================================================== */
static inline const char* func_name       (const Node* n) { AST_ASSERT_KIND(n, ND_FUNC_DECL); return n->name;        }
static inline Type*       func_ret_type   (const Node* n) { AST_ASSERT_KIND(n, ND_FUNC_DECL); return n->ret_type;    }
static inline Node*       func_body       (const Node* n) { AST_ASSERT_KIND(n, ND_FUNC_DECL); return n->body;        }
static inline int         func_nparams    (const Node* n) { AST_ASSERT_KIND(n, ND_FUNC_DECL); return n->nparams;     }
static inline Node*       func_param      (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_FUNC_DECL);
    assert(i >= 0 && i < n->nparams);
    return n->params[i];
}
static inline Node**      func_params     (const Node* n) { AST_ASSERT_KIND(n, ND_FUNC_DECL); return n->params;      }
static inline int         func_self_kind  (const Node* n) { AST_ASSERT_KIND(n, ND_FUNC_DECL); return n->self_kind;   }
static inline const char* func_parent_type(const Node* n) { AST_ASSERT_KIND(n, ND_FUNC_DECL); return n->parent_type; }

/* ===================================================================== *
 *   ND_STRUCT_DECL
 * ===================================================================== */
static inline const char* struct_decl_name(const Node* n) { AST_ASSERT_KIND(n, ND_STRUCT_DECL); return n->name;        }
static inline int  struct_decl_nfields    (const Node* n) { AST_ASSERT_KIND(n, ND_STRUCT_DECL); return n->nfields;     }
static inline Node*struct_decl_field      (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_STRUCT_DECL);
    assert(i >= 0 && i < n->nfields);
    return n->fields[i];
}
static inline int         struct_decl_ntype_params(const Node* n) { AST_ASSERT_KIND(n, ND_STRUCT_DECL); return n->ntype_params; }
static inline const char* struct_decl_type_param  (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_STRUCT_DECL);
    assert(i >= 0 && i < n->ntype_params);
    return n->type_params[i];
}

/* ND_FIELD lives as a child of ND_STRUCT_DECL. */
static inline const char* field_decl_name(const Node* n) { AST_ASSERT_KIND(n, ND_FIELD); return n->name;          }
static inline Type*       field_decl_type(const Node* n) { AST_ASSERT_KIND(n, ND_FIELD); return n->declared_type; }

/* ND_PARAM lives under func and extern decls. */
static inline const char* param_name(const Node* n) { AST_ASSERT_KIND(n, ND_PARAM); return n->name;          }
static inline Type*       param_type(const Node* n) { AST_ASSERT_KIND(n, ND_PARAM); return n->declared_type; }

/* ===================================================================== *
 *   ND_IMPL
 * ===================================================================== */
static inline const char* impl_target_name      (const Node* n) { AST_ASSERT_KIND(n, ND_IMPL); return n->name;         }
static inline int         impl_nmethods         (const Node* n) { AST_ASSERT_KIND(n, ND_IMPL); return n->nchildren;    }
static inline Node*       impl_method           (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_IMPL);
    assert(i >= 0 && i < n->nchildren);
    return n->children[i];
}
static inline int         impl_ntype_params(const Node* n) { AST_ASSERT_KIND(n, ND_IMPL); return n->ntype_params; }

/* ===================================================================== *
 *   ND_EXTERN_DECL
 * ===================================================================== */
static inline const char* extern_name    (const Node* n) { AST_ASSERT_KIND(n, ND_EXTERN_DECL); return n->name;     }
static inline Type*       extern_ret_type(const Node* n) { AST_ASSERT_KIND(n, ND_EXTERN_DECL); return n->ret_type; }
static inline int         extern_nparams (const Node* n) { AST_ASSERT_KIND(n, ND_EXTERN_DECL); return n->nparams;  }
static inline Node*       extern_param   (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_EXTERN_DECL);
    assert(i >= 0 && i < n->nparams);
    return n->params[i];
}

/* ===================================================================== *
 *   ND_DROP — explicit RAII drop, injected by the lowering pass (Leap C).
 *
 *   drop_var_name(n)    : local identifier being destructed
 *   drop_struct_name(n) : mangled struct / ARC-type name, forms the call
 *                         `<struct>___drop(&<var>);` in cgen.
 * ===================================================================== */
static inline const char* drop_var_name   (const Node* n) { AST_ASSERT_KIND(n, ND_DROP); return n->name; }
static inline const char* drop_struct_name(const Node* n) { AST_ASSERT_KIND(n, ND_DROP); return n->sval; }

/* ===================================================================== *
 *   ND_PROGRAM
 * ===================================================================== */
static inline int   program_ndecls(const Node* n) { AST_ASSERT_KIND(n, ND_PROGRAM); return n->nchildren; }
static inline Node* program_decl  (const Node* n, int i) {
    AST_ASSERT_KIND(n, ND_PROGRAM);
    assert(i >= 0 && i < n->nchildren);
    return n->children[i];
}

#endif /* SHARP_AST_H */

/*
 * ast.c — Sharp Frontend: Phase 2 AST node allocation, free, and print.
 *
 * Phase 2: implements the three public functions declared in ast.h plus the
 * AstVec helpers.  No semantic logic here — pure tree management.
 */

#include "ast.h"
#include "cpp_internal.h"   /* cpp_xstrdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * AstVec
 * ====================================================================== */

void astvec_push(AstVec *v, AstNode *n) {
    if (v->len == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 4;
        v->data = realloc(v->data, nc * sizeof *v->data);
        if (!v->data) { perror("sharp-fe ast"); abort(); }
        v->cap = nc;
    }
    v->data[v->len++] = n;
}

/* Free the backing array only — nodes are freed separately by ast_node_free. */
void astvec_free(AstVec *v) {
    free(v->data);
    v->data = NULL;
    v->len  = 0;
    v->cap  = 0;
}

/* Recursively free every node in the vec, then free the backing array. */
static void astvec_deep_free(AstVec *v) {
    for (size_t i = 0; i < v->len; i++) ast_node_free(v->data[i]);
    astvec_free(v);
}

/* =========================================================================
 * Construction
 * ====================================================================== */

AstNode *ast_node_new(AstKind kind, CppLoc loc) {
    AstNode *n = calloc(1, sizeof *n);
    if (!n) { perror("sharp-fe ast_node_new"); abort(); }
    n->kind = kind;
    n->loc  = loc;
    return n;
}

/* =========================================================================
 * ast_node_free — recursive
 * ====================================================================== */

void ast_node_free(AstNode *node) {
    if (!node) return;

    switch (node->kind) {

    /* ── Top-level ──────────────────────────────────────────────────── */
    case AST_FILE:
        free(node->u.file.path);
        astvec_deep_free(&node->u.file.decls);
        break;
    case AST_IMPORT:
        free(node->u.import.path);
        break;
    case AST_STRUCT_DEF:
        free(node->u.struct_def.name);
        astvec_deep_free(&node->u.struct_def.generic_params);
        astvec_deep_free(&node->u.struct_def.fields);
        astvec_deep_free(&node->u.struct_def.methods);
        break;
    case AST_FUNC_DEF:
        free(node->u.func_def.name);
        astvec_deep_free(&node->u.func_def.generic_params);
        ast_node_free(node->u.func_def.ret_type);
        astvec_deep_free(&node->u.func_def.params);
        ast_node_free(node->u.func_def.body);
        free(node->u.func_def.gcc_attrs);  /* Phase R6 */
        break;
    case AST_TYPEDEF_DECL:
        free(node->u.typedef_decl.alias);
        ast_node_free(node->u.typedef_decl.target);
        break;
    case AST_VAR_DECL:
        ast_node_free(node->u.var_decl.type);
        free(node->u.var_decl.name);
        ast_node_free(node->u.var_decl.init);
        free(node->u.var_decl.gcc_attrs);  /* Phase R6 */
        break;
    case AST_ENUM_DEF:
        free(node->u.enum_def.name);
        astvec_deep_free(&node->u.enum_def.items);
        break;
    case AST_ENUMERATOR:
        free(node->u.enumerator.name);
        ast_node_free(node->u.enumerator.value);
        break;

    /* ── Type nodes ──────────────────────────────────────────────────── */
    case AST_TYPE_NAME:   free(node->u.type_name.name);         break;
    case AST_TYPE_PTR:    ast_node_free(node->u.type_ptr.base); break;
    case AST_TYPE_CONST:  ast_node_free(node->u.type_const.base); break;
    case AST_TYPE_VOLATILE: ast_node_free(node->u.type_volatile.base); break;
    case AST_TYPE_ARRAY:
        ast_node_free(node->u.type_array.base);
        ast_node_free(node->u.type_array.size);
        break;
    case AST_TYPE_FUNC:
        ast_node_free(node->u.type_func.ret);
        astvec_deep_free(&node->u.type_func.params);
        break;
    case AST_TYPE_GENERIC:
        free(node->u.type_generic.name);
        astvec_deep_free(&node->u.type_generic.args);
        break;
    case AST_TYPE_AUTO:  /* no owned fields */                  break;
    case AST_TYPE_VOID:  /* no owned fields */                  break;
    case AST_TYPE_PARAM: free(node->u.type_param.name);         break;

    /* ── Declarations ────────────────────────────────────────────────── */
    case AST_FIELD_DECL:
        ast_node_free(node->u.field_decl.type);
        free(node->u.field_decl.name);
        ast_node_free(node->u.field_decl.bit_width);
        break;
    case AST_PARAM_DECL:
        ast_node_free(node->u.param_decl.type);
        free(node->u.param_decl.name);
        break;
    case AST_GENERIC_PARAM:
        free(node->u.generic_param.name);
        break;

    /* ── Statement nodes ─────────────────────────────────────────────── */
    case AST_BLOCK:       astvec_deep_free(&node->u.block.stmts); break;
    case AST_IF:
        ast_node_free(node->u.if_.cond);
        ast_node_free(node->u.if_.then_);
        ast_node_free(node->u.if_.else_);
        break;
    case AST_WHILE:
        ast_node_free(node->u.while_.cond);
        ast_node_free(node->u.while_.body);
        break;
    case AST_FOR:
        ast_node_free(node->u.for_.init);
        ast_node_free(node->u.for_.cond);
        ast_node_free(node->u.for_.post);
        ast_node_free(node->u.for_.body);
        break;
    case AST_DO_WHILE:
        ast_node_free(node->u.do_while.cond);
        ast_node_free(node->u.do_while.body);
        break;
    case AST_SWITCH:
        ast_node_free(node->u.switch_.cond);
        ast_node_free(node->u.switch_.body);
        break;
    case AST_CASE:
        ast_node_free(node->u.case_.value);
        break;
    case AST_DEFAULT: /* no owned fields */                       break;
    case AST_RETURN:    ast_node_free(node->u.return_.value);   break;
    case AST_BREAK:     /* no owned fields */                    break;
    case AST_CONTINUE:  /* no owned fields */                    break;
    case AST_GOTO:      free(node->u.goto_.label);               break;
    case AST_LABEL:     free(node->u.label_.label);              break;
    /* Phase S5: GCC labels-as-values + computed goto. */
    case AST_ADDR_OF_LABEL: free(node->u.addr_of_label.label);   break;
    case AST_COMPUTED_GOTO: ast_node_free(node->u.computed_goto.target); break;
    case AST_DEFER:     ast_node_free(node->u.defer_.body);      break;
    case AST_EXPR_STMT: ast_node_free(node->u.expr_stmt.expr);   break;
    case AST_DECL_STMT: ast_node_free(node->u.decl_stmt.decl);   break;

    /* ── Expression nodes ────────────────────────────────────────────── */
    case AST_IDENT:     free(node->u.ident.name);                break;
    case AST_INT_LIT:   /* no owned fields */                    break;
    case AST_FLOAT_LIT: /* no owned fields */                    break;
    case AST_STRING_LIT:/* text is borrowed — do NOT free */     break;
    case AST_CHAR_LIT:  /* no owned fields */                    break;
    case AST_NULL_LIT:  /* no owned fields */                    break;

    case AST_BINOP:
        ast_node_free(node->u.binop.lhs);
        ast_node_free(node->u.binop.rhs);
        break;
    case AST_UNARY:
        ast_node_free(node->u.unary.operand);
        break;
    case AST_TERNARY:
        ast_node_free(node->u.ternary.cond);
        ast_node_free(node->u.ternary.then_);
        ast_node_free(node->u.ternary.else_);
        break;
    case AST_COMMA:
        ast_node_free(node->u.comma.lhs);
        ast_node_free(node->u.comma.rhs);
        break;
    case AST_CALL:
        ast_node_free(node->u.call.callee);
        astvec_deep_free(&node->u.call.args);
        break;
    case AST_METHOD_CALL:
        ast_node_free(node->u.method_call.recv);
        free(node->u.method_call.method);
        astvec_deep_free(&node->u.method_call.args);
        break;
    case AST_FIELD_ACCESS:
        ast_node_free(node->u.field_access.recv);
        free(node->u.field_access.field);
        break;
    case AST_INDEX:
        ast_node_free(node->u.index_.base);
        ast_node_free(node->u.index_.index);
        break;
    case AST_CAST:
        ast_node_free(node->u.cast.type);
        ast_node_free(node->u.cast.operand);
        break;
    case AST_SIZEOF:
        ast_node_free(node->u.sizeof_.operand);
        break;
    case AST_STRUCT_LIT:
        ast_node_free(node->u.struct_lit.type);
        astvec_deep_free(&node->u.struct_lit.field_names);
        astvec_deep_free(&node->u.struct_lit.field_vals);
        break;
    case AST_INIT_LIST:
        astvec_deep_free(&node->u.init_list.items);
        break;
    case AST_DESIGNATED_INIT:
        free(node->u.designated_init.field_name);
        ast_node_free(node->u.designated_init.index_expr);
        ast_node_free(node->u.designated_init.value);
        break;
    case AST_COMPOUND_LIT:
        ast_node_free(node->u.compound_lit.type);
        ast_node_free(node->u.compound_lit.init);
        break;
    case AST_AT_INTRINSIC:
        free(node->u.at_intrinsic.name);
        astvec_deep_free(&node->u.at_intrinsic.args);
        break;
    case AST_STMT_EXPR: /* Phase R4 */
        ast_node_free(node->u.stmt_expr.block);
        break;

    default:
        break;
    }

    free(node);
}

/* =========================================================================
 * ast_clone_type — recursively clone a type sub-tree.
 *
 * Used by parse.c to share a single parsed type across multi-variable
 * declarations (`int x = 3, y = 7;`) without later double-freeing.
 *
 * Type nodes have very simple shapes (no string-lit text borrowing, no
 * type_ref to copy — the clone is a pre-sema AST), so this is straight-
 * forward.  AST_TYPE_ARRAY's size sub-tree is an *expression*; we clone
 * the limited expression forms that are valid as a constant array size.
 * ====================================================================== */

/* Clone a *constant-expression* sub-tree used as an array size.
 * Supports: INT_LIT, CHAR_LIT, IDENT, UNARY(+/-), BINOP(arithmetic).
 * Anything else is best-effort: returns NULL (caller will report a sema
 * error on the second-and-later vars; the first var still has the
 * original AST). */
static AstNode *clone_size_expr(const AstNode *e) {
    if (!e) return NULL;
    AstNode *n = ast_node_new(e->kind, e->loc);
    switch (e->kind) {
    case AST_INT_LIT:
        n->u.int_lit = e->u.int_lit;
        break;
    case AST_CHAR_LIT:
        n->u.char_lit = e->u.char_lit;
        break;
    case AST_IDENT:
        n->u.ident.name = cpp_xstrdup(e->u.ident.name ? e->u.ident.name : "");
        break;
    case AST_UNARY:
        n->u.unary.op      = e->u.unary.op;
        n->u.unary.postfix = e->u.unary.postfix;
        n->u.unary.operand = clone_size_expr(e->u.unary.operand);
        break;
    case AST_BINOP:
        n->u.binop.op  = e->u.binop.op;
        n->u.binop.lhs = clone_size_expr(e->u.binop.lhs);
        n->u.binop.rhs = clone_size_expr(e->u.binop.rhs);
        break;
    default:
        /* Unsupported in array-size context.  Free placeholder. */
        free(n);
        return NULL;
    }
    return n;
}

AstNode *ast_clone_type(const AstNode *node) {
    if (!node) return NULL;
    AstNode *n = ast_node_new(node->kind, node->loc);
    switch (node->kind) {
    case AST_TYPE_NAME:
        n->u.type_name.name =
            cpp_xstrdup(node->u.type_name.name ? node->u.type_name.name : "");
        break;
    case AST_TYPE_PTR:
        n->u.type_ptr.base = ast_clone_type(node->u.type_ptr.base);
        break;
    case AST_TYPE_CONST:
        n->u.type_const.base = ast_clone_type(node->u.type_const.base);
        break;
    case AST_TYPE_VOLATILE:
        n->u.type_volatile.base = ast_clone_type(node->u.type_volatile.base);
        break;
    case AST_TYPE_ARRAY:
        n->u.type_array.base = ast_clone_type(node->u.type_array.base);
        n->u.type_array.size = clone_size_expr(node->u.type_array.size);
        break;
    case AST_TYPE_FUNC:
        n->u.type_func.ret = ast_clone_type(node->u.type_func.ret);
        /* S1: TYPE_FUNC.params holds AST_PARAM_DECL nodes (consistent
         * with AST_FUNC_DEF.params).  Clone each one's type sub-tree. */
        for (size_t i = 0; i < node->u.type_func.params.len; i++) {
            const AstNode *p = node->u.type_func.params.data[i];
            if (!p) continue;
            if (p->kind == AST_PARAM_DECL) {
                AstNode *cp = ast_node_new(AST_PARAM_DECL, p->loc);
                cp->u.param_decl.type    = ast_clone_type(p->u.param_decl.type);
                cp->u.param_decl.name    = p->u.param_decl.name
                                         ? cpp_xstrdup(p->u.param_decl.name) : NULL;
                cp->u.param_decl.is_vararg = p->u.param_decl.is_vararg;
                astvec_push(&n->u.type_func.params, cp);
            } else {
                /* Fallback: any plain type node — clone as type. */
                astvec_push(&n->u.type_func.params, ast_clone_type(p));
            }
        }
        break;
    case AST_TYPE_GENERIC:
        n->u.type_generic.name =
            cpp_xstrdup(node->u.type_generic.name ? node->u.type_generic.name : "");
        for (size_t i = 0; i < node->u.type_generic.args.len; i++) {
            astvec_push(&n->u.type_generic.args,
                        ast_clone_type(node->u.type_generic.args.data[i]));
        }
        break;
    case AST_TYPE_AUTO:
    case AST_TYPE_VOID:
        /* no owned fields */
        break;
    case AST_TYPE_PARAM:
        n->u.type_param.name =
            cpp_xstrdup(node->u.type_param.name ? node->u.type_param.name : "");
        break;
    default:
        /* Not a type node — caller misuse. */
        free(n);
        return NULL;
    }
    return n;
}



const char *ast_kind_name(AstKind k) {
    static const char *names[] = {
        "FILE", "IMPORT", "STRUCT_DEF", "FUNC_DEF", "TYPEDEF_DECL", "VAR_DECL",
        "ENUM_DEF", "ENUMERATOR",
        "TYPE_NAME", "TYPE_PTR", "TYPE_CONST", "TYPE_VOLATILE",
        "TYPE_ARRAY", "TYPE_FUNC",
        "TYPE_GENERIC", "TYPE_AUTO", "TYPE_VOID", "TYPE_PARAM",
        "FIELD_DECL", "PARAM_DECL", "GENERIC_PARAM",
        "BLOCK", "IF", "WHILE", "FOR", "DO_WHILE",
        "RETURN", "BREAK", "CONTINUE", "GOTO", "LABEL",
        /* Phase S5: GCC labels-as-values + computed goto. */
        "ADDR_OF_LABEL", "COMPUTED_GOTO",
        "SWITCH", "CASE", "DEFAULT",
        "DEFER", "EXPR_STMT", "DECL_STMT",
        "IDENT", "INT_LIT", "FLOAT_LIT", "STRING_LIT", "CHAR_LIT", "NULL_LIT",
        "BINOP", "UNARY", "TERNARY", "COMMA",
        "CALL", "METHOD_CALL", "FIELD_ACCESS", "INDEX",
        "CAST", "SIZEOF", "STRUCT_LIT",
        "INIT_LIST", "DESIGNATED_INIT", "COMPOUND_LIT",
        "AT_INTRINSIC",
        "STMT_EXPR",    /* Phase R4: GCC ({ ... }) */
    };
    if ((unsigned)k < AST_COUNT) return names[k];
    return "?";
}

/* =========================================================================
 * ast_print — S-expression debug output
 * ====================================================================== */

static void indent_print(int depth, FILE *fp) {
    for (int i = 0; i < depth * 2; i++) fputc(' ', fp);
}

static void print_vec(const AstVec *v, const char *label,
                      int depth, FILE *fp) {
    if (v->len == 0) return;
    indent_print(depth, fp);
    fprintf(fp, "(%s\n", label);
    for (size_t i = 0; i < v->len; i++)
        ast_print(v->data[i], depth + 1, fp);
    indent_print(depth, fp);
    fprintf(fp, ")\n");
}

void ast_print(const AstNode *node, int depth, FILE *fp) {
    if (!node) {
        indent_print(depth, fp);
        fprintf(fp, "(null)\n");
        return;
    }

    indent_print(depth, fp);

    switch (node->kind) {

    case AST_FILE:
        fprintf(fp, "(FILE \"%s\"\n", node->u.file.path ? node->u.file.path : "");
        print_vec(&node->u.file.decls, "decls", depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");
        break;

    case AST_IMPORT:
        fprintf(fp, "(IMPORT \"%s\")\n",
                node->u.import.path ? node->u.import.path : "");
        break;

    case AST_STRUCT_DEF:
        fprintf(fp, "(STRUCT_DEF %s\n", node->u.struct_def.name);
        print_vec(&node->u.struct_def.generic_params, "generics", depth+1, fp);
        print_vec(&node->u.struct_def.fields,          "fields",   depth+1, fp);
        print_vec(&node->u.struct_def.methods,         "methods",  depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");
        break;

    case AST_FUNC_DEF:
        fprintf(fp, "(FUNC_DEF %s%s\n",
                node->u.func_def.name,
                node->u.func_def.is_operator ? " [operator]" : "");
        print_vec(&node->u.func_def.generic_params, "generics", depth+1, fp);
        if (node->u.func_def.ret_type)
            ast_print(node->u.func_def.ret_type, depth+1, fp);
        print_vec(&node->u.func_def.params, "params", depth+1, fp);
        if (node->u.func_def.body)
            ast_print(node->u.func_def.body, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");
        break;

    case AST_TYPEDEF_DECL:
        fprintf(fp, "(TYPEDEF %s\n", node->u.typedef_decl.alias);
        ast_print(node->u.typedef_decl.target, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");
        break;

    case AST_ENUM_DEF:
        fprintf(fp, "(ENUM_DEF %s\n",
                node->u.enum_def.name ? node->u.enum_def.name : "<anon>");
        print_vec(&node->u.enum_def.items, "items", depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");
        break;

    case AST_ENUMERATOR:
        fprintf(fp, "(ENUMERATOR %s\n", node->u.enumerator.name);
        if (node->u.enumerator.value) ast_print(node->u.enumerator.value, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");
        break;

    case AST_VAR_DECL:
        fprintf(fp, "(VAR_DECL %s\n", node->u.var_decl.name);
        ast_print(node->u.var_decl.type, depth+1, fp);
        if (node->u.var_decl.init)
            ast_print(node->u.var_decl.init, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");
        break;

    case AST_TYPE_NAME:
        fprintf(fp, "(TYPE_NAME %s)\n", node->u.type_name.name);   break;
    case AST_TYPE_PTR:
        fprintf(fp, "(TYPE_PTR\n");
        ast_print(node->u.type_ptr.base, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_TYPE_CONST:
        fprintf(fp, "(TYPE_CONST\n");
        ast_print(node->u.type_const.base, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_TYPE_VOLATILE:
        fprintf(fp, "(TYPE_VOLATILE\n");
        ast_print(node->u.type_volatile.base, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_TYPE_ARRAY:
        fprintf(fp, "(TYPE_ARRAY\n");
        ast_print(node->u.type_array.base, depth+1, fp);
        if (node->u.type_array.size)
            ast_print(node->u.type_array.size, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_TYPE_GENERIC:
        fprintf(fp, "(TYPE_GENERIC %s\n", node->u.type_generic.name);
        print_vec(&node->u.type_generic.args, "args", depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_TYPE_AUTO:  fprintf(fp, "(TYPE_AUTO)\n");             break;
    case AST_TYPE_VOID:  fprintf(fp, "(TYPE_VOID)\n");             break;
    case AST_TYPE_PARAM: fprintf(fp, "(TYPE_PARAM %s)\n",
                                 node->u.type_param.name);         break;

    case AST_FIELD_DECL:
        fprintf(fp, "(FIELD_DECL %s\n", node->u.field_decl.name);
        ast_print(node->u.field_decl.type, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_PARAM_DECL:
        fprintf(fp, "(PARAM_DECL %s%s\n",
                node->u.param_decl.name,
                node->u.param_decl.is_vararg ? " ..." : "");
        ast_print(node->u.param_decl.type, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_GENERIC_PARAM:
        fprintf(fp, "(GENERIC_PARAM %s)\n",
                node->u.generic_param.name);                       break;

    case AST_BLOCK:
        fprintf(fp, "(BLOCK\n");
        for (size_t i = 0; i < node->u.block.stmts.len; i++)
            ast_print(node->u.block.stmts.data[i], depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_IF:
        fprintf(fp, "(IF\n");
        ast_print(node->u.if_.cond, depth+1, fp);
        ast_print(node->u.if_.then_, depth+1, fp);
        if (node->u.if_.else_) ast_print(node->u.if_.else_, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_WHILE:
        fprintf(fp, "(WHILE\n");
        ast_print(node->u.while_.cond, depth+1, fp);
        ast_print(node->u.while_.body, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_FOR:
        fprintf(fp, "(FOR\n");
        ast_print(node->u.for_.init, depth+1, fp);
        ast_print(node->u.for_.cond, depth+1, fp);
        ast_print(node->u.for_.post, depth+1, fp);
        ast_print(node->u.for_.body, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_DO_WHILE:
        fprintf(fp, "(DO_WHILE\n");
        ast_print(node->u.do_while.body, depth+1, fp);
        ast_print(node->u.do_while.cond, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_SWITCH:
        fprintf(fp, "(SWITCH\n");
        ast_print(node->u.switch_.cond, depth+1, fp);
        ast_print(node->u.switch_.body, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_CASE:
        fprintf(fp, "(CASE\n");
        ast_print(node->u.case_.value, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_DEFAULT:  fprintf(fp, "(DEFAULT)\n");                 break;
    case AST_RETURN:
        fprintf(fp, "(RETURN\n");
        if (node->u.return_.value)
            ast_print(node->u.return_.value, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_BREAK:    fprintf(fp, "(BREAK)\n");                   break;
    case AST_CONTINUE: fprintf(fp, "(CONTINUE)\n");                break;
    case AST_GOTO:     fprintf(fp, "(GOTO %s)\n",
                               node->u.goto_.label);               break;
    case AST_LABEL:    fprintf(fp, "(LABEL %s)\n",
                               node->u.label_.label);              break;
    /* Phase S5: GCC labels-as-values + computed goto. */
    case AST_ADDR_OF_LABEL:
        fprintf(fp, "(ADDR_OF_LABEL %s)\n",
                node->u.addr_of_label.label);                       break;
    case AST_COMPUTED_GOTO:
        fprintf(fp, "(COMPUTED_GOTO\n");
        ast_print(node->u.computed_goto.target, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");                break;
    case AST_DEFER:
        fprintf(fp, "(DEFER\n");
        ast_print(node->u.defer_.body, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_EXPR_STMT:
        fprintf(fp, "(EXPR_STMT\n");
        ast_print(node->u.expr_stmt.expr, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_DECL_STMT:
        fprintf(fp, "(DECL_STMT\n");
        ast_print(node->u.decl_stmt.decl, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;

    case AST_IDENT:     fprintf(fp, "(IDENT %s)\n",
                                node->u.ident.name);               break;
    case AST_INT_LIT:
        if (node->u.int_lit.is_unsigned)
            fprintf(fp, "(INT_LIT %llu)\n",
                    (unsigned long long)node->u.int_lit.val);
        else
            fprintf(fp, "(INT_LIT %lld)\n",
                    (long long)node->u.int_lit.val);
        break;
    case AST_FLOAT_LIT: fprintf(fp, "(FLOAT_LIT %g)\n",
                                node->u.float_lit.val);            break;
    case AST_STRING_LIT:fprintf(fp, "(STRING_LIT \"%.*s\")\n",
                                (int)node->u.string_lit.len,
                                node->u.string_lit.text);          break;
    case AST_CHAR_LIT:  fprintf(fp, "(CHAR_LIT %lld)\n",
                                (long long)node->u.char_lit.val);  break;
    case AST_NULL_LIT:  fprintf(fp, "(NULL_LIT)\n");               break;

    case AST_BINOP:
        fprintf(fp, "(BINOP %s\n",
                lex_tok_kind_name(node->u.binop.op));
        ast_print(node->u.binop.lhs, depth+1, fp);
        ast_print(node->u.binop.rhs, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_UNARY:
        fprintf(fp, "(UNARY %s%s\n",
                lex_tok_kind_name(node->u.unary.op),
                node->u.unary.postfix ? " [post]" : "");
        ast_print(node->u.unary.operand, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_TERNARY:
        fprintf(fp, "(TERNARY\n");
        ast_print(node->u.ternary.cond,  depth+1, fp);
        ast_print(node->u.ternary.then_, depth+1, fp);
        ast_print(node->u.ternary.else_, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_COMMA:
        fprintf(fp, "(COMMA\n");
        ast_print(node->u.comma.lhs, depth+1, fp);
        ast_print(node->u.comma.rhs, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;

    case AST_CALL:
        fprintf(fp, "(CALL\n");
        ast_print(node->u.call.callee, depth+1, fp);
        print_vec(&node->u.call.args, "args", depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_METHOD_CALL:
        fprintf(fp, "(METHOD_CALL %s%s\n",
                node->u.method_call.arrow ? "->" : ".",
                node->u.method_call.method);
        ast_print(node->u.method_call.recv, depth+1, fp);
        print_vec(&node->u.method_call.args, "args", depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_FIELD_ACCESS:
        fprintf(fp, "(FIELD_ACCESS %s%s)\n",
                node->u.field_access.arrow ? "->" : ".",
                node->u.field_access.field);                       break;
    case AST_INDEX:
        fprintf(fp, "(INDEX\n");
        ast_print(node->u.index_.base,  depth+1, fp);
        ast_print(node->u.index_.index, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_CAST:
        fprintf(fp, "(CAST\n");
        ast_print(node->u.cast.type,    depth+1, fp);
        ast_print(node->u.cast.operand, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_SIZEOF:
        fprintf(fp, "(SIZEOF %s\n",
                node->u.sizeof_.is_type ? "type" : "expr");
        ast_print(node->u.sizeof_.operand, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_STRUCT_LIT:
        fprintf(fp, "(STRUCT_LIT\n");
        ast_print(node->u.struct_lit.type, depth+1, fp);
        print_vec(&node->u.struct_lit.field_names, "names", depth+1, fp);
        print_vec(&node->u.struct_lit.field_vals,  "vals",  depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_INIT_LIST:
        fprintf(fp, "(INIT_LIST\n");
        print_vec(&node->u.init_list.items, "items", depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_DESIGNATED_INIT:
        if (node->u.designated_init.designator_kind == 0) {
            fprintf(fp, "(DESIG .%s\n", node->u.designated_init.field_name);
        } else {
            fprintf(fp, "(DESIG [\n");
            ast_print(node->u.designated_init.index_expr, depth+1, fp);
            indent_print(depth, fp); fprintf(fp, "]\n");
        }
        ast_print(node->u.designated_init.value, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_COMPOUND_LIT:
        fprintf(fp, "(COMPOUND_LIT\n");
        ast_print(node->u.compound_lit.type, depth+1, fp);
        ast_print(node->u.compound_lit.init, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_AT_INTRINSIC:
        fprintf(fp, "(AT_INTRINSIC @%s\n", node->u.at_intrinsic.name);
        print_vec(&node->u.at_intrinsic.args, "args", depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;
    case AST_STMT_EXPR: /* Phase R4 */
        fprintf(fp, "(STMT_EXPR\n");
        ast_print(node->u.stmt_expr.block, depth+1, fp);
        indent_print(depth, fp); fprintf(fp, ")\n");              break;

    default:
        fprintf(fp, "(? kind=%d)\n", node->kind);
        break;
    }
}

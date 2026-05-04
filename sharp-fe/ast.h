/*
 * ast.h — Sharp Frontend: Phase 2 Abstract Syntax Tree.
 *
 * Defines AstNode — the tagged-union tree produced by parse.c (Phase 3)
 * and consumed by sema.c (Phase 6-10), mono.c (Phase 11), and cg.c (Phase 12).
 *
 * Memory model
 * ------------
 *   Every AstNode is individually malloc'd via ast_node_new().
 *   String fields (char *name, char *path, …) are strdup'd — the node owns them.
 *   Child lists use AstVec, whose .data array is malloc'd by astvec_push().
 *   Call ast_node_free() to recursively free a node tree.
 *   The node.type_ref field is reserved for Phase 5 (type system) and is
 *   initialised to NULL by ast_node_new().
 *
 * Operator encoding
 * -----------------
 *   .u.binop.op and .u.unary.op store SharpTokKind values (from lex.h) so
 *   that the operator token kind acts as a first-class discriminant with no
 *   additional enum.  Examples: STOK_PLUS, STOK_EQEQ, STOK_PLUSEQ.
 *
 * Phase 2 note
 * ------------
 *   AstNode.type_ref is a void* placeholder.  Phase 5 (type.h) will replace
 *   it with a TypeRef* once the type system is defined.  All Phase 2-4 code
 *   must leave it NULL.
 */
#ifndef SHARP_FE_AST_H
#define SHARP_FE_AST_H

#include "lex.h"   /* SharpTokKind, CppLoc (via sharp.h) */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Forward declaration
 * ---------------------------------------------------------------------- */
typedef struct AstNode AstNode;

/* -------------------------------------------------------------------------
 * AstVec — growable AstNode* array (used for child lists)
 * ---------------------------------------------------------------------- */
typedef struct {
    AstNode **data;
    size_t    len;
    size_t    cap;
} AstVec;

void astvec_push(AstVec *v, AstNode *n);   /* amortised O(1) */
void astvec_free(AstVec *v);               /* frees .data; nodes freed separately */

/* -------------------------------------------------------------------------
 * Storage class (ISO C 6.7.1)
 *
 *   At most one storage-class specifier may appear in a declaration.
 *   SC_NONE means "no storage class given" (the implicit default).
 *   SC_TYPEDEF is a storage class in the C grammar even though it semantically
 *   creates a type alias rather than a variable; we produce AST_TYPEDEF_DECL
 *   in that case and never carry SC_TYPEDEF on a VAR_DECL or FUNC_DEF.
 *   SC_AUTO_C is C's `auto` storage class — distinct from Sharp's
 *   AST_TYPE_AUTO (type deduction).  Modern C code never uses it.
 * ---------------------------------------------------------------------- */
typedef enum {
    SC_NONE     = 0,
    SC_STATIC,
    SC_EXTERN,
    SC_TYPEDEF,
    SC_REGISTER,
    SC_AUTO_C,
} StorageClass;

/* -------------------------------------------------------------------------
 * AST node kinds
 * ---------------------------------------------------------------------- */
typedef enum {
    /* ── Top-level ──────────────────────────────────────────────────────── */
    AST_FILE,            /* root of a .sp compilation unit                  */
    AST_IMPORT,          /* import "path.sp";                               */
    AST_STRUCT_DEF,      /* struct Name<T> { fields; methods; }             */
    AST_FUNC_DEF,        /* RetType name<T>(params) { body }                */
    AST_TYPEDEF_DECL,    /* typedef OldType NewName;                        */
    AST_VAR_DECL,        /* Type name [= init];  or  auto name = init;      */
    /* S2: enum.  AST_ENUM_DEF carries an optional tag and a vector of
     * AST_ENUMERATOR children.  Each enumerator has a name and an
     * optional explicit value-expression; absent values default to one
     * more than the previous (or 0 for the first). */
    AST_ENUM_DEF,
    AST_ENUMERATOR,

    /* ── Type expressions ────────────────────────────────────────────────── */
    AST_TYPE_NAME,       /* bare type name: int, float, Buffer, …           */
    AST_TYPE_PTR,        /* T*                                              */
    AST_TYPE_CONST,      /* const T                                         */
    AST_TYPE_VOLATILE,   /* volatile T  (S1: C type qualifier)              */
    AST_TYPE_ARRAY,      /* T[N]  (N may be NULL for incomplete)            */
    AST_TYPE_FUNC,       /* RetType (*)(ParamTypes…)                        */
    AST_TYPE_GENERIC,    /* Vec<int>,  Pair<K,V>                            */
    AST_TYPE_AUTO,       /* auto  (type-deduction placeholder, spec §auto)  */
    AST_TYPE_VOID,       /* void                                            */
    AST_TYPE_PARAM,      /* T  inside a generic body (unbound param)        */

    /* ── Declarations ────────────────────────────────────────────────────── */
    AST_FIELD_DECL,      /* Type name;  inside struct body                  */
    AST_PARAM_DECL,      /* Type name  in function parameter list           */
    AST_GENERIC_PARAM,   /* T  in  <T, U, …>  of struct/func definition    */

    /* ── Statements ──────────────────────────────────────────────────────── */
    AST_BLOCK,           /* { stmts… }                                      */
    AST_IF,              /* if (cond) then [else alt]                       */
    AST_WHILE,           /* while (cond) body                               */
    AST_FOR,             /* for (init; cond; post) body                     */
    AST_DO_WHILE,        /* do body while (cond);                           */
    AST_RETURN,          /* return [value];                                 */
    AST_BREAK,           /* break;                                          */
    AST_CONTINUE,        /* continue;                                       */
    AST_GOTO,            /* goto label;                                     */
    AST_LABEL,           /* label:                                          */
    /* Phase S5: GCC labels-as-values extension (used by Lua's lvm.c
     * interpreter dispatch).  AST_ADDR_OF_LABEL is the prefix `&&label`
     * expression — its value is `void *`.  AST_COMPUTED_GOTO is
     * `goto *expr;` where expr evaluates to an address taken with
     * `&&label`.  cg emits both verbatim because gcc and clang accept
     * the extension (it predates ISO C). */
    AST_ADDR_OF_LABEL,   /* &&label                                         */
    AST_COMPUTED_GOTO,   /* goto *expr;                                     */
    /* S2: switch / case / default.  AST_SWITCH carries (cond, body) where
     * body is an AST_BLOCK that interleaves AST_CASE / AST_DEFAULT labels
     * with regular statements.  AST_CASE carries (value-expr, sub-stmt
     * NULL — labels are bare in the block).  AST_DEFAULT carries no
     * payload.  Codegen emits straight C switch syntax. */
    AST_SWITCH,
    AST_CASE,
    AST_DEFAULT,
    AST_DEFER,           /* defer EXPR;  or  defer { stmts }               */
    AST_EXPR_STMT,       /* expr;                                           */
    AST_DECL_STMT,       /* wraps AST_VAR_DECL used as a statement          */

    /* ── Expressions ─────────────────────────────────────────────────────── */
    AST_IDENT,           /* identifier                                      */
    AST_INT_LIT,         /* integer constant                                */
    AST_FLOAT_LIT,       /* floating-point constant                         */
    AST_STRING_LIT,      /* string literal (text/len borrowed from token)   */
    AST_CHAR_LIT,        /* character constant                              */
    AST_NULL_LIT,        /* null                                            */
    AST_BINOP,           /* lhs OP rhs  (op = SharpTokKind; incl. assign)  */
    AST_UNARY,           /* OP operand  or  operand OP  (postfix)           */
    AST_TERNARY,         /* cond ? then : else                              */
    AST_COMMA,           /* lhs , rhs                                       */
    AST_CALL,            /* callee(args…)                                   */
    AST_METHOD_CALL,     /* recv.method(args…)  or  recv->method(args…)    */
    AST_FIELD_ACCESS,    /* recv.field  or  recv->field                     */
    AST_INDEX,           /* base[index]                                     */
    AST_CAST,            /* (type)operand                                   */
    AST_SIZEOF,          /* sizeof(type)  or  sizeof(expr)                  */
    AST_STRUCT_LIT,      /* Type { field: val, … }                         */
    /* S4: braced initializer machinery.
     *   AST_INIT_LIST       — `{ a, b, c }` or `{ .x=1, [3]=2 }`.  Items
     *                          may be plain expressions (positional) or
     *                          AST_DESIGNATED_INIT.  Init lists nest:
     *                          `{ {1,2}, {3,4} }` is INIT_LIST of
     *                          INIT_LISTs.
     *   AST_DESIGNATED_INIT — single `.field = expr` or `[idx] = expr`
     *                          item.  designator_kind selects which.
     *   AST_COMPOUND_LIT    — `(Type){ init-list }` as an expression.
     *                          Distinguished from a CAST at parse time
     *                          by whether `(type)` is followed by `{`. */
    AST_INIT_LIST,
    AST_DESIGNATED_INIT,
    AST_COMPOUND_LIT,
    AST_AT_INTRINSIC,    /* @has_operator(T,+) or @static_assert(c,"msg")  */
    /* Phase R4: GCC statement-expression extension.  `({ stmts; expr; })`
     * evaluates a block of statements as an expression; the value is the
     * last expression-statement's value (or void if none).  Used heavily
     * in glibc's assert() and type-safe MAX/MIN macros. */
    AST_STMT_EXPR,       /* ({ block })  — value = last expr-stmt          */

    AST_COUNT
} AstKind;

/* -------------------------------------------------------------------------
 * AstNode
 * ---------------------------------------------------------------------- */
struct AstNode {
    AstKind kind;
    CppLoc  loc;

    /* Phase 5 (type.h) fills this in.  Must be NULL until then. */
    void   *type_ref;

    union {
        /* ── AST_FILE ────────────────────────────────────────────────── */
        struct {
            char   *path;    /* source file path (owned) */
            AstVec  decls;   /* top-level declarations   */
        } file;

        /* ── AST_IMPORT ──────────────────────────────────────────────── */
        struct {
            char *path;      /* "vec.sp" (owned, without quotes) */
        } import;

        /* ── AST_STRUCT_DEF ──────────────────────────────────────────── */
        struct {
            char   *name;
            AstVec  generic_params;  /* AstNode* AST_GENERIC_PARAM */
            AstVec  fields;          /* AstNode* AST_FIELD_DECL    */
            AstVec  methods;         /* AstNode* AST_FUNC_DEF      */
            /* S2: same node also represents `union Name { ... }` when
             * is_union is true.  Field-access semantics are identical;
             * only storage layout differs and is_union flips the cg.c
             * keyword from `struct` to `union`. */
            bool    is_union;
        } struct_def;

        /* ── AST_ENUM_DEF (S2) ───────────────────────────────────────── */
        struct {
            char  *name;          /* may be NULL for anonymous enums */
            AstVec items;         /* AstNode* AST_ENUMERATOR         */
        } enum_def;

        /* ── AST_ENUMERATOR (S2) ─────────────────────────────────────── */
        struct {
            char    *name;
            AstNode *value;       /* optional explicit value expr (may be NULL) */
        } enumerator;

        /* ── AST_FUNC_DEF ────────────────────────────────────────────── */
        struct {
            char    *name;
            AstVec   generic_params; /* AstNode* AST_GENERIC_PARAM        */
            AstNode *ret_type;       /* return type node                  */
            AstVec   params;         /* AstNode* AST_PARAM_DECL           */
            AstNode *body;           /* AST_BLOCK; NULL = forward decl    */
            bool     is_static;      /* static — not exported by import   */
            bool     is_operator;    /* defined as operator+, operator[], …*/
            bool     is_const_method;/* 'const' suffix present after ')'  */
            /* S1: ISO-C storage / function-specifier fields.  is_static
             * remains for back-compat with import.c & sema.c; we keep it
             * synchronized with `storage == SC_STATIC` for new code. */
            StorageClass storage;    /* SC_NONE / SC_STATIC / SC_EXTERN  */
            bool     is_inline;      /* `inline` (C99 function-specifier)*/
            /* Phase R2: C11 _Thread_local storage-class.  Coexists with
             * static / extern (per ISO 6.7.1).  stb_image's
             * `static _Thread_local const char *stbi__g_failure_reason;`
             * is the canonical real-world case. */
            bool     is_thread_local;
            /* Phase R6: GCC __attribute__((...)) text captured verbatim
             * from the declaration.  NULL if no attributes were present.
             * Emitted into the generated C after the closing ')' of the
             * parameter list (trailing-attribute position), which is
             * unambiguous and accepted by all modern C compilers.
             * Example: "static inline __attribute__((always_inline))" in
             * the source stores "__attribute__((always_inline))" here. */
            char    *gcc_attrs;
        } func_def;

        /* ── AST_TYPEDEF_DECL ────────────────────────────────────────── */
        struct {
            char    *alias;
            AstNode *target;
        } typedef_decl;

        /* ── AST_VAR_DECL ────────────────────────────────────────────── */
        struct {
            AstNode *type;   /* NULL when auto (type_ref filled by sema)  */
            char    *name;
            AstNode *init;   /* NULL = no initialiser (illegal for auto)  */
            /* S1: storage-class specifier from the declaration.
             * SC_TYPEDEF on a VAR_DECL never appears (we emit
             * AST_TYPEDEF_DECL instead).  SC_REGISTER / SC_EXTERN /
             * SC_STATIC pass through to the generated C. */
            StorageClass storage;
            /* Phase R2: C11 _Thread_local storage-class. */
            bool     is_thread_local;
            /* Phase R6: GCC __attribute__((...)) text, or NULL.
             * Emitted after the declarator (before `=` or `;`) in the
             * generated C.  Covers aligned, unused, deprecated, etc. */
            char    *gcc_attrs;
        } var_decl;

        /* ── Type nodes ──────────────────────────────────────────────── */
        struct { char   *name; } type_name;      /* int / float / Buffer   */
        struct { AstNode *base; } type_ptr;
        struct { AstNode *base; } type_const;
        struct { AstNode *base; } type_volatile;  /* S1: volatile T        */
        struct { AstNode *base; AstNode *size; } type_array;  /* size==NULL for []*/
        struct {                                               /* RetT(*)(P…)     */
            AstNode *ret;
            AstVec   params;
        } type_func;
        struct {                                               /* Vec<int,float>  */
            char   *name;
            AstVec  args;    /* AstNode* type expressions */
        } type_generic;
        struct { char *name; } type_param;       /* T (unbound generic param)*/

        /* ── Declaration nodes ───────────────────────────────────────── */
        /* ── AST_FIELD_DECL ──────────────────────────────────────────── */
        struct {
            AstNode *type;
            char    *name;
            /* S4: optional bit-field width.  When non-NULL, this field is
             * a C bit-field and cg emits `T name : <expr>;`.  The width
             * must be an integer constant expression; we don't evaluate
             * it — the C compiler enforces constraints. */
            AstNode *bit_width;
        } field_decl;
        struct { AstNode *type; char *name; bool is_vararg; } param_decl;
        struct { char *name; } generic_param;

        /* ── Statement nodes ─────────────────────────────────────────── */
        struct { AstVec stmts; } block;

        struct {
            AstNode *cond;
            AstNode *then_;
            AstNode *else_;   /* NULL = no else */
        } if_;

        struct { AstNode *cond; AstNode *body; } while_;

        struct {
            AstNode *init;   /* AST_DECL_STMT or AST_EXPR_STMT; NULL ok */
            AstNode *cond;   /* NULL = infinite loop                     */
            AstNode *post;   /* NULL ok                                  */
            AstNode *body;
        } for_;

        struct { AstNode *cond; AstNode *body; } do_while;
        /* S2: switch (cond) body  ---  body is an AST_BLOCK whose
         *     statements interleave AST_CASE / AST_DEFAULT pseudo-labels
         *     with regular statements.  This matches C's syntax exactly. */
        struct { AstNode *cond; AstNode *body; } switch_;
        struct { AstNode *value; } case_;            /* value: integer constant expr */
        /* AST_DEFAULT: no payload */
        struct { AstNode *value; } return_;          /* value == NULL for bare return */
        struct { char *label; } goto_;
        struct { char *label; } label_;
        /* Phase S5: GCC labels-as-values.  addr_of_label is the operand
         * of unary `&&` followed by an identifier; computed_goto holds
         * the target expression of `goto *expr;`. */
        struct { char *label;   } addr_of_label;
        struct { AstNode *target; } computed_goto;
        struct { AstNode *body; } defer_;            /* body: AST_BLOCK or AST_EXPR_STMT */
        struct { AstNode *expr; } expr_stmt;
        struct { AstNode *decl; } decl_stmt;
        /* break / continue: no extra fields */

        /* ── Expression nodes ────────────────────────────────────────── */
        struct { char *name; } ident;

        struct {
            int64_t  val;
            bool     is_unsigned;
            bool     is_long;      /* L suffix */
            bool     is_longlong;  /* LL suffix */
        } int_lit;

        struct { double val; } float_lit;

        struct {
            const char *text;   /* borrowed from token (CppResult lifetime) */
            size_t      len;
        } string_lit;

        struct { int64_t val; } char_lit;
        /* null_lit: no extra fields */

        struct {
            SharpTokKind op;   /* STOK_PLUS, STOK_EQEQ, STOK_PLUSEQ, … */
            AstNode     *lhs;
            AstNode     *rhs;
        } binop;

        struct {
            SharpTokKind op;       /* STOK_MINUS, STOK_BANG, STOK_PLUSPLUS, … */
            AstNode     *operand;
            bool         postfix;  /* true for x++, x-- */
        } unary;

        struct { AstNode *cond; AstNode *then_; AstNode *else_; } ternary;
        struct { AstNode *lhs;  AstNode *rhs;                  } comma;

        struct { AstNode *callee; AstVec args; } call;

        struct {
            AstNode     *recv;
            bool         arrow;   /* true for -> */
            char        *method;
            AstVec       args;
            /* Sema annotation: true when sema discovered this is not a
             * struct method but a function-pointer field that happens
             * to match `recv.name(args)` syntax — see C-superset code
             * patterns like glibc's vtable-style structs.  When set,
             * cg emits the call as `recv.field(args)` (resp. `->`)
             * instead of routing through the method-mangling path. */
            bool         is_field_call;
        } method_call;

        struct {
            AstNode *recv;
            bool     arrow;
            char    *field;
        } field_access;

        struct { AstNode *base; AstNode *index; } index_;

        struct { AstNode *type;    AstNode *operand; } cast;
        struct {
            AstNode *operand;
            bool     is_type;
            /* Phase R7: true when this node represents `_Alignof`/`__alignof`
             * rather than `sizeof`.  cg emits `_Alignof(T)` instead of
             * `sizeof(T)`.  The AST node is reused (same operand shape). */
            bool     is_alignof;
        } sizeof_;

        struct {                     /* Buffer { ptr: x, len: n }          */
            AstNode *type;
            AstVec   field_names;    /* AST_IDENT nodes (field names)      */
            AstVec   field_vals;     /* expression nodes (initialisers)    */
        } struct_lit;

        /* ── AST_INIT_LIST (S4) ──────────────────────────────────────── */
        struct {
            AstVec items;            /* AstNode* — expr or DESIGNATED_INIT */
        } init_list;

        /* ── AST_DESIGNATED_INIT (S4) ────────────────────────────────── */
        struct {
            /* designator_kind: 0 = '.field', 1 = '[index]' */
            int      designator_kind;
            char    *field_name;     /* used when kind == 0; owned         */
            AstNode *index_expr;     /* used when kind == 1; owned         */
            AstNode *value;          /* the initializer (any expr or list) */
        } designated_init;

        /* ── AST_COMPOUND_LIT (S4) ───────────────────────────────────── */
        struct {
            AstNode *type;           /* the (Type) before the braces       */
            AstNode *init;           /* AST_INIT_LIST                      */
        } compound_lit;

        struct {
            char  *name;    /* "has_operator" or "static_assert" (no @)   */
            AstVec args;
        } at_intrinsic;

        /* ── AST_STMT_EXPR (R4) ──────────────────────────────────────── */
        /* GCC statement-expression: ({ stmts; expr; }).  `block` is a
         * regular AST_BLOCK.  The value/type of the whole expression is
         * the last expression-statement's value, or void if the block is
         * empty or ends with a non-expression statement. */
        struct {
            AstNode *block;          /* AST_BLOCK containing the stmts     */
        } stmt_expr;

    } u;
};

/* -------------------------------------------------------------------------
 * Construction
 * ---------------------------------------------------------------------- */

/** Allocate a new node of the given kind at loc.
 *  All pointer fields are NULL, all numeric fields are 0,
 *  type_ref is NULL.  The caller fills in u.* fields. */
AstNode *ast_node_new(AstKind kind, CppLoc loc);

/** Recursively free node and all owned children.
 *  Frees char* fields (strdup'd names), AstVec data arrays,
 *  and child AstNode* pointers.  Does NOT free borrowed pointers
 *  (string_lit.text, type_ref). */
void ast_node_free(AstNode *node);

/** Recursively clone a *type* AST sub-tree (AST_TYPE_NAME, AST_TYPE_PTR,
 *  AST_TYPE_CONST, AST_TYPE_ARRAY, AST_TYPE_FUNC, AST_TYPE_GENERIC,
 *  AST_TYPE_AUTO, AST_TYPE_VOID, AST_TYPE_PARAM).
 *  Used by parse.c to share a single parsed type across multi-variable
 *  declarations like `int x = 3, y = 7;` without double-freeing.
 *  AST_TYPE_ARRAY's size sub-tree (an expression) is cloned too.
 *  Returns NULL if node is NULL or not a type node. */
AstNode *ast_clone_type(const AstNode *node);

/* -------------------------------------------------------------------------
 * Printing (debug / test)
 * ---------------------------------------------------------------------- */

/** Print a human-readable S-expression representation of node to fp.
 *  indent controls the initial indentation depth (0 = top-level).        */
void ast_print(const AstNode *node, int indent, FILE *fp);

/** Return the name string for a kind (e.g. "AST_BINOP"). */
const char *ast_kind_name(AstKind k);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_AST_H */

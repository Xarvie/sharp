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
 *
 *   Two semantic annotation fields are set by sema.c (Phase 5+) and read
 *   by cg.c; both are NULL after parsing:
 *     sem_type  (Type *)  — inferred type of an expression / variable
 *     sem_scope (Scope *) — scope opened for a function / block / for-stmt
 *
 * Operator encoding
 * -----------------
 *   .u.binop.op and .u.unary.op store SharpTokKind values (from lex.h) so
 *   that the operator token kind acts as a first-class discriminant with no
 *   additional enum.  Examples: STOK_PLUS, STOK_EQEQ, STOK_PLUSEQ.
 */
#ifndef SHARP_FE_AST_H
#define SHARP_FE_AST_H

#include "lex.h"   /* SharpTokKind, CppLoc (via sharp.h) */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ── FmtFlags — source-order qualifier/specifier recording ─────────────────
 * A uint32_t bit-field used by DeclSpecs (parse.c) and AST nodes (func_def,
 * var_decl) to remember the exact token order of qualifiers/specifiers from
 * the source file, so the CG can reproduce it faithfully in C output.
 *
 * Using a single integer instead of 8 individual bools saves struct padding
 * and makes the intent explicit.
 */
typedef uint32_t FmtFlags;
#define FMTF_INLINE_LEADING      (1u << 0) /* `inline` before storage class  */
#define FMTF_INLINE_AFTER_ATTRS  (1u << 1) /* `__attribute__` before `inline` */
#define FMTF_CONST_LEADING       (1u << 2) /* `const` before storage class   */
#define FMTF_CONST_POSTFIX       (1u << 3) /* `const` after base type        */
#define FMTF_CONST_BEFORE_VOL    (1u << 4) /* `const` precedes `volatile`    */
#define FMTF_VOLATILE_POSTFIX    (1u << 5) /* `volatile` after base type     */
#define FMTF_ATTRS_LEADING       (1u << 6) /* `__attribute__` before storage */
#define FMTF_NAME_PAREN          (1u << 7) /* function name written as (name)*/



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
    AST_EXTERN_INST,     /* extern struct Vec<int>; — explicit instantiation  */
    /* C7: Preserved GCC extension — text captured verbatim from source.
     * Emitted as-is in C mode; silently ignored in Sharp mode.
     * Used for: _Static_assert, __asm__, __extension__, etc. */
    AST_GCC_VERBATIM,
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
    AST_TYPEOF_TYPE,     /* __typeof__(expr) as a type specifier — carries
                            verbatim text + parsed inner expression        */
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
    AST_BINOP,           /* lhs OP rhs  (op = SharpTokKind; incl. assign)  */
    AST_UNARY,           /* OP operand  or  operand OP  (postfix)           */
    AST_TERNARY,         /* cond ? then : else                              */
    AST_COMMA,           /* lhs , rhs                                       */
    AST_CALL,            /* callee(args…)                                   */
    AST_METHOD_CALL,     /* recv.method(args…)  or  recv->method(args…)    */
    AST_FIELD_ACCESS,    /* recv.field  or  recv->field                     */
    AST_INDEX,           /* base[index]                                     */
    AST_CAST,            /* (type)operand                                   */
    /* C8: parenthesised expression.  Preserves user-written parentheses so
     * token-identical round-trip output keeps `return (expr)` intact. */
    AST_PAREN,           /* ( expr )                                        */
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
    /* GCC statement-expression extension.  `({ stmts; expr; })`
     * evaluates a block of statements as an expression; the value is the
     * last expression-statement's value (or void if none).  Used heavily
     * in glibc's assert() and type-safe MAX/MIN macros. */
    AST_STMT_EXPR,       /* ({ block })  — value = last expr-stmt          */

    /* _Generic expression (C11) */
    AST_GENERIC_EXPR,    /* _Generic(expr, type: val, ...)                */
    AST_GENERIC_ASSOC,   /* type: value  or  default: value               */

    /* ── Generic function call (Phase G) ─────────────────────────────── */
    /* func<int>(a, b)  — explicit type-argument call.
     *
     * Previously the parser created AST_CAST{TYPE_GENERIC, operand=NULL}
     * as a callee placeholder and let parse_postfix wrap it in AST_CALL.
     * That hack required special-casing in sema (NULL-operand guard) and
     * cg (callee-type detection).  AST_GENERIC_CALL makes the intent
     * explicit: one node, one meaning.
     *
     * .name       — function name ("swap", "max", …)
     * .type_args  — explicit type arguments <T1, T2, …> (AST_TYPE_* nodes)
     * .call_args  — value arguments (expression nodes)
     *
     * The node participates in parse_postfix chaining so that
     *   swap<int>(&a, &b)[0].field
     * works: parse_primary creates AST_GENERIC_CALL then hands it to
     * parse_postfix which wraps it in INDEX / FIELD_ACCESS as needed. */
    AST_GENERIC_CALL,

    AST_COUNT
} AstKind;

/* -------------------------------------------------------------------------
 * AstNode
 * ---------------------------------------------------------------------- */
struct AstNode {
    AstKind kind;
    CppLoc  loc;

    /* Phase 5 (type.h) fills this in.  Must be NULL until then. */
    /* Semantic annotations set by sema.c; always NULL after parsing.
     *
     * sem_type  — the inferred C type of an expression / variable / typedef.
     *             Set on: AST_VAR_DECL, AST_TYPEDEF_DECL, and all expression
     *             nodes.  Read by cg.c to avoid re-running ty_from_ast.
     *             Type: Type *  (from type.h).
     *
     * sem_scope — the Scope that was opened for a compound statement or
     *             function definition.  Set on: AST_FUNC_DEF, AST_BLOCK,
     *             AST_IF, AST_FOR, AST_WHILE, AST_DO.
     *             Type: Scope *  (from scope.h).
     *
     * These two fields replace the old single `void *type_ref` which was
     * used for both purposes without any compile-time type safety.
     */
    struct Type  *sem_type;   /* replaces (Type*)type_ref  */
    struct Scope *sem_scope;  /* replaces (Scope*)type_ref */

    union {
        /* ── AST_FILE ────────────────────────────────────────────────── */
        struct {
            char   *path;    /* source file path (owned) */
            AstVec  decls;   /* top-level decls from the user source file */
            AstVec  sys_decls; /* decls from system/external headers (sema only,
                                * not emitted by CG — replaced by #includes) */
            /* List of #include directives that appear directly in the user
             * source file (not transitively).  CG emits these verbatim
             * instead of re-emitting the expanded sys_decls content.
             * Each entry is a heap-allocated string like:
             *   "#include <stdio.h>"  or  "#include \"mylib.h\"" */
            char  **user_includes;   /* owned array of owned strings */
            size_t  nuser_includes;
            size_t  user_includes_cap;
        } file;

        /* ── AST_EXTERN_INST ─────────────────────────────────────────── */
        /* extern struct Vec<int>; — explicit generic instantiation.
         * Tells the compiler to emit the monomorphised struct definition
         * for the named generic type.  No C output of its own. */
        struct {
            AstNode *type;   /* AST_TYPE_GENERIC or AST_TYPE_NAME (owned) */
        } extern_inst;

        /* ── AST_GCC_VERBATIM ────────────────────────────────────────── */
        /* C7: verbatim text preserved from source for GCC extensions.
         * `text` is a heap-allocated NUL-terminated string of the original
         * token spellings, space-separated.  Does NOT include trailing `;`. */
        struct {
            char *text;   /* owned */
            bool  is_stmt; /* true for statement context, false for decl */
        } gcc_verbatim;

        /* ── AST_STRUCT_DEF ──────────────────────────────────────────── */
        struct {
            char   *name;
            AstVec  generic_params;  /* AstNode* AST_GENERIC_PARAM */
            AstVec  fields;          /* AstNode* AST_FIELD_DECL    */
            AstVec  methods;         /* AstNode* AST_FUNC_DEF      */
            bool    is_union;
            bool    is_class;            /* declared with 'class' keyword (auto-typedef, methods allowed) */
            bool    is_transparent_union;
            bool    is_synthetic;    /* C8: forward stubs created by scope.c */
            bool    from_inline_typedef; /* parsed as `typedef struct X{..}X;` */
            bool    is_nested_in_struct; /* defined inside another struct body */
            bool    from_inline_var;    /* defined inline in a var declaration */
            char   *leading_attrs;    /* __attribute__ after tag, before '{' */
            bool    has_body;         /* true if '{...}' was present (even if empty) */
        } struct_def;

        /* ── AST_ENUM_DEF (S2) ───────────────────────────────────────── */
        struct {
            char  *name;          /* may be NULL for anonymous enums */
            AstVec items;         /* AstNode* AST_ENUMERATOR         */
            bool   inline_typedef;/* C8: emitted inline by typedef; skip standalone */
            bool   trailing_comma;/* source had trailing comma after last enumerator */
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
            bool     is_operator;
            bool     is_const_method;
            bool     is_constexpr;         /* C23: constexpr function */
            StorageClass storage;
            bool     is_inline;
            /* C11 _Thread_local storage-class.  Coexists with
             * static / extern (per ISO 6.7.1).  stb_image's
             * `static _Thread_local const char *stbi__g_failure_reason;`
             * is the canonical real-world case. */
            bool     is_thread_local;
            /* GCC __attribute__((...)) text captured verbatim
             * from the declaration.  NULL if no attributes were present.
             * Emitted into the generated C after the closing ')' of the
             * parameter list (trailing-attribute position), which is
             * unambiguous and accepted by all modern C compilers.
             * Example: "static inline __attribute__((always_inline))" in
             * the source stores "__attribute__((always_inline))" here. */
            char    *gcc_attrs;
            char    *trailing_gcc_attrs; /* C8: attrs after parameter list */
            /* C3: distinguish `f()` (unspecified/K&R) from `f(void)` (ISO).
             * true  → original source had empty parens `()`
             * false → original had `(void)` or had actual parameters
             * Used by cg to preserve the original param-list form in C mode. */
            bool     params_unspecified;
            /* C8: `extern T (name)(params)` — name had extra parens. */
            bool     name_paren;
            /* p21: K&R old-style function definition — preserve in C mode. */
            bool     is_kr_style;
            const char *inline_kw;
            /* fmt: source-order flags for CG faithful output (FMTF_* from ast.h).
             * Replaces: gcc_attrs_leading, inline_leading, inline_after_attrs,
             *           const_leading. See FmtFlags definition above.          */
            FmtFlags    fmt;
            bool     is_noreturn; /* C11 _Noreturn */
            char    *struct_name; /* extension method: target struct/class name */
            bool     has_receiver; /* extension method with explicit 'this' receiver */
        } func_def;

        /* ── AST_TYPEDEF_DECL ────────────────────────────────────────── */
        struct {
            char    *alias;
            AstNode *target;
            bool     target_unowned; /* C8: true when target is owned by file->decls */
            char    *gcc_attrs;      /* C8: trailing __attribute__ after alias name */
            bool     has_extension;  /* C8: __extension__ prefix in source */
        } typedef_decl;

        /* ── AST_VAR_DECL ────────────────────────────────────────────── */
        struct {
            bool     has_extension;
            AstNode *type;
            char    *name;
            AstNode *init;
            StorageClass storage;
            bool     is_thread_local;
            bool     is_constexpr;       /* C23: constexpr variable */
            char    *gcc_attrs;
            char    *gcc_attrs_trailing; /* p43: attr AFTER declarator name */
            bool     is_comma_cont;
            /* fmt: source-order flags for CG faithful output (FMTF_ATTRS_LEADING etc.) */
            FmtFlags fmt;
            char    *alignas_text; /* _Alignas(...) raw inner text */
        } var_decl;

        /* ── Type nodes ──────────────────────────────────────────────── */
        struct { char *name; bool is_struct_tag; bool is_enum_tag;
                 const char *display_name; } type_name;      /* int / float / Buffer   */
        struct { AstNode *base; bool ptr_const; bool ptr_volatile; bool ptr_restrict;
                 const char *restrict_kw; /* p45: original __restrict/__restrict__ */
                 const char *nullability;  /* macOS: "_Nullable", "_Nonnull", "_Null_unspecified" */
               } type_ptr;
        struct { AstNode *base; bool is_postfix; const char *kw; } type_const;
        struct { AstNode *base; const char *kw; bool is_postfix; bool is_atomic; } type_volatile;
        struct { AstNode *base; AstNode *size;
                 bool has_static;   /* C8: int a[static 10]  */
                 bool has_restrict; /* C8: char *v[restrict] */
                 bool has_vla_star; /* C8: int a[const *] VLA-unspecified */
        } type_array;  /* size==NULL for []*/
        struct {                                               /* RetT(*)(P…)     */
            AstNode *ret;
            AstVec   params;
            bool     params_unspecified; /* C3: true for `()` vs `(void)` */
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
            /* S4: optional bit-field width. */
            AstNode *bit_width;
            /* C7: GCC __attribute__((...)) on the field. */
            char    *gcc_attrs;
            /* A8: true for fields 2..N in a comma-list declaration
             * (`unsigned a:1, b:1` → a has false, b has true). */
            bool     is_comma_cont;
            /* Field name was written as (name) in source — preserve the parens. */
            bool     name_paren;
            bool     is_static;    /* static member — belongs to type, not instance */
        } field_decl;
        struct { AstNode *type; char *name; bool is_vararg;
                 char *gcc_attrs; /* C7: __attribute__ after param name */
                 StorageClass storage; /* register/const on params */
               } param_decl;
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
            char    *orig_text;    /* C3: original token text e.g. "0xFF", "42UL" */
        } int_lit;

        struct {
            double   val;
            char    *orig_text;    /* C3: original token text e.g. "1.5f", "2.0L" */
        } float_lit;

        struct {
            const char *text;   /* borrowed from token (CppResult lifetime) */
            size_t      len;
        } string_lit;

        struct {
            int64_t  val;
            char    *orig_text; /* C3: original token text e.g. "'n'", "'\\n'" */
        } char_lit;
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
        /* C8: AST_PAREN — user-written parentheses preserved for round-trip */
        struct { AstNode *inner; } paren;
        struct {
            AstNode *operand;
            bool     is_type;
            /* true when this node represents `_Alignof`/`__alignof`
             * rather than `sizeof`.  cg emits `_Alignof(T)` instead of
             * `sizeof(T)`.  The AST node is reused (same operand shape). */
            bool     is_alignof;
            /* C8: original spelling of alignof keyword (NULL = _Alignof) */
            const char *alignof_spelling;
            /* C8: true when expr-operand had parentheses in source:
             * `sizeof(x)` → true, `sizeof x` → false.  Only meaningful
             * when is_type=false.  Drives whether cg wraps in parens. */
            bool     expr_has_parens;
        } sizeof_;

        struct {                     /* Buffer { ptr: x, len: n }          */
            AstNode *type;
            AstVec   field_names;    /* AST_IDENT nodes (field names)      */
            AstVec   field_vals;     /* expression nodes (initialisers)    */
        } struct_lit;

        /* ── AST_INIT_LIST (S4) ──────────────────────────────────────── */
        struct {
            AstVec items;            /* AstNode* — expr or DESIGNATED_INIT */
            bool   trailing_comma;   /* original had trailing comma before } */
        } init_list;

        /* ── AST_DESIGNATED_INIT (S4) ────────────────────────────────── */
        struct {
            /* designator_kind: 0 = '.field', 1 = '[index]' */
            int      designator_kind;
            char    *field_name;     /* used when kind == 0; owned         */
            AstNode *index_expr;     /* used when kind == 1; owned         */
            AstNode *index_end;      /* range end for [lo ... hi]; NULL if none */
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
            /* v0.13: sema-computed result for bool intrinsics
             * (has_operator/has_method/has_field/is_arithmetic/is_pointer).
             * resolved_set==true means sema already evaluated; resolved_val
             * holds the 0/1 result.  For @type_name, resolved_str holds the
             * mangled name string. */
            int   resolved_val;
            char *resolved_str;
            bool  resolved_set;
        } at_intrinsic;

        /* ── AST_STMT_EXPR (R4) ──────────────────────────────────────── */
        /* GCC statement-expression: ({ stmts; expr; }).  `block` is a
         * regular AST_BLOCK.  The value/type of the whole expression is
         * the last expression-statement's value, or void if the block is
         * empty or ends with a non-expression statement. */
        struct {
            AstNode *block;          /* AST_BLOCK containing the stmts     */
        } stmt_expr;

        /* ── AST_GENERIC_EXPR (R9) ─────────────────────────────────────── */
        /* _Generic(controlling_expr, T1: val1, ..., default: valdef)
         * controlling: the expression whose type controls selection.
         * associations: vector of AST_GENERIC_ASSOC. */
        struct {
            AstNode *controlling;    /* the controlling expression          */
            AstVec   associations;   /* vector of AST_GENERIC_ASSOC nodes   */
        } generic_expr;

        /* ── AST_GENERIC_ASSOC (R9) ────────────────────────────────────── */
        /* One association in a _Generic expression: type-name : expression
         * or 'default' : expression (is_default=1, type=NULL). */
        struct {
            bool     is_default;     /* true for "default:"                 */
            AstNode *type_name;      /* the type (NULL for default)         */
            AstNode *value;          /* the result expression               */
        } generic_assoc;

        /* ── AST_GENERIC_CALL (Phase G) ────────────────────────────────── */
        /* func<int>(a, b)  — explicit generic function call.
         * Replaces the old AST_CAST{TYPE_GENERIC, operand=NULL} hack.
         * name      — owned heap string, function name
         * type_args — type-parameter AST nodes (AST_TYPE_* owned)
         * call_args — value argument expression nodes (owned) */
        struct {
            char   *name;       /* function name, owned                    */
            AstVec  type_args;  /* explicit type arguments <T1,T2,…>       */
            AstVec  call_args;  /* value arguments (expression nodes)      */
        } generic_call;

        /* ── AST_TYPEOF_TYPE ─────────────────────────────────────────── */
        /* __typeof__(expr) as a type specifier.  verbatim is the original
         * spelling (e.g. "__typeof__((-5))") used by cg for faithful output;
         * expr is a lightweight expression AST parsed from the inner expr
         * and used by ty_from_ast to infer the concrete C type. */
        struct {
            char    *verbatim;  /* owned  — "__typeof__((-5))"            */
            AstNode *expr;      /* owned  — parsed inner expression, or NULL */
        } typeof_type;

    } u;
};

/* -------------------------------------------------------------------------
 * Construction
 * ---------------------------------------------------------------------- */

/** Allocate a new node of the given kind at loc.
 *  All pointer fields are NULL, all numeric fields are 0.
 *  The caller fills in u.* fields. */
AstNode *ast_node_new(AstKind kind, CppLoc loc);

/** Recursively free node and all owned children.
 *  Frees char* fields (strdup'd names), AstVec data arrays,
 *  and child AstNode* pointers.  Does NOT free borrowed pointers
 *  (string_lit.text). sem_type and sem_scope are not freed here —
 *  they are owned by TyStore and scope_free_chain respectively. */
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

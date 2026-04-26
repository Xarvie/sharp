/* sharp.h - Sharp compiler, all shared types and declarations.
 *
 * Sharp is C + Rust-style impl + C++-style templates + RAII. Nothing else.
 *
 * Surface summary:
 *   - primitive types: i8..u64, f32/f64, bool, char, void, isize/usize
 *   - struct declarations with impl blocks (static / value-self / ref-self)
 *   - destructors `~T() { ... }` with scope-exit RAII
 *   - generic struct + impl, monomorphized at emit time
 *   - operator overloading via `static R operator<op>(...)`
 *   - raw pointers T* with plain C semantics (subscript included)
 *   - `&e` / `*e` / `->` / postfix `++`/`--`
 *   - expression-body functions (=> expr ;)
 *   - extern C-linkage declarations, built-in print / println
 *
 * Semantics match C's where they overlap:
 *   - Struct assignment, value-parameter passing, and return are memcpy.
 *   - Integer overflow: signed = UB, unsigned = wrap-around (C rules).
 *   - Array subscript is raw-pointer arithmetic. The language provides no
 *     bounds checks and no fat pointers.
 *
 * RAII contract:
 *   - Every local variable (including value parameters) whose type has a
 *     destructor gets that destructor called exactly once, at the point it
 *     leaves its scope (natural `}`, return, break, continue).
 *   - Destructors fire in reverse declaration order.
 *   - Copy / move / return of destructible types is bitwise. If a type
 *     owns a unique resource, the programmer is responsible for not
 *     copying it (use T*, or write a non-copying discipline). Sharp
 *     does not track ownership.
 */
#ifndef SHARP_H
#define SHARP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

/* ===================================================================== *
 *   Arena allocator
 * ===================================================================== */
typedef struct Arena {
    char*           buf;
    size_t          cap;
    size_t          used;
    struct Arena*   next;
} Arena;

void*  arena_alloc  (Arena** a, size_t size);
void*  arena_zalloc (Arena** a, size_t size);
char*  arena_strndup(Arena** a, const char* s, int n);
void   arena_free_all(Arena** a);

#define ARENA_NEW(a, T)         ((T*)arena_zalloc((a), sizeof(T)))
#define ARENA_NEW_ARR(a, T, n)  ((T*)arena_zalloc((a), sizeof(T) * (size_t)(n)))

/* ===================================================================== *
 *   String buffer
 * ===================================================================== */
typedef struct {
    char*   data;
    size_t  len;
    size_t  cap;
} StrBuf;

void sb_init  (StrBuf* sb);
void sb_free  (StrBuf* sb);
void sb_putc  (StrBuf* sb, char c);
void sb_puts  (StrBuf* sb, const char* s);
void sb_putn  (StrBuf* sb, const char* s, int n);
void sb_printf(StrBuf* sb, const char* fmt, ...);

/* ===================================================================== *
 *   Errors & diagnostics
 * ===================================================================== */
extern int  g_error_count;
extern int  g_warning_count;
extern bool g_silent;           /* suppress diagnostics during speculative parsing */
extern int  g_error_limit;      /* abort after this many errors (default: 25) */

/* Register the source file being compiled so diagnostics can print
 * context lines with caret indicators. Both pointers must outlive every
 * diagnostic call (typically: held by main(), freed at exit). */
void diag_set_source(const char* filename, const char* source);

/* Error / warning severity. */
typedef enum { DIAG_ERROR, DIAG_WARNING, DIAG_NOTE } DiagLevel;

/* Diagnostic codes. Allocated in groups:
 *   E1xxx : lexer / parser       (syntax errors)
 *   E2xxx : sema structural      (duplicate struct, unknown type, ...)
 *   E3xxx : sema type-checking   (assignment mismatch, arity, ...)
 *   E4xxx : cgen                 (internal consistency)
 * Each entry maps to a stable short identifier rendered in diagnostics
 * so scripts / editors can filter by code. */
typedef enum {
    E_NONE = 0,

    /* E1xxx — parse-time */
    E_PARSE_GENERIC        = 1000,   /* generic parser message, no code */

    /* E2xxx — sema structural */
    E_DUP_STRUCT           = 2001,
    E_DUP_METHOD           = 2002,
    E_DUP_DTOR             = 2003,
    E_IMPL_UNKNOWN_STRUCT  = 2004,
    E_DTOR_SHAPE           = 2005,
    E_OP_SHAPE             = 2006,
    E_UNKNOWN_TYPE         = 2010,
    E_UNKNOWN_FIELD        = 2011,
    E_UNKNOWN_METHOD       = 2012,
    E_UNKNOWN_IDENT        = 2013,
    E_DUP_LOCAL            = 2014,

    /* E3xxx — sema type-checking */
    E_TYPE_MISMATCH        = 3001,   /* assignment / init */
    E_ARG_COUNT            = 3002,
    E_ARG_TYPE             = 3003,
    E_RETURN_TYPE          = 3004,
    E_COND_NOT_BOOL        = 3005,
    E_BIN_OPERAND_TYPE     = 3006,
    E_NOT_CALLABLE         = 3007,
    E_NOT_ASSIGNABLE       = 3008,
    E_INDEX_TYPE           = 3011,
    E_MEMBER_ON_NON_STRUCT = 3012,

    /* E4xxx — codegen consistency */
    E_CGEN_INTERNAL        = 4001,

    /* E5xxx — HIR / control-flow analysis (Leap D) */
    E_MISSING_RETURN       = 5001    /* non-void function may fall off end  */
} DiagCode;

/* Legacy API kept so old call sites compile unchanged. Internally these
 * now route through diag_emit with E_PARSE_GENERIC. New code should call
 * diag_emit directly with a proper code and column. */
void error_at(int line, const char* fmt, ...);
void fatal   (const char* fmt, ...);

/* Preferred diagnostic entrypoint. `col` may be 0 if unknown.
 * Returns after emitting; caller decides whether to keep parsing. */
void diag_emit(DiagLevel level, DiagCode code,
               int line, int col, int span,
               const char* fmt, ...);

/* ===================================================================== *
 *   Tokens
 * ===================================================================== */
typedef enum {
    TK_EOF = 0,

    /* literals */
    TK_INT, TK_FLOAT, TK_STRING, TK_CHAR_LIT, TK_IDENT,

    /* keywords */
    TK_STRUCT, TK_IMPL, TK_UNION, TK_FN, TK_STATIC, TK_CONST, TK_RETURN,
    TK_IF, TK_ELSE, TK_WHILE, TK_FOR,
    TK_TRUE, TK_FALSE, TK_NULL,
    TK_LET, TK_VAR, TK_UNSAFE,
    TK_ENUM,
    TK_BREAK, TK_CONTINUE,
    TK_SIZEOF, TK_AS,
    TK_OPERATOR,
    TK_EXTERN,       /* extern func_decl with C linkage */
    TK_TYPEDEF,      /* typedef Type name; */
    TK_PRINT,        /* print(expr) built-in */
    TK_PRINTLN,      /* println(expr) built-in */

    /* primitive-type keywords — Sharp style (modern, clean) */
    TK_VOID, TK_BOOL,
    TK_I8,  TK_I16, TK_I32, TK_I64,
    TK_U8,  TK_U16, TK_U32, TK_U64,
    TK_F32, TK_F64, TK_ISIZE, TK_USIZE,

    /* C-style type modifiers */
    TK_SHORT, TK_LONG, TK_UNSIGNED, TK_SIGNED, TK_CHAR,
    TK_INT_TYPE, TK_FLOAT_TYPE, TK_DOUBLE_TYPE,
    TK___INT64,

    /* C declaration modifiers */
    TK___INLINE__, TK___INLINE,
    TK___ATTRIBUTE__,
    TK___DECLSPEC,
    TK_STATIC_ASSERT,

    /* Calling conventions (TDD-2.1~2.4) */
    TK___CDECL, TK___STDCALL, TK___FASTCALL, TK___UNALIGNED,
    TK___RESTRICT, TK___RESTRICT__,
    TK_RESTRICT,

    /* inline assembly */
    TK_ASM,

    /* punctuation */
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE, TK_LBRACKET, TK_RBRACKET,
    TK_COMMA, TK_SEMI, TK_COLON, TK_DCOLON, TK_DOT, TK_ELLIPSIS, TK_ARROW,

    /* arithmetic */
    TK_PLUS, TK_MINUS, TK_STAR, TK_SLASH, TK_PERCENT,
    TK_PLUSPLUS, TK_MINUSMINUS,   /* postfix `++` `--` */

    /* assignment */
    TK_ASSIGN, TK_PLUSEQ, TK_MINUSEQ, TK_STAREQ, TK_SLASHEQ, TK_PERCENTEQ,

    /* comparison */
    TK_EQ, TK_NEQ, TK_LT, TK_GT, TK_LE, TK_GE,

    /* logical */
    TK_AND_AND, TK_OR_OR, TK_NOT,

    /* bitwise */
    TK_AMP, TK_PIPE, TK_CARET, TK_TILDE, TK_SHL, TK_SHR
} TokKind;

typedef struct {
    TokKind       kind;
    const char*   start;   /* pointer into source */
    int           len;
    int           line;
    int           col;
    long long     ival;    /* for TK_INT */
    double        fval;    /* for TK_FLOAT */
} Tok;

/* ===================================================================== *
 *   Types
 * ===================================================================== */
typedef struct Type Type;

typedef enum {
    TY_VOID,
    TY_BOOL,
    TY_I8,  TY_I16, TY_I32, TY_I64,
    TY_U8,  TY_U16, TY_U32, TY_U64,
    TY_F32, TY_F64, TY_ISIZE, TY_USIZE,
    TY_PTR,      /* base pointer */
    TY_NAMED,    /* user-declared struct, resolved by name */
    TY_FUNC,     /* function type: ret(params...) — for function pointers */
    TY_BITFIELD  /* bitfield: base type with width */
} TypeKind;

struct Type {
    TypeKind      kind;
    Type*         base;       /* pointee for TY_PTR / TY_REF / TY_BITFIELD base */
    const char*   name;       /* for TY_NAMED (NUL-terminated) */
    /* Phase 4: type arguments for generic instantiations.
     * For `Stack<i32>`, kind==TY_NAMED, name=="Stack", targs=[i32], ntargs=1.
     * For a type-parameter placeholder inside a generic body, targs is unused
     * and the name holds the parameter identifier (e.g. "T"). */
    Type**        targs;
    int           ntargs;
    /* Const qualifier — applies to the type itself, not transitively
     * into the base. For `const u8*`, the TY_PTR has is_const=false
     * and its base (TY_U8) has is_const=true. For `u8* const`, the
     * TY_PTR itself has is_const=true. */
    bool          is_const;
    /* Phase C: function type parameters (TY_FUNC) */
    Type**        func_params;  /* parameter types for TY_FUNC */
    int           nfunc_params;
    bool          func_variadic;
    /* Phase C: bitfield width (TY_BITFIELD) */
    int           bit_width;
};

Type* type_prim  (Arena** a, TypeKind k);
Type* type_ptr   (Arena** a, Type* base);
Type* type_named (Arena** a, const char* name);  /* name must be arena-owned and NUL-terminated */
Type* type_named_generic(Arena** a, const char* name, Type** targs, int ntargs); /* phase 4 */
Type* type_const (Arena** a, Type* base);        /* const-qualified type */
/* Phase C: function type and bitfield constructors */
Type* type_func  (Arena** a, Type* ret, Type** params, int nparams, bool variadic);
Type* type_bitfield(Arena** a, Type* base, int width);

bool  type_is_primitive (TypeKind k);
bool  type_is_pointerlike(Type* t);   /* TY_PTR only */
bool  ty_assignable(const Type* to, const Type* from);

/* ===================================================================== *
 *   AST
 * ===================================================================== */
typedef struct Node Node;

typedef enum {
    /* top-level */
    ND_PROGRAM, ND_STRUCT_DECL, ND_FUNC_DECL, ND_IMPL, ND_FIELD, ND_PARAM,
    ND_UNION_DECL, ND_STRUCT_FWD, ND_UNION_FWD,
    ND_STATIC_ASSERT,

    /* statements */
    ND_BLOCK, ND_VARDECL, ND_IF, ND_WHILE, ND_FOR,
    ND_RETURN, ND_EXPR_STMT, ND_BREAK, ND_CONTINUE,

    /* expressions */
    ND_INT, ND_FLOAT, ND_STR, ND_CHAR, ND_BOOL, ND_NULL,
    ND_IDENT,
    ND_BINOP, ND_UNOP, ND_ASSIGN,
    ND_CALL, ND_MEMBER,
    ND_STRUCT_LIT, ND_FIELD_INIT,
    ND_INDEX,          /* a[i] — raw pointer subscript */
    /* I/O built-ins */
    ND_PRINT,          /* print(expr)   — writes to stdout, no newline */
    ND_PRINTLN,        /* println(expr) — writes to stdout + '\n' */
    ND_CAST,           /* (Type)expr — C-style type cast */
    ND_SIZEOF,         /* sizeof(Type) or sizeof(expr) — compile-time size */
    ND_EXTERN_DECL,    /* extern return_type name(params); — C linkage decl */
    ND_TYPEDEF_DECL,   /* typedef Type name; — type alias */
    ND_EXTERN_VAR,     /* extern Type name; — external variable decl */
    ND_CONST_DECL,     /* const Type name = expr; — global const */
    /* Explicit RAII. The lowering pass injects one ND_DROP per destructible
     * local at every scope exit (natural `}`, return, break, continue).
     * CGen just emits `T___drop(&name);` — it does not walk any scope stack
     * to figure out who needs dropping. */
    ND_DROP,           /* drop_var_name(n) is the local identifier to drop;
                        * drop_struct_name(n) is the mangled struct/type
                        * used for the  T___drop(&name)  call. */
    ND_ASM,            /* __asm__ stmt — raw_text holds the C asm string */
    /* Phase C: C language compatibility nodes */
    ND_ANON_STRUCT,    /* anonymous struct field (no name) */
    ND_ANON_UNION,     /* anonymous union field (no name) */
    ND_COMPOUND_LIT,   /* (Type){ init_list } — C99 compound literal */
    ND_INIT_LIST,      /* { expr, expr, ... } — initializer list */
    ND_DESIG_INIT      /* .field = expr — designated initializer */
} NodeKind;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_EQ,  OP_NEQ, OP_LT,  OP_GT,  OP_LE,  OP_GE,
    OP_AND, OP_OR,  OP_NOT, OP_NEG, OP_POS,
    OP_BAND, OP_BOR, OP_BXOR, OP_BNOT, OP_SHL, OP_SHR,
    OP_ASSIGN, OP_ADDEQ, OP_SUBEQ, OP_MULEQ, OP_DIVEQ, OP_MODEQ,
    OP_DEREF, OP_ADDR,         /* phase 3: unary `*e` and `&e` */
    OP_POSTINC, OP_POSTDEC     /* phase 4: postfix `e++` / `e--` */
} OpKind;

struct Node {
    NodeKind     kind;
    Type*        type;       /* filled by codegen-time type resolver if needed */
    int          line;

    /* children list (ND_PROGRAM, ND_BLOCK) */
    Node**       children;
    int          nchildren;

    /* binary / unary / assign */
    OpKind       op;
    Node*        lhs;
    Node*        rhs;

    /* literals */
    long long    ival;
    double       fval;
    const char*  sval;       /* for ND_STR (raw, escapes kept) and ND_CHAR */
    int          slen;
    bool         bval;

    /* identifier / member name / call name */
    const char*  name;       /* NUL-terminated */

    /* call */
    Node*        callee;
    Node**       args;
    int          nargs;

    /* if / loops */
    Node*        cond;
    Node*        then_b;
    Node*        else_b;
    Node*        init;       /* for for-init; may be a vardecl or expr-stmt */
    Node*        update;
    Node*        body;

    /* var decl */
    Type*        declared_type;

    /* struct / func decl */
    Node**       fields;     /* ND_FIELD[] */
    int          nfields;
    Node**       params;     /* ND_PARAM[] */
    int          nparams;
    bool         is_variadic;   /* true for extern functions with ... */
    Type*        ret_type;

    /* method metadata (on ND_FUNC_DECL nodes emitted from impl blocks) */
    int          self_kind;      /* SELF_NONE / SELF_VALUE / SELF_REF */
    const char*  parent_type;    /* NULL for free functions; struct name otherwise */

    /* Phase 4: generic type-parameter names, e.g. ["T"] for `struct Stack<T>`.
     * Valid on ND_STRUCT_DECL and ND_IMPL nodes. Empty array / 0 means the
     * declaration is not parametric. */
    const char** type_params;
    int          ntype_params;

    /* Calling convention modifier (e.g. "__cdecl", "__stdcall").
     * Only used on ND_FUNC_DECL / ND_EXTERN_DECL / ND_VARDECL. */
    const char*  cc;

    /* __declspec attribute captured content (e.g. "noreturn", "dllimport").
     * Used on ND_FUNC_DECL / ND_EXTERN_VAR / ND_EXTERN_DECL.
     * For noreturn → C output: __attribute__((noreturn)) before return type.
     * For dllimport → C output: __declspec(dllimport) as-is. */
    const char*  declspec;

    /* Raw text for pass-through constructs (e.g. _Static_assert) */
    const char*  raw_text;

    /* Phase C: bitfield width (on ND_FIELD nodes) */
    int          bit_width;

    /* Phase C: __attribute__ content for pass-through */
    const char*  attribute;
};

/* self-receiver kinds */
enum { SELF_NONE = 0, SELF_VALUE = 1, SELF_REF = 2 };

/* ===================================================================== *
 *   Lexer
 * ===================================================================== */
typedef struct {
    const char*   src;
    const char*   cur;
    int           line;
    int           col;
    const char*   filename;
    Tok           peek0;     /* current token  */
    Tok           peek1;     /* next token     */
} Lexer;

void   lex_init  (Lexer* lx, const char* src, const char* filename);
Tok    lex_peek  (Lexer* lx);
Tok    lex_peek2 (Lexer* lx);
Tok    lex_peek_n(Lexer* lx, int n);
bool   lex_ident_is(Tok t, const char* name);
Tok    lex_next  (Lexer* lx);
const char* tok_name(TokKind k);

/* Phase 4: snapshot / restore the lexer state. Used by parser speculation
 * for constructs whose classification depends on tokens further than the
 * 2-token lookahead reveals (e.g. `Name<T>.foo()` vs `a < b > c`). */
typedef struct {
    const char*  cur;
    int          line;
    int          col;
    Tok          peek0;
    Tok          peek1;
} LexerState;

LexerState lex_save   (Lexer* lx);
void       lex_restore(Lexer* lx, LexerState s);

/* ===================================================================== *
 *   Parser
 * ===================================================================== */
Node* parse_program(Lexer* lx, Arena** arena);

/* ===================================================================== *
 *   Symbol table (built from an AST after parsing)
 * ===================================================================== */
typedef struct {
    const char*   name;     /* field name */
    Type*         type;     /* field type */
} SymField;

typedef struct {
    const char*   name;     /* struct name (arena-owned) */
    Node*         decl;     /* ND_STRUCT_DECL */
    SymField*     fields;
    int           nfields;
    /* Phase 4: if the struct is generic, these mirror decl->type_params.
     * Otherwise ntype_params == 0. */
    const char**  type_params;
    int           ntype_params;
} SymStruct;

typedef struct {
    const char*   parent;   /* parent struct name (arena-owned) */
    const char*   name;     /* method name */
    int           self_kind;/* SELF_NONE / SELF_VALUE / SELF_REF */
    Node*         decl;     /* ND_FUNC_DECL for the method (params already adjusted) */
} SymMethod;

/* Phase 4: a concrete monomorphization, e.g. Stack with <i32>. Collected by
 * sema from every use site and iterated by cgen to emit one specialised
 * struct + impl per (generic_name, targ-tuple). Equality is structural. */
typedef struct {
    const char*   generic_name;   /* arena-owned name of the template struct */
    Type**        targs;          /* arena-owned type argument array */
    int           ntargs;
    const char*   mangled;        /* e.g. "Stack_i32" — arena-owned */
} SymMono;

/* Phase 7: an `extern` function declaration visible to the rest of the
 * program (C linkage; no body). The compiler emits a C extern declaration
 * for it in the generated file so the C compiler knows the signature. */
typedef struct {
    const char*   name;
    Type*         ret_type;
    Node*         decl;     /* ND_EXTERN_DECL node (params live there) */
} SymExtern;

/* Phase 8: every user-defined free function is registered here so sema
 * can type-check calls to them and so expr_type can resolve their return
 * type for nested expressions. Methods live in SymMethod — this table is
 * for non-method, non-extern functions only. */
typedef struct {
    const char*   name;
    Type*         ret_type;
    Node*         decl;     /* ND_FUNC_DECL */
} SymFunc;

typedef struct {
    const char*   name;     /* typedef alias name (arena-owned) */
    Type*         base;     /* the underlying Type it aliases */
    Node*         decl;     /* ND_TYPEDEF_DECL node */
} SymTypedef;

/* Global constant declaration: `const Type name = expr;` */
typedef struct {
    const char*   name;
    Type*         type;
    Node*         value;    /* RHS expression */
    Node*         decl;     /* ND_CONST_DECL node */
} SymConst;

/* External variable declaration: `extern Type name;` */
typedef struct {
    const char*   name;
    Type*         type;
    Node*         decl;     /* ND_EXTERN_VAR node */
} SymValue;

typedef struct {
    SymStruct*    structs;
    int           nstructs;
    SymMethod*    methods;
    int           nmethods;
    SymMono*      monos;          /* phase 4 */
    int           nmonos;
    SymExtern*    externs;        /* phase 7 */
    int           nexterns;
    SymFunc*      funcs;          /* phase 8 */
    int           nfuncs;
    SymTypedef*   typedefs;       /* type aliases from typedef declarations */
    int           ntypedefs;
    SymConst*     consts;         /* global const declarations */
    int           nconsts;
    SymValue*     values;         /* extern variable declarations */
    int           nvalues;
} SymTable;

SymTable*   sema_build       (Node* program, Arena** arena);
SymStruct*  sema_find_struct (SymTable* st, const char* name);
SymMethod*  sema_find_method (SymTable* st, const char* parent, const char* name);
SymField*   sema_find_field  (SymStruct* s, const char* name);
SymMethod*  sema_find_dtor   (SymTable* st, const char* struct_name);  /* phase 3 */
SymExtern*  sema_find_extern (SymTable* st, const char* name);          /* phase 7 */
SymFunc*    sema_find_func   (SymTable* st, const char* name);          /* phase 8 */
SymTypedef* sema_find_typedef(SymTable* st, const char* name);          /* phase 9 */
Type*       sema_resolve_type(SymTable* st, Type* t);                   /* resolve typedef chain */

/* ===================================================================== *
 *   AST Lowering pass (Leap C)
 *
 *   Runs between sema and cgen. Rewrites the AST in place so that cgen
 *   sees a simpler, flatter shape:
 *     - Every scope exit that needs destruction is marked by explicit
 *       ND_DROP nodes. cgen no longer walks a scope stack to decide
 *       what to destruct.
 *   (A later sub-step will also desugar ND_MATCH and ND_PROPAGATE into
 *    primitive ND_IF / ND_VARDECL / ND_RETURN / ND_BLOCK shapes.)
 *
 *   The lowering pass may allocate new nodes from the same arena used
 *   by parse / sema.
 * ===================================================================== */
void lower_program(Node* program, SymTable* st, Arena** arena);

/* ===================================================================== *
 *   Code generator (C backend)
 * ===================================================================== */
void cgen_c   (Node* program, SymTable* st, FILE* out);
void cgen_buf (Node* program, SymTable* st, StrBuf* sb);

#endif /* SHARP_H */

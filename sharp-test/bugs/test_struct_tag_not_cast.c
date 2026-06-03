/* Test: struct tag name should not be treated as type in (tag_name) context.
 * This tests the fix for the issue where:
 *   struct port_state { int x; };
 *   void f(port_state_t* port_state) { ((void)(port_state)); }
 * would fail because sharpc incorrectly treated (port_state) as a cast.
 *
 * The rule: In Sharp, struct names are auto-added to typedefs (C++ semantics),
 * but for the IDENT-RPAREN cast heuristic, a bare (tag_name) is NOT a cast.
 */

/* Case 1: Basic struct tag name as variable */
void test_basic_struct_tag(void) {
    struct port_state { int x; } ps;
    struct port_state port_state;
    ((void)(port_state));  /* Should NOT be treated as cast */
    ((void)(ps));
}

/* Case 2: Macro expansion with struct tag name */
#define UNUSED_VAR(v) ((void)(v))
void test_macro_with_struct_tag(void) {
    struct my_context { int x; };
    struct my_context my_context;
    UNUSED_VAR(my_context);  /* Macro expands to ((void)(my_context)) */
}

/* Case 3: Function parameter with struct tag name (exact wepoll.c pattern) */
struct port_state3 { int x; };
struct sock_state3 { int y; };
#define unused_var(v) ((void) (v))
void test_wepoll_pattern(struct port_state3* port_state3, struct sock_state3* sock_state3) {
    unused_var(port_state3);
    unused_var(sock_state3);
}

/* Case 4: Enum tag name as variable */
void test_enum_tag_name(void) {
    enum my_status { OK = 0, ERROR = 1 };
    enum my_status my_status;
    ((void)(my_status));  /* Should NOT be treated as cast */
}

/* Case 6: Nested parentheses with struct tag */
void test_nested_parens(void) {
    struct state { int x; };
    struct state state;
    (((void)(state)));  /* Triple parens */
}

/* Case 7: Multiple variables with same prefix */
void test_multiple_vars(void) {
    struct handler { int x; };
    struct handler handler;
    struct handler handler2;
    ((void)(handler));
    ((void)(handler2));
}

/* Case 8: Struct tag in inner scope */
void test_inner_scope(void) {
    struct outer { int x; };
    struct outer outer;
    {
        struct inner { int y; };
        struct inner inner;
        ((void)(outer));
        ((void)(inner));
    }
}

/* Case 9: Pointer variable with struct tag name */
void test_pointer_var(void) {
    struct node { int x; };
    struct node* node;
    ((void)(node));  /* Pointer variable, not struct tag */
}

/* Case 10: Array variable with struct tag name */
void test_array_var(void) {
    struct buffer { int x; };
    struct buffer buffer[10];
    ((void)(buffer));  /* Array variable */
}

int main(void) {
    test_basic_struct_tag();
    test_macro_with_struct_tag();
    test_wepoll_pattern(0, 0);
    test_enum_tag_name();
    test_nested_parens();
    test_multiple_vars();
    test_inner_scope();
    test_pointer_var();
    test_array_var();
    return 0;
}

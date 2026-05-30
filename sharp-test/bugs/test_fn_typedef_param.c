/*
 * test_fn_typedef_param.c — Bug: function-type typedef used as parameter
 * does not decay to function pointer in sema.
 *
 * Root cause: sema_expr's SYM_PARAM and SYM_VAR cases returned TY_FUNC
 * directly from ty_from_ast without applying C 6.7.6.3p8 function-to-
 * pointer decay.  This caused "if condition must be scalar" errors when
 * the parameter was used in boolean contexts (if/while/ternary), and
 * prevented correct type-checking for calls through the parameter.
 *
 * Fix: after ty_from_ast resolves a parameter/variable type, if the
 * result is TY_FUNC, wrap it in TY_PTR to match C's adjustment rules.
 */

/* === Case 1: Original bug — typedef char *(NameTransform)(const char *) === */
typedef char *(NameTransform)(const char *name);

int test_fn_typedef_param(NameTransform ntf, const char *name) {
    if (ntf) {
        const char *filtered = ntf(name);
        return filtered != 0;
    }
    return 0;
}

/* === Case 2: Simple function type typedef as parameter === */
typedef int BinaryOp(int a, int b);

int test_simple_fn_typedef(BinaryOp op, int x, int y) {
    if (op) {
        return op(x, y);
    }
    return 0;
}

/* === Case 3: Function type typedef in while condition === */
typedef void Callback(int event);

int test_while_fn_typedef(Callback cb, int ev) {
    while (cb) {
        cb(ev);
        break;
    }
    return 0;
}

/* === Case 4: Function type typedef in ternary condition === */
typedef double MathFn(double);

double test_ternary_fn_typedef(MathFn f, double x) {
    return f ? f(x) : 0.0;
}

/* === Case 5: Function type typedef in comparison === */
typedef int Comparator(const void *a, const void *b);

int test_cmp_fn_typedef(Comparator cmp1, Comparator cmp2) {
    return cmp1 != cmp2;
}

/* === Case 6: Function type typedef assigned to function pointer === */
typedef unsigned long HashFn(const char *s);

unsigned long test_assign_to_fnptr(HashFn hf, const char *key) {
    unsigned long (*fp)(const char *) = hf;
    return fp ? fp(key) : 0;
}

/* === Case 7: Void-returning function type typedef === */
typedef void Action(const char *msg);

int test_void_fn_typedef(Action act, const char *msg) {
    if (act) {
        act(msg);
        return 1;
    }
    return 0;
}

/* === Case 8: Function type typedef with no params === */
typedef int Getter(void);

int test_noparam_fn_typedef(Getter g) {
    if (g) {
        return g();
    }
    return -1;
}

/* === Case 9: Function type typedef with pointer return === */
typedef void *Allocator(unsigned int size);

int test_ptr_ret_fn_typedef(Allocator alloc, unsigned int sz) {
    if (alloc) {
        void *p = alloc(sz);
        return p != 0;
    }
    return 0;
}

/* === Case 10: Multiple function-type typedef params === */
typedef int Transform(int);
typedef int Predicate(int);

int test_multi_fn_typedef(Transform t, Predicate p, int val) {
    if (t && p) {
        return p(t(val));
    }
    return 0;
}

/* === Case 11: Function type typedef in for-loop condition === */
typedef int Condition(int);

int test_for_fn_typedef(Condition cond, int start) {
    int count = 0;
    for (int i = start; cond && cond(i); i++) {
        count++;
        if (count > 100) break;
    }
    return count;
}

/* === Case 12: Function type typedef in negation context === */
typedef int Evaluator(int);

int test_negation_fn_typedef(Evaluator ev, int val) {
    if (!ev) {
        return -1;
    }
    return ev(val);
}

/* === Case 13: Function type typedef with array parameter === */
typedef int Processor(int items[], int count);

int test_array_param_fn_typedef(Processor proc, int *items, int n) {
    if (proc) {
        return proc(items, n);
    }
    return 0;
}

/* === Case 14: Function type typedef called multiple times === */
typedef int Accumulator(int state, int value);

int test_multi_call_fn_typedef(Accumulator acc, int *vals, int n) {
    int state = 0;
    if (acc) {
        for (int i = 0; i < n; i++) {
            state = acc(state, vals[i]);
        }
    }
    return state;
}

/* === Case 15: Function type typedef in return expression === */
typedef long Reducer(long acc, int val);

long test_return_call_fn_typedef(Reducer r, long init, int v) {
    return r ? r(init, v) : init;
}

/* === Case 16: Function type typedef with unsigned params === */
typedef unsigned int HashFunc(const char *key, unsigned int seed);

unsigned int test_unsigned_fn_typedef(HashFunc hf, const char *k, unsigned int s) {
    if (hf) {
        return hf(k, s);
    }
    return 0;
}

int main(void) {
    return 0;
}

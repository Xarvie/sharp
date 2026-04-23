// examples/raii.sp - Phase 3 smoke test.
//
// Exercises every Phase-3 feature:
//   - ~T() destructor in an impl block, with ref-self field access inside.
//   - Destructors firing in reverse declaration order at the natural `}` of
//     a nested block.
//   - Destructors firing on an early `return` (value evaluated before the
//     dtors run, so the returned number reflects pre-destruction state).
//   - Destructors firing on `break` inside a `for`, including the for-init
//     variable's dtor when the init itself declares a destructible local.
//   - Binary operator overloading: Vec3 + Vec3, Vec3 * i32.
//   - Unary operator overloading: -Vec3 (as operator!).
//   - Unary address-of (&log) to let a dtor poke a caller-owned counter.
//
// Expected exit code: 154. See the bottom of the file for the breakdown.

struct Log {
    i32 sum;
    i32 hits;
}

struct T {
    i32  id;
    Log* log;
}

impl T {
    static T make(i32 id, Log* log) { return T { id: id, log: log }; }

    // ref-self destructor: `id` and `log` are fields of the parent struct
    // and so resolve to `self->id` / `self->log` in the generated C.
    ~T() {
        log->sum  = log->sum + id;
        log->hits = log->hits + 1;
    }
}

struct Vec3 {
    i32 x;
    i32 y;
    i32 z;
}

impl Vec3 {
    static Vec3 new(i32 x, i32 y, i32 z) { return Vec3 { x, y, z }; }

    // binary operator overloads (both are static, 2 args)
    static Vec3 operator+(Vec3 a, Vec3 b)
        { return Vec3.new(a.x + b.x, a.y + b.y, a.z + b.z); }

    static Vec3 operator*(Vec3 v, i32 k)
        { return Vec3.new(v.x * k, v.y * k, v.z * k); }

    // unary operator overload (static, 1 arg). We use `!` here because `-`
    // is reserved for binary subtract in phase 3.
    static Vec3 operator!(Vec3 v)
        { return Vec3.new(-v.x, -v.y, -v.z); }

    i32 sum() { return x + y + z; }
}

// Returns early while three trackers are live. The dtors must fire in
// reverse declaration order before control leaves `run_return`.
// Adds 1 + 10 + 100 = 111 to log.sum ; hits += 3.
i32 run_return(Log* log) {
    T t1 = T.make(1,   log);
    T t2 = T.make(10,  log);
    T t3 = T.make(100, log);
    return t1.id + t2.id + t3.id;   // 111; dtors run AFTER this is stashed.
}

// `break` out of a for-loop whose init declares a destructible local.
// Adds 1000 (from the "outer" tracker declared in for-init) plus the
// tracker built at the iteration where break fires (id = 50).
// hits += 2.
i32 run_break(Log* log) {
    for (T outer = T.make(1000, log); outer.id > 0; outer.id = outer.id - 1) {
        T inner = T.make(50, log);
        if (inner.id == 50) {
            break;   // dtors: inner, then outer (for-init unwind).
        }
    }
    return 0;
}

i32 main() {
    Log log = Log { sum: 0, hits: 0 };

    // 1. Scope-end dtors (reverse order): 100 + 10 + 1 = 111, hits += 3.
    {
        T a = T.make(1,   &log);
        T b = T.make(10,  &log);
        T c = T.make(100, &log);
    }

    // 2. Early return while trackers are live: returns 1+10+100 = 111.
    //    The return itself is verified: the dtors must run AFTER the value
    //    is stashed into __sharp_ret, so we get back 111 (pre-destruction).
    i32 early_ret = run_return(&log);

    // 3. break unwinding. Returns 0 but has side effects on log.
    i32 break_ret = run_break(&log);

    // 4. Operator overloading. (u + v) * 2 = (10, 14, 18); !w = (-10, -14, -18).
    //    Sums are 42 and -42; s = 0. nw.x = -10 confirms the unary overload.
    Vec3 u  = Vec3.new(1, 2, 3);
    Vec3 v  = Vec3.new(4, 5, 6);
    Vec3 w  = (u + v) * 2;
    Vec3 nw = !w;
    i32 s   = w.sum() + nw.sum();         // 0 (exact cancellation)
    i32 nwx = nw.x;                        // -10

    // Expected final state of `log`:
    //   sum  = 111 + 111 + 1050 = 1272
    //   hits = 3   + 3   + 2    = 8
    //
    // Exit code must fit in a byte. Encode every signal into the answer so
    // that ANY off-by-one in the dtor machinery would change the result:
    //
    //   log.hits      = 8                 (all dtors fired)
    //   early_ret     = 111               (return value stashed BEFORE dtors)
    //   break_ret     = 0                 (for loop ran, broke out cleanly)
    //   -nwx          = 10                (unary operator overload dispatched)
    //   s             = 0                 (binary + and * operator overload)
    //
    // 8 + 111 + 0 + 10 + s + 25 = 154.
    return log.hits + early_ret + break_ret + (-nwx) + s + 25;
}

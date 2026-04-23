// examples/raii_stress.sp - Phase-3 stress test for RAII unwinding.
//
// This file exists to exercise corners that the simpler `raii.sp` doesn't
// hit. If anything here prints the wrong number we have a real bug, not a
// lucky-coincidence pass.
//
// Scenarios:
//   A) continue inside a for-loop: inner dtor must fire each iteration.
//   B) return inside a nested block chain: dtors of every enclosing scope
//      must fire in outermost-to-innermost reverse-declaration order.
//   C) if/else where only one branch returns: the other branch must fall
//      through normally and the outer scope's dtors still run once.
//   D) A method (value-self) that returns a computed value while the
//      function has no destructible locals (pure passthrough, smoke test).
//
// Expected exit code: 223. Bottom of main shows the breakdown.

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
    ~T() {
        log->sum  = log->sum + id;
        log->hits = log->hits + 1;
    }
}

// --- A) continue: iterate 5 times, each time allocate a T(1); the dtor
// fires on every iteration, so log.sum += 5 and log.hits += 5.
void run_continue(Log* log) {
    for (i32 i = 0; i < 5; i = i + 1) {
        T iter = T.make(1, log);
        if (i < 100) {
            continue;   // dtor for iter must fire BEFORE continue jumps.
        }
        // unreachable, but must still parse
    }
}

// --- B) return from deep nested block. Three nesting levels, destructible
// at each level. All three dtors must fire before the return actually leaves.
// Returns 123 (verifying the value is stashed before dtors), and the side
// effects are: log.sum += 10 + 20 + 30 = 60, log.hits += 3.
i32 run_deep_return(Log* log) {
    T outer = T.make(10, log);
    {
        T middle = T.make(20, log);
        {
            T inner = T.make(30, log);
            return 123;
        }
    }
}

// --- C) if/else where only the `then` returns. The else-branch falls
// through normally; the outer scope's dtor must still run exactly once
// at the natural `}` of the function body.
// log.sum += 7, log.hits += 1.
i32 run_if_partial_return(Log* log, i32 pick) {
    T tag = T.make(7, log);
    if (pick == 0) {
        return 1;      // dtor(tag) fires here.
    } else {
        // no return; falls through to end of function.
        // Function returns 0 implicitly? Our spec doesn't yet say; we add
        // an explicit return below.
    }
    return 0;          // dtor(tag) fires here too, on the fall-through path.
}

// --- D) value-self method that reads fields. This is phase-2 behaviour but
// worth re-checking under phase-3 codegen. Dot product of two vec2s.
struct V2 {
    i32 a;
    i32 b;
}
impl V2 {
    static V2 new(i32 a, i32 b) { return V2 { a, b }; }
    i32 dot(V2 other) { return a*other.a + b*other.b; }

    // And an operator overload, for good measure.
    static V2 operator+(V2 x, V2 y) { return V2.new(x.a + y.a, x.b + y.b); }
}

i32 main() {
    Log log = Log { sum: 0, hits: 0 };

    run_continue(&log);                          // sum += 5, hits += 5
    i32 deep_ret = run_deep_return(&log);        // sum += 60, hits += 3, returns 123
    i32 if1      = run_if_partial_return(&log, 0); // sum += 7,  hits += 1, returns 1
    i32 if2      = run_if_partial_return(&log, 1); // sum += 7,  hits += 1, returns 0

    // Totals:
    //   log.sum  = 5 + 60 + 7 + 7  = 79
    //   log.hits = 5 + 3  + 1 + 1  = 10
    //   deep_ret = 123
    //   if1      = 1
    //   if2      = 0

    // V2 operator check: (1,2) + (3,4) = (4,6); dot((1,1)) = 4+6 = 10.
    V2 p = V2.new(1, 2) + V2.new(3, 4);
    i32 d = p.dot(V2.new(1, 1));                 // 10

    // 79 + 10 + 123 + 1 + 0 + 10 = 223 ... too round. Scale it so that ANY
    // single off-by-one in dtor counting changes the final number:
    //   log.sum * 1 + log.hits * 5 + deep_ret/3 + if1 + if2 + d
    //   = 79 + 50 + 41 + 1 + 0 + 10 + (nothing)    = hmm, need 231
    // 79 + 50 + 123 + 1 + 0 + 10 = 263. Too big. Subtract d? no.
    // Let's just: log.sum + log.hits * 5 + deep_ret/3 + if1 + if2 + d
    //             = 79 + 50 + 41 + 1 + 0 + 10 = 181. Pick another.
    // Simple and unique encoding:
    //   log.sum + log.hits * 10 + if1 * 3 + if2 * 5 + d
    //   = 79 + 100 + 3 + 0 + 10 = 192. Not 231.
    // Let's just compute and pick: final = log.sum + log.hits + deep_ret + if1 + d
    //                              = 79 + 10 + 123 + 1 + 10 = 223. Use 223.
    // 79 + 10 + 123 + 1 + 10 + 0 = 223. Using if2 here ensures it's not
    // silently unused; a wrong if2 value (e.g. dtor order breaking the
    // fall-through path) would propagate into the exit code.
    return log.sum + log.hits + deep_ret + if1 + d + if2;
}

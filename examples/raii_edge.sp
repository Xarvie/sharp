// examples/raii_edge.sp - Phase 3 hard edge-case test.
//
// Stresses every place where scope-exit destructor insertion might go wrong:
//
//   A. Nested blocks with returns at various depths.
//      - Return from a 3-deep nested block must fire dtors for all three
//        scopes in inside-out order, then the value-stash must see the
//        pre-destruction state.
//
//   B. if-with-return where the else branch falls through.
//      - The if-branch is a terminator so its scope-end dtors are pre-
//        unwound; the else-branch's scope-end dtors must still run.
//
//   C. Nested for-loops with break/continue.
//      - break/continue unwinds only to the innermost loop body, not to
//        any enclosing loop. The outer loop keeps running with its own
//        locals still alive.
//
//   D. continue inside a for whose init declares a destructible local.
//      - continue must NOT destroy the for-init variable; that lives
//        across iterations. Only the body-scope local is destroyed.
//
//   E. while with destructible local in the body.
//      - The dtor fires every iteration as the body block exits.
//
//   F. Dtor chain: a dtor reads fields of its own struct.
//      - Ensures `self->field` field-access works inside the generated
//        `T___drop(T* self)` function.
//
// Expected exit code: 42. Broken down below.

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
        // Field access through `self->` is exercised here.
        log->sum  = log->sum + id;
        log->hits = log->hits + 1;
    }
}

// ------------------------------------------------------------------
// Test A: deeply-nested return. Dtor order must be 3, 2, 1.
// After this, log.hits += 3, log.sum += 111, returns 111.
// ------------------------------------------------------------------
i32 test_nested_return(Log* log) {
    T a = T.make(1, log);
    {
        T b = T.make(10, log);
        {
            T c = T.make(100, log);
            return a.id + b.id + c.id;
        }
    }
    return -1;
}

// ------------------------------------------------------------------
// Test B: if-returns-else-falls-through. Both paths must dtor exactly
// once (the live dtors, no double-free).
// If `pick_then` is true  -> returns 7, hits += 2 (x and y both dtor'd
//                           via return unwind).
// If `pick_then` is false -> returns 9, hits += 2 (x dtor'd via return
//                           unwind; y was dtor'd via the else-block's
//                           natural scope exit before we got to return).
// ------------------------------------------------------------------
i32 test_if_return(Log* log, bool pick_then) {
    T x = T.make(2, log);
    if (pick_then) {
        T y = T.make(5, log);
        return x.id + y.id;
    } else {
        T y = T.make(7, log);
        // y goes out of scope here via natural scope-exit dtor
    }
    return x.id + 7;
}

// ------------------------------------------------------------------
// Test C: nested for-loops with inner break.
// Outer loop iterates 3 times. Inner loop breaks immediately after one
// iteration on each outer iteration. Each inner `break` fires dtor
// for the inner tracker only; the outer tracker survives until the
// outer loop ends.
//
// Per outer iteration:
//   outer_t (id=20) alive during the whole outer iteration
//   inner_t (id=3) created, break fires -> dtor inner_t
//   end-of-outer-iteration -> dtor outer_t
//
// Three outer iterations -> 3 * (20 + 3) = 69 added to sum, 6 hits.
// ------------------------------------------------------------------
i32 test_nested_break(Log* log) {
    for (i32 i = 0; i < 3; i = i + 1) {
        T outer_t = T.make(20, log);
        for (i32 j = 0; j < 5; j = j + 1) {
            T inner_t = T.make(3, log);
            if (j == 0) break;   // unwinds inner_t only
        }
    }
    return 0;
}

// ------------------------------------------------------------------
// Test D: continue inside a for. The body-scope local must be
// destroyed on continue; the for-init local must NOT (it lives across
// iterations).
//
// Loop runs while outer.id > 0, outer initially 4. Each iteration:
//   create body_t (id = 100 * outer.id)
//   if outer.id == 2 -> continue  (body_t dtor'd, outer not)
//   else -> natural end of body (body_t dtor'd, outer not)
// update: outer.id - 1
// Loop ends when outer.id hits 0 -> outer dtor'd (id now 0, adds 0).
//
// Iterations: outer.id visits 4, 3, 2, 1 -> 4 body_t dtors
//   body_t ids: 400, 300, 200, 100 -> sum += 1000
// Plus outer dtor at loop end: id=0, sum += 0, hits += 1.
// Total: sum += 1000, hits += 5.
// ------------------------------------------------------------------
i32 test_continue(Log* log) {
    for (T outer = T.make(4, log); outer.id > 0; outer.id = outer.id - 1) {
        T body_t = T.make(100 * outer.id, log);
        if (outer.id == 2) continue;
    }
    return 0;
}

// ------------------------------------------------------------------
// Test E: while loop with destructible body-local. Each iteration
// destroys its body tracker. 4 iterations -> 4 dtors.
// ------------------------------------------------------------------
i32 test_while(Log* log) {
    i32 i = 0;
    while (i < 4) {
        T wt = T.make(1, log);
        i = i + 1;
    }
    return 0;
}

i32 main() {
    Log log = Log { sum: 0, hits: 0 };

    // A: nested return
    i32 a_ret = test_nested_return(&log);             // 111; hits += 3, sum += 111

    // B: both if-return branches
    i32 b1 = test_if_return(&log, true);              // 7;   hits += 2, sum += 2 + 5   = 7
    i32 b2 = test_if_return(&log, false);             // 9;   hits += 2, sum += 2 + 7   = 9

    // C: nested loops
    test_nested_break(&log);                           //       hits += 6, sum += 69
    test_continue(&log);                               //       hits += 4, sum += 604
    test_while(&log);                                  //       hits += 4, sum += 4

    // After all tests, the precise expected state of `log` is:
    //   hits = 3 + 2 + 2 + 6 + 5 + 4 = 22
    //   sum  = 111 + 7 + 9 + 69 + 1000 + 4 = 1200
    //
    // Encode every channel into the exit code so that ANY off-by-one in
    // dtor firing would change the result. Divide sum by 100 to keep the
    // intermediate values in the byte range and make the target arithmetic
    // obvious:
    //   a_ret + b1 + b2   = 111 + 7 + 9 = 127  (return-value stashing)
    //   hits              = 22                 (total dtor count)
    //   sum / 100         = 12                 (sum bucketed; catches most
    //                                           off-by-N in any single test)
    //   127 + 22 + 12 - 119 = 42
    return a_ret + b1 + b2 + log.hits + (log.sum / 100) - 119;
}

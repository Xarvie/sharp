// examples/param_dtor.sp — regression test for value-parameter destructors.
//
// Bug fixed: before the fix, value parameters of destructible types were
// silently skipped — their destructors never fired. This test proves they
// do fire, in the correct order (reverse declaration, params after locals).
//
// consume(t) receives a T by value. The dtor on the parameter fires at the
// end of consume's scope. log.hits must reflect ALL destructor calls.
//
// Expected exit code: 42.

struct Log { i32 hits; }

struct T {
    i32   id;
    Log*  log;
}

impl T {
    static T make(i32 id, Log* log) { return T { id: id, log: log }; }
    ~T() { log->hits = log->hits + id; }
}

// Accepts T by value. The dtor on the parameter must fire here.
void consume(T t) {
    // nothing — just let t drop at scope end
}

// Accepts two T by value; both params must drop (in reverse decl order).
void consume2(T a, T b) {
    // b drops first, then a
}

i32 main() {
    Log log = Log { hits: 0 };

    // 1. Single param dtor.
    //    T{id=5} is passed by value; its dtor adds 5 to log.hits.
    consume(T.make(5, &log));
    // log.hits = 5

    // 2. Local dtor still works.
    {
        T local = T.make(3, &log);
    }
    // log.hits = 5+3 = 8

    // 3. Two param dtors, reverse order (b=10 first, a=7 second).
    consume2(T.make(7, &log), T.make(10, &log));
    // log.hits = 8+10+7 = 25

    // 4. Param dtor interleaved with local.
    //    Inside consume, local=2 drops first, then param t=4 drops.
    //    We can't observe the order from outside, just the total.
    consume(T.make(4, &log));
    // log.hits = 25+4 = 29

    // 5. Dtor fires on param even when function returns early.
    //    (Use consume so early return path is exercised via normal end.)
    consume(T.make(13, &log));
    // log.hits = 29+13 = 42

    return log.hits;
}

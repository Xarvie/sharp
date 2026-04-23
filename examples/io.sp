// examples/io.sp - Phase 7 smoke test.
//
// Exercises Phase-7 I/O and extern features:
//   1. extern func declaration  — brings C stdlib into scope.
//   2. print(i32)               — built-in: printf("%d", v).
//   3. println(i32)             — built-in: printf("%d\n", v).
//   4. print(bool)              — built-in: "true"/"false".
//   5. println(bool)
//   6. Extern call              — direct call to declared C function.
//   7. Interaction with control flow and RAII.
//
// The test writes to stdout which the harness redirects to /dev/null;
// correctness is verified purely through the exit code.
//
// Expected exit code: 99.
//   sum of visible results = 40 + 30 + 20 + 9 = 99

extern i32 abs(i32 n);

struct Box { i32 val; }
impl Box {
    static Box make(i32 v) { return Box { val: v }; }
    ~Box() { }
}

// A function that uses print/println inside RAII scope.
i32 compute(i32 x) {
    Box b = Box.make(x * 2);
    println(b.val);           // prints x*2 then newline
    return b.val;
}

i32 main() {
    // 1. Basic print / println with integers.
    print(42);               // "42"
    println(0);              // "0\n"

    // 2. Bool printing.
    println(true);           // "true\n"
    println(false);          // "false\n"

    // 3. Expressions.
    i32 a = 40;
    i32 b = 30;
    println(a);              // "40\n"
    println(b);              // "30\n"

    // 4. extern call.
    i32 neg = -20;
    i32 pos = abs(neg);      // 20 via C stdlib abs()
    println(pos);            // "20\n"

    // 5. print/println inside functions with RAII.
    i32 c = compute(10);     // prints "20\n", returns 20; Box dtor fires

    // 6. Conditional print.
    i32 x = 5;
    if (x > 3) {
        println(x);          // "5\n"
    }

    // 7. Loop with print.
    i32 sum = 0;
    for (i32 k = 0; k < 3; k++) {
        sum = sum + k;
    }
    println(sum);            // "3\n"

    // exit = a + b + pos + sum + x + c - 19
    //      = 40 + 30 + 20 + 3 + 5 + 20 - 19 = 99
    return a + b + pos + sum + x + c - 19;
}

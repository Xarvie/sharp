// examples/hello.sp -- phase 1 smoke test.
// Exercises structs, functions, recursion, loops, conditionals, and arithmetic.

struct Point {
    i32 x;
    i32 y;
}

i32 factorial(i32 n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

i32 fib(i32 n) {
    if (n < 2) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

i32 sum_to(i32 n) {
    i32 total = 0;
    i32 i = 1;
    while (i <= n) {
        total = total + i;
        i = i + 1;
    }
    return total;
}

i32 count_even(i32 n) {
    i32 c = 0;
    for (i32 i = 0; i < n; i = i + 1) {
        if (i % 2 == 0) {
            c = c + 1;
        }
    }
    return c;
}

// exit code is used to report the "answer" back to the shell
i32 main() {
    i32 a = factorial(5);       //   120
    i32 b = fib(8);             //    21
    i32 c = sum_to(10);         //    55
    i32 d = count_even(10);     //     5
    i32 result = a - b - c - d; //   120-21-55-5 = 39
    return result;
}

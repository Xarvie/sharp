[[nodiscard]] int must_use(void) { return 42; }

[[deprecated("use must_use instead")]] int old_func(void) { return 0; }

[[maybe_unused]] static int unused_var = 10;

int switch_test(int x) {
    switch (x) {
        case 1: return 1;
        case 2: [[fallthrough]];
        case 3: return 3;
    }
    return 0;
}

[[noreturn]] void fatal(void) { while(1); }

int main(void) {
    must_use();
    return 0;
}

// compiler: gcc
// TDD-§8: GCC 内联汇编 __asm__

int main() {
    int x = 42;
    __asm__ volatile ("nop");
    return x;
}
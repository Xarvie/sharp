/* p131 — static inline function (standalone inline definition) */
static inline int fast(int x) { return x * 2; }
int main(void) { return fast(1) - 2; }

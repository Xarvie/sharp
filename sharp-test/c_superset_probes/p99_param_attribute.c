/* p99 — __attribute__((unused)) on function parameters
 * GCC extension: attributes may follow the declarator in a parameter list.
 * Pattern: `int func(int x __attribute__((unused)))` */
static int
count(int n __attribute__((unused)),
      int base __attribute__((unused)),
      const char *s __attribute__((unused)))
{
    return 0;
}
int main(void) { return count(0, 10, "hello"); }

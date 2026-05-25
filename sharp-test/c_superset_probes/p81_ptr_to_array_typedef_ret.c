/* p81_ptr_to_array_typedef_ret.c
 * Phase R9: function returning pointer-to-array-typedef.
 * Must emit nested declarator form:  base_type (*fn(args))[N]
 * Not base_type **fn(args) (wrong pointer depth / dereference semantics). */
typedef int IntPair[2];

static int storage[2] = {42, 99};

IntPair *get_pair(void) { return (IntPair *)storage; }

int main(void) {
    IntPair *p = get_pair();
    int *arr = *p;   /* dereference IntPair* → int[2] → decays to int* */
    if (arr[0] != 42) return 1;
    if (arr[1] != 99) return 2;
    return 0;
}

/* p119 — __attribute__ after typedef declarator name
 * KNOWN-DIFFER: sharpc drops the __attribute__
 * Expected: typedef int aligned_int __attribute__((aligned(8)));
 * Actual:   typedef int aligned_int;
 */
typedef int aligned_int __attribute__((aligned(8)));
typedef unsigned long aligned_ul __attribute__((aligned(16)));
int main(void) { return sizeof(aligned_int) > 0 ? 0 : 1; }

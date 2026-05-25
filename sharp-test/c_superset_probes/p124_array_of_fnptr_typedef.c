/* p124 — array of function pointer typedef: typedef R (*name[N])(params)
 * KNOWN-DIFFER: sharpc emits reversed declarator: typedef R (*)(params) name[N]
 * Expected: typedef void ( * handlers [ 8 ] ) ( int ) ;
 * Actual:   typedef void ( * ) ( int ) handlers [ 8 ] ;
 */
typedef void (*handlers[8])(int);
typedef int (*ops[4])(void *, int);
int main(void) { return 0; }

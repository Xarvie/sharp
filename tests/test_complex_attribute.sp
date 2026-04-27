/* compiler: gcc */
/* C Feature: Complex __attribute__ on variables */
/* Expected: Parser should handle __attribute__((aligned(16), unused)) */

int x __attribute__((aligned(16), unused));
int y __attribute__((packed));
void* z __attribute__((malloc));

int main() {
    return 0;
}
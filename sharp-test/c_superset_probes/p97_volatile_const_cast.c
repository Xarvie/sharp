/* p97_volatile_const_cast.c
 * Phase R15: cast to `volatile const T *` — volatile before const
 * in type qualifier list. Used by mbedtls constant_time.c. */
int compare(const void *a, const void *b, int n) {
    volatile const unsigned char *A = (volatile const unsigned char *) a;
    volatile const unsigned char *B = (volatile const unsigned char *) b;
    int diff = 0;
    for (int i = 0; i < n; i++) diff |= (int)(A[i] ^ B[i]);
    return diff;
}
int main(void) { return compare("abc", "abc", 3); }

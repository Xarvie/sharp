/* p52_anon_typedef_arr.c — anonymous-tagged typedef + 1-element array + `->`.
 *
 * Real-world pattern: cJSON 1.7.18's `print()` uses
 *     typedef struct { unsigned char *buffer; size_t length; ... } printbuffer;
 *     printbuffer buffer[1];
 *     buffer->buffer = malloc(N);
 *
 * The typedef is anonymous (no struct tag), the local is a 1-element
 * array (so `buffer` decays to `printbuffer*`), and the `->` should
 * resolve fields on the underlying anonymous struct.
 */
typedef struct {
    int x;
    int y;
} pair;

int main(void) {
    pair p[1];
    p->x = 3;
    p->y = 4;
    return p->x + p->y - 7;
}

/* p93_enum_forward.c
 * Phase R14: `enum Tag;` forward declaration (GCC extension used by
 * libevent http-internal.h).  Sharpc must accept it without error, and
 * the later full definition must promote the forward entry. */
enum color;               /* forward declaration */
enum size { SMALL, LARGE }; /* unrelated enum */

enum color { RED, GREEN, BLUE };  /* full definition */

int main(void) {
    enum color c = GREEN;
    enum size  s = LARGE;
    return (c == GREEN && s == LARGE) ? 0 : 1;
}

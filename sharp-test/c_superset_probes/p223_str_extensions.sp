/* p223 — extension methods on std::Str
 *
 * Verifies that users can freely extend std::Str with custom
 * extension methods defined outside the standard library.
 */

#include "../../sharp/std/str.sph"

/* Count occurrences of a byte in the Str. */
isize Str.count_byte(this, char c) const {
    isize n = 0;
    isize i = 0;
    while (i < this->len) {
        if (this->ptr[i] == c) n = n + 1;
        i = i + 1;
    }
    return n;
}

/* Naive substring search: return true if `sub` appears anywhere in `this`. */
bool Str.contains(this, Str sub) const {
    if (sub.len > this->len) return false;
    if (sub.len == 0) return true;
    isize max_start = this->len - sub.len;
    isize start = 0;
    while (start <= max_start) {
        isize j = 0;
        bool match = true;
        while (j < sub.len) {
            if (this->ptr[start + j] != sub.ptr[j]) {
                match = false;
                break;
            }
            j = j + 1;
        }
        if (match) return true;
        start = start + 1;
    }
    return false;
}

int main() {
    Str s = str_from_lit("hello world hello");

    /* --- count_byte --- */
    isize n = s.count_byte('l');
    if (n != 5) return 1;       /* h e l l o _ w o r l d _ h e l l o → 5 'l' */

    isize n2 = s.count_byte('z');
    if (n2 != 0) return 2;      /* no 'z' */

    /* --- contains --- */
    Str world = str_from_lit("world");
    if (!s.contains(world)) return 3;

    Str lo = str_from_lit("lo");
    if (!s.contains(lo)) return 4;   /* "lo" appears twice */

    Str nope = str_from_lit("xyz");
    if (s.contains(nope)) return 5;

    /* empty substring is always contained */
    Str empty = str_from_lit("");
    if (!s.contains(empty)) return 6;

    /* substring equal to the whole string */
    if (!s.contains(s)) return 7;

    /* substring longer than the haystack */
    Str longer = str_from_lit("hello world hello!");
    if (s.contains(longer)) return 8;

    /* built-in method still works alongside custom extensions */
    Str sliced = s.slice(6, 11);
    if (!sliced.eq(str_from_lit("world"))) return 9;

    return 0;
}
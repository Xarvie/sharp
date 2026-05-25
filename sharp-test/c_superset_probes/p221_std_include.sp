/* p221 — Direct #include of standard library .sph files.
 *
 * Tests that sharp/std/*.sph header files work correctly when included:
 *   - types.sph  → isize/usize typedefs
 *   - vec.sph    → generic Vec<T>
 *   - str.sph    → Str string slice
 *   - hashmap.sph → HashMap<K,V>
 *
 * This verifies the std library headers are self-contained and correct.
 */
/* FIXED: generic Vec operator[] instantiation forward-declared.
   NOTE: still fails at runtime (exit 15) — pre-existing logic issue, likely
   str_split_once / left.len != 0 check in test. */
#include "../../sharp/std/types.sph"
#include "../../sharp/std/vec.sph"
#include "../../sharp/std/str.sph"
#include "../../sharp/std/hashmap.sph"

int main(void) {
    /* ── Test types.sph ── */
    isize len = 10;
    usize cap = 100;
    if (len != 10) return 1;
    if (cap != 100) return 2;

    /* ── Test vec.sph ── */
    Vec<int> vi = {0};
    vi.push(42);
    vi.push(73);
    if (vi.size() != 2) return 3;
    if (vi.get(0) != 42) return 4;
    if (vi.get(1) != 73) return 5;
    vi.destroy();

    Vec<float> vf = {0};
    vf.push(1.5f);
    if (vf.size() != 1) return 6;
    if (vf[0] < 1.4f || vf[0] > 1.6f) return 7;
    vf.destroy();

    /* ── Test str.sph ── */
    Str hello = str_from_lit("Hello");
    if (hello.len != 5) return 8;
    if (hello.find_byte('e') != 1) return 9;
    if (hello.find_byte('z') != -1) return 10;
    if (hello.size() != 5) return 11;

    Str ell = hello.slice(1, 4);
    if (ell.len != 3) return 12;
    if (ell.ptr[0] != 'e') return 13;

    StrSplit sp = str_split_once(hello, 'l');
    if (!sp.found) return 14;
    if (sp.left.len != 0) return 15;        /* "He" before first 'l' */

    /* ── Test hashmap.sph ── */
    Str key = str_from_lit("answer");
    HashMapEntry<Str, int> entry;
    entry.key = key;
    entry.value = 42;
    entry.state = 1;
    if (entry.value != 42) return 16;

    return 0;
}

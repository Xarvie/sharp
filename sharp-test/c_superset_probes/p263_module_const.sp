/* p263 — Module with const/constexpr exports
 *
 * Tests importing a module with constants and constexpr values.
 */

/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
#include "p263_const.sph"

int main() {
    if (MAX_SIZE != 256) return 1;
    if (MIN_SIZE != 16)  return 2;
    if (LIMIT != 210)    return 3;

    /* use constexpr in array sizing */
    int arr[MAX_SIZE];
    if (sizeof(arr) != sizeof(int) * 256) return 4;

    /* float const */
    if (PI < 3.13f || PI > 3.15f) return 5;

    return 0;
}
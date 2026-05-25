/* p229 — function pointer fields + extension methods
 *
 * Verifies that structs with function pointer fields work,
 * and extension methods can call through them.
 */

/* ===== struct with callback ===== */
class Button {
    int id;
    void (*on_click)(int id);
};

void Button.click(this) {
    if (this->on_click) this->on_click(this->id);
}

void Button.set_callback(this, void (*cb)(int)) {
    this->on_click = cb;
}

/* ===== free function: return function pointer ===== */
typedef int (*IntUnary)(int);
IntUnary make_doubler() {
    /* returning function pointer — Sharp should support typedef */
    return 0;  /* placeholder: just verify type system accepts it */
}

/* callback implementations */
void cb_a(int id) { /* no-op */ }
void cb_b(int id) { /* no-op */ }

int main() {
    Button b;
    b.id = 1;
    b.set_callback(cb_a);
    /* should not crash — cb_a is no-op */
    b.click();

    b.id = 2;
    b.set_callback(cb_b);
    b.click();

    /* null callback (zero init) */
    Button b2 = {0};
    b2.click();  /* should be safe: checks null before call */

    return 0;
}
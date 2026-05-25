/* p271 — Codegen: const pointer chain in method calls
 *
 * Verifies that const methods generate correct const-qualified this pointers.
 */

class Data {
    int secret;
};

int  Data.get_val(this) const { return this->secret; }
void Data.set_val(this, int v) { this->secret = v; }

/* Takes const ref and calls const method */
int read_val(const class Data* d) {
    return d->get_val();
}

int main() {
    Data d;
    d.set_val(128);
    if (d.get_val() != 128) return 1;

    int v = read_val(&d);
    if (v != 128) return 2;

    return 0;
}
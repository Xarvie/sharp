/* p116 — method chaining: class-returning methods chained */
class Sb {
    int v;
}
Sb Sb.add(this, int x) { this->v = this->v + x; return *this; }
Sb Sb.mul(this, int x) { this->v = this->v * x; return *this; }
int Sb.val(this) { return this->v; }

int main(void) {
    Sb s; s.v = 1;
    if (s.add(3).mul(2).val() != 8) return 1;
    Sb t; t.v = 5;
    if (t.add(1).val() != 6) return 2;
    return 0;
}
/* p278 — Volatile and const qualifier mix
 *
 * Tests that volatile and const qualifiers work together.
 */

class Sensor {
    volatile int reading;
    const    int id;
};

int Sensor.get_reading(this) const { return this->reading; }
int Sensor.get_id(this) const { return this->id; }

int main() {
    Sensor s = {0, 42};

    /* const field */
    if (s.id != 42) return 1;

    /* volatile field read */
    int v1 = s.reading;
    int v2 = s.reading;
    if (v1 != v2) return 2;  /* volatile not changed externally */

    /* const method on const-qualified ptr */
    const Sensor* sp = &s;
    if (sp->get_id() != 42) return 3;

    return 0;
}
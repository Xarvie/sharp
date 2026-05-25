/* p245 — composition-based "inheritance" via extension methods
 *
 * Tests the pattern where a struct embeds another and
 * extension methods delegate to the inner struct's methods,
 * simulating inheritance via composition.
 */

#include <stdbool.h>

/* "base" */
class Animal {
    int age;
};

void Animal.speak(this) const {
    /* no-op base */
}

int Animal.get_age(this) const { return this->age; }
void Animal.set_age(this, int a) { this->age = a; }

/* "derived" via composition */
class Dog {
    Animal base;
    int    bark_count;
};

/* delegate extension methods to the base */
int  Dog.get_age(this) const { return this->base.get_age(); }
void Dog.set_age(this, int a) { this->base.set_age(a); }
void Dog.bark(this) { this->bark_count = this->bark_count + 1; }
int  Dog.barks(this) const { return this->bark_count; }

int main() {
    Dog d = {0};
    d.set_age(5);
    d.bark();
    d.bark();
    d.bark();

    if (d.get_age() != 5) return 1;
    if (d.barks() != 3) return 2;

    return 0;
}
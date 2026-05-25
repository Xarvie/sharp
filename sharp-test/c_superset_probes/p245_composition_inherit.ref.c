

#line 11 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
typedef struct Animal Animal;
struct Animal {
    int age;
};

#line 15 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
void Animal__speak(Animal * this);

#line 19 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
int Animal__get_age(Animal * this);

#line 20 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
void Animal__set_age(Animal * this, int a);

#line 23 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
typedef struct Dog Dog;
struct Dog {
    Animal base;
    int bark_count;
};

#line 29 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
int Dog__get_age(Dog * this);

#line 30 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
void Dog__set_age(Dog * this, int a);

#line 31 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
void Dog__bark(Dog * this);

#line 32 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
int Dog__barks(Dog * this);

#line 15 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
void Animal__speak(Animal * this) {
}

#line 19 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
int Animal__get_age(Animal * this) {
    return this->age;
}

#line 20 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
void Animal__set_age(Animal * this, int a) {
    this->age = a;
}

#line 29 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
int Dog__get_age(Dog * this) {
    return Animal__get_age(&this->base);
}

#line 30 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
void Dog__set_age(Dog * this, int a) {
    Animal__set_age(&this->base, a);
}

#line 31 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
void Dog__bark(Dog * this) {
    this->bark_count = this->bark_count + 1;
}

#line 32 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
int Dog__barks(Dog * this) {
    return this->bark_count;
}

#line 34 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
int main() {

#line 35 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
    Dog d = { 0 };

#line 36 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
    Dog__set_age(&d, 5);

#line 37 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
    Dog__bark(&d);

#line 38 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
    Dog__bark(&d);

#line 39 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
    Dog__bark(&d);

#line 41 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
    if (Dog__get_age(&d) != 5) 
        return 1;

#line 42 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
    if (Dog__barks(&d) != 3) 
        return 2;

#line 44 "sharp-test/c_superset_probes/p245_composition_inherit.sp"
    return 0;
}

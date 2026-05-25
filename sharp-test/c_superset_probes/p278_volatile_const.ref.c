

#line 6 "c_superset_probes/p278_volatile_const.sp"
typedef struct Sensor Sensor;
struct Sensor {
    volatile int reading;
    const int id;
};

#line 11 "c_superset_probes/p278_volatile_const.sp"
int Sensor__get_reading(Sensor * this);

#line 12 "c_superset_probes/p278_volatile_const.sp"
int Sensor__get_id(Sensor * this);

#line 11 "c_superset_probes/p278_volatile_const.sp"
int Sensor__get_reading(Sensor * this) {
    return this->reading;
}

#line 12 "c_superset_probes/p278_volatile_const.sp"
int Sensor__get_id(Sensor * this) {
    return this->id;
}

#line 14 "c_superset_probes/p278_volatile_const.sp"
int main() {

#line 15 "c_superset_probes/p278_volatile_const.sp"
    Sensor s = { 0, 42 };

#line 18 "c_superset_probes/p278_volatile_const.sp"
    if (s.id != 42) 
        return 1;

#line 21 "c_superset_probes/p278_volatile_const.sp"
    int v1 = s.reading;

#line 22 "c_superset_probes/p278_volatile_const.sp"
    int v2 = s.reading;

#line 23 "c_superset_probes/p278_volatile_const.sp"
    if (v1 != v2) 
        return 2;

#line 26 "c_superset_probes/p278_volatile_const.sp"
    const Sensor * sp = &s;

#line 27 "c_superset_probes/p278_volatile_const.sp"
    if (Sensor__get_id(sp) != 42) 
        return 3;

#line 29 "c_superset_probes/p278_volatile_const.sp"
    return 0;
}

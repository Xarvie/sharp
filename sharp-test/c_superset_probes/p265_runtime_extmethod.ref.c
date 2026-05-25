

#line 6 "c_superset_probes/p265_runtime_extmethod.sp"
typedef struct Accumulator Accumulator;
struct Accumulator {
    int total;
};

#line 10 "c_superset_probes/p265_runtime_extmethod.sp"
void Accumulator__reset(Accumulator * this);

#line 11 "c_superset_probes/p265_runtime_extmethod.sp"
void Accumulator__add(Accumulator * this, int n);

#line 12 "c_superset_probes/p265_runtime_extmethod.sp"
int Accumulator__get(Accumulator * this);

#line 10 "c_superset_probes/p265_runtime_extmethod.sp"
void Accumulator__reset(Accumulator * this) {
    this->total = 0;
}

#line 11 "c_superset_probes/p265_runtime_extmethod.sp"
void Accumulator__add(Accumulator * this, int n) {
    this->total = this->total + n;
}

#line 12 "c_superset_probes/p265_runtime_extmethod.sp"
int Accumulator__get(Accumulator * this) {
    return this->total;
}

#line 14 "c_superset_probes/p265_runtime_extmethod.sp"
int main() {

#line 15 "c_superset_probes/p265_runtime_extmethod.sp"
    Accumulator acc;

#line 16 "c_superset_probes/p265_runtime_extmethod.sp"
    Accumulator__reset(&acc);

#line 18 "c_superset_probes/p265_runtime_extmethod.sp"
    Accumulator__add(&acc, 10);

#line 19 "c_superset_probes/p265_runtime_extmethod.sp"
    Accumulator__add(&acc, 20);

#line 20 "c_superset_probes/p265_runtime_extmethod.sp"
    Accumulator__add(&acc, 30);

#line 22 "c_superset_probes/p265_runtime_extmethod.sp"
    if (Accumulator__get(&acc) != 60) 
        return 1;

#line 24 "c_superset_probes/p265_runtime_extmethod.sp"
    Accumulator__reset(&acc);

#line 25 "c_superset_probes/p265_runtime_extmethod.sp"
    if (Accumulator__get(&acc) != 0) 
        return 2;

#line 27 "c_superset_probes/p265_runtime_extmethod.sp"
    Accumulator__add(&acc, 5);

#line 28 "c_superset_probes/p265_runtime_extmethod.sp"
    if (Accumulator__get(&acc) != 5) 
        return 3;

#line 31 "c_superset_probes/p265_runtime_extmethod.sp"
    int sum = 0;

#line 32 "c_superset_probes/p265_runtime_extmethod.sp"
    for (int i = 1; i <= 100; i = i + 1) {

#line 33 "c_superset_probes/p265_runtime_extmethod.sp"
        sum = sum + i;
    }

#line 35 "c_superset_probes/p265_runtime_extmethod.sp"
    if (sum != 5050) 
        return 4;

#line 37 "c_superset_probes/p265_runtime_extmethod.sp"
    return 0;
}

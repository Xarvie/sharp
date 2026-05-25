

#line 9 "sharp-test/c_superset_probes/p237_generic_embed.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 15 "sharp-test/c_superset_probes/p237_generic_embed.sp"

#line 15 "sharp-test/c_superset_probes/p237_generic_embed.sp"
typedef struct Labeled__int Labeled__int;
struct Labeled__int {
    int label;
    Point pos;
};


typedef struct Labeled__float Labeled__float;
struct Labeled__float {
    float label;
    Point pos;
};



#line 21 "sharp-test/c_superset_probes/p237_generic_embed.sp"
void Labeled__int__set_label(Labeled__int * this, int val);

#line 22 "sharp-test/c_superset_probes/p237_generic_embed.sp"
void Labeled__int__set_xy(Labeled__int * this, int x, int y);

#line 20 "sharp-test/c_superset_probes/p237_generic_embed.sp"
int Labeled__int__get_label(Labeled__int * this);

#line 23 "sharp-test/c_superset_probes/p237_generic_embed.sp"
int Labeled__int__pos_x(Labeled__int * this);

#line 21 "sharp-test/c_superset_probes/p237_generic_embed.sp"
void Labeled__float__set_label(Labeled__float * this, float val);

#line 20 "sharp-test/c_superset_probes/p237_generic_embed.sp"
float Labeled__float__get_label(Labeled__float * this);

#line 25 "sharp-test/c_superset_probes/p237_generic_embed.sp"
int main() {

#line 26 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    Labeled__int a = { 0 };

#line 27 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    Labeled__int__set_label(&a, 42);

#line 28 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    Labeled__int__set_xy(&a, 10, 20);

#line 30 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    if (Labeled__int__get_label(&a) != 42) 
        return 1;

#line 31 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    if (Labeled__int__pos_x(&a) != 10) 
        return 2;

#line 34 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    Labeled__float b = { 0 };

#line 35 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    Labeled__float__set_label(&b, 3.14f);

#line 36 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    if (Labeled__float__get_label(&b) < 3.13f || Labeled__float__get_label(&b) > 3.15f) 
        return 3;

#line 38 "sharp-test/c_superset_probes/p237_generic_embed.sp"
    return 0;
}

#line 21 "sharp-test/c_superset_probes/p237_generic_embed.sp"
__attribute__((weak)) void Labeled__int__set_label(Labeled__int * this, int val) {
    this->label = val;
}


#line 22 "sharp-test/c_superset_probes/p237_generic_embed.sp"
__attribute__((weak)) void Labeled__int__set_xy(Labeled__int * this, int x, int y) {
    this->pos.x = x;
    this->pos.y = y;
}


#line 20 "sharp-test/c_superset_probes/p237_generic_embed.sp"
__attribute__((weak)) int Labeled__int__get_label(Labeled__int * this) {
    return this->label;
}


#line 23 "sharp-test/c_superset_probes/p237_generic_embed.sp"
__attribute__((weak)) int Labeled__int__pos_x(Labeled__int * this) {
    return this->pos.x;
}


#line 21 "sharp-test/c_superset_probes/p237_generic_embed.sp"
__attribute__((weak)) void Labeled__float__set_label(Labeled__float * this, float val) {
    this->label = val;
}


#line 20 "sharp-test/c_superset_probes/p237_generic_embed.sp"
__attribute__((weak)) float Labeled__float__get_label(Labeled__float * this) {
    return this->label;
}


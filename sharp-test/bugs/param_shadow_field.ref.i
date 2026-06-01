
#line 1 "sharp-test/bugs/param_shadow_field.ce"
typedef struct Color Color;
struct Color {
    int r;
    int g;
    int b;
};

#line 7 "sharp-test/bugs/param_shadow_field.ce"
void Color__set_rgb(Color * this, int r, int g, int b);

#line 13 "sharp-test/bugs/param_shadow_field.ce"
int Color__sum_rgb(Color * this);

#line 17 "sharp-test/bugs/param_shadow_field.ce"
typedef struct Vec2 Vec2;
struct Vec2 {
    int x;
    int y;
};

#line 22 "sharp-test/bugs/param_shadow_field.ce"
void Vec2__set(Vec2 * this, int x, int y);

#line 27 "sharp-test/bugs/param_shadow_field.ce"
int Vec2__dot(Vec2 * this, int x, int y);

#line 31 "sharp-test/bugs/param_shadow_field.ce"
typedef struct Node Node;
struct Node {
    int val;
    Node * next;
};

#line 36 "sharp-test/bugs/param_shadow_field.ce"
void Node__init(Node * this, int val, Node * next);

#line 41 "sharp-test/bugs/param_shadow_field.ce"
int Node__get_val(Node * this);

#line 7 "sharp-test/bugs/param_shadow_field.ce"
void Color__set_rgb(Color * this, int r, int g, int b) {

#line 8 "sharp-test/bugs/param_shadow_field.ce"
    this->r = r;

#line 9 "sharp-test/bugs/param_shadow_field.ce"
    this->g = g;

#line 10 "sharp-test/bugs/param_shadow_field.ce"
    this->b = b;
}

#line 13 "sharp-test/bugs/param_shadow_field.ce"
int Color__sum_rgb(Color * this) {

#line 14 "sharp-test/bugs/param_shadow_field.ce"
    return this->r + this->g + this->b;
}

#line 22 "sharp-test/bugs/param_shadow_field.ce"
void Vec2__set(Vec2 * this, int x, int y) {

#line 23 "sharp-test/bugs/param_shadow_field.ce"
    this->x = x;

#line 24 "sharp-test/bugs/param_shadow_field.ce"
    this->y = y;
}

#line 27 "sharp-test/bugs/param_shadow_field.ce"
int Vec2__dot(Vec2 * this, int x, int y) {

#line 28 "sharp-test/bugs/param_shadow_field.ce"
    return this->x * x + this->y * y;
}

#line 36 "sharp-test/bugs/param_shadow_field.ce"
void Node__init(Node * this, int val, Node * next) {

#line 37 "sharp-test/bugs/param_shadow_field.ce"
    this->val = val;

#line 38 "sharp-test/bugs/param_shadow_field.ce"
    this->next = next;
}

#line 41 "sharp-test/bugs/param_shadow_field.ce"
int Node__get_val(Node * this) {
    return this->val;
}

#line 43 "sharp-test/bugs/param_shadow_field.ce"
int main(void) {

#line 44 "sharp-test/bugs/param_shadow_field.ce"
    Color c;

#line 45 "sharp-test/bugs/param_shadow_field.ce"
    c.r = 0;
    c.g = 0;
    c.b = 0;

#line 46 "sharp-test/bugs/param_shadow_field.ce"
    Color__set_rgb(&c, 100, 200, 50);

#line 47 "sharp-test/bugs/param_shadow_field.ce"
    if (c.r != 100) 
        return 1;

#line 48 "sharp-test/bugs/param_shadow_field.ce"
    if (c.g != 200) 
        return 2;

#line 49 "sharp-test/bugs/param_shadow_field.ce"
    if (c.b != 50) 
        return 3;

#line 50 "sharp-test/bugs/param_shadow_field.ce"
    if (Color__sum_rgb(&c) != 350) 
        return 4;

#line 52 "sharp-test/bugs/param_shadow_field.ce"
    Vec2 v;

#line 53 "sharp-test/bugs/param_shadow_field.ce"
    v.x = 0;
    v.y = 0;

#line 54 "sharp-test/bugs/param_shadow_field.ce"
    Vec2__set(&v, 3, 7);

#line 55 "sharp-test/bugs/param_shadow_field.ce"
    if (v.x != 3) 
        return 5;

#line 56 "sharp-test/bugs/param_shadow_field.ce"
    if (v.y != 7) 
        return 6;

#line 57 "sharp-test/bugs/param_shadow_field.ce"
    if (Vec2__dot(&v, 2, 5) != 41) 
        return 7;

#line 59 "sharp-test/bugs/param_shadow_field.ce"
    Node a;

#line 60 "sharp-test/bugs/param_shadow_field.ce"
    Node__init(&a, 42, (void *)0);

#line 61 "sharp-test/bugs/param_shadow_field.ce"
    if (Node__get_val(&a) != 42) 
        return 8;

#line 62 "sharp-test/bugs/param_shadow_field.ce"
    Node b;

#line 63 "sharp-test/bugs/param_shadow_field.ce"
    Node__init(&b, 99, &a);

#line 64 "sharp-test/bugs/param_shadow_field.ce"
    if (Node__get_val(&b) != 99) 
        return 9;

#line 65 "sharp-test/bugs/param_shadow_field.ce"
    if (b.next != &a) 
        return 10;

#line 67 "sharp-test/bugs/param_shadow_field.ce"
    return 0;
}

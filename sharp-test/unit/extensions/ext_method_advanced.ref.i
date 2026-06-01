
#line 12 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct IntPtr IntPtr;
struct IntPtr {
    int * ptr;
    long len;
};

#line 17 "sharp-test/unit/extensions/ext_method_advanced.ce"
int IntPtr__at(IntPtr * this, long i);

#line 21 "sharp-test/unit/extensions/ext_method_advanced.ce"
void IntPtr__set(IntPtr * this, long i, int v);

#line 25 "sharp-test/unit/extensions/ext_method_advanced.ce"
long IntPtr__length(IntPtr * this);

#line 29 "sharp-test/unit/extensions/ext_method_advanced.ce"
int IntPtr__sum(IntPtr * this);

#line 36 "sharp-test/unit/extensions/ext_method_advanced.ce"
void IntPtr__reverse(IntPtr * this);

#line 49 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct Color Color;
struct Color {
    int r;
    int g;
    int b;
};

#line 55 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Color__luminance(Color * this);

#line 59 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Color__set_rgb(Color * this, int nr, int ng, int nb);

#line 65 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Color__blend(Color * this, int dr, int dg, int db, int alpha);

#line 71 "sharp-test/unit/extensions/ext_method_advanced.ce"
_Bool Color__equals_rgb(Color * this, int er, int eg, int eb);

#line 76 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 81 "sharp-test/unit/extensions/ext_method_advanced.ce"
Point Point__origin();

#line 88 "sharp-test/unit/extensions/ext_method_advanced.ce"
Point Point__translate(Point * this, int dx, int dy);

#line 95 "sharp-test/unit/extensions/ext_method_advanced.ce"
Point Point__midpoint(Point * this, Point other);

#line 102 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct Line Line;
struct Line {
    Point start;
    Point end;
};

#line 107 "sharp-test/unit/extensions/ext_method_advanced.ce"
Line Line__from_points(Point a, Point b);

#line 114 "sharp-test/unit/extensions/ext_method_advanced.ce"
long Line__length_sq(Line * this);

#line 121 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct Node Node;
struct Node {
    int value;
    Node * next;
};

#line 126 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Node__get_value(Node * this);

#line 130 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Node__set_value(Node * this, int v);

#line 134 "sharp-test/unit/extensions/ext_method_advanced.ce"
Node * Node__get_next(Node * this);

#line 138 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Node__set_next(Node * this, Node * n);

#line 142 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Node__chain_sum(Node * this);

#line 153 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct FluentCounter FluentCounter;
struct FluentCounter {
    int val;
};

#line 157 "sharp-test/unit/extensions/ext_method_advanced.ce"
FluentCounter * FluentCounter__inc(FluentCounter * this);

#line 162 "sharp-test/unit/extensions/ext_method_advanced.ce"
FluentCounter * FluentCounter__add(FluentCounter * this, int n);

#line 167 "sharp-test/unit/extensions/ext_method_advanced.ce"
FluentCounter * FluentCounter__mul(FluentCounter * this, int n);

#line 172 "sharp-test/unit/extensions/ext_method_advanced.ce"
int FluentCounter__get(FluentCounter * this);

#line 176 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct Painter Painter;
struct Painter {
    int x;
    int y;
    int color;
};

#line 182 "sharp-test/unit/extensions/ext_method_advanced.ce"
Painter * Painter__move_to(Painter * this, int mx, int my);

#line 188 "sharp-test/unit/extensions/ext_method_advanced.ce"
Painter * Painter__set_color(Painter * this, int c);

#line 193 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Painter__render(Painter * this);

#line 198 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 225 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 198 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct Container__int Container__int;
struct Container__int {
    int data;
    _Bool valid;
};


typedef struct Container__long Container__long;
struct Container__long {
    long data;
    _Bool valid;
};



#line 225 "sharp-test/unit/extensions/ext_method_advanced.ce"
typedef struct Pair__int Pair__int;
struct Pair__int {
    int first;
    int second;
};



#line 212 "sharp-test/unit/extensions/ext_method_advanced.ce"
_Bool Container__int__is_valid(Container__int * this);

#line 220 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Container__int__get_or(Container__int * this, int default_val);

#line 207 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Container__int__set(Container__int * this, int v);

#line 203 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Container__int__get(Container__int * this);

#line 216 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Container__int__reset(Container__int * this);

#line 207 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Container__long__set(Container__long * this, long v);

#line 203 "sharp-test/unit/extensions/ext_method_advanced.ce"
long Container__long__get(Container__long * this);

#line 230 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Pair__int__get_first(Pair__int * this);

#line 234 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Pair__int__get_second(Pair__int * this);

#line 238 "sharp-test/unit/extensions/ext_method_advanced.ce"
Pair__int Pair__int__swap(Pair__int * this);

#line 17 "sharp-test/unit/extensions/ext_method_advanced.ce"
int IntPtr__at(IntPtr * this, long i) {

#line 18 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->ptr[i];
}

#line 21 "sharp-test/unit/extensions/ext_method_advanced.ce"
void IntPtr__set(IntPtr * this, long i, int v) {

#line 22 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->ptr[i] = v;
}

#line 25 "sharp-test/unit/extensions/ext_method_advanced.ce"
long IntPtr__length(IntPtr * this) {

#line 26 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->len;
}

#line 29 "sharp-test/unit/extensions/ext_method_advanced.ce"
int IntPtr__sum(IntPtr * this) {

#line 30 "sharp-test/unit/extensions/ext_method_advanced.ce"
    int s = 0;

#line 31 "sharp-test/unit/extensions/ext_method_advanced.ce"
    for (long i = 0; i < this->len; i = i + 1) 

#line 32 "sharp-test/unit/extensions/ext_method_advanced.ce"
        s = s + this->ptr[i];

#line 33 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return s;
}

#line 36 "sharp-test/unit/extensions/ext_method_advanced.ce"
void IntPtr__reverse(IntPtr * this) {

#line 37 "sharp-test/unit/extensions/ext_method_advanced.ce"
    long i = 0;

#line 38 "sharp-test/unit/extensions/ext_method_advanced.ce"
    long j = this->len - 1;

#line 39 "sharp-test/unit/extensions/ext_method_advanced.ce"
    while (i < j) {

#line 40 "sharp-test/unit/extensions/ext_method_advanced.ce"
        int tmp = this->ptr[i];

#line 41 "sharp-test/unit/extensions/ext_method_advanced.ce"
        this->ptr[i] = this->ptr[j];

#line 42 "sharp-test/unit/extensions/ext_method_advanced.ce"
        this->ptr[j] = tmp;

#line 43 "sharp-test/unit/extensions/ext_method_advanced.ce"
        i = i + 1;

#line 44 "sharp-test/unit/extensions/ext_method_advanced.ce"
        j = j - 1;
    }
}

#line 55 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Color__luminance(Color * this) {

#line 56 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return (this->r * 299 + this->g * 587 + this->b * 114) / 1000;
}

#line 59 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Color__set_rgb(Color * this, int nr, int ng, int nb) {

#line 60 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->r = nr;

#line 61 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->g = ng;

#line 62 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->b = nb;
}

#line 65 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Color__blend(Color * this, int dr, int dg, int db, int alpha) {

#line 66 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->r = this->r + (dr - this->r) * alpha / 255;

#line 67 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->g = this->g + (dg - this->g) * alpha / 255;

#line 68 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->b = this->b + (db - this->b) * alpha / 255;
}

#line 71 "sharp-test/unit/extensions/ext_method_advanced.ce"
_Bool Color__equals_rgb(Color * this, int er, int eg, int eb) {

#line 72 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->r == er && this->g == eg && this->b == eb;
}

#line 81 "sharp-test/unit/extensions/ext_method_advanced.ce"
Point Point__origin() {

#line 82 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Point p;

#line 83 "sharp-test/unit/extensions/ext_method_advanced.ce"
    p.x = 0;

#line 84 "sharp-test/unit/extensions/ext_method_advanced.ce"
    p.y = 0;

#line 85 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return p;
}

#line 88 "sharp-test/unit/extensions/ext_method_advanced.ce"
Point Point__translate(Point * this, int dx, int dy) {

#line 89 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Point result;

#line 90 "sharp-test/unit/extensions/ext_method_advanced.ce"
    result.x = this->x + dx;

#line 91 "sharp-test/unit/extensions/ext_method_advanced.ce"
    result.y = this->y + dy;

#line 92 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return result;
}

#line 95 "sharp-test/unit/extensions/ext_method_advanced.ce"
Point Point__midpoint(Point * this, Point other) {

#line 96 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Point m;

#line 97 "sharp-test/unit/extensions/ext_method_advanced.ce"
    m.x = (this->x + other.x) / 2;

#line 98 "sharp-test/unit/extensions/ext_method_advanced.ce"
    m.y = (this->y + other.y) / 2;

#line 99 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return m;
}

#line 107 "sharp-test/unit/extensions/ext_method_advanced.ce"
Line Line__from_points(Point a, Point b) {

#line 108 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Line l;

#line 109 "sharp-test/unit/extensions/ext_method_advanced.ce"
    l.start = a;

#line 110 "sharp-test/unit/extensions/ext_method_advanced.ce"
    l.end = b;

#line 111 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return l;
}

#line 114 "sharp-test/unit/extensions/ext_method_advanced.ce"
long Line__length_sq(Line * this) {

#line 115 "sharp-test/unit/extensions/ext_method_advanced.ce"
    long dx = (long)(this->end.x - this->start.x);

#line 116 "sharp-test/unit/extensions/ext_method_advanced.ce"
    long dy = (long)(this->end.y - this->start.y);

#line 117 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return dx * dx + dy * dy;
}

#line 126 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Node__get_value(Node * this) {

#line 127 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->value;
}

#line 130 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Node__set_value(Node * this, int v) {

#line 131 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->value = v;
}

#line 134 "sharp-test/unit/extensions/ext_method_advanced.ce"
Node * Node__get_next(Node * this) {

#line 135 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->next;
}

#line 138 "sharp-test/unit/extensions/ext_method_advanced.ce"
void Node__set_next(Node * this, Node * n) {

#line 139 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->next = n;
}

#line 142 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Node__chain_sum(Node * this) {

#line 143 "sharp-test/unit/extensions/ext_method_advanced.ce"
    int s = 0;

#line 144 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Node * cur = this;

#line 145 "sharp-test/unit/extensions/ext_method_advanced.ce"
    while (cur != (void *)0) {

#line 146 "sharp-test/unit/extensions/ext_method_advanced.ce"
        s = s + Node__get_value(cur);

#line 147 "sharp-test/unit/extensions/ext_method_advanced.ce"
        cur = Node__get_next(cur);
    }

#line 149 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return s;
}

#line 157 "sharp-test/unit/extensions/ext_method_advanced.ce"
FluentCounter * FluentCounter__inc(FluentCounter * this) {

#line 158 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->val = this->val + 1;

#line 159 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this;
}

#line 162 "sharp-test/unit/extensions/ext_method_advanced.ce"
FluentCounter * FluentCounter__add(FluentCounter * this, int n) {

#line 163 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->val = this->val + n;

#line 164 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this;
}

#line 167 "sharp-test/unit/extensions/ext_method_advanced.ce"
FluentCounter * FluentCounter__mul(FluentCounter * this, int n) {

#line 168 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->val = this->val * n;

#line 169 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this;
}

#line 172 "sharp-test/unit/extensions/ext_method_advanced.ce"
int FluentCounter__get(FluentCounter * this) {

#line 173 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->val;
}

#line 182 "sharp-test/unit/extensions/ext_method_advanced.ce"
Painter * Painter__move_to(Painter * this, int mx, int my) {

#line 183 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->x = mx;

#line 184 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->y = my;

#line 185 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this;
}

#line 188 "sharp-test/unit/extensions/ext_method_advanced.ce"
Painter * Painter__set_color(Painter * this, int c) {

#line 189 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->color = c;

#line 190 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this;
}

#line 193 "sharp-test/unit/extensions/ext_method_advanced.ce"
int Painter__render(Painter * this) {

#line 194 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->x + this->y + this->color;
}

#line 203 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 207 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 212 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 216 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 220 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 230 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 234 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 238 "sharp-test/unit/extensions/ext_method_advanced.ce"

#line 245 "sharp-test/unit/extensions/ext_method_advanced.ce"
int main(void) {

#line 247 "sharp-test/unit/extensions/ext_method_advanced.ce"
    int arr[5] = { 10, 20, 30, 40, 50 };

#line 248 "sharp-test/unit/extensions/ext_method_advanced.ce"
    IntPtr ip;

#line 249 "sharp-test/unit/extensions/ext_method_advanced.ce"
    ip.ptr = arr;

#line 250 "sharp-test/unit/extensions/ext_method_advanced.ce"
    ip.len = 5;

#line 251 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (IntPtr__at(&ip, 0) != 10) 
        return 1;

#line 252 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (IntPtr__at(&ip, 4) != 50) 
        return 2;

#line 253 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (IntPtr__length(&ip) != 5) 
        return 3;

#line 254 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (IntPtr__sum(&ip) != 150) 
        return 4;

#line 255 "sharp-test/unit/extensions/ext_method_advanced.ce"
    IntPtr__set(&ip, 2, 99);

#line 256 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (IntPtr__at(&ip, 2) != 99) 
        return 5;

#line 257 "sharp-test/unit/extensions/ext_method_advanced.ce"
    IntPtr__reverse(&ip);

#line 258 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (IntPtr__at(&ip, 0) != 50) 
        return 6;

#line 259 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (IntPtr__at(&ip, 4) != 10) 
        return 7;

#line 262 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Color c;

#line 263 "sharp-test/unit/extensions/ext_method_advanced.ce"
    c.r = 100;
    c.g = 150;
    c.b = 200;

#line 264 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Color__luminance(&c) != 140) 
        return 8;

#line 265 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Color__set_rgb(&c, 255, 128, 0);

#line 266 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (c.r != 255 || c.g != 128 || c.b != 0) 
        return 9;

#line 267 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (!Color__equals_rgb(&c, 255, 128, 0)) 
        return 10;

#line 268 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Color__blend(&c, 0, 0, 255, 128);

#line 269 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Color__equals_rgb(&c, 255, 128, 0)) 
        return 11;

#line 270 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Color c2;

#line 271 "sharp-test/unit/extensions/ext_method_advanced.ce"
    c2.r = 200;
    c2.g = 200;
    c2.b = 200;

#line 272 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Color__blend(&c2, 100, 100, 100, 128);

#line 273 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (c2.r != 150) 
        return 12;

#line 276 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Point p1;

#line 277 "sharp-test/unit/extensions/ext_method_advanced.ce"
    p1.x = 3;
    p1.y = 4;

#line 278 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Point translated = Point__translate(&p1, 10, 20);

#line 279 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (translated.x != 13 || translated.y != 24) 
        return 13;

#line 280 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Point p2;

#line 281 "sharp-test/unit/extensions/ext_method_advanced.ce"
    p2.x = 7;
    p2.y = 10;

#line 282 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Point mid = Point__midpoint(&p1, p2);

#line 283 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (mid.x != 5 || mid.y != 7) 
        return 14;

#line 284 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Point origin = Point__origin();

#line 285 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (origin.x != 0 || origin.y != 0) 
        return 15;

#line 286 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Line ln = Line__from_points(p1, p2);

#line 287 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Line__length_sq(&ln) != 52) 
        return 16;

#line 290 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Node n1;
    n1.value = 10;
    n1.next = (void *)0;

#line 291 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Node n2;
    n2.value = 20;
    n2.next = &n1;

#line 292 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Node n3;
    n3.value = 30;
    n3.next = &n2;

#line 293 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Node__get_value(&n3) != 30) 
        return 17;

#line 294 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Node__set_value(&n3, 33);

#line 295 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Node__get_value(&n3) != 33) 
        return 18;

#line 296 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Node * pn = &n2;

#line 297 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Node__get_value(pn) != 20) 
        return 19;

#line 298 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Node__set_value(pn, 22);

#line 299 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Node__get_value(pn) != 22) 
        return 20;

#line 300 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Node__chain_sum(&n3) != 65) 
        return 21;

#line 301 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Node * cn = &n3;

#line 302 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Node__get_value(cn) != 33) 
        return 22;

#line 305 "sharp-test/unit/extensions/ext_method_advanced.ce"
    FluentCounter fc;

#line 306 "sharp-test/unit/extensions/ext_method_advanced.ce"
    fc.val = 0;

#line 307 "sharp-test/unit/extensions/ext_method_advanced.ce"
    FluentCounter__mul(FluentCounter__add(FluentCounter__inc(&fc), 5), 3);

#line 308 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (FluentCounter__get(&fc) != 18) 
        return 23;

#line 309 "sharp-test/unit/extensions/ext_method_advanced.ce"
    FluentCounter__inc(FluentCounter__inc(FluentCounter__add(&fc, 2)));

#line 310 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (FluentCounter__get(&fc) != 22) 
        return 24;

#line 311 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Painter pt;

#line 312 "sharp-test/unit/extensions/ext_method_advanced.ce"
    pt.x = 0;
    pt.y = 0;
    pt.color = 0;

#line 313 "sharp-test/unit/extensions/ext_method_advanced.ce"
    int rendered = Painter__render(Painter__set_color(Painter__move_to(&pt, 5, 10), 3));

#line 314 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (rendered != 18) 
        return 25;

#line 317 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Container__int ci;

#line 318 "sharp-test/unit/extensions/ext_method_advanced.ce"
    ci.valid = 0;

#line 319 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Container__int__is_valid(&ci)) 
        return 26;

#line 320 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Container__int__get_or(&ci, 42) != 42) 
        return 27;

#line 321 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Container__int__set(&ci, 100);

#line 322 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (!Container__int__is_valid(&ci)) 
        return 28;

#line 323 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Container__int__get(&ci) != 100) 
        return 29;

#line 324 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Container__int__get_or(&ci, 0) != 100) 
        return 30;

#line 325 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Container__int__reset(&ci);

#line 326 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Container__int__is_valid(&ci)) 
        return 31;

#line 328 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Container__long cl;

#line 329 "sharp-test/unit/extensions/ext_method_advanced.ce"
    cl.valid = 0;

#line 330 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Container__long__set(&cl, 999L);

#line 331 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Container__long__get(&cl) != 999L) 
        return 32;

#line 333 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Pair__int pi;

#line 334 "sharp-test/unit/extensions/ext_method_advanced.ce"
    pi.first = 42;

#line 335 "sharp-test/unit/extensions/ext_method_advanced.ce"
    pi.second = 99;

#line 336 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Pair__int__get_first(&pi) != 42) 
        return 33;

#line 337 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (Pair__int__get_second(&pi) != 99) 
        return 34;

#line 338 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Pair__int swapped = Pair__int__swap(&pi);

#line 339 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (swapped.first != 99) 
        return 35;

#line 340 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (swapped.second != 42) 
        return 36;

#line 342 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return 0;
}

#line 212 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) _Bool Container__int__is_valid(Container__int * this) {

#line 213 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->valid;
}


#line 220 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) int Container__int__get_or(Container__int * this, int default_val) {

#line 221 "sharp-test/unit/extensions/ext_method_advanced.ce"
    if (this->valid) 
        return this->data;

#line 222 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return default_val;
}


#line 207 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) void Container__int__set(Container__int * this, int v) {

#line 208 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->data = v;

#line 209 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->valid = 1;
}


#line 203 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) int Container__int__get(Container__int * this) {

#line 204 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->data;
}


#line 216 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) void Container__int__reset(Container__int * this) {

#line 217 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->valid = 0;
}


#line 207 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) void Container__long__set(Container__long * this, long v) {

#line 208 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->data = v;

#line 209 "sharp-test/unit/extensions/ext_method_advanced.ce"
    this->valid = 1;
}


#line 203 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) long Container__long__get(Container__long * this) {

#line 204 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->data;
}


#line 230 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) int Pair__int__get_first(Pair__int * this) {

#line 231 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->first;
}


#line 234 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) int Pair__int__get_second(Pair__int * this) {

#line 235 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return this->second;
}


#line 238 "sharp-test/unit/extensions/ext_method_advanced.ce"
__attribute__((weak)) Pair__int Pair__int__swap(Pair__int * this) {

#line 239 "sharp-test/unit/extensions/ext_method_advanced.ce"
    Pair__int result;

#line 240 "sharp-test/unit/extensions/ext_method_advanced.ce"
    result.first = this->second;

#line 241 "sharp-test/unit/extensions/ext_method_advanced.ce"
    result.second = this->first;

#line 242 "sharp-test/unit/extensions/ext_method_advanced.ce"
    return result;
}


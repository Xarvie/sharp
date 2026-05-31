
#line 7 "sharp-test/unit/extensions/composition.ce"
typedef struct Position Position;
struct Position {
    int x;
    int y;
};

#line 10 "sharp-test/unit/extensions/composition.ce"
void Position__move(Position * this, int dx, int dy);

#line 11 "sharp-test/unit/extensions/composition.ce"
int Position__get_x(Position * this);

#line 12 "sharp-test/unit/extensions/composition.ce"
int Position__get_y(Position * this);

#line 14 "sharp-test/unit/extensions/composition.ce"
typedef struct Size Size;
struct Size {
    int w;
    int h;
};

#line 17 "sharp-test/unit/extensions/composition.ce"
int Size__area(Size * this);

#line 20 "sharp-test/unit/extensions/composition.ce"
typedef struct Entity Entity;
struct Entity {
    Position pos;
    Size size;
};

#line 26 "sharp-test/unit/extensions/composition.ce"
void Entity__move_x(Entity * this, int d);

#line 27 "sharp-test/unit/extensions/composition.ce"
int Entity__area(Entity * this);

#line 30 "sharp-test/unit/extensions/composition.ce"
typedef struct Player Player;
struct Player {
    Position origin;
    int score;
};

#line 34 "sharp-test/unit/extensions/composition.ce"
void Player__reset(Player * this);

#line 39 "sharp-test/unit/extensions/composition.ce"
int Player__read_score(Player * this);

#line 41 "sharp-test/unit/extensions/composition.ce"
typedef struct Transform Transform;
struct Transform {
    Position pos;
    Size size;
    int rotation;
};

#line 46 "sharp-test/unit/extensions/composition.ce"
void Transform__move(Transform * this, int dx, int dy);

#line 47 "sharp-test/unit/extensions/composition.ce"
int Transform__area(Transform * this);

#line 10 "sharp-test/unit/extensions/composition.ce"
void Position__move(Position * this, int dx, int dy) {
    this->x = this->x + dx;
    this->y = this->y + dy;
}

#line 11 "sharp-test/unit/extensions/composition.ce"
int Position__get_x(Position * this) {
    return this->x;
}

#line 12 "sharp-test/unit/extensions/composition.ce"
int Position__get_y(Position * this) {
    return this->y;
}

#line 17 "sharp-test/unit/extensions/composition.ce"
int Size__area(Size * this) {
    return this->w * this->h;
}

#line 26 "sharp-test/unit/extensions/composition.ce"
void Entity__move_x(Entity * this, int d) {
    Position__move(&this->pos, d, 0);
}

#line 27 "sharp-test/unit/extensions/composition.ce"
int Entity__area(Entity * this) {
    return Size__area(&this->size);
}

#line 34 "sharp-test/unit/extensions/composition.ce"
void Player__reset(Player * this) {

#line 35 "sharp-test/unit/extensions/composition.ce"
    Position p = { 0, 0 };

#line 36 "sharp-test/unit/extensions/composition.ce"
    this->origin = p;

#line 37 "sharp-test/unit/extensions/composition.ce"
    this->score = 0;
}

#line 39 "sharp-test/unit/extensions/composition.ce"
int Player__read_score(Player * this) {
    return this->score;
}

#line 46 "sharp-test/unit/extensions/composition.ce"
void Transform__move(Transform * this, int dx, int dy) {
    Position__move(&this->pos, dx, dy);
}

#line 47 "sharp-test/unit/extensions/composition.ce"
int Transform__area(Transform * this) {
    return Size__area(&this->size);
}

#line 49 "sharp-test/unit/extensions/composition.ce"
int main() {

#line 51 "sharp-test/unit/extensions/composition.ce"
    Entity e = { { 1, 2 }, { 3, 4 } };

#line 52 "sharp-test/unit/extensions/composition.ce"
    int area1 = Entity__area(&e);

#line 53 "sharp-test/unit/extensions/composition.ce"
    Entity__move_x(&e, 5);

#line 54 "sharp-test/unit/extensions/composition.ce"
    if (Position__get_x(&e.pos) != 6) 
        return 1;

#line 55 "sharp-test/unit/extensions/composition.ce"
    if (Position__get_y(&e.pos) != 2) 
        return 2;

#line 56 "sharp-test/unit/extensions/composition.ce"
    if (area1 != 12) 
        return 3;

#line 59 "sharp-test/unit/extensions/composition.ce"
    Player pl = { { 10, 20 }, 5 };

#line 60 "sharp-test/unit/extensions/composition.ce"
    int s0 = Player__read_score(&pl);

#line 61 "sharp-test/unit/extensions/composition.ce"
    Player__reset(&pl);

#line 62 "sharp-test/unit/extensions/composition.ce"
    int s1 = Player__read_score(&pl);

#line 63 "sharp-test/unit/extensions/composition.ce"
    if (s0 != 5) 
        return 4;

#line 64 "sharp-test/unit/extensions/composition.ce"
    if (s1 != 0) 
        return 5;

#line 65 "sharp-test/unit/extensions/composition.ce"
    if (Position__get_x(&pl.origin) != 0) 
        return 6;

#line 66 "sharp-test/unit/extensions/composition.ce"
    if (Position__get_y(&pl.origin) != 0) 
        return 7;

#line 69 "sharp-test/unit/extensions/composition.ce"
    Transform t = { { 0, 0 }, { 5, 6 }, 0 };

#line 70 "sharp-test/unit/extensions/composition.ce"
    if (Transform__area(&t) != 30) 
        return 8;

#line 71 "sharp-test/unit/extensions/composition.ce"
    Transform__move(&t, 10, 20);

#line 72 "sharp-test/unit/extensions/composition.ce"
    if (Position__get_x(&t.pos) != 10) 
        return 9;

#line 73 "sharp-test/unit/extensions/composition.ce"
    if (Position__get_y(&t.pos) != 20) 
        return 10;

#line 75 "sharp-test/unit/extensions/composition.ce"
    return 0;
}

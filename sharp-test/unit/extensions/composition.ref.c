
#include <stdlib.h>

#line 7 "unit/extensions/composition.sp"
typedef struct Position Position;
struct Position {
    int x;
    int y;
};

#line 10 "unit/extensions/composition.sp"
void Position__move(Position * this, int dx, int dy);

#line 11 "unit/extensions/composition.sp"
int Position__get_x(Position * this);

#line 12 "unit/extensions/composition.sp"
int Position__get_y(Position * this);

#line 14 "unit/extensions/composition.sp"
typedef struct Size Size;
struct Size {
    int w;
    int h;
};

#line 17 "unit/extensions/composition.sp"
int Size__area(Size * this);

#line 20 "unit/extensions/composition.sp"
typedef struct Entity Entity;
struct Entity {
    Position pos;
    Size size;
};

#line 26 "unit/extensions/composition.sp"
void Entity__move_x(Entity * this, int d);

#line 27 "unit/extensions/composition.sp"
int Entity__area(Entity * this);

#line 30 "unit/extensions/composition.sp"
typedef struct Player Player;
struct Player {
    Position origin;
    int score;
};

#line 34 "unit/extensions/composition.sp"
void Player__reset(Player * this);

#line 39 "unit/extensions/composition.sp"
int Player__read_score(Player * this);

#line 10 "unit/extensions/composition.sp"
void Position__move(Position * this, int dx, int dy) {
    this->x = this->x + dx;
    this->y = this->y + dy;
}

#line 11 "unit/extensions/composition.sp"
int Position__get_x(Position * this) {
    return this->x;
}

#line 12 "unit/extensions/composition.sp"
int Position__get_y(Position * this) {
    return this->y;
}

#line 17 "unit/extensions/composition.sp"
int Size__area(Size * this) {
    return this->w * this->h;
}

#line 26 "unit/extensions/composition.sp"
void Entity__move_x(Entity * this, int d) {
    Position__move(&this->pos, d, 0);
}

#line 27 "unit/extensions/composition.sp"
int Entity__area(Entity * this) {
    return Size__area(&this->size);
}

#line 34 "unit/extensions/composition.sp"
void Player__reset(Player * this) {

#line 35 "unit/extensions/composition.sp"
    Position p = { 0, 0 };

#line 36 "unit/extensions/composition.sp"
    this->origin = p;

#line 37 "unit/extensions/composition.sp"
    this->score = 0;
}

#line 39 "unit/extensions/composition.sp"
int Player__read_score(Player * this) {
    return this->score;
}

#line 41 "unit/extensions/composition.sp"
int main() {

#line 43 "unit/extensions/composition.sp"
    Entity e = { { 1, 2 }, { 3, 4 } };

#line 44 "unit/extensions/composition.sp"
    int area1 = Entity__area(&e);

#line 45 "unit/extensions/composition.sp"
    Entity__move_x(&e, 5);

#line 46 "unit/extensions/composition.sp"
    if (Position__get_x(&e.pos) != 6) 
        return 1;

#line 47 "unit/extensions/composition.sp"
    if (Position__get_y(&e.pos) != 2) 
        return 2;

#line 48 "unit/extensions/composition.sp"
    if (area1 != 12) 
        return 3;

#line 51 "unit/extensions/composition.sp"
    Player pl = { { 10, 20 }, 5 };

#line 52 "unit/extensions/composition.sp"
    int s0 = Player__read_score(&pl);

#line 53 "unit/extensions/composition.sp"
    Player__reset(&pl);

#line 54 "unit/extensions/composition.sp"
    int s1 = Player__read_score(&pl);

#line 55 "unit/extensions/composition.sp"
    if (s0 != 5) 
        return 4;

#line 56 "unit/extensions/composition.sp"
    if (s1 != 0) 
        return 5;

#line 57 "unit/extensions/composition.sp"
    if (Position__get_x(&pl.origin) != 0) 
        return 6;

#line 58 "unit/extensions/composition.sp"
    if (Position__get_y(&pl.origin) != 0) 
        return 7;

#line 60 "unit/extensions/composition.sp"
    return 0;
}

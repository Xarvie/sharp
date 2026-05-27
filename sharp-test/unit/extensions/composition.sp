// 来源: p245_composition_inherit.sp
// 组合(inheritance) + 扩展方法: 嵌套struct/field 组合

#include <stdbool.h>

// Base component - "Animal" pattern
class Position {
    int x, y;
};
void Position.move(this, int dx, int dy) { this->x = this->x + dx; this->y = this->y + dy; }
int  Position.get_x(this) const { return this->x; }
int  Position.get_y(this) const { return this->y; }

class Size {
    int w, h;
};
int Size.area(this) const { return this->w * this->h; }

// "Derived" via composition — Entity embeds Position and Size
class Entity {
    Position pos;
    Size     size;
};

// Extension delegates to sub-objects
void Entity.move_x(this, int d) { this->pos.move(d, 0); }
int  Entity.area(this) const { return this->size.area(); }

// Another composition pattern
class Player {
    Position origin;
    int      score;
};
void Player.reset(this) {
    Position p = {0, 0};
    this->origin = p;
    this->score = 0;
}
int  Player.read_score(this) const { return this->score; }

int main() {
    // --- Entity ---
    Entity e = { {1, 2}, {3, 4} };
    int area1 = e.area();
    e.move_x(5);
    if (e.pos.get_x() != 6) return 1;
    if (e.pos.get_y() != 2) return 2;
    if (area1 != 12) return 3;

    // --- Player ---
    Player pl = { {10, 20}, 5 };
    int s0 = pl.read_score();
    pl.reset();
    int s1 = pl.read_score();
    if (s0 != 5) return 4;
    if (s1 != 0) return 5;
    if (pl.origin.get_x() != 0) return 6;
    if (pl.origin.get_y() != 0) return 7;

    return 0;
}
class BBox {
    long x, y, w, h;
};

/* const method with params */
long BBox.area(this) const { return this->w * this->h; }

/* non-const setter */
void BBox.move(this, long dx, long dy) { this->x = this->x + dx; this->y = this->y + dy; }

/* const getter with this check */
long BBox.at_x(this) const { return this->x; }
long BBox.at_y(this) const { return this->y; }

int main(void) {
    BBox b = { 1, 2, 10, 20 };
    long a = b.area();        /* 200 */
    b.move(3, 4);             /* x=4, y=6 */
    long sx = b.at_x();       /* 4 */
    long sy = b.at_y();       /* 6 */
    return a + sx + sy - 210; /* 200+4+6-210=0 */
}
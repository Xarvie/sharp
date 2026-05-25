struct P { int x; int y; };
int main() { struct P p = (struct P){.x=3, .y=4}; return p.x + p.y - 7; }

struct P { int x; };
int main() { struct P p; struct P* pp = &p; pp->x = 42; return p.x - 42; }

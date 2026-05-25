/* p115 — generic struct literal syntax: Name<T>{ .field = val } */
struct Pair<A, B> { A first; B second; }
int main(void) {
    Pair<int, float> p = Pair<int, float> { .first = 7, .second = 3.14f };
    if (p.first != 7) return 1;
    Pair<int, int> q = Pair<int, int> { .first = 1, .second = 2 };
    if (q.second != 2) return 2;
    return 0;
}

/* p103 — nested generic: Pair<A,B> */
struct Pair<A, B> {
    A first;
    B second;
}

int main(void) {
    Pair<int, int> p;
    p.first  = 10;
    p.second = 20;
    if (p.first  != 10) return 1;
    if (p.second != 20) return 2;

    Pair<int, float> pif;
    pif.first  = 7;
    pif.second = 1.5f;
    if (pif.first != 7) return 3;
    return 0;
}

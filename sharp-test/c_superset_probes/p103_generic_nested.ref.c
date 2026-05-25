typedef struct Pair__int__int Pair__int__int;
struct Pair__int__int {
    int first;
    int second;
};

typedef struct Pair__int__float Pair__int__float;
struct Pair__int__float {
    int first;
    float second;
};

int main(void) {
    Pair__int__int p;
    p.first = 10;
    p.second = 20;
    if (p.first != 10) 
        return 1;
    if (p.second != 20) 
        return 2;
    Pair__int__float pif;
    pif.first = 7;
    pif.second = 1.5f;
    if (pif.first != 7) 
        return 3;
    return 0;
}

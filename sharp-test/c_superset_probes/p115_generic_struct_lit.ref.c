typedef struct Pair__int__float Pair__int__float;
struct Pair__int__float {
    int first;
    float second;
};

typedef struct Pair__int__int Pair__int__int;
struct Pair__int__int {
    int first;
    int second;
};

int main(void) {
    Pair__int__float p = (Pair__int__float){ .first = 7, .second = 3.14f };
    if (p.first != 7) 
        return 1;
    Pair__int__int q = (Pair__int__int){ .first = 1, .second = 2 };
    if (q.second != 2) 
        return 2;
    return 0;
}

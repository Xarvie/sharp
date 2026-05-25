typedef struct Map__int__int Map__int__int;
struct Map__int__int {
    int key;
    int value;
};

typedef struct Map__int__float Map__int__float;
struct Map__int__float {
    int key;
    float value;
};

int main(void) {
    Map__int__int m1;
    m1.key = 1;
    m1.value = 100;
    Map__int__float m2;
    m2.key = 2;
    m2.value = 3.14f;
    if (m1.key != 1) 
        return 1;
    if (m1.value != 100) 
        return 2;
    if (m2.key != 2) 
        return 3;
    return 0;
}

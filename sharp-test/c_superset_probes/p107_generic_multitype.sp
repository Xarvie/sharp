/* p107 — generic with two type params */
struct Map<K, V> {
    K key;
    V value;
}

int main(void) {
    Map<int, int>   m1;
    m1.key   = 1;
    m1.value = 100;

    Map<int, float> m2;
    m2.key   = 2;
    m2.value = 3.14f;

    if (m1.key   != 1)   return 1;
    if (m1.value != 100) return 2;
    if (m2.key   != 2)   return 3;
    return 0;
}

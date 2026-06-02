
#line 1 "sharp-test/c_superset_probes/p21_kr_func.c"
int add(int a, int b) {
    return a + b;
}

#line 2 "sharp-test/c_superset_probes/p21_kr_func.c"
int main() {
    return add(3, 4) - 7;
}

void first<T>(T* a, T* b) {
    (void)a; (void)b;
}

int main(void) {
    int ia = 3, ib = 4;
    /* const int → first__cint */
    first<const int>(&ia, &ib);
    
    float fa = 1.0f, fb = 2.0f;
    /* float → first__float (pointer is in fn signature, not in T) */
    first<float>(&fa, &fb);
    
    return 0;
}

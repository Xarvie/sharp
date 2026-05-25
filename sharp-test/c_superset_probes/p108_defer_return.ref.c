int seq[8];

int seqn = 0;

int counter = 0;

int void_ran = 0;

void mark(int v) {
    seq[seqn++] = v;
}

void bump(void) {
    counter = counter + 1;
}

int snapshot(void) {

    int __sharp_ret = counter;
    
    bump();
    return __sharp_ret;
}

void lifo(void) {
    
    mark(3);
    
    mark(2);
    
    mark(1);
}

void set_void(void) {
    void_ran = 1;
}

void vfunc(void) {
    
    set_void();
}

int main(void) {

    counter = 0;

    int v = snapshot();

    if (v != 0) 
        return 1;

    if (counter != 1) 
        return 2;

    seqn = 0;

    lifo();

    if (seqn != 3) 
        return 3;

    if (seq[0] != 3) 
        return 4;

    if (seq[1] != 2) 
        return 5;

    if (seq[2] != 1) 
        return 6;

    void_ran = 0;

    vfunc();

    if (void_ran != 1) 
        return 7;

    return 0;
}

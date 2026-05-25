

typedef struct Stack__int Stack__int;
struct Stack__int {
    int top;
    int size;
};



int Stack__int__push(Stack__int * this, int val);

int Stack__int__peek(Stack__int * this);

int main(void) {

    Stack__int s;

    s.size = 0;

    Stack__int__push(&s, 42);

    if (Stack__int__peek(&s) != 42) 
        return 1;

    if (s.size != 1) 
        return 2;

    Stack__int__push(&s, 99);

    if (Stack__int__peek(&s) != 99) 
        return 3;

    if (s.size != 2) 
        return 4;

    return 0;
}

__attribute__((weak)) int Stack__int__push(Stack__int * this, int val) {

    this->top = val;

    this->size = this->size + 1;

    return this->size;
}


__attribute__((weak)) int Stack__int__peek(Stack__int * this) {

    return this->top;
}


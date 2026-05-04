struct B { unsigned int a:3; unsigned int b:5; };
int main() { struct B b; b.a = 7; b.b = 31; return (int)(b.a + b.b) - 38; }

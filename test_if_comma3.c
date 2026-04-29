#define COMMA ,

#if 1 COMMA 0
int a = 1;
#else
int a = 0;
#endif

int main() { return a; }

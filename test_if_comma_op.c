/* Simple test: comma operator in #if */
#if (1, 1)
int a = 1;
#else
int a = 0;
#endif

int main() { return a; }

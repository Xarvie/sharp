#define F(...) ,
#define G(x) x

#if defined(G(,))
int a = 1;
#else
int a = 0;
#endif

int main() { return 0; }

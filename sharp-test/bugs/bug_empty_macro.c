#define DEFINE_ENUM_FLAG_OPERATORS(x)
typedef enum { A = 1, B = 2 } MyEnum;
DEFINE_ENUM_FLAG_OPERATORS(MyEnum);
int main(void) { return 0; }

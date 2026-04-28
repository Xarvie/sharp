#define TEST(MAC, ...) MAC ## __VA_ARGS__
return TEST(A, B);

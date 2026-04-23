/* test_c_header.h - C header with macros and function declarations */
#ifndef TEST_C_HEADER_H
#define TEST_C_HEADER_H

#define MAX_VAL 100
#define MIN_VAL 0

/* C function declarations - no extern keyword needed in Sharp */
int c_add(int a, int b);
int c_mul(int a, int b);
void c_nothing(void);

#endif /* TEST_C_HEADER_H */

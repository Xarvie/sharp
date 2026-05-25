/*
 * p160b_pthread_cross.sp — Cross-compile probe with pthread.h
 */

#include <pthread.h>

int main(void) {
    pthread_t tid = 0;
    (void)tid;
    return 0;
}

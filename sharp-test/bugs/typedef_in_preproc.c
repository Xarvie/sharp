#include <stddef.h>
#include <stdint.h>

typedef enum {
    MY_OP_ENCRYPT = 0,
    MY_OP_DECRYPT = 1
} my_operation_t;

typedef struct {
    int (*ecb_func)(void *ctx, my_operation_t mode, const unsigned char *input, unsigned char *output);
    int (*cbc_func)(void *ctx, my_operation_t mode, size_t length, unsigned char *iv, const unsigned char *input, unsigned char *output);
} my_cipher_info_t;

static int aes_crypt_ecb_wrap(void *ctx, my_operation_t operation, const unsigned char *input, unsigned char *output);
static int aes_crypt_cbc_wrap(void *ctx, my_operation_t operation, size_t length, unsigned char *iv, const unsigned char *input, unsigned char *output);

int main(void) {
    return 0;
}

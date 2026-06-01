struct _ACTIVATION_CONTEXT;
typedef struct _ACTIVATION_CONTEXT *PACTIVATION_CONTEXT;

struct _TEB;
typedef struct _TEB *PTEB;

int main(void) {
    PACTIVATION_CONTEXT ctx = 0;
    PTEB teb = 0;
    (void)ctx;
    (void)teb;
    return 0;
}

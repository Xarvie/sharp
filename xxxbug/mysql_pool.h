#ifndef MITMYSQL_POOL_H
#define MITMYSQL_POOL_H

#include "mysql.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mm_pool mm_pool_t;

typedef struct {
    const char *host;
    const char *user;
    const char *passwd;
    const char *db;
    unsigned int port;
    const char *unix_socket;
    unsigned long client_flag;
    unsigned int min_conns;
    unsigned int max_conns;
    unsigned int idle_timeout_sec;
    unsigned int ping_interval_sec;
    unsigned int connect_timeout;
    unsigned int read_timeout;
    unsigned int write_timeout;
    int          ssl_mode;
    const char  *ssl_ca;
    const char  *ssl_capath;
    const char  *ssl_cert;
    const char  *ssl_key;
    const char  *ssl_cipher;
} mm_pool_config_t;

mm_pool_t    *mm_pool_create(const mm_pool_config_t *cfg);
void          mm_pool_destroy(mm_pool_t *pool);

MYSQL        *mm_pool_get(mm_pool_t *pool);
void          mm_pool_put(mm_pool_t *pool, MYSQL *conn);
void          mm_pool_discard(mm_pool_t *pool, MYSQL *conn);

unsigned int  mm_pool_active(mm_pool_t *pool);
unsigned int  mm_pool_idle(mm_pool_t *pool);

#ifdef __cplusplus
}
#endif

#endif

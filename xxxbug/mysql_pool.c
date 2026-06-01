#include "mysql_pool.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

typedef struct pool_entry {
    MYSQL              *conn;
    time_t              last_used;
    struct pool_entry  *next;
} pool_entry_t;

struct mm_pool {
    mm_pool_config_t    cfg;
    pool_entry_t       *idle_list;
    unsigned int        idle_count;
    unsigned int        active_count;
    unsigned int        total_created;
    pthread_mutex_t     lock;
};

static time_t now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec;
}

static MYSQL *create_conn(mm_pool_t *pool) {
    MYSQL *m = mysql_init(NULL);
    if (!m) return NULL;
    if (pool->cfg.connect_timeout) {
        unsigned int v = pool->cfg.connect_timeout;
        mysql_options(m, MYSQL_OPT_CONNECT_TIMEOUT, &v);
    }
    if (pool->cfg.read_timeout) {
        unsigned int v = pool->cfg.read_timeout;
        mysql_options(m, MYSQL_OPT_READ_TIMEOUT, &v);
    }
    if (pool->cfg.write_timeout) {
        unsigned int v = pool->cfg.write_timeout;
        mysql_options(m, MYSQL_OPT_WRITE_TIMEOUT, &v);
    }
    if (pool->cfg.ssl_mode) {
        unsigned int v = (unsigned int)pool->cfg.ssl_mode;
        mysql_options(m, MYSQL_OPT_SSL_MODE, &v);
    }
    if (pool->cfg.ssl_ca)     mysql_options(m, MYSQL_OPT_SSL_CA,     pool->cfg.ssl_ca);
    if (pool->cfg.ssl_capath) mysql_options(m, MYSQL_OPT_SSL_CAPATH, pool->cfg.ssl_capath);
    if (pool->cfg.ssl_cert)   mysql_options(m, MYSQL_OPT_SSL_CERT,   pool->cfg.ssl_cert);
    if (pool->cfg.ssl_key)    mysql_options(m, MYSQL_OPT_SSL_KEY,    pool->cfg.ssl_key);
    if (pool->cfg.ssl_cipher) mysql_options(m, MYSQL_OPT_SSL_CIPHER, pool->cfg.ssl_cipher);
    my_bool reconnect = 1;
    mysql_options(m, MYSQL_OPT_RECONNECT, &reconnect);
    MYSQL *r = mysql_real_connect(m, pool->cfg.host, pool->cfg.user, pool->cfg.passwd,
                                   pool->cfg.db, pool->cfg.port, pool->cfg.unix_socket,
                                   pool->cfg.client_flag);
    if (!r) { mysql_close(m); return NULL; }
    pool->total_created++;
    return m;
}

mm_pool_t *mm_pool_create(const mm_pool_config_t *cfg) {
    if (!cfg) return NULL;
    mm_pool_t *pool = (mm_pool_t*)calloc(1, sizeof(mm_pool_t));
    if (!pool) return NULL;
    pool->cfg = *cfg;
    pthread_mutex_init(&pool->lock, NULL);
    unsigned int min = cfg->min_conns;
    for (unsigned int i = 0; i < min; i++) {
        MYSQL *c = create_conn(pool);
        if (!c) break;
        pool_entry_t *e = (pool_entry_t*)calloc(1, sizeof(pool_entry_t));
        e->conn = c;
        e->last_used = now_sec();
        e->next = pool->idle_list;
        pool->idle_list = e;
        pool->idle_count++;
    }
    return pool;
}

void mm_pool_destroy(mm_pool_t *pool) {
    if (!pool) return;
    pool_entry_t *e = pool->idle_list;
    while (e) {
        pool_entry_t *next = e->next;
        mysql_close(e->conn);
        free(e);
        e = next;
    }
    pool->idle_list = NULL;
    pool->idle_count = 0;
    pthread_mutex_destroy(&pool->lock);
    free(pool);
}

static int conn_alive(MYSQL *m) {
    if (!m || m->fd < 0) return 0;
    return mysql_ping(m) == 0;
}

MYSQL *mm_pool_get(mm_pool_t *pool) {
    if (!pool) return NULL;
    pthread_mutex_lock(&pool->lock);
    unsigned int idle_timeout = pool->cfg.idle_timeout_sec;
    unsigned int ping_interval = pool->cfg.ping_interval_sec;
    while (pool->idle_list) {
        pool_entry_t *e = pool->idle_list;
        pool->idle_list = e->next;
        pool->idle_count--;
        pthread_mutex_unlock(&pool->lock);
        time_t age = now_sec() - e->last_used;
        int ok = 1;
        if (idle_timeout > 0 && (unsigned int)age > idle_timeout) ok = 0;
        if (ok && ping_interval > 0 && (unsigned int)age > ping_interval) {
            if (!conn_alive(e->conn)) ok = 0;
        }
        if (ok && e->conn && e->conn->fd >= 0) {
            MYSQL *c = e->conn;
            free(e);
            pool->active_count++;
            return c;
        }
        mysql_close(e->conn);
        free(e);
        pthread_mutex_lock(&pool->lock);
    }
    if (pool->active_count + pool->idle_count >= pool->cfg.max_conns && pool->cfg.max_conns > 0) {
        pthread_mutex_unlock(&pool->lock);
        return NULL;
    }
    pthread_mutex_unlock(&pool->lock);
    MYSQL *c = create_conn(pool);
    if (c) {
        pthread_mutex_lock(&pool->lock);
        pool->active_count++;
        pthread_mutex_unlock(&pool->lock);
    }
    return c;
}

void mm_pool_put(mm_pool_t *pool, MYSQL *conn) {
    if (!pool || !conn) return;
    pool_entry_t *e = (pool_entry_t*)calloc(1, sizeof(pool_entry_t));
    if (!e) { mysql_close(conn); return; }
    e->conn = conn;
    e->last_used = now_sec();
    pthread_mutex_lock(&pool->lock);
    e->next = pool->idle_list;
    pool->idle_list = e;
    pool->idle_count++;
    pool->active_count--;
    pthread_mutex_unlock(&pool->lock);
}

void mm_pool_discard(mm_pool_t *pool, MYSQL *conn) {
    if (!pool || !conn) return;
    mysql_close(conn);
    pthread_mutex_lock(&pool->lock);
    pool->active_count--;
    pthread_mutex_unlock(&pool->lock);
}

unsigned int mm_pool_active(mm_pool_t *pool) {
    if (!pool) return 0;
    pthread_mutex_lock(&pool->lock);
    unsigned int n = pool->active_count;
    pthread_mutex_unlock(&pool->lock);
    return n;
}

unsigned int mm_pool_idle(mm_pool_t *pool) {
    if (!pool) return 0;
    pthread_mutex_lock(&pool->lock);
    unsigned int n = pool->idle_count;
    pthread_mutex_unlock(&pool->lock);
    return n;
}

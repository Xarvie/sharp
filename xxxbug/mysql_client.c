/*
 * sp-mysql-client - MySQL/MariaDB client library for Sharp ecosystem.
 * Clean-room implementation of the MySQL wire protocol (text + binary).
 */
#include "mysql.h"

volatile int _dbg_fetch_count = 0;
volatile int _dbg_fetch_rc = -999;
volatile int _dbg_row_rc = -999;
#include "sha.h"
#include "transport.h"

#include <zlib.h>
#include <zstd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <poll.h>
#include <fcntl.h>

#define MM_VERSION_STR "mitmysql 0.5.0"
#define SERVER_MORE_RESULTS_EXISTS 0x0008
#define MAX_PACKET     0xFFFFFF

/* ----------------------- internal connection state -----------------------
 * Hangs off MYSQL.st so the public struct stays free of mbedtls types.
 * Owns the live transport and the user's TLS configuration. */
typedef struct {
    mm_transport *trans;
    int           ssl_mode;
    char         *ssl_ca, *ssl_capath, *ssl_cert, *ssl_key, *ssl_cipher;
    char         *ssl_crl, *ssl_crlpath;
    char         *tls_version, *tls_ciphersuites;
    int           tls_active;
    int           free_handle;
    char          cipher_in_use[64];
    uint8_t       saved_scramble[20];
    int           saved_scramble_set;
    int           deprecate_eof;
    int           reconnect;
    char         *saved_host;
    char         *saved_user;
    char         *saved_passwd;
    char         *saved_db;
    unsigned int  saved_port;
    char         *saved_unix_socket;
    unsigned long saved_client_flag;
    int           compress_algo;
    unsigned char comp_seq;
    uint8_t      *comp_buf;
    size_t        comp_buf_len;
    size_t        comp_buf_pos;
    int           reconnect_attempts;
    int           streaming_active;
    my_bool       report_data_truncation;
    char         *plugin_dir;
    char         *default_auth;
    char         *server_public_key;
    my_bool       enable_cleartext_plugin;
    my_bool       can_handle_expired_passwords;
    unsigned long max_allowed_packet;
    unsigned long net_buffer_length;
    unsigned int  retry_count;
    my_bool       optional_resultset_metadata;
    int           ssl_fips_mode;
    char         *compression_algorithms;
    unsigned int  zstd_compression_level;
    char         *load_data_local_dir;
    char         *init_command;
    char         *read_default_file;
    char         *read_default_group;
    char         *shared_memory_base_name;
    unsigned int  protocol;
    my_bool       local_infile;
    my_bool       use_result;
    unsigned int  charsetnr;
    unsigned int  mbmaxlen;
    uint8_t      *session_track[6];
    size_t        session_track_len[6];
    unsigned int  session_track_pos[6];
    char        **conn_attr_keys;
    char        **conn_attr_vals;
    unsigned int  conn_attr_count;
    unsigned int  conn_attr_cap;
} mm_state;

#define ST(m) ((mm_state*)((m)->st))

static unsigned int charset_mbmaxlen(unsigned int nr) {
    switch (nr) {
        case 63: return 1;
        case 8: case 14: case 31: case 47: return 1;
        case 33: case 83: return 3;
        case 45: case 46: case 255: case 256: case 246: return 4;
        case 192: case 193: return 2;
        case 28: case 24: case 1: case 13: case 19: return 2;
        case 248: return 4;
        case 12: case 97: return 3;
        default: return 4;
    }
}

static unsigned int charset_name_to_nr(const char *name) {
    if (!name) return 0;
    if (strcmp(name, "utf8mb4") == 0 || strcmp(name, "utf8mb4_0900_ai_ci") == 0) return 255;
    if (strcmp(name, "utf8mb4_general_ci") == 0) return 45;
    if (strcmp(name, "utf8mb4_bin") == 0) return 46;
    if (strcmp(name, "utf8") == 0 || strcmp(name, "utf8_general_ci") == 0) return 33;
    if (strcmp(name, "utf8_bin") == 0) return 83;
    if (strcmp(name, "utf8mb3_general_ci") == 0) return 33;
    if (strcmp(name, "utf8mb3_bin") == 0) return 83;
    if (strcmp(name, "latin1") == 0 || strcmp(name, "latin1_swedish_ci") == 0) return 8;
    if (strcmp(name, "binary") == 0) return 63;
    if (strcmp(name, "gbk") == 0 || strcmp(name, "gbk_chinese_ci") == 0) return 28;
    if (strcmp(name, "gb2312") == 0 || strcmp(name, "gb2312_chinese_ci") == 0) return 24;
    if (strcmp(name, "gb18030") == 0 || strcmp(name, "gb18030_chinese_ci") == 0) return 248;
    if (strcmp(name, "big5") == 0 || strcmp(name, "big5_chinese_ci") == 0) return 1;
    if (strcmp(name, "sjis") == 0 || strcmp(name, "sjis_japanese_ci") == 0) return 13;
    if (strcmp(name, "euckr") == 0 || strcmp(name, "euckr_korean_ci") == 0) return 19;
    if (strcmp(name, "eucjpms") == 0 || strcmp(name, "eucjpms_japanese_ci") == 0) return 97;
    if (strcmp(name, "ujis") == 0 || strcmp(name, "ujis_japanese_ci") == 0) return 12;
    return 0;
}

/* ----------------------- stmt cache ----------------------- */
#define MM_STMT_CACHE_SIZE 32

typedef struct mm_cache_entry {
    char          *sql;
    size_t         sql_len;
    unsigned long  stmt_id;
    unsigned int   ref_count;
} mm_cache_entry;

typedef struct mm_stmt_cache {
    mm_cache_entry entries[MM_STMT_CACHE_SIZE];
    unsigned int   count;
} mm_stmt_cache;

static mm_stmt_cache *mm_cache_new(void) {
    mm_stmt_cache *c = (mm_stmt_cache*)calloc(1, sizeof(mm_stmt_cache));
    return c;
}

static mm_cache_entry *mm_cache_find(mm_stmt_cache *c, const char *sql, size_t sql_len) {
    if (!c) return NULL;
    for (unsigned int i = 0; i < c->count; i++) {
        if (c->entries[i].sql_len == sql_len && memcmp(c->entries[i].sql, sql, sql_len) == 0)
            return &c->entries[i];
    }
    return NULL;
}

static void mm_cache_put(mm_stmt_cache *c, const char *sql, size_t sql_len, unsigned long stmt_id) {
    if (!c || c->count >= MM_STMT_CACHE_SIZE) return;
    mm_cache_entry *e = &c->entries[c->count++];
    e->sql = (char*)malloc(sql_len + 1);
    if (!e->sql) { c->count--; return; }
    memcpy(e->sql, sql, sql_len); e->sql[sql_len] = 0;
    e->sql_len = sql_len;
    e->stmt_id = stmt_id;
    e->ref_count = 1;
}

static void mm_cache_remove(mm_stmt_cache *c, unsigned long stmt_id) {
    if (!c) return;
    for (unsigned int i = 0; i < c->count; i++) {
        if (c->entries[i].stmt_id == stmt_id) {
            free(c->entries[i].sql);
            c->entries[i] = c->entries[c->count - 1];
            c->count--;
            return;
        }
    }
}

static void mm_cache_free(mm_stmt_cache *c) {
    if (!c) return;
    for (unsigned int i = 0; i < c->count; i++) free(c->entries[i].sql);
    free(c);
}

/* ----------------------- error helpers ----------------------- */
static size_t lenenc_size(unsigned long v) {
    if (v < 251) return 1;
    if (v < 65536) return 3;
    if (v < 16777216) return 4;
    return 9;
}

static size_t lenenc_write(uint8_t *buf, unsigned long v) {
    if (v < 251) { buf[0] = (uint8_t)v; return 1; }
    if (v < 65536) { buf[0] = 0xFC; buf[1] = v & 0xFF; buf[2] = (v >> 8) & 0xFF; return 3; }
    if (v < 16777216) { buf[0] = 0xFD; buf[1] = v & 0xFF; buf[2] = (v >> 8) & 0xFF; buf[3] = (v >> 16) & 0xFF; return 4; }
    buf[0] = 0xFE; memcpy(buf + 1, &v, 8); return 9;
}

static void set_err(MYSQL *m, unsigned int code, const char *state, const char *msg) {
    m->last_errno = code;
    strncpy(m->sqlstate, state ? state : "HY000", sizeof(m->sqlstate) - 1);
    m->sqlstate[sizeof(m->sqlstate) - 1] = 0;
    strncpy(m->last_error, msg ? msg : "", sizeof(m->last_error) - 1);
    m->last_error[sizeof(m->last_error) - 1] = 0;
}
static void clear_err(MYSQL *m) { m->last_errno = 0; m->last_error[0] = 0; strcpy(m->sqlstate, "00000"); }

/* ----------------------- low-level I/O (via transport) ----------------------- */
static int io_read(MYSQL *m, void *buf, size_t n) {
    mm_transport *t = ST(m)->trans;
    if (!t) { set_err(m, 2013, "HY000", "Not connected"); return -1; }
    if (m->read_timeout > 0) {
        struct pollfd pfd;
        pfd.fd = m->fd;
        pfd.events = POLLIN;
        int pr = poll(&pfd, 1, (int)m->read_timeout * 1000);
        if (pr <= 0) { set_err(m, 2013, "HY000", pr == 0 ? "Read timeout" : "Poll error"); return -1; }
    }
    int r = t->read(t, buf, n);
    if (r == 1) { set_err(m, 2013, "HY000", "Lost connection to server during query"); return -1; }
    if (r < 0) { set_err(m, 2013, "HY000", t->errstr ? t->errstr : "I/O error"); return -1; }
    return 0;
}
static int io_write(MYSQL *m, const void *buf, size_t n) {
    mm_transport *t = ST(m)->trans;
    if (!t) { set_err(m, 2006, "HY000", "Server has gone away"); return -1; }
    if (m->write_timeout > 0) {
        struct pollfd pfd;
        pfd.fd = m->fd;
        pfd.events = POLLOUT;
        int pr = poll(&pfd, 1, (int)m->write_timeout * 1000);
        if (pr <= 0) { set_err(m, 2006, "HY000", pr == 0 ? "Write timeout" : "Poll error"); return -1; }
    }
    int w = t->write(t, buf, n);
    if (w == 1) { set_err(m, 2006, "HY000", "Server has gone away"); return -1; }
    if (w < 0) { set_err(m, 2006, "HY000", t->errstr ? t->errstr : "I/O error"); return -1; }
    return 0;
}

/* ----------------------- packet framing ----------------------- */
/* Reads one logical packet (handling 16MB continuation). Returns malloc'd
 * payload in *out (caller frees) and length in *outlen. */
static int read_packet(MYSQL *m, uint8_t **out, size_t *outlen) {
    uint8_t *buf = NULL;
    size_t total = 0;
    for (;;) {
        uint8_t hdr[4];
        if (io_read(m, hdr, 4) < 0) { free(buf); return -1; }
        size_t len = (size_t)hdr[0] | (size_t)hdr[1] << 8 | (size_t)hdr[2] << 16;
        m->seq = (unsigned char)(hdr[3] + 1);
        uint8_t *nb = (uint8_t*)realloc(buf, total + len + 1);
        if (!nb) { free(buf); set_err(m, 2008, "HY000", "Out of memory"); return -1; }
        buf = nb;
        if (len && io_read(m, buf + total, len) < 0) { free(buf); return -1; }
        total += len;
        if (len < MAX_PACKET) break;   /* last fragment */
    }
    if (!buf) { buf = (uint8_t*)malloc(1); if (!buf) { set_err(m,2008,"HY000","Out of memory"); return -1; } }
    buf[total] = 0;
    *out = buf; *outlen = total;
    return 0;
}

static int write_packet(MYSQL *m, const uint8_t *payload, size_t len, unsigned char seq) {
    size_t off = 0;
    unsigned char s = seq;
    while (off < len) {
        size_t chunk = len - off;
        if (chunk > MAX_PACKET) chunk = MAX_PACKET;
        uint8_t hdr[4];
        hdr[0] = (uint8_t)(chunk & 0xFF);
        hdr[1] = (uint8_t)((chunk >> 8) & 0xFF);
        hdr[2] = (uint8_t)((chunk >> 16) & 0xFF);
        hdr[3] = s++;
        if (io_write(m, hdr, 4) < 0) return -1;
        if (chunk && io_write(m, payload + off, chunk) < 0) return -1;
        off += chunk;
    }
    if (len == 0) {
        uint8_t hdr[4] = { 0, 0, 0, s };
        if (io_write(m, hdr, 4) < 0) return -1;
        s++;
    }
    if (off > 0 && (off % MAX_PACKET) == 0) {
        uint8_t hdr[4] = { 0, 0, 0, s };
        if (io_write(m, hdr, 4) < 0) return -1;
        s++;
    }
    m->seq = s;
    return 0;
}

/* ----------------------- compressed packet framing ----------------------- */
/* MySQL compression: after auth OK, every packet is wrapped in a compressed
 * frame. 7-byte header: 3-byte compressed length, 1-byte comp_seq,
 * 3-byte uncompressed length. If uncompressed_length == 0 the payload is
 * stored raw (not compressed — used for small packets where compression
 * would inflate the data). */

static int write_compressed_packet(MYSQL *m, const uint8_t *payload, size_t len, unsigned char seq) {
    mm_state *st = ST(m);
    if (!st->compress_algo) return write_packet(m, payload, len, seq);

    size_t raw_cap = len + 4 * ((len / MAX_PACKET) + 2);
    uint8_t *raw = (uint8_t*)malloc(raw_cap);
    if (!raw) { set_err(m, 2008, "HY000", "Out of memory"); return -1; }

    size_t raw_len = 0;
    size_t off = 0;
    unsigned char s = seq;
    while (off < len) {
        size_t chunk = len - off;
        if (chunk > MAX_PACKET) chunk = MAX_PACKET;
        raw[raw_len++] = (uint8_t)(chunk & 0xFF);
        raw[raw_len++] = (uint8_t)((chunk >> 8) & 0xFF);
        raw[raw_len++] = (uint8_t)((chunk >> 16) & 0xFF);
        raw[raw_len++] = s++;
        memcpy(raw + raw_len, payload + off, chunk);
        raw_len += chunk;
        off += chunk;
    }
    if (len == 0) {
        raw[raw_len++] = 0; raw[raw_len++] = 0; raw[raw_len++] = 0;
        raw[raw_len++] = s++;
    }
    if (off > 0 && (off % MAX_PACKET) == 0) {
        raw[raw_len++] = 0; raw[raw_len++] = 0; raw[raw_len++] = 0;
        raw[raw_len++] = s++;
    }
    m->seq = s;

    size_t comp_cap;
    uint8_t *comp;
    size_t comp_len;

    if (st->compress_algo == 2) {
        comp_cap = (size_t)ZSTD_compressBound(raw_len);
        comp = (uint8_t*)malloc(comp_cap);
        if (!comp) { free(raw); set_err(m, 2008, "HY000", "Out of memory"); return -1; }
        comp_len = ZSTD_compress(comp, comp_cap, raw, raw_len, ZSTD_CLEVEL_DEFAULT);
        if (ZSTD_isError(comp_len)) { free(comp); free(raw); set_err(m, 2008, "HY000", "zstd compress failed"); return -1; }
    } else {
        uLongf cl = compressBound((uLong)raw_len);
        comp = (uint8_t*)malloc(cl);
        if (!comp) { free(raw); set_err(m, 2008, "HY000", "Out of memory"); return -1; }
        int zr = compress2(comp, &cl, raw, (uLong)raw_len, Z_DEFAULT_COMPRESSION);
        if (zr != Z_OK) { free(comp); free(raw); set_err(m, 2008, "HY000", "zlib compress failed"); return -1; }
        comp_len = (size_t)cl;
    }

    unsigned char cseq = st->comp_seq++;

    if (comp_len >= raw_len) {
        free(comp);
        uint8_t chdr[7];
        chdr[0] = (uint8_t)(raw_len & 0xFF);
        chdr[1] = (uint8_t)((raw_len >> 8) & 0xFF);
        chdr[2] = (uint8_t)((raw_len >> 16) & 0xFF);
        chdr[3] = cseq;
        chdr[4] = 0; chdr[5] = 0; chdr[6] = 0;
        if (io_write(m, chdr, 7) < 0) { free(raw); return -1; }
        if (raw_len && io_write(m, raw, raw_len) < 0) { free(raw); return -1; }
        free(raw);
    } else {
        free(raw);
        uint8_t chdr[7];
        chdr[0] = (uint8_t)(comp_len & 0xFF);
        chdr[1] = (uint8_t)((comp_len >> 8) & 0xFF);
        chdr[2] = (uint8_t)((comp_len >> 16) & 0xFF);
        chdr[3] = cseq;
        chdr[4] = (uint8_t)(raw_len & 0xFF);
        chdr[5] = (uint8_t)((raw_len >> 8) & 0xFF);
        chdr[6] = (uint8_t)((raw_len >> 16) & 0xFF);
        if (io_write(m, chdr, 7) < 0) { free(comp); return -1; }
        if (io_write(m, comp, comp_len) < 0) { free(comp); return -1; }
        free(comp);
    }
    return 0;
}

static int read_compressed_packet(MYSQL *m, uint8_t **out, size_t *outlen) {
    mm_state *st = ST(m);
    if (!st->compress_algo) return read_packet(m, out, outlen);

    uint8_t *buf = NULL;
    size_t total = 0;
    for (;;) {
        if (st->comp_buf && st->comp_buf_pos < st->comp_buf_len) {
            size_t avail = st->comp_buf_len - st->comp_buf_pos;
            uint8_t *cb = st->comp_buf + st->comp_buf_pos;
            if (avail < 4) goto refill;
            size_t pkt_len = (size_t)cb[0] | (size_t)cb[1] << 8 | (size_t)cb[2] << 16;
            if (4 + pkt_len > avail) goto refill;
            m->seq = (unsigned char)(cb[3] + 1);
            uint8_t *nb = (uint8_t*)realloc(buf, total + pkt_len + 1);
            if (!nb) { free(buf); set_err(m, 2008, "HY000", "Out of memory"); return -1; }
            buf = nb;
            memcpy(buf + total, cb + 4, pkt_len);
            total += pkt_len;
            st->comp_buf_pos += 4 + pkt_len;
            if (pkt_len < MAX_PACKET) break;
            continue;
        }
refill:
        if (st->comp_buf) { free(st->comp_buf); st->comp_buf = NULL; }
        st->comp_buf_len = 0; st->comp_buf_pos = 0;

        uint8_t chdr[7];
        if (io_read(m, chdr, 7) < 0) { free(buf); return -1; }
        size_t comp_len = (size_t)chdr[0] | (size_t)chdr[1] << 8 | (size_t)chdr[2] << 16;
        st->comp_seq = (unsigned char)(chdr[3] + 1);
        size_t orig_len = (size_t)chdr[4] | (size_t)chdr[5] << 8 | (size_t)chdr[6] << 16;
        unsigned long max_pkt = st->max_allowed_packet ? st->max_allowed_packet : 0x1000000;
        if (comp_len > max_pkt || orig_len > max_pkt) {
            free(buf); set_err(m, 2020, "HY000", "Packet too large"); return -1;
        }

        uint8_t *comp = (uint8_t*)malloc(comp_len ? comp_len : 1);
        if (!comp) { free(buf); set_err(m, 2008, "HY000", "Out of memory"); return -1; }
        if (comp_len && io_read(m, comp, comp_len) < 0) { free(comp); free(buf); return -1; }

        if (orig_len == 0) {
            st->comp_buf = comp;
            st->comp_buf_len = comp_len;
            st->comp_buf_pos = 0;
            continue;
        }

        uint8_t *raw = (uint8_t*)malloc(orig_len + 1);
        if (!raw) { free(comp); free(buf); set_err(m, 2008, "HY000", "Out of memory"); return -1; }
        size_t dest_len;
        if (st->compress_algo == 2) {
            dest_len = ZSTD_decompress(raw, orig_len, comp, comp_len);
            if (ZSTD_isError(dest_len)) { free(raw); free(comp); free(buf); set_err(m, 2008, "HY000", "zstd decompress failed"); return -1; }
        } else {
            uLongf dl = (uLongf)orig_len;
            int zr = uncompress(raw, &dl, comp, (uLong)comp_len);
            if (zr != Z_OK) { free(raw); free(comp); free(buf); set_err(m, 2008, "HY000", "zlib decompress failed"); return -1; }
            dest_len = (size_t)dl;
        }
        free(comp);
        raw[dest_len] = 0;
        st->comp_buf = raw;
        st->comp_buf_len = (size_t)dest_len;
        st->comp_buf_pos = 0;
    }
    if (!buf) { buf = (uint8_t*)malloc(1); if (!buf) { set_err(m,2008,"HY000","Out of memory"); return -1; } }
    buf[total] = 0;
    *out = buf; *outlen = total;
    return 0;
}

/* ----------------------- length-encoded helpers ----------------------- */
/* Decode a length-encoded integer. Advances *p. Sets *is_null for 0xFB. */
static uint64_t lenenc_int(const uint8_t **p, const uint8_t *end, int *is_null) {
    if (is_null) *is_null = 0;
    if (*p >= end) { if (is_null) *is_null = 1; return 0; }
    uint8_t b = **p; (*p)++;
    if (b < 0xFB) return b;
    if (b == 0xFB) { if (is_null) *is_null = 1; return 0; }
    if (b == 0xFC) { if (*p + 2 > end) { if (is_null) *is_null = 1; return 0; } uint64_t v = (uint64_t)(*p)[0] | (uint64_t)(*p)[1] << 8; *p += 2; return v; }
    if (b == 0xFD) { if (*p + 3 > end) { if (is_null) *is_null = 1; return 0; } uint64_t v = (uint64_t)(*p)[0] | (uint64_t)(*p)[1] << 8 |
                                  (uint64_t)(*p)[2] << 16; *p += 3; return v; }
    if (*p + 8 > end) { if (is_null) *is_null = 1; return 0; }
    { uint64_t v = 0; int i; for (i = 0; i < 8; i++) v |= (uint64_t)(*p)[i] << (i*8); *p += 8; return v; }
}

static size_t lenenc_put(uint8_t *buf, uint64_t v) {
    if (v < 0xFB) { buf[0] = (uint8_t)v; return 1; }
    if (v <= 0xFFFF) { buf[0] = 0xFC; buf[1] = (uint8_t)v; buf[2] = (uint8_t)(v>>8); return 3; }
    if (v <= 0xFFFFFF) { buf[0] = 0xFD; buf[1] = (uint8_t)v; buf[2] = (uint8_t)(v>>8); buf[3] = (uint8_t)(v>>16); return 4; }
    buf[0] = 0xFE; int i; for (i = 0; i < 8; i++) buf[1+i] = (uint8_t)(v >> (i*8)); return 9;
}

/* Read a length-encoded string: returns pointer to bytes, sets *slen. */
static const uint8_t *lenenc_str(const uint8_t **p, const uint8_t *end, uint64_t *slen, int *is_null) {
    uint64_t n = lenenc_int(p, end, is_null);
    if (is_null && *is_null) { *slen = 0; return NULL; }
    if (*p + n > end) { *slen = 0; if (is_null) *is_null = 1; return NULL; }
    const uint8_t *s = *p;
    *p += n;
    *slen = n;
    return s;
}
/* ----------------------- OK / ERR / EOF ----------------------- */
static int parse_ok(MYSQL *m, const uint8_t *p, size_t len) {
    const uint8_t *cur = p + 1, *end = p + len;
    m->affected_rows = lenenc_int(&cur, end, NULL);
    m->insert_id = lenenc_int(&cur, end, NULL);
    if (m->server_capabilities & CLIENT_PROTOCOL_41) {
        if (cur + 4 <= end) {
            m->server_status = (unsigned int)cur[0] | (unsigned int)cur[1] << 8;
            m->warning_count = (unsigned int)cur[2] | (unsigned int)cur[3] << 8;
            cur += 4;
        }
    }
    if (cur < end) {
        size_t ilen = lenenc_int(&cur, end, NULL);
        if (cur + ilen <= end) {
            free(m->info);
            m->info = (char*)malloc(ilen + 1);
            if (m->info) {
                memcpy(m->info, cur, ilen);
                m->info[ilen] = 0;
            }
            cur += ilen;
        }
        else cur = end;
        if (m->client_flag & CLIENT_SESSION_TRACK) {
            if (cur < end) {
                for (int i = 0; i < 6; i++) {
                    free(ST(m)->session_track[i]);
                    ST(m)->session_track[i] = NULL;
                    ST(m)->session_track_len[i] = 0;
                    ST(m)->session_track_pos[i] = 0;
                }
                size_t stlen = lenenc_int(&cur, end, NULL);
                const uint8_t *stend = cur + stlen;
                if (stend > end) stend = end;
                while (cur < stend) {
                    if (cur >= stend) break;
                    unsigned int type = (unsigned int)*cur; cur++;
                    size_t dlen = lenenc_int(&cur, stend, NULL);
                    if (cur + dlen > stend) dlen = (size_t)(stend - cur);
                    if (type < 6) {
                        free(ST(m)->session_track[type]);
                        ST(m)->session_track[type] = (uint8_t*)malloc(dlen);
                        if (ST(m)->session_track[type]) {
                            memcpy(ST(m)->session_track[type], cur, dlen);
                            ST(m)->session_track_len[type] = dlen;
                        }
                    }
                    cur += dlen;
                }
            }
        }
    }
    return 0;
}
static int parse_err(MYSQL *m, const uint8_t *p, size_t len) {
    if (len < 3) { set_err(m, 2013, "HY000", "Malformed error packet"); return -1; }
    unsigned int code = (unsigned int)p[1] | (unsigned int)p[2] << 8;
    const uint8_t *cur = p + 3, *end = p + len;
    char state[6] = "HY000";
    if (cur < end && *cur == '#') {
        cur++;
        memcpy(state, cur, 5); state[5] = 0; cur += 5;
    }
    char msg[512];
    size_t mlen = (size_t)(end - cur);
    if (mlen >= sizeof(msg)) mlen = sizeof(msg) - 1;
    memcpy(msg, cur, mlen); msg[mlen] = 0;
    set_err(m, code, state, msg);
    return -1;
}
/* returns 1=OK, 2=ERR, 3=EOF/auth-more, 0=other(data) */
static int packet_kind(MYSQL *m, const uint8_t *p, size_t len) {
    if (len == 0) return 0;
    if (p[0] == 0xFF) return 2;
    if (ST(m)->deprecate_eof) {
        if (p[0] == 0xFE) return 1;
        if (p[0] == 0x00 && len >= 7) return 1;
    } else {
        if (p[0] == 0x00 && len >= 7) return 1;
        if (p[0] == 0xFE && len < 9) return 3;
    }
    (void)m;
    return 0;
}

/* ----------------------- auth scrambles ----------------------- */
/* mysql_native_password: SHA1(pw) XOR SHA1(scramble + SHA1(SHA1(pw))) */
static void scramble_native(const char *pw, const uint8_t scramble[20], uint8_t out[20]) {
    uint8_t s1[20], s2[20], t[20];
    int i;
    if (!pw || !*pw) { memset(out, 0, 20); return; }
    mm_sha1(pw, strlen(pw), s1);
    mm_sha1(s1, 20, s2);
    mm_sha1_2(scramble, 20, s2, 20, t);
    for (i = 0; i < 20; i++) out[i] = s1[i] ^ t[i];
}
/* caching_sha2_password fast path:
 * SHA256(pw) XOR SHA256(SHA256(SHA256(pw)) + nonce) */
static void scramble_sha2(const char *pw, const uint8_t nonce[20], uint8_t out[32]) {
    uint8_t s1[32], s2[32], s3[32];
    int i;
    if (!pw || !*pw) { memset(out, 0, 32); return; }
    mm_sha256(pw, strlen(pw), s1);
    mm_sha256(s1, 32, s2);
    mm_sha256_2(s2, 32, nonce, 20, s3);
    for (i = 0; i < 32; i++) out[i] = s1[i] ^ s3[i];
}

/* ----------------------- handshake ----------------------- */
static int do_connect_socket(MYSQL *m, const char *host, unsigned int port) {
    char portstr[16];
    struct addrinfo hints, *res = NULL, *ai;
    int fd = -1, rc;
    snprintf(portstr, sizeof(portstr), "%u", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    rc = getaddrinfo(host && *host ? host : "127.0.0.1", portstr, &hints, &res);
    if (rc != 0) { set_err(m, 2005, "HY000", gai_strerror(rc)); return -1; }
    for (ai = res; ai; ai = ai->ai_next) {
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) continue;
        if (m->connect_timeout > 0) {
            int flags = fcntl(fd, F_GETFL, 0);
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            int cr = connect(fd, ai->ai_addr, ai->ai_addrlen);
            if (cr < 0 && errno != EINPROGRESS) { close(fd); fd = -1; continue; }
            if (cr < 0) {
                struct pollfd pfd;
                pfd.fd = fd; pfd.events = POLLOUT;
                int pr = poll(&pfd, 1, (int)m->connect_timeout * 1000);
                if (pr <= 0) {
                    set_err(m, 2003, "HY000", pr == 0 ? "Connection timeout" : "Poll error");
                    close(fd); fd = -1; continue;
                }
                int err = 0; socklen_t elen = sizeof(err);
                getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &elen);
                if (err) { close(fd); fd = -1; continue; }
            }
            fcntl(fd, F_SETFL, flags);
        } else {
            if (connect(fd, ai->ai_addr, ai->ai_addrlen) < 0) { close(fd); fd = -1; continue; }
        }
        break;
    }
    freeaddrinfo(res);
    if (fd < 0) { set_err(m, 2003, "HY000", "Can't connect to MySQL server"); return -1; }
    { int one = 1; setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)); }
    m->fd = fd;
    return 0;
}

static int do_connect_unix(MYSQL *m, const char *path) {
    struct sockaddr_un addr;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) { set_err(m, 2002, "HY000", "Can't create Unix socket"); return -1; }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Can't connect to local MySQL server through socket '%s'", path);
        set_err(m, 2002, "HY000", msg);
        close(fd);
        return -1;
    }
    m->fd = fd;
    return 0;
}

/* Parse server greeting. Fills capabilities, scramble (20 bytes), plugin name. */
static int parse_handshake(MYSQL *m, const uint8_t *p, size_t len,
                           uint8_t scramble[20], char plugin[64]) {
    const uint8_t *cur = p, *end = p + len;
    if (len < 1 || *cur != 10) { set_err(m, 2012, "HY000", "Unsupported protocol version"); return -1; }
    m->protocol_version = *cur;
    cur++;
    /* server version (NUL string) */
    const char *ver = (const char*)cur;
    size_t vlen = strnlen(ver, (size_t)(end - cur));
    free(m->server_version);
    m->server_version = (char*)malloc(vlen + 1);
    memcpy(m->server_version, ver, vlen); m->server_version[vlen] = 0;
    cur += vlen + 1;
    if (cur + 4 > end) goto bad;
    m->thread_id = (uint32_t)cur[0] | (uint32_t)cur[1]<<8 | (uint32_t)cur[2]<<16 | (uint32_t)cur[3]<<24;
    cur += 4;
    /* auth-plugin-data-part-1: 8 bytes */
    memcpy(scramble, cur, 8); cur += 8;
    cur += 1; /* filler */
    unsigned long cap = 0;
    cap |= (unsigned long)cur[0] | (unsigned long)cur[1] << 8; cur += 2;
    int auth_len = 0;
    if (cur < end) { ST(m)->charsetnr = (unsigned int)*cur; ST(m)->mbmaxlen = charset_mbmaxlen(ST(m)->charsetnr); cur += 1; }
    if (cur + 2 <= end) { m->server_status = (unsigned int)cur[0] | (unsigned int)cur[1]<<8; cur += 2; }
    if (cur + 2 <= end) { cap |= ((unsigned long)cur[0] | (unsigned long)cur[1]<<8) << 16; cur += 2; }
    if (cur < end) { auth_len = *cur; cur += 1; }
    cur += 10; /* reserved */
    m->server_capabilities = cap;
    /* auth-plugin-data-part-2 */
    int part2 = auth_len - 8;
    if (part2 < 13) part2 = 13;
    if (part2 > 0 && cur + part2 <= end + 1) {
        int copy = part2 - 1; /* drop trailing NUL */
        if (copy > 12) copy = 12;
        if (copy < 0) copy = 0;
        memcpy(scramble + 8, cur, (size_t)copy);
        cur += part2;
    }
    /* auth plugin name */
    plugin[0] = 0;
    if ((cap & CLIENT_PLUGIN_AUTH) && cur < end) {
        size_t plen = strnlen((const char*)cur, (size_t)(end - cur));
        if (plen >= 64) plen = 63;
        memcpy(plugin, cur, plen); plugin[plen] = 0;
    }
    if (!plugin[0]) strcpy(plugin, "mysql_native_password");
    return 0;
bad:
    set_err(m, 2012, "HY000", "Malformed handshake packet");
    return -1;
}

/* Compute the client capability flags we advertise. */
static unsigned long build_caps(MYSQL *m, const char *db, int with_ssl) {
    unsigned long flags = CLIENT_PROTOCOL_41 | CLIENT_LONG_PASSWORD | CLIENT_LONG_FLAG |
                          CLIENT_TRANSACTIONS | CLIENT_SECURE_CONNECTION |
                          CLIENT_PLUGIN_AUTH | CLIENT_MULTI_RESULTS |
                          CLIENT_PS_MULTI_RESULTS |
                          CLIENT_DEPRECATE_EOF | CLIENT_CONNECT_ATTRS |
                          CLIENT_SESSION_TRACK;
    if (db && *db) flags |= CLIENT_CONNECT_WITH_DB;
    if (with_ssl)  flags |= CLIENT_SSL;
    flags |= (m->client_flag & (CLIENT_FOUND_ROWS | CLIENT_MULTI_STATEMENTS));
    return flags;
}

/* Write the 32-byte fixed prefix shared by SSLRequest and HandshakeResponse41. */
static size_t put_caps_prefix(uint8_t *buf, unsigned long flags, unsigned int charsetnr) {
    size_t o = 0;
    buf[o++] = (uint8_t)(flags);       buf[o++] = (uint8_t)(flags>>8);
    buf[o++] = (uint8_t)(flags>>16);   buf[o++] = (uint8_t)(flags>>24);
    buf[o++] = 0xFF; buf[o++] = 0xFF; buf[o++] = 0xFF; buf[o++] = 0x00; /* max packet */
    buf[o++] = (uint8_t)(charsetnr);               /* charset from server handshake */
    memset(buf + o, 0, 23); o += 23;              /* reserved */
    return o;
}

/* SSLRequest: just the capability prefix, sent before upgrading to TLS. */
static int send_ssl_request(MYSQL *m) {
    uint8_t buf[32];
    size_t o = put_caps_prefix(buf, m->client_flag, ST(m)->charsetnr);
    return write_compressed_packet(m, buf, o, m->seq);
}

/* Build and send HandshakeResponse41 (capabilities already set in m->client_flag). */
static int send_handshake_response(MYSQL *m, const char *user, const uint8_t *auth,
                                   size_t authlen, const char *db, const char *plugin) {
    unsigned long flags = m->client_flag;
    mm_state *st = ST(m);

    size_t cap = 64 + (user ? strlen(user) : 0) + authlen + (db ? strlen(db) : 0) +
                 (plugin ? strlen(plugin) : 0) + 32 + 256 + st->conn_attr_count * 256;
    uint8_t *buf = (uint8_t*)malloc(cap);
    if (!buf) { set_err(m, 2008, "HY000", "Out of memory"); return -1; }
    size_t o = put_caps_prefix(buf, flags, st->charsetnr);
    if (user) { size_t l = strlen(user); memcpy(buf+o, user, l); o += l; }
    buf[o++] = 0;
    o += lenenc_put(buf + o, authlen);
    memcpy(buf + o, auth, authlen); o += authlen;
    if (flags & CLIENT_CONNECT_WITH_DB) { size_t l = strlen(db); memcpy(buf+o, db, l); o += l; buf[o++] = 0; }
    if (plugin) { size_t l = strlen(plugin); memcpy(buf+o, plugin, l); o += l; } buf[o++] = 0;

    if (flags & CLIENT_CONNECT_ATTRS) {
        size_t acap = 512 + st->conn_attr_count * 256;
        uint8_t *attrs = (uint8_t*)malloc(acap);
        size_t alen = 0;
        const char *kv[4][2] = {
            { "_client_name",    MM_VERSION_STR },
            { "_client_version", "8.0.45" },
#ifdef _WIN32
            { "_os",             "Windows" },
#elif defined(__APPLE__)
            { "_os",             "macOS" },
#elif defined(__linux__)
            { "_os",             "Linux" },
#elif defined(__FreeBSD__)
            { "_os",             "FreeBSD" },
#else
            { "_os",             "Unknown" },
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
            { "_platform",       "aarch64" },
#elif defined(__x86_64__) || defined(_M_X64)
            { "_platform",       "x86_64" },
#elif defined(__arm__)
            { "_platform",       "armv7l" },
#elif defined(__i386__) || defined(_M_IX86)
            { "_platform",       "i386" },
#else
            { "_platform",       "unknown" },
#endif
        };
        for (int i = 0; i < 4; i++) {
            size_t kl = strlen(kv[i][0]), vl = strlen(kv[i][1]);
            alen += lenenc_put(attrs + alen, kl);
            memcpy(attrs + alen, kv[i][0], kl); alen += kl;
            alen += lenenc_put(attrs + alen, vl);
            memcpy(attrs + alen, kv[i][1], vl); alen += vl;
        }
        for (unsigned int i = 0; i < st->conn_attr_count; i++) {
            size_t kl = strlen(st->conn_attr_keys[i]);
            size_t vl = strlen(st->conn_attr_vals[i]);
            if (alen + kl + vl + 20 > acap) {
                acap = (alen + kl + vl + 20) * 2;
                uint8_t *na = (uint8_t*)realloc(attrs, acap);
                if (!na) { free(attrs); set_err(m, 2008, "HY000", "Out of memory"); return 1; }
                attrs = na;
            }
            alen += lenenc_put(attrs + alen, kl);
            memcpy(attrs + alen, st->conn_attr_keys[i], kl); alen += kl;
            alen += lenenc_put(attrs + alen, vl);
            memcpy(attrs + alen, st->conn_attr_vals[i], vl); alen += vl;
        }
        o += lenenc_put(buf + o, alen);
        memcpy(buf + o, attrs, alen); o += alen;
        free(attrs);
    }

    int rc = write_compressed_packet(m, buf, o, m->seq);
    free(buf);
    return rc;
}

/* Handle the auth result loop: OK, ERR, AuthSwitch, or AuthMoreData. */
static int finish_auth(MYSQL *m, const char *pw, const uint8_t scramble[20], const char *plugin) {
    uint8_t cur_nonce[20];
    memcpy(cur_nonce, scramble, 20);
    char active_plugin[64] = {0};
    if (plugin) { strncpy(active_plugin, plugin, 63); active_plugin[63] = 0; }
    for (;;) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) return -1;
        if (len == 0) { free(p); continue; }
        uint8_t tag = p[0];
        if (tag == 0x00) { parse_ok(m, p, len); free(p); return 0; }     /* success */
        if (tag == 0xFF) { parse_err(m, p, len); free(p); return -1; }
        if (tag == 0xFE) {                                                /* AuthSwitchRequest */
            const char *newplug = (const char*)(p + 1);
            const uint8_t *data = (const uint8_t*)(newplug + strlen(newplug) + 1);
            uint8_t newscr[20]; memset(newscr, 0, 20);
            size_t dlen = (size_t)(p + len - data);
            memcpy(newscr, data, dlen > 20 ? 20 : dlen);
            memcpy(cur_nonce, newscr, 20);   /* later full-auth uses this nonce */
            uint8_t resp[32]; size_t rlen = 0;
            if (strcmp(newplug, "mysql_native_password") == 0) {
                scramble_native(pw, newscr, resp); rlen = (pw && *pw) ? 20 : 0;
            } else if (strcmp(newplug, "caching_sha2_password") == 0) {
                scramble_sha2(pw, newscr, resp); rlen = (pw && *pw) ? 32 : 0;
            } else {
                free(p); set_err(m, 2059, "HY000", "Authentication plugin not supported"); return -1;
            }
            strncpy(active_plugin, newplug, 63); active_plugin[63] = 0;
            if (write_compressed_packet(m, resp, rlen, m->seq) < 0) { free(p); return -1; }
            free(p);
            continue;
        }
        if (tag == 0x01) {
            if (strcmp(active_plugin, "caching_sha2_password") != 0) {
                free(p); set_err(m, 2059, "HY000", "Unexpected AuthMoreData for non-caching_sha2 plugin"); return -1;
            }
            if (len >= 2 && p[1] == 0x03) { free(p); continue; }   /* fast auth success -> read OK next */
            if (len >= 2 && p[1] == 0x04) {                        /* full auth required */
                free(p);
                if (ST(m)->tls_active) {
                    size_t pl = pw ? strlen(pw) : 0;
                    uint8_t *resp = (uint8_t*)malloc(pl + 1);
                    if (!resp) { set_err(m, 2008, "HY000", "Out of memory"); return -1; }
                    if (pl) memcpy(resp, pw, pl);
                    resp[pl] = 0;
                    int wr = write_compressed_packet(m, resp, pl + 1, m->seq);
                    free(resp);
                    if (wr < 0) return -1;
                    continue;
                }
                if (ST(m)->server_public_key && ST(m)->server_public_key[0]) {
                    unsigned char *ct = NULL; size_t ctlen = 0; char ferr[160] = {0};
                    int sr = mm_rsa_seal_password(
                        (const unsigned char*)ST(m)->server_public_key,
                        strlen(ST(m)->server_public_key),
                        pw, cur_nonce, &ct, &ctlen, ferr, sizeof ferr);
                    if (sr != 0) {
                        set_err(m, 2061, "HY000", ferr[0] ? ferr : "RSA password seal with preset key failed");
                        return -1;
                    }
                    int wr = write_compressed_packet(m, ct, ctlen, m->seq);
                    free(ct);
                    if (wr < 0) return -1;
                    continue;
                }
                uint8_t req = 0x02;
                if (write_compressed_packet(m, &req, 1, m->seq) < 0) return -1;
                uint8_t *kp = NULL; size_t klen = 0;
                if (read_compressed_packet(m, &kp, &klen) < 0) return -1;
                if (klen < 2 || kp[0] != 0x01) {
                    if (klen && kp[0] == 0xFF) parse_err(m, kp, klen);
                    else set_err(m, 2061, "HY000", "Expected RSA public key from server");
                    free(kp); return -1;
                }
                unsigned char *ct = NULL; size_t ctlen = 0; char ferr[160] = {0};
                int sr = mm_rsa_seal_password(kp + 1, klen - 1, pw, cur_nonce,
                                              &ct, &ctlen, ferr, sizeof ferr);
                free(kp);
                if (sr != 0) {
                    set_err(m, 2061, "HY000", ferr[0] ? ferr : "RSA password seal failed");
                    return -1;
                }
                int wr = write_compressed_packet(m, ct, ctlen, m->seq);
                free(ct);
                if (wr < 0) return -1;
                continue;
            }
            free(p); continue;
        }
        free(p);
        set_err(m, 2013, "HY000", "Unexpected packet during authentication");
        return -1;
    }
}

/* ----------------------- public API ----------------------- */
MYSQL *mysql_init(MYSQL *mysql) {
    MYSQL *m = mysql;
    if (!m) { m = (MYSQL*)calloc(1, sizeof(MYSQL)); if (!m) return NULL; }
    else memset(m, 0, sizeof(MYSQL));
    m->fd = -1;
    m->status = MYSQL_STATUS_READY;
    strcpy(m->sqlstate, "00000");
    mm_state *st = (mm_state*)calloc(1, sizeof(mm_state));
    if (!st) { if (!mysql) free(m); return NULL; }
    st->ssl_mode = SSL_MODE_PREFERRED;   /* use TLS when the server offers it */
    st->free_handle = (mysql == NULL);   /* we own m only if we allocated it  */
    m->st = st;
    m->stmt_cache = mm_cache_new();
    return m;
}

static char *dup_or_null(const char *s) { return (s && *s) ? strdup(s) : NULL; }
static void set_str(char **slot, const char *v) { free(*slot); *slot = dup_or_null(v); }

int mysql_options(MYSQL *m, enum mysql_option option, const void *arg) {
    if (!m) return 1;
    mm_state *st = ST(m);
    switch (option) {
        case MYSQL_OPT_CONNECT_TIMEOUT: m->connect_timeout = *(const unsigned int*)arg; break;
        case MYSQL_OPT_COMPRESS: break;
        case MYSQL_OPT_NAMED_PIPE: break;
        case MYSQL_INIT_COMMAND: set_str(&st->init_command, (const char*)arg); break;
        case MYSQL_READ_DEFAULT_FILE: set_str(&st->read_default_file, (const char*)arg); break;
        case MYSQL_READ_DEFAULT_GROUP: set_str(&st->read_default_group, (const char*)arg); break;
        case MYSQL_SET_CHARSET_DIR: break;
        case MYSQL_SET_CHARSET_NAME:
            free(m->charset_name);
            m->charset_name = arg ? strdup((const char*)arg) : NULL;
            break;
        case MYSQL_OPT_LOCAL_INFILE: st->local_infile = arg ? *(const unsigned int*)arg : 0; break;
        case MYSQL_OPT_PROTOCOL: st->protocol = arg ? *(const unsigned int*)arg : 0; break;
        case MYSQL_SHARED_MEMORY_BASE_NAME: set_str(&st->shared_memory_base_name, (const char*)arg); break;
        case MYSQL_OPT_READ_TIMEOUT:    m->read_timeout = *(const unsigned int*)arg; break;
        case MYSQL_OPT_WRITE_TIMEOUT:   m->write_timeout = *(const unsigned int*)arg; break;
        case MYSQL_OPT_USE_RESULT: st->use_result = arg ? *(const my_bool*)arg : 0; break;
        case MYSQL_REPORT_DATA_TRUNCATION: st->report_data_truncation = arg ? *(const my_bool*)arg : 0; break;
        case MYSQL_OPT_RECONNECT:  st->reconnect = arg ? *(const my_bool*)arg : 0; break;
        case MYSQL_PLUGIN_DIR: set_str(&st->plugin_dir, (const char*)arg); break;
        case MYSQL_DEFAULT_AUTH: set_str(&st->default_auth, (const char*)arg); break;
        case MYSQL_OPT_BIND: break;
        case MYSQL_OPT_SSL_KEY:    set_str(&st->ssl_key,    (const char*)arg); break;
        case MYSQL_OPT_SSL_CERT:   set_str(&st->ssl_cert,   (const char*)arg); break;
        case MYSQL_OPT_SSL_CA:     set_str(&st->ssl_ca,     (const char*)arg); break;
        case MYSQL_OPT_SSL_CAPATH: set_str(&st->ssl_capath, (const char*)arg); break;
        case MYSQL_OPT_SSL_CIPHER: set_str(&st->ssl_cipher, (const char*)arg); break;
        case MYSQL_OPT_SSL_CRL:     set_str(&st->ssl_crl,     (const char*)arg); break;
        case MYSQL_OPT_SSL_CRLPATH: set_str(&st->ssl_crlpath, (const char*)arg); break;
        case MYSQL_OPT_CONNECT_ATTR_RESET:
            for (unsigned int i = 0; i < st->conn_attr_count; i++) {
                free(st->conn_attr_keys[i]); free(st->conn_attr_vals[i]);
            }
            st->conn_attr_count = 0;
            break;
        case MYSQL_OPT_CONNECT_ATTR_ADD: {
            const char *key = (const char*)arg;
            if (!key) break;
            for (unsigned int i = 0; i < st->conn_attr_count; i++) {
                if (strcmp(st->conn_attr_keys[i], key) == 0) { free(st->conn_attr_vals[i]); st->conn_attr_vals[i] = strdup(""); break; }
            }
            if (st->conn_attr_count >= st->conn_attr_cap) {
                unsigned int nc = st->conn_attr_cap ? st->conn_attr_cap * 2 : 8;
                char **nk = (char**)realloc(st->conn_attr_keys, nc * sizeof(char*));
                if (!nk) { set_err(m, 2008, "HY000", "Out of memory"); return 1; }
                st->conn_attr_keys = nk;
                char **nv = (char**)realloc(st->conn_attr_vals, nc * sizeof(char*));
                if (!nv) { set_err(m, 2008, "HY000", "Out of memory"); return 1; }
                st->conn_attr_vals = nv;
                st->conn_attr_cap = nc;
            }
            st->conn_attr_keys[st->conn_attr_count] = strdup(key);
            st->conn_attr_vals[st->conn_attr_count] = strdup("");
            st->conn_attr_count++;
            break;
        }
        case MYSQL_OPT_CONNECT_ATTR_DELETE: {
            const char *key = (const char*)arg;
            if (!key) break;
            for (unsigned int i = 0; i < st->conn_attr_count; i++) {
                if (strcmp(st->conn_attr_keys[i], key) == 0) {
                    free(st->conn_attr_keys[i]); free(st->conn_attr_vals[i]);
                    st->conn_attr_keys[i] = st->conn_attr_keys[st->conn_attr_count - 1];
                    st->conn_attr_vals[i] = st->conn_attr_vals[st->conn_attr_count - 1];
                    st->conn_attr_count--;
                    break;
                }
            }
            break;
        }
        case MYSQL_SERVER_PUBLIC_KEY: set_str(&st->server_public_key, (const char*)arg); break;
        case MYSQL_ENABLE_CLEARTEXT_PLUGIN: st->enable_cleartext_plugin = arg ? *(const my_bool*)arg : 0; break;
        case MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS: st->can_handle_expired_passwords = arg ? *(const my_bool*)arg : 0; break;
        case MYSQL_OPT_MAX_ALLOWED_PACKET: st->max_allowed_packet = arg ? *(const unsigned long*)arg : 0; break;
        case MYSQL_OPT_NET_BUFFER_LENGTH: st->net_buffer_length = arg ? *(const unsigned long*)arg : 0; break;
        case MYSQL_OPT_TLS_VERSION: set_str(&st->tls_version, (const char*)arg); break;
        case MYSQL_OPT_SSL_MODE:   st->ssl_mode = (int)*(const unsigned int*)arg; break;
        case MYSQL_OPT_GET_SERVER_PUBLIC_KEY: set_str(&st->server_public_key, (const char*)arg); break;
        case MYSQL_OPT_RETRY_COUNT: st->retry_count = arg ? *(const unsigned int*)arg : 0; break;
        case MYSQL_OPT_OPTIONAL_RESULTSET_METADATA: st->optional_resultset_metadata = arg ? *(const my_bool*)arg : 0; break;
        case MYSQL_OPT_SSL_FIPS_MODE: st->ssl_fips_mode = arg ? *(const int*)arg : 0; break;
        case MYSQL_OPT_TLS_CIPHERSUITES: set_str(&st->tls_ciphersuites, (const char*)arg); break;
        case MYSQL_OPT_COMPRESSION_ALGORITHMS: set_str(&st->compression_algorithms, (const char*)arg); break;
        case MYSQL_OPT_ZSTD_COMPRESSION_LEVEL: st->zstd_compression_level = arg ? *(const unsigned int*)arg : 0; break;
        case MYSQL_OPT_LOAD_DATA_LOCAL_DIR: set_str(&st->load_data_local_dir, (const char*)arg); break;
        case MYSQL_OPT_USER_PASSWORD: break;
        case MYSQL_OPT_SSL_SESSION_DATA: break;
        default: break;
    }
    return 0;
}

int mysql_ssl_set(MYSQL *m, const char *key, const char *cert,
                  const char *ca, const char *capath, const char *cipher) {
    if (!m) return 1;
    mm_state *st = ST(m);
    set_str(&st->ssl_key,    key);
    set_str(&st->ssl_cert,   cert);
    set_str(&st->ssl_ca,     ca);
    set_str(&st->ssl_capath, capath);
    set_str(&st->ssl_cipher, cipher);
    if ((ca && *ca) || (capath && *capath)) {
        if (st->ssl_mode < SSL_MODE_VERIFY_CA) st->ssl_mode = SSL_MODE_VERIFY_CA;
    }
    return 0;
}

const char *mysql_get_ssl_cipher(MYSQL *m) {
    if (!m || !ST(m)->tls_active) return NULL;
    return ST(m)->cipher_in_use;
}

/* Close the active transport (if any) and mark the handle disconnected. */
static void conn_abort(MYSQL *m) {
    mm_state *st = ST(m);
    if (st->trans) { st->trans->close(st->trans); st->trans = NULL; }
    st->tls_active = 0;
    free(st->comp_buf); st->comp_buf = NULL; st->comp_buf_len = 0; st->comp_buf_pos = 0;
    m->fd = -1;
}

MYSQL *mysql_real_connect(MYSQL *m, const char *host, const char *user,
                          const char *passwd, const char *db,
                          unsigned int port, const char *unix_socket,
                          unsigned long client_flag) {
    if (!m) return NULL;
    if (m->fd >= 0) {
        set_err(m, CR_ALREADY_CONNECTED, "HY000", "Connection already open");
        return NULL;
    }
    mm_state *st = ST(m);
    clear_err(m);
    m->client_flag |= client_flag;
    if (port == 0) port = 3306;

    if (unix_socket && *unix_socket) {
        if (do_connect_unix(m, unix_socket) < 0) return NULL;
    } else {
        if (do_connect_socket(m, host, port) < 0) return NULL;
    }
    st->trans = mm_transport_plain_new(m->fd);
    if (!st->trans) { set_err(m, 2008, "HY000", "Out of memory"); close(m->fd); m->fd = -1; return NULL; }

    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) { conn_abort(m); return NULL; }
    if (len && p[0] == 0xFF) { parse_err(m, p, len); free(p); conn_abort(m); return NULL; }

    uint8_t scramble[20]; memset(scramble, 0, 20);
    char plugin[64];
    if (parse_handshake(m, p, len, scramble, plugin) < 0) { free(p); conn_abort(m); return NULL; }
    free(p);

    /* ---- decide on TLS and, if so, upgrade the transport mid-handshake ---- */
    int server_ssl = (m->server_capabilities & CLIENT_SSL) != 0;
    int want_tls = (st->ssl_mode == SSL_MODE_DISABLED)  ? 0 :
                   (st->ssl_mode == SSL_MODE_PREFERRED) ? server_ssl : 1;
    if (want_tls && !server_ssl && st->ssl_mode >= SSL_MODE_REQUIRED) {
        set_err(m, 2026, "HY000", "TLS required but the server does not support it");
        conn_abort(m); return NULL;
    }

    m->client_flag = build_caps(m, db, want_tls);

    if (want_tls) {
        if (send_ssl_request(m) < 0) { conn_abort(m); return NULL; }
        int verify = (st->ssl_mode == SSL_MODE_VERIFY_IDENTITY) ? MM_TLS_VERIFY_IDENTITY :
                     (st->ssl_mode == SSL_MODE_VERIFY_CA)       ? MM_TLS_VERIFY_CA :
                                                                  MM_TLS_VERIFY_NONE;
        mm_ssl_opts so = { verify, st->ssl_ca, st->ssl_capath,
                           st->ssl_cert, st->ssl_key, st->ssl_cipher };
        char cipher[64] = {0}, ferr[200] = {0};
        mm_transport *tls = mm_transport_tls_upgrade(st->trans, &so, host,
                                                     cipher, sizeof(cipher),
                                                     ferr, sizeof(ferr));
        if (!tls) {
            set_err(m, 2026, "HY000", ferr[0] ? ferr : "TLS negotiation failed");
            conn_abort(m); return NULL;
        }
        st->trans = tls;
        st->tls_active = 1;
        snprintf(st->cipher_in_use, sizeof(st->cipher_in_use), "%s", cipher);
    }

    uint8_t auth[32]; size_t authlen = 0;
    if (strcmp(plugin, "caching_sha2_password") == 0) {
        scramble_sha2(passwd, scramble, auth); authlen = (passwd && *passwd) ? 32 : 0;
    } else { /* default to native */
        strcpy(plugin, "mysql_native_password");
        scramble_native(passwd, scramble, auth); authlen = (passwd && *passwd) ? 20 : 0;
    }

    if (send_handshake_response(m, user, auth, authlen, db, plugin) < 0) { conn_abort(m); return NULL; }
    if (finish_auth(m, passwd, scramble, plugin) < 0) { conn_abort(m); return NULL; }

    { mm_state *s = ST(m); memcpy(s->saved_scramble, scramble, 20); s->saved_scramble_set = 1;
      s->deprecate_eof = (m->server_capabilities & CLIENT_DEPRECATE_EOF) ? 1 : 0;
      s->compress_algo = 0;
      if (m->client_flag & CLIENT_ZSTD_COMPRESSION) {
          if (m->server_capabilities & CLIENT_ZSTD_COMPRESSION)
              s->compress_algo = 2;
          else if ((m->client_flag & CLIENT_COMPRESS) && (m->server_capabilities & CLIENT_COMPRESS))
              s->compress_algo = 1;
      } else if ((m->client_flag & CLIENT_COMPRESS) && (m->server_capabilities & CLIENT_COMPRESS)) {
          s->compress_algo = 1;
      }
      s->comp_seq = 0;
    }

    { char *th = host ? strdup(host) : NULL;
      char *tu = user ? strdup(user) : NULL;
      char *td = (db && *db) ? strdup(db) : NULL;
      free(m->host); m->host = th;
      free(m->user); m->user = tu;
      free(m->db);   m->db = td;
    }
    m->port = port;
    /* save for auto-reconnect */
    { mm_state *s = ST(m);
      char *nh = host ? strdup(host) : NULL;
      char *nu = user ? strdup(user) : NULL;
      char *np = passwd ? strdup(passwd) : NULL;
      char *nd = (db && *db) ? strdup(db) : NULL;
      char *nus = unix_socket ? strdup(unix_socket) : NULL;
      free(s->saved_host); s->saved_host = nh;
      free(s->saved_user); s->saved_user = nu;
      free(s->saved_passwd); s->saved_passwd = np;
      free(s->saved_db); s->saved_db = nd;
      s->saved_port = port;
      free(s->saved_unix_socket); s->saved_unix_socket = nus;
      s->saved_client_flag = client_flag;
    }
    if (m->charset_name) {
        unsigned int nr = charset_name_to_nr(m->charset_name);
        if (nr) {
            ST(m)->charsetnr = nr;
            ST(m)->mbmaxlen = charset_mbmaxlen(nr);
        }
    }
    else {
        m->charset_name = strdup("utf8mb4");
    }
    if (ST(m)->init_command) {
        mysql_query(m, ST(m)->init_command);
    }
    m->status = MYSQL_STATUS_READY;
    return m;
}

/* COM_QUERY = 0x03 */
static int try_reconnect(MYSQL *m) {
    mm_state *st = ST(m);
    if (!st->reconnect) return -1;
    if (st->reconnect_attempts >= 1) return -1;
    st->reconnect_attempts++;
    if (m->fd >= 0) { close(m->fd); m->fd = -1; }
    if (st->trans) { st->trans->close(st->trans); st->trans = NULL; }
    st->tls_active = 0;
    st->saved_scramble_set = 0;
    st->compress_algo = 0;
    st->comp_seq = 0;
    free(st->comp_buf); st->comp_buf = NULL; st->comp_buf_len = 0; st->comp_buf_pos = 0;
    mm_stmt_cache *cache = (mm_stmt_cache*)m->stmt_cache;
    if (cache) {
        for (unsigned int i = 0; i < cache->count; i++) free(cache->entries[i].sql);
        cache->count = 0;
    }
    MYSQL *r = mysql_real_connect(m, st->saved_host, st->saved_user,
                                   st->saved_passwd, st->saved_db,
                                   st->saved_port, st->saved_unix_socket,
                                   st->saved_client_flag);
    if (r) {
        st->reconnect_attempts = 0;
        if (m->charset_name) {
            char q[256];
            int n = snprintf(q, sizeof(q), "SET NAMES %s", m->charset_name);
            mysql_real_query(m, q, (unsigned long)n);
        }
    }
    return r ? 0 : -1;
}

int mysql_real_query(MYSQL *m, const char *q, unsigned long qlen) {
    if (!m) return 1;
    if (qlen == (unsigned long)(-1)) qlen = (unsigned long)strlen(q);
    if (ST(m)->streaming_active) {
        set_err(m, 2014, "HY000", "Commands out of sync; you can't run this command now");
        return 1;
    }
    if (m->fd < 0) {
        if (try_reconnect(m) < 0) { set_err(m, 2006, "HY000", "Server has gone away"); return 1; }
    }
    else {
        ST(m)->reconnect_attempts = 0;
    }
    clear_err(m);
    m->seq = 0;
    uint8_t *buf = (uint8_t*)malloc(qlen + 1);
    if (!buf) { set_err(m, 2008, "HY000", "Out of memory"); return 1; }
    buf[0] = 0x03;
    memcpy(buf + 1, q, qlen);
    int rc = write_compressed_packet(m, buf, qlen + 1, 0);
    free(buf);
    if (rc < 0) {
        if (try_reconnect(m) == 0) return mysql_real_query(m, q, qlen);
        return 1;
    }

    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) {
        if (try_reconnect(m) == 0) return mysql_real_query(m, q, qlen);
        return 1;
    }
    int kind = packet_kind(m, p, len);
    if (kind == 2) { parse_err(m, p, len); free(p); return 1; }
    if (kind == 1 || kind == 3) {
        parse_ok(m, p, len);
        m->field_count = 0;
        m->status = MYSQL_STATUS_READY;
        m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
        free(p);
        return 0;
    }
    if (len > 0 && p[0] == 0xFB) {
        free(p);
        set_err(m, 2050, "HY000", "LOCAL INFILE is not implemented");
        return 1;
    }
    /* result set header: column count */
    const uint8_t *cur = p, *end = p + len;
    uint64_t cols = lenenc_int(&cur, end, NULL);
    m->field_count = (unsigned int)cols;
    m->status = MYSQL_STATUS_GET_RESULT;
    free(p);
    return 0;
}

int mysql_query(MYSQL *m, const char *q) {
    return mysql_real_query(m, q, (unsigned long)strlen(q));
}

/* Parse a column-definition packet into a MYSQL_FIELD (strings strdup'd). */
static void parse_field(const uint8_t *p, size_t len, MYSQL_FIELD *f) {
    const uint8_t *cur = p, *end = p + len;
    uint64_t l; const uint8_t *s;
    memset(f, 0, sizeof(*f));
    s = lenenc_str(&cur, end, &l, NULL); if (s) { f->catalog = strndup((const char*)s, l); f->catalog_length=(unsigned)l; }
    s = lenenc_str(&cur, end, &l, NULL); if (s) { f->db = strndup((const char*)s, l); f->db_length=(unsigned)l; }
    s = lenenc_str(&cur, end, &l, NULL); if (s) { f->table = strndup((const char*)s, l); f->table_length=(unsigned)l; }
    s = lenenc_str(&cur, end, &l, NULL); if (s) { f->org_table = strndup((const char*)s, l); f->org_table_length=(unsigned)l; }
    s = lenenc_str(&cur, end, &l, NULL); if (s) { f->name = strndup((const char*)s, l); f->name_length=(unsigned)l; }
    s = lenenc_str(&cur, end, &l, NULL); if (s) { f->org_name = strndup((const char*)s, l); f->org_name_length=(unsigned)l; }
    (void)lenenc_int(&cur, end, NULL);
    if (cur + 10 <= end) {
        f->charsetnr = (unsigned int)cur[0] | (unsigned int)cur[1]<<8;
        f->length = (unsigned long)cur[2] | (unsigned long)cur[3]<<8 |
                    (unsigned long)cur[4]<<16 | (unsigned long)cur[5]<<24;
        f->type = (enum enum_field_types)cur[6];
        f->flags = (unsigned int)cur[7] | (unsigned int)cur[8]<<8;
        f->decimals = cur[9];
    }
}

MYSQL_RES *mysql_store_result(MYSQL *m) {
    if (!m || m->fd < 0) return NULL;
    if (m->field_count == 0) return NULL;   /* query had no result set */

    MYSQL_RES *res = (MYSQL_RES*)calloc(1, sizeof(MYSQL_RES));
    if (!res) { set_err(m, 2008, "HY000", "Out of memory"); return NULL; }
    res->handle = m;
    res->field_count = m->field_count;
    res->fields = (MYSQL_FIELD*)calloc(res->field_count, sizeof(MYSQL_FIELD));

    /* column definitions */
    unsigned int i;
    for (i = 0; i < res->field_count; i++) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) { mysql_free_result(res); return NULL; }
        parse_field(p, len, &res->fields[i]);
        free(p);
    }
    /* optional EOF after columns (absent with CLIENT_DEPRECATE_EOF) */
    if (!ST(m)->deprecate_eof) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) { mysql_free_result(res); return NULL; }
        free(p);
    }

    /* rows until EOF/OK/ERR — strdup per field to avoid realloc pointer invalidation */
    for (;;) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) { mysql_free_result(res); return NULL; }
        if (len > 0 && p[0] == 0xFF) { parse_err(m, p, len); free(p); mysql_free_result(res); return NULL; }
        if (ST(m)->deprecate_eof && len > 0 && p[0] == 0xFE) {
            parse_ok(m, p, len);
            m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
            free(p);
            break;
        }
        if (!ST(m)->deprecate_eof && len > 0 && p[0] == 0xFE && len < 9) {
            if (len >= 5) {
                m->warning_count = (unsigned int)p[1] | (unsigned int)p[2]<<8;
                m->server_status = (unsigned int)p[3] | (unsigned int)p[4]<<8;
            }
            m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
            free(p);
            break;
        }
        const uint8_t *cur = p, *end = p + len;
        MYSQL_ROW row = (MYSQL_ROW)calloc(res->field_count, sizeof(char*));
        unsigned long *lens = (unsigned long*)calloc(res->field_count, sizeof(unsigned long));
        if (!row || !lens) { free(row); free(lens); free(p); mysql_free_result(res); set_err(m,2008,"HY000","Out of memory"); return NULL; }
        for (i = 0; i < res->field_count; i++) {
            int isnull = 0; uint64_t fl;
            const uint8_t *s = lenenc_str(&cur, end, &fl, &isnull);
            if (isnull) { row[i] = NULL; lens[i] = 0; }
            else {
                row[i] = strndup((const char*)s, (size_t)fl);
                lens[i] = (unsigned long)fl;
                if (!row[i]) {
                    unsigned int j;
                    for (j = 0; j <= i; j++) free(row[j]);
                    free(row); free(lens); free(p); mysql_free_result(res);
                    set_err(m, 2008, "HY000", "Out of memory"); return NULL;
                }
                if (fl > res->fields[i].max_length) res->fields[i].max_length = (unsigned long)fl;
            }
        }
        free(p);
        MYSQL_ROW *nd = (MYSQL_ROW*)realloc(res->store_data, (res->row_count+1)*sizeof(MYSQL_ROW));
        unsigned long **nl = (unsigned long**)realloc(res->store_lengths, (res->row_count+1)*sizeof(unsigned long*));
        if (!nd || !nl) {
            free(nd ? nd : res->store_data);
            free(nl ? nl : res->store_lengths);
            res->store_data = nd ? nd : res->store_data;
            res->store_lengths = nl ? nl : res->store_lengths;
            for (i = 0; i < res->field_count; i++) free(row[i]);
            free(row); free(lens);
            mysql_free_result(res);
            set_err(m, 2008, "HY000", "Out of memory"); return NULL;
        }
        res->store_data = nd;
        res->store_lengths = nl;
        res->store_data[res->row_count] = row;
        res->store_lengths[res->row_count] = lens;
        res->row_count++;
    }
    res->store_cursor = 0;
    res->metadata = RESULTSET_METADATA_FULL;
    res->eof = 0;
    m->status = MYSQL_STATUS_READY;
    return res;
}

MYSQL_RES *mysql_use_result(MYSQL *m) {
    if (!m || m->fd < 0) return NULL;
    if (m->field_count == 0) return NULL;

    MYSQL_RES *res = (MYSQL_RES*)calloc(1, sizeof(MYSQL_RES));
    if (!res) { set_err(m, 2008, "HY000", "Out of memory"); return NULL; }
    res->handle = m;
    res->field_count = m->field_count;
    res->streaming = 1;
    ST(m)->streaming_active = 1;
    m->status = MYSQL_STATUS_USE_RESULT;
    res->fields = (MYSQL_FIELD*)calloc(res->field_count, sizeof(MYSQL_FIELD));
    if (!res->fields) { free(res); set_err(m, 2008, "HY000", "Out of memory"); return NULL; }

    unsigned int i;
    for (i = 0; i < res->field_count; i++) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) { mysql_free_result(res); return NULL; }
        parse_field(p, len, &res->fields[i]);
        free(p);
    }
    /* EOF after column definitions (absent with CLIENT_DEPRECATE_EOF) */
    if (!ST(m)->deprecate_eof) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) { mysql_free_result(res); return NULL; }
        free(p);
    }
    return res;
}

void mysql_free_result(MYSQL_RES *res) {
    if (!res) return;
    if (res->streaming && res->handle) ST(res->handle)->streaming_active = 0;
    if (res->streaming && !res->eof && res->handle && res->handle->fd >= 0) {
        MYSQL *m = res->handle;
        for (;;) {
            uint8_t *p = NULL; size_t len = 0;
            if (read_compressed_packet(m, &p, &len) < 0) break;
            if (ST(m)->deprecate_eof && len > 0 && p[0] == 0xFE) {
                parse_ok(m, p, len);
                free(p); break;
            }
            if (!ST(m)->deprecate_eof && len > 0 && p[0] == 0xFE && len < 9) {
                if (len >= 5) {
                    m->warning_count = (unsigned int)p[1] | (unsigned int)p[2]<<8;
                    m->server_status = (unsigned int)p[3] | (unsigned int)p[4]<<8;
                }
                free(p); break;
            }
            free(p);
        }
        m->status = MYSQL_STATUS_READY;
    }
    if (res->streaming && res->eof && res->handle) {
        res->handle->status = MYSQL_STATUS_READY;
    }
    if (res->cur_row) {
        if (res->streaming) {
            for (unsigned int i = 0; i < res->field_count; i++) free(res->cur_row[i]);
        }
        free(res->cur_row);
    }
    if (res->cur_lengths && res->cur_lengths != res->cur_row_lens)
        free(res->cur_lengths);
    res->cur_lengths = NULL;
    free(res->cur_row_lens);
    res->cur_row_lens = NULL;
    my_ulonglong r; unsigned int i;
    if (res->store_data) {
        for (r = 0; r < res->row_count; r++) {
            if (res->store_data[r]) {
                for (i = 0; i < res->field_count; i++) free(res->store_data[r][i]);
            }
            free(res->store_data[r]);
            if (res->store_lengths && res->store_lengths[r]) free(res->store_lengths[r]);
        }
        free(res->store_data);
    }
    free(res->store_lengths);
    if (res->fields) {
        for (i = 0; i < res->field_count; i++) {
            free(res->fields[i].name); free(res->fields[i].org_name);
            free(res->fields[i].table); free(res->fields[i].org_table);
            free(res->fields[i].db); free(res->fields[i].catalog); free(res->fields[i].def);
        }
        free(res->fields);
    }
    free(res);
}

MYSQL_ROW mysql_fetch_row(MYSQL_RES *res) {
    if (!res) return NULL;
    if (res->streaming) {
        MYSQL *m = res->handle;
        if (res->eof || !m || m->fd < 0) return NULL;
        /* free previous streaming row */
        if (res->cur_row) {
            for (unsigned int i = 0; i < res->field_count; i++) free(res->cur_row[i]);
            free(res->cur_row);
            res->cur_row = NULL;
        }
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) return NULL;
        if (len > 0 && p[0] == 0xFF) { parse_err(m, p, len); free(p); return NULL; }
        if (ST(m)->deprecate_eof && len > 0 && p[0] == 0xFE) {
            parse_ok(m, p, len);
            m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
            free(p);
            res->eof = 1;
            ST(m)->streaming_active = 0;
            return NULL;
        }
        if (!ST(m)->deprecate_eof && len > 0 && p[0] == 0xFE && len < 9) {
            if (len >= 5) {
                m->warning_count = (unsigned int)p[1] | (unsigned int)p[2]<<8;
                m->server_status = (unsigned int)p[3] | (unsigned int)p[4]<<8;
            }
            m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
            free(p);
            res->eof = 1;
            ST(m)->streaming_active = 0;
            return NULL;
        }
        const uint8_t *cur = p, *end = p + len;
        MYSQL_ROW row = (MYSQL_ROW)calloc(res->field_count, sizeof(char*));
        if (!res->cur_row_lens)
            res->cur_row_lens = (unsigned long*)calloc(res->field_count, sizeof(unsigned long));
        for (unsigned int i = 0; i < res->field_count; i++) {
            int is_null = 0; uint64_t fl;
            const uint8_t *s = lenenc_str(&cur, end, &fl, &is_null);
            if (is_null) { row[i] = NULL; res->cur_row_lens[i] = 0; }
            else { row[i] = strndup((const char*)s, fl); res->cur_row_lens[i] = (unsigned long)fl; }
        }
        free(p);
        res->cur_row = row;
        res->cur_lengths = res->cur_row_lens;
        res->row_count++;
        return row;
    }
    if (res->store_cursor >= res->row_count) {
        free(res->cur_lengths); res->cur_lengths = NULL;
        return NULL;
    }
    MYSQL_ROW row = res->store_data[res->store_cursor];
    free(res->cur_lengths);
    res->cur_lengths = (unsigned long*)malloc(res->field_count * sizeof(unsigned long));
    if (res->cur_lengths)
        memcpy(res->cur_lengths, res->store_lengths[res->store_cursor],
               res->field_count * sizeof(unsigned long));
    res->store_cursor++;
    return row;
}

unsigned long *mysql_fetch_lengths(MYSQL_RES *res) {
    if (!res) return NULL;
    if (res->streaming) return res->eof ? NULL : res->cur_lengths;
    if (res->store_cursor == 0) return NULL;
    return res->cur_lengths;
}

void mysql_data_seek(MYSQL_RES *res, my_ulonglong off) {
    if (!res) return;
    res->store_cursor = (off < res->row_count) ? off : res->row_count;
    if (res->store_cursor >= res->row_count) {
        free(res->cur_lengths); res->cur_lengths = NULL;
    }
}

unsigned int  mysql_num_fields(MYSQL_RES *res) { return res ? res->field_count : 0; }
my_ulonglong  mysql_num_rows(MYSQL_RES *res)   { return res ? res->row_count : 0; }
unsigned int  mysql_field_count(MYSQL *m)      { return m ? m->field_count : 0; }

MYSQL_FIELD *mysql_fetch_field(MYSQL_RES *res) {
    if (!res || res->current_field >= res->field_count) return NULL;
    return &res->fields[res->current_field++];
}
MYSQL_FIELD *mysql_fetch_fields(MYSQL_RES *res) { return res ? res->fields : NULL; }
MYSQL_FIELD *mysql_fetch_field_direct(MYSQL_RES *res, unsigned int n) {
    if (!res || n >= res->field_count) return NULL;
    return &res->fields[n];
}
void mysql_field_seek(MYSQL_RES *res, MYSQL_FIELD_OFFSET o) { if (res) res->current_field = o; }

my_ulonglong mysql_affected_rows(MYSQL *m) { return m ? m->affected_rows : 0; }
my_ulonglong mysql_insert_id(MYSQL *m)     { return m ? m->insert_id : 0; }
unsigned int mysql_warning_count(MYSQL *m) { return m ? m->warning_count : 0; }

const char  *mysql_error(MYSQL *m)    { return m ? m->last_error : ""; }
unsigned int mysql_errno(MYSQL *m)    { return m ? m->last_errno : 0; }
const char  *mysql_sqlstate(MYSQL *m) { return m ? m->sqlstate : "00000"; }

int mysql_ping(MYSQL *m) {
    if (!m) return 1;
    if (m->fd < 0) {
        if (try_reconnect(m) < 0) return 1;
    }
    m->seq = 0;
    uint8_t cmd = 0x0E;
    if (write_compressed_packet(m, &cmd, 1, 0) < 0) {
        if (try_reconnect(m) < 0) return 1;
        m->seq = 0;
        if (write_compressed_packet(m, &cmd, 1, 0) < 0) return 1;
    }
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) {
        if (try_reconnect(m) < 0) return 1;
        m->seq = 0;
        if (write_compressed_packet(m, &cmd, 1, 0) < 0) return 1;
        if (read_compressed_packet(m, &p, &len) < 0) return 1;
    }
    int err = (len && p[0] == 0xFF);
    if (err) parse_err(m, p, len);
    free(p);
    return err ? 1 : 0;
}

int mysql_change_user(MYSQL *m, const char *user, const char *passwd, const char *db) {
    if (!m || m->fd < 0) { if (m) set_err(m, 2006, "HY000", "Server has gone away"); return 1; }
    mm_state *st = ST(m);
    if (st->streaming_active) {
        set_err(m, 2014, "HY000", "Commands out of sync; you can't run this command now");
        return 1;
    }
    clear_err(m);
    m->seq = 0;
    const char *plugin = "caching_sha2_password";
    uint8_t scramble[20];
    memset(scramble, 0, 20);
    if (st->saved_scramble_set) {
        memcpy(scramble, st->saved_scramble, 20);
    }
    size_t ulen = user ? strlen(user) : 0;
    size_t dlen = (db && *db) ? strlen(db) : 0;
    size_t plen = strlen(plugin);
    uint8_t auth_resp[32];
    size_t auth_len = 0;
    if (passwd && *passwd) {
        if (strcmp(plugin, "caching_sha2_password") == 0) {
            scramble_sha2(passwd, scramble, auth_resp); auth_len = 32;
        } else {
            scramble_native(passwd, scramble, auth_resp); auth_len = 20;
        }
    }
    size_t acap = 512 + st->conn_attr_count * 256;
    uint8_t *attrs = NULL;
    size_t alen = 0;
    if (m->client_flag & CLIENT_CONNECT_ATTRS) {
        attrs = (uint8_t*)malloc(acap);
        const char *kv[4][2] = {
            { "_client_name",    MM_VERSION_STR },
            { "_client_version", "8.0.45" },
#ifdef _WIN32
            { "_os",             "Windows" },
#elif defined(__APPLE__)
            { "_os",             "macOS" },
#elif defined(__linux__)
            { "_os",             "Linux" },
#elif defined(__FreeBSD__)
            { "_os",             "FreeBSD" },
#else
            { "_os",             "Unknown" },
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
            { "_platform",       "aarch64" },
#elif defined(__x86_64__) || defined(_M_X64)
            { "_platform",       "x86_64" },
#elif defined(__arm__)
            { "_platform",       "armv7l" },
#elif defined(__i386__) || defined(_M_IX86)
            { "_platform",       "i386" },
#else
            { "_platform",       "unknown" },
#endif
        };
        for (int i = 0; i < 4; i++) {
            size_t kl = strlen(kv[i][0]), vl = strlen(kv[i][1]);
            alen += lenenc_put(attrs + alen, kl);
            memcpy(attrs + alen, kv[i][0], kl); alen += kl;
            alen += lenenc_put(attrs + alen, vl);
            memcpy(attrs + alen, kv[i][1], vl); alen += vl;
        }
        for (unsigned int i = 0; i < st->conn_attr_count; i++) {
            size_t kl = strlen(st->conn_attr_keys[i]);
            size_t vl = strlen(st->conn_attr_vals[i]);
            if (alen + kl + vl + 20 > acap) {
                acap = (alen + kl + vl + 20) * 2;
                attrs = (uint8_t*)realloc(attrs, acap);
            }
            alen += lenenc_put(attrs + alen, kl);
            memcpy(attrs + alen, st->conn_attr_keys[i], kl); alen += kl;
            alen += lenenc_put(attrs + alen, vl);
            memcpy(attrs + alen, st->conn_attr_vals[i], vl); alen += vl;
        }
    }
    size_t pkt_len = 1 + (ulen + 1) + lenenc_size(auth_len) + auth_len + (dlen + 1) + 2 + (plen + 1) + lenenc_size(alen) + alen;
    uint8_t *buf = (uint8_t*)malloc(pkt_len + 64);
    if (!buf) { set_err(m, 2008, "HY000", "Out of memory"); return 1; }
    size_t o = 0;
    buf[o++] = 0x11;
    memcpy(buf + o, user, ulen); o += ulen; buf[o++] = 0;
    o += lenenc_put(buf + o, auth_len);
    if (auth_len > 0) { memcpy(buf + o, auth_resp, auth_len); o += auth_len; }
    memcpy(buf + o, db ? db : "", dlen); o += dlen; buf[o++] = 0;
    buf[o++] = (uint8_t)(st->charsetnr); buf[o++] = (uint8_t)(st->charsetnr >> 8);
    memcpy(buf + o, plugin, plen); o += plen; buf[o++] = 0;
    if (m->client_flag & CLIENT_CONNECT_ATTRS) {
        o += lenenc_put(buf + o, alen);
        memcpy(buf + o, attrs, alen); o += alen;
    }
    int rc = write_compressed_packet(m, buf, o, 0);
    free(buf);
    free(attrs);
    if (rc < 0) return 1;
    if (finish_auth(m, passwd, scramble, plugin) < 0) return 1;
    free(m->user); m->user = user ? strdup(user) : NULL;
    free(m->db);   m->db = (db && *db) ? strdup(db) : NULL;
    free(st->saved_user); st->saved_user = user ? strdup(user) : NULL;
    free(st->saved_passwd); st->saved_passwd = passwd ? strdup(passwd) : NULL;
    free(st->saved_db); st->saved_db = (db && *db) ? strdup(db) : NULL;
    st->saved_scramble_set = 0;
    m->multi_status = 0;
    mm_stmt_cache *cache = (mm_stmt_cache*)m->stmt_cache;
    if (cache) {
        for (unsigned int i = 0; i < cache->count; i++) free(cache->entries[i].sql);
        cache->count = 0;
    }
    for (int i = 0; i < 6; i++) {
        free(st->session_track[i]); st->session_track[i] = NULL;
        st->session_track_len[i] = 0; st->session_track_pos[i] = 0;
    }
    return 0;
}

int mysql_more_results(MYSQL *m) {
    return (m && m->multi_status) ? 1 : 0;
}

int mysql_next_result(MYSQL *m) {
    if (!m || m->fd < 0) return -1;
    if (!m->multi_status) return -1;
    clear_err(m);
    if (m->status != MYSQL_STATUS_READY && m->field_count > 0) {
        for (unsigned int i = 0; i < m->field_count; i++) {
            uint8_t *dp = NULL; size_t dl = 0;
            if (read_compressed_packet(m, &dp, &dl) < 0) return 1;
            free(dp);
        }
        if (!ST(m)->deprecate_eof) {
            uint8_t *dp = NULL; size_t dl = 0;
            if (read_compressed_packet(m, &dp, &dl) < 0) return 1;
            free(dp);
        }
        for (;;) {
            uint8_t *dp = NULL; size_t dl = 0;
            if (read_compressed_packet(m, &dp, &dl) < 0) return 1;
            if (dl > 0 && dp[0] == 0xFF) { parse_err(m, dp, dl); free(dp); return 1; }
            if ((ST(m)->deprecate_eof && dl > 0 && dp[0] == 0xFE) ||
                (!ST(m)->deprecate_eof && dl > 0 && dp[0] == 0xFE && dl < 9)) {
                parse_ok(m, dp, dl);
                m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
                free(dp); break;
            }
            free(dp);
        }
        m->field_count = 0;
        m->status = MYSQL_STATUS_READY;
        if (!m->multi_status) return -1;
    }
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) return 1;
    if (len > 0 && p[0] == 0xFB) {
        free(p);
        set_err(m, 2050, "HY000", "LOCAL INFILE is not implemented");
        return 1;
    }
    int kind = packet_kind(m, p, len);
    if (kind == 2) { parse_err(m, p, len); free(p); return 1; }
    if (kind == 1 || kind == 3) {
        parse_ok(m, p, len);
        m->field_count = 0;
        m->status = MYSQL_STATUS_READY;
        m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
        free(p);
        return 0;
    }
    const uint8_t *cur = p, *end = p + len;
    uint64_t cols = lenenc_int(&cur, end, NULL);
    m->field_count = (unsigned int)cols;
    m->status = MYSQL_STATUS_GET_RESULT;
    free(p);
    return 0;
}

int mysql_select_db(MYSQL *m, const char *db) {
    if (!m || m->fd < 0) return 1;
    m->seq = 0;
    size_t l = strlen(db);
    uint8_t *buf = (uint8_t*)malloc(l + 1);
    if (!buf) { set_err(m, 2008, "HY000", "Out of memory"); return 1; }
    buf[0] = 0x02; /* COM_INIT_DB */
    memcpy(buf + 1, db, l);
    int rc = write_compressed_packet(m, buf, l + 1, 0);
    free(buf);
    if (rc < 0) return 1;
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) return 1;
    int err = (len && p[0] == 0xFF);
    if (err) parse_err(m, p, len);
    else { free(m->db); m->db = strdup(db); }
    free(p);
    return err ? 1 : 0;
}

int mysql_set_character_set(MYSQL *m, const char *cs) {
    if (!m || !cs) return 1;
    unsigned int nr = charset_name_to_nr(cs);
    if (!nr) { set_err(m, 2019, "HY000", "Unknown character set"); return 1; }
    char q[256];
    char esc[128];
    unsigned long el = mysql_real_escape_string_quote(m, esc, cs, (unsigned long)strlen(cs), '`');
    int n = snprintf(q, sizeof(q), "SET NAMES `%.*s`", (int)el, esc);
    int rc = mysql_real_query(m, q, (unsigned long)n);
    if (rc) return rc;
    if (m->charset_name) free(m->charset_name);
    m->charset_name = strdup(cs);
    ST(m)->charsetnr = nr;
    ST(m)->mbmaxlen = charset_mbmaxlen(nr);
    return 0;
}

static int is_mb_lead_gbk(unsigned char c) {
    return c >= 0x81 && c <= 0xFE;
}

static int is_mb_trail_gbk(unsigned char c) {
    return (c >= 0x40 && c <= 0x7E) || (c >= 0x80 && c <= 0xFE);
}

static int is_mb_lead_big5(unsigned char c) {
    return c >= 0x81 && c <= 0xFE;
}

static int is_mb_trail_big5(unsigned char c) {
    return (c >= 0x40 && c <= 0x7E) || (c >= 0xA1 && c <= 0xFE);
}

static int is_mb_lead_sjis(unsigned char c) {
    return (c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFC);
}

static int is_mb_trail_sjis(unsigned char c) {
    return (c >= 0x40 && c <= 0x7E) || (c >= 0x80 && c <= 0xFC);
}

static int is_mb_lead_euckr(unsigned char c) {
    return c >= 0xA1 && c <= 0xFE;
}

static int is_mb_trail_euckr(unsigned char c) {
    return c >= 0xA1 && c <= 0xFE;
}

static int mbcharlen_utf8(unsigned char c) {
    if (c < 0x80) return 1;
    if (c < 0xC0) return 0;
    if (c < 0xE0) return 2;
    if (c < 0xF0) return 3;
    if (c < 0xF8) return 4;
    return 0;
}

static int is_valid_utf8_cont(unsigned char c) {
    return c >= 0x80 && c <= 0xBF;
}

static int is_valid_mb_gbk(const unsigned char *p, const unsigned char *end) {
    if (p + 2 > end) return 0;
    return is_mb_lead_gbk(p[0]) && is_mb_trail_gbk(p[1]);
}

static int is_valid_mb_big5(const unsigned char *p, const unsigned char *end) {
    if (p + 2 > end) return 0;
    return is_mb_lead_big5(p[0]) && is_mb_trail_big5(p[1]);
}

static int is_valid_mb_sjis(const unsigned char *p, const unsigned char *end) {
    if (p + 2 > end) return 0;
    return is_mb_lead_sjis(p[0]) && is_mb_trail_sjis(p[1]);
}

static int is_valid_mb_euckr(const unsigned char *p, const unsigned char *end) {
    if (p + 2 > end) return 0;
    return is_mb_lead_euckr(p[0]) && is_mb_trail_euckr(p[1]);
}

static int is_valid_mb_utf8(const unsigned char *p, const unsigned char *end) {
    int l = mbcharlen_utf8(p[0]);
    if (l <= 1) return l == 1;
    if (p + l > end) return 0;
    for (int i = 1; i < l; i++) {
        if (!is_valid_utf8_cont(p[i])) return 0;
    }
    return 1;
}

static int is_valid_mb_eucjpms(const unsigned char *p, const unsigned char *end) {
    if (p + 2 > end) return 0;
    if (p[0] >= 0xA1 && p[0] <= 0xFE && p[1] >= 0xA1 && p[1] <= 0xFE) return 1;
    if (p[0] == 0x8E && p[1] >= 0xA1 && p[1] <= 0xFE) return 1;
    if (p + 3 <= end && p[0] == 0x8F && p[1] >= 0xA1 && p[1] <= 0xFE && p[2] >= 0xA1 && p[2] <= 0xFE) return 1;
    return 0;
}

static int eucjpms_mbcharlen(const unsigned char *p, const unsigned char *end) {
    if (p[0] == 0x8F && p + 3 <= end && p[1] >= 0xA1 && p[1] <= 0xFE && p[2] >= 0xA1 && p[2] <= 0xFE) return 3;
    if ((p[0] >= 0xA1 && p[0] <= 0xFE) || p[0] == 0x8E) return 2;
    return 1;
}

static int is_valid_mb_ujis(const unsigned char *p, const unsigned char *end) {
    if (p + 2 > end) return 0;
    if (p[0] >= 0xA1 && p[0] <= 0xFE && p[1] >= 0xA1 && p[1] <= 0xFE) return 1;
    if (p[0] == 0x8E && p[1] >= 0xA1 && p[1] <= 0xFE) return 1;
    if (p + 3 <= end && p[0] == 0x8F && p[1] >= 0xA1 && p[1] <= 0xFE && p[2] >= 0xA1 && p[2] <= 0xFE) return 1;
    return 0;
}

static int ujis_mbcharlen(const unsigned char *p, const unsigned char *end) {
    if (p[0] == 0x8F && p + 3 <= end && p[1] >= 0xA1 && p[1] <= 0xFE && p[2] >= 0xA1 && p[2] <= 0xFE) return 3;
    if ((p[0] >= 0xA1 && p[0] <= 0xFE) || p[0] == 0x8E) return 2;
    return 1;
}

static int is_valid_mb_gb18030(const unsigned char *p, const unsigned char *end) {
    if (is_mb_lead_gbk(p[0]) && p + 2 <= end && is_mb_trail_gbk(p[1])) return 2;
    if (p[0] >= 0x81 && p[0] <= 0xFE && p + 4 <= end &&
        p[1] >= 0x30 && p[1] <= 0x39 &&
        p[2] >= 0x81 && p[2] <= 0xFE &&
        p[3] >= 0x30 && p[3] <= 0x39) return 4;
    return 0;
}

unsigned long mysql_real_escape_string_quote(MYSQL *m, char *to, const char *from,
                                              unsigned long len, char quote) {
    unsigned long i, o = 0;
    unsigned int mbmax = 1;
    unsigned int csnr = 0;
    if (m && ST(m)) {
        mbmax = ST(m)->mbmaxlen;
        csnr = ST(m)->charsetnr;
    }
    const unsigned char *p = (const unsigned char*)from;
    const unsigned char *end = p + len;
    for (i = 0; i < len; ) {
        unsigned char c = p[i];
        if (mbmax > 1) {
            int mblen = 0;
            int valid = 0;
            switch (csnr) {
                case 28: case 24:
                    if (is_mb_lead_gbk(c)) {
                        valid = is_valid_mb_gbk(p + i, end);
                        mblen = valid ? 2 : 0;
                    }
                    break;
                case 248:
                    if (is_mb_lead_gbk(c) || (c >= 0x81 && c <= 0xFE)) {
                        int v = is_valid_mb_gb18030(p + i, end);
                        if (v) { valid = 1; mblen = v; }
                        else { valid = 0; mblen = 0; }
                    }
                    break;
                case 1:
                    if (is_mb_lead_big5(c)) {
                        valid = is_valid_mb_big5(p + i, end);
                        mblen = valid ? 2 : 0;
                    }
                    break;
                case 13:
                    if (is_mb_lead_sjis(c)) {
                        valid = is_valid_mb_sjis(p + i, end);
                        mblen = valid ? 2 : 0;
                    }
                    break;
                case 19:
                    if (is_mb_lead_euckr(c)) {
                        valid = is_valid_mb_euckr(p + i, end);
                        mblen = valid ? 2 : 0;
                    }
                    break;
                case 97:
                    if (c >= 0xA1 || c == 0x8E || c == 0x8F) {
                        valid = is_valid_mb_eucjpms(p + i, end);
                        if (valid) mblen = eucjpms_mbcharlen(p + i, end);
                    }
                    break;
                case 12:
                    if (c >= 0xA1 || c == 0x8E || c == 0x8F) {
                        valid = is_valid_mb_ujis(p + i, end);
                        if (valid) mblen = ujis_mbcharlen(p + i, end);
                    }
                    break;
                default:
                    if (c >= 0x80) {
                        valid = is_valid_mb_utf8(p + i, end);
                        if (valid) mblen = mbcharlen_utf8(c);
                    }
                    break;
            }
            if (valid && mblen > 1) {
                for (int j = 0; j < mblen && i < len; j++)
                    to[o++] = p[i++];
                continue;
            }
            if (!valid && mblen == 0 && c >= 0x80) {
                int lead_len = 0;
                switch (csnr) {
                    case 28: case 24: lead_len = 2; break;
                    case 248: lead_len = 2; break;
                    case 1: lead_len = 2; break;
                    case 13: lead_len = 2; break;
                    case 19: lead_len = 2; break;
                    case 97: lead_len = eucjpms_mbcharlen(p + i, end); if (lead_len < 2) lead_len = 0; break;
                    case 12: lead_len = ujis_mbcharlen(p + i, end); if (lead_len < 2) lead_len = 0; break;
                    default: lead_len = mbcharlen_utf8(c); break;
                }
                if (lead_len > 1) {
                    to[o++] = '\\';
                    to[o++] = c;
                    i++;
                    continue;
                }
            }
        }
        char e = 0;
        switch (c) {
            case '\0': e = '0'; break;
            case '\n': e = 'n'; break;
            case '\r': e = 'r'; break;
            case '\\': e = '\\'; break;
            case '\'': if (quote == '\'') e = '\''; break;
            case '"':  if (quote == '"')  e = '"';  break;
            case 0x1a: e = 'Z'; break;
            default: to[o++] = c; i++; continue;
        }
        if (e) { to[o++] = '\\'; to[o++] = e; }
        else   { to[o++] = c; }
        i++;
    }
    to[o] = 0;
    return o;
}

unsigned long mysql_real_escape_string(MYSQL *m, char *to, const char *from, unsigned long len) {
    return mysql_real_escape_string_quote(m, to, from, len, '\'');
}

unsigned long mysql_escape_string(char *to, const char *from, unsigned long len) {
    return mysql_real_escape_string_quote(NULL, to, from, len, '\'');
}

const char *mysql_get_server_info(MYSQL *m) { return (m && m->server_version) ? m->server_version : ""; }
const char *mysql_get_host_info(MYSQL *m)   {
    if (!m) return "";
    if (m->fd < 0) return "";
    free(m->host_info);
    size_t n = snprintf(NULL, 0, "%s via TCP/IP", m->host ? m->host : "localhost");
    m->host_info = (char*)malloc(n + 1);
    if (!m->host_info) return "";
    snprintf(m->host_info, n + 1, "%s via TCP/IP", m->host ? m->host : "localhost");
    return m->host_info;
}
const char *mysql_get_client_info(void)     { return MM_VERSION_STR; }
unsigned long mysql_thread_id(MYSQL *m)     { return m ? m->thread_id : 0; }
const char *mysql_character_set_name(MYSQL *m) {
    if (!m) return "";
    return m->charset_name ? m->charset_name : "utf8mb4";
}
unsigned int mysql_get_proto_info(MYSQL *m) {
    return m ? m->protocol_version : 0;
}

int mysql_autocommit(MYSQL *m, my_bool mode) {
    if (!m) return 1;
    char q[64];
    snprintf(q, sizeof(q), "SET AUTOCOMMIT=%d", mode ? 1 : 0);
    return mysql_real_query(m, q, (unsigned long)strlen(q));
}

int mysql_commit(MYSQL *m) {
    if (!m) return 1;
    return mysql_real_query(m, "COMMIT", 6);
}

int mysql_rollback(MYSQL *m) {
    if (!m) return 1;
    return mysql_real_query(m, "ROLLBACK", 8);
}

unsigned long mysql_get_server_version(MYSQL *m) {
    if (!m || !m->server_version) return 0;
    unsigned int a = 0, b = 0, c = 0;
    sscanf(m->server_version, "%u.%u.%u", &a, &b, &c);
    return a * 10000UL + b * 100UL + c;
}

void mysql_close(MYSQL *m) {
    if (!m) return;
    mm_state *st = ST(m);
    if (st && st->trans) {
        m->seq = 0;
        uint8_t cmd = 0x01; /* COM_QUIT */
        write_compressed_packet(m, &cmd, 1, 0);
        st->trans->close(st->trans);
        st->trans = NULL;
    }
    m->fd = -1;
    int free_handle = 0;
    if (st) {
        free_handle = st->free_handle;
        free(st->ssl_ca); free(st->ssl_capath); free(st->ssl_cert);
        free(st->ssl_key); free(st->ssl_cipher);
        free(st->saved_host); free(st->saved_user); free(st->saved_passwd);
        free(st->saved_db); free(st->saved_unix_socket);
        free(st->comp_buf);
        for (int i = 0; i < 6; i++) free(st->session_track[i]);
        for (unsigned int i = 0; i < st->conn_attr_count; i++) {
            free(st->conn_attr_keys[i]); free(st->conn_attr_vals[i]);
        }
        free(st->conn_attr_keys); free(st->conn_attr_vals);
        free(st);
        m->st = NULL;
    }
    free(m->host); free(m->user); free(m->db);
    free(m->server_version); free(m->charset_name); free(m->info); free(m->host_info);
    mm_cache_free((mm_stmt_cache*)m->stmt_cache);
    m->host = m->user = m->db = m->server_version = m->charset_name = m->info = m->host_info = NULL;
    m->stmt_cache = NULL;
    if (free_handle) free(m);   /* allocated by mysql_init(NULL) */
}

/* ===================================================================== *
 *  Prepared statements (binary protocol) — Stage 3 foundation.
 *
 *  Implemented this round: mysql_stmt_init, mysql_stmt_prepare (sends
 *  COM_STMT_PREPARE and parses the full response: prepare-OK header, the
 *  parameter- and column-definition packets and their EOFs), mysql_stmt_close,
 *  and the metadata/error accessors. mysql_stmt_bind_* record the caller's
 *  binds. Execution and binary row fetching arrive in the next round; those
 *  entry points return a clear "not yet implemented" error until then.
 * ===================================================================== */

#define COM_STMT_PREPARE        0x16
#define COM_STMT_EXECUTE        0x17
#define COM_STMT_SEND_LONG_DATA 0x18
#define COM_STMT_CLOSE          0x19
#define COM_STMT_RESET          0x1A

static void stmt_err(MYSQL_STMT *s, unsigned int code, const char *state, const char *msg) {
    s->last_errno = code;
    strncpy(s->sqlstate, state ? state : "HY000", sizeof(s->sqlstate) - 1);
    s->sqlstate[sizeof(s->sqlstate) - 1] = 0;
    strncpy(s->last_error, msg ? msg : "", sizeof(s->last_error) - 1);
    s->last_error[sizeof(s->last_error) - 1] = 0;
}
static void stmt_clear_err(MYSQL_STMT *s) {
    s->last_errno = 0; s->last_error[0] = 0; strcpy(s->sqlstate, "00000");
}
/* Lift the connection's last error onto the statement. */
static void stmt_err_from_conn(MYSQL_STMT *s) {
    stmt_err(s, s->mysql->last_errno, s->mysql->sqlstate, s->mysql->last_error);
}
static void stmt_result_free(MYSQL_STMT *s);   /* defined with the fetch code */

static void free_field_array(MYSQL_FIELD *f, unsigned int n) {
    if (!f) return;
    for (unsigned int i = 0; i < n; i++) {
        free(f[i].name); free(f[i].org_name); free(f[i].table);
        free(f[i].org_table); free(f[i].db); free(f[i].catalog); free(f[i].def);
    }
    free(f);
}
static void stmt_free_metadata(MYSQL_STMT *s) {
    free_field_array(s->param_fields, (unsigned int)s->param_count);
    free_field_array(s->fields, s->field_count);
    s->param_fields = NULL; s->fields = NULL;
    s->param_count = 0; s->field_count = 0;
}

/* Read and discard `n` column-definition packets plus the trailing EOF. */
static int stmt_skip_defs(MYSQL *m, unsigned int n) {
    if (n == 0) return 0;
    for (unsigned int i = 0; i < n; i++) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) return -1;
        if (len && p[0] == 0xFF) { parse_err(m, p, len); free(p); return -1; }
        free(p);
    }
    if (!ST(m)->deprecate_eof) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) return -1;
        free(p);
    }
    return 0;
}

/* Read `n` column-definition packets into a fresh array, then the trailing EOF
 * (present because we do not advertise CLIENT_DEPRECATE_EOF). */
static int stmt_read_defs(MYSQL *m, MYSQL_FIELD **arr, unsigned int n) {
    *arr = NULL;
    if (n == 0) return 0;
    MYSQL_FIELD *f = (MYSQL_FIELD*)calloc(n, sizeof(MYSQL_FIELD));
    if (!f) { set_err(m, 2008, "HY000", "Out of memory"); return -1; }
    for (unsigned int i = 0; i < n; i++) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) { free_field_array(f, n); return -1; }
        if (len && p[0] == 0xFF) { parse_err(m, p, len); free(p); free_field_array(f, n); return -1; }
        parse_field(p, len, &f[i]);
        free(p);
    }
    if (!ST(m)->deprecate_eof) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) { free_field_array(f, n); return -1; }
        if (len >= 5) m->server_status = (unsigned)(p[3] | (p[4] << 8));
        free(p);
    }
    *arr = f;
    return 0;
}

MYSQL_STMT *mysql_stmt_init(MYSQL *m) {
    if (!m) return NULL;
    MYSQL_STMT *s = (MYSQL_STMT*)calloc(1, sizeof(MYSQL_STMT));
    if (!s) { set_err(m, 2008, "HY000", "Out of memory"); return NULL; }
    s->mysql = m;
    s->state = MYSQL_STMT_INIT_DONE;
    s->flags = 0;
    s->prefetch_rows = 0;
    s->server_status = 0;
    s->send_types_to_server = 0;
    s->unbuffered_fetch_cancelled = 0;
    s->update_max_length = 0;
    strcpy(s->sqlstate, "00000");
    return s;
}

int mysql_stmt_prepare(MYSQL_STMT *s, const char *q, unsigned long qlen) {
    if (!s || !s->mysql) return 1;
    if (!q) { stmt_err(s, CR_COMMANDS_OUT_OF_SYNC, "HY000", "Null statement"); return 1; }
    if (qlen == (unsigned long)(-1)) qlen = (unsigned long)strlen(q);
    MYSQL *m = s->mysql;
    if (m->fd < 0) { stmt_err(s, 2006, "HY000", "Server has gone away"); return 1; }
    stmt_clear_err(s);
    stmt_result_free(s);
    stmt_free_metadata(s);
    s->bind_param_done = 0; s->bind_result_done = 0;

    s->stmt_id = 0;
    m->seq = 0;
    uint8_t *buf = (uint8_t*)malloc(qlen + 1);
    if (!buf) { stmt_err(s, 2008, "HY000", "Out of memory"); return 1; }
    buf[0] = COM_STMT_PREPARE;
    memcpy(buf + 1, q, qlen);
    int rc = write_compressed_packet(m, buf, qlen + 1, 0);
    free(buf);
    if (rc < 0) { stmt_err_from_conn(s); return 1; }

    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) { stmt_err_from_conn(s); return 1; }
    if (len && p[0] == 0xFF) { parse_err(m, p, len); stmt_err_from_conn(s); free(p); return 1; }
    if (len < 12 || p[0] != 0x00) { free(p); stmt_err(s, 2013, "HY000", "Malformed COM_STMT_PREPARE response"); return 1; }

    const uint8_t *c = p + 1;
    s->stmt_id     = (uint32_t)c[0] | (uint32_t)c[1]<<8 | (uint32_t)c[2]<<16 | (uint32_t)c[3]<<24; c += 4;
    s->field_count = (unsigned int)c[0] | (unsigned int)c[1]<<8; c += 2;
    s->param_count = (unsigned long)c[0] | (unsigned long)c[1]<<8; c += 2;
    if (s->field_count > 4096 || s->param_count > 4096) {
        free(p); stmt_err(s, 2013, "HY000", "Too many columns/parameters in prepared statement");
        return 1;
    }
    c += 1;
    s->warning_count = (unsigned int)c[0] | (unsigned int)c[1]<<8;
    free(p);

    if (stmt_read_defs(m, &s->param_fields, (unsigned int)s->param_count) < 0) {
        stmt_err_from_conn(s); stmt_free_metadata(s); return 1;
    }
    if (stmt_read_defs(m, &s->fields, s->field_count) < 0) {
        stmt_err_from_conn(s); stmt_free_metadata(s); return 1;
    }

    mm_cache_put((mm_stmt_cache*)m->stmt_cache, q, (size_t)qlen, s->stmt_id);
    m->last_errno = 0; m->last_error[0] = '\0'; m->sqlstate[0] = '\0';
    s->state = MYSQL_STMT_PREPARE_DONE;
    return 0;
}

/* Binds are recorded now; they are consumed by execute/fetch next round. */
int mysql_stmt_bind_param(MYSQL_STMT *s, MYSQL_BIND *b) {
    if (!s) return 1;
    s->bind_params = b; s->bind_param_done = 1;
    return 0;
}
int mysql_stmt_bind_result(MYSQL_STMT *s, MYSQL_BIND *b) {
    if (!s) return 1;
    s->bind_results = b; s->bind_result_done = 1;
    s->bind_result_count = s->field_count;
    return 0;
}

/* ----------------------- binary protocol: helpers ----------------------- */

/* small growable byte buffer */
typedef struct { uint8_t *p; size_t n, cap; int oom; } bbuf;
static void bb_need(bbuf *b, size_t k) {
    if (b->n + k > b->cap) {
        size_t nc = (b->n + k) * 2 + 64;
        uint8_t *np = (uint8_t*)realloc(b->p, nc);
        if (!np) { b->oom = 1; return; }
        b->p = np; b->cap = nc;
    }
}
static void bb_put(bbuf *b, const void *d, size_t k) { if (k == 0) return; bb_need(b, k); if (!b->oom) { memcpy(b->p+b->n, d, k); b->n += k; } }
static void bb_u8(bbuf *b, uint8_t v) { bb_put(b, &v, 1); }
static void bb_u16(bbuf *b, uint16_t v){ uint8_t t[2]={(uint8_t)v,(uint8_t)(v>>8)}; bb_put(b,t,2); }
static void bb_u32(bbuf *b, uint32_t v){ uint8_t t[4]={(uint8_t)v,(uint8_t)(v>>8),(uint8_t)(v>>16),(uint8_t)(v>>24)}; bb_put(b,t,4); }
static void bb_u64(bbuf *b, uint64_t v){ uint8_t t[8]; for(int i=0;i<8;i++) t[i]=(uint8_t)(v>>(8*i)); bb_put(b,t,8); }
static void bb_lenenc(bbuf *b, uint64_t v) {
    if (v < 251) bb_u8(b, (uint8_t)v);
    else if (v < 0x10000) { bb_u8(b, 0xFC); bb_u16(b, (uint16_t)v); }
    else if (v < 0x1000000) { bb_u8(b, 0xFD); uint8_t t[3]={(uint8_t)v,(uint8_t)(v>>8),(uint8_t)(v>>16)}; bb_put(b,t,3); }
    else { bb_u8(b, 0xFE); bb_u64(b, v); }
}

/* Is this column type carried on the wire as a length-encoded string? */
static int type_is_string(enum enum_field_types t) {
    switch (t) {
        case MYSQL_TYPE_DECIMAL: case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_BIT:     case MYSQL_TYPE_JSON:
        case MYSQL_TYPE_ENUM:    case MYSQL_TYPE_SET:
        case MYSQL_TYPE_TINY_BLOB: case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:  case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_VAR_STRING: case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VARCHAR:    case MYSQL_TYPE_GEOMETRY:
            return 1;
        default: return 0;
    }
}

/* A decoded column value in its natural form. str points into the row buffer. */
typedef struct {
    enum { V_NULL, V_INT, V_DBL, V_TIME, V_STR } kind;
    int           is_unsigned;
    int           is_bit;
    long long     i;
    double        d;
    MYSQL_TIME    t;
    const uint8_t *str; size_t slen;
} cval;

/* Decode one binary-protocol value of `type` at *cur (advancing it). */
static int decode_value(const uint8_t **cur, const uint8_t *end,
                        enum enum_field_types type, int is_unsigned, cval *v) {
    memset(v, 0, sizeof(*v));
    v->is_unsigned = is_unsigned;
    if (type_is_string(type)) {
        int isn = 0; uint64_t n = lenenc_int(cur, end, &isn);
        if (*cur + n > end) return -1;
        v->kind = V_STR; v->str = *cur; v->slen = (size_t)n; *cur += n;
        if (type == MYSQL_TYPE_BIT) v->is_bit = 1;
        return 0;
    }
    switch (type) {
        case MYSQL_TYPE_TINY:
            if (*cur + 1 > end) return -1;
            v->kind = V_INT; v->i = is_unsigned ? (long long)(*cur)[0] : (long long)(int8_t)(*cur)[0]; *cur += 1; return 0;
        case MYSQL_TYPE_SHORT: case MYSQL_TYPE_YEAR: {
            if (*cur + 2 > end) return -1;
            uint16_t u = (uint16_t)((*cur)[0] | (*cur)[1]<<8);
            v->kind = V_INT; v->i = is_unsigned ? (long long)u : (long long)(int16_t)u; *cur += 2; return 0; }
        case MYSQL_TYPE_LONG: case MYSQL_TYPE_INT24: {
            if (*cur + 4 > end) return -1;
            uint32_t u = (uint32_t)((*cur)[0] | (*cur)[1]<<8 | (*cur)[2]<<16 | (uint32_t)(*cur)[3]<<24);
            v->kind = V_INT; v->i = is_unsigned ? (long long)u : (long long)(int32_t)u; *cur += 4; return 0; }
        case MYSQL_TYPE_LONGLONG: {
            if (*cur + 8 > end) return -1;
            uint64_t u = 0; for (int k=0;k<8;k++) u |= (uint64_t)(*cur)[k] << (8*k);
            v->kind = V_INT; v->i = (long long)u; *cur += 8; return 0; }
        case MYSQL_TYPE_FLOAT: {
            if (*cur + 4 > end) return -1;
            float f; memcpy(&f, *cur, 4); v->kind = V_DBL; v->d = (double)f; *cur += 4; return 0; }
        case MYSQL_TYPE_DOUBLE: {
            if (*cur + 8 > end) return -1;
            double dd; memcpy(&dd, *cur, 8); v->kind = V_DBL; v->d = dd; *cur += 8; return 0; }
        case MYSQL_TYPE_NULL:
            v->kind = V_NULL; return 0;
        case MYSQL_TYPE_DATE: case MYSQL_TYPE_DATETIME: case MYSQL_TYPE_TIMESTAMP: {
            if (*cur + 1 > end) return -1;
            uint8_t L = (*cur)[0]; const uint8_t *q = *cur + 1;
            if (q + L > end) return -1;
            v->kind = V_TIME; v->t.time_type = (type==MYSQL_TYPE_DATE)?MYSQL_TIMESTAMP_DATE:MYSQL_TIMESTAMP_DATETIME;
            if (L >= 4) { v->t.year=(unsigned)(q[0]|q[1]<<8); v->t.month=q[2]; v->t.day=q[3]; }
            if (L >= 7) { v->t.hour=q[4]; v->t.minute=q[5]; v->t.second=q[6]; }
            if (L >= 11){ v->t.second_part=(unsigned long)(q[7]|q[8]<<8|q[9]<<16|(uint32_t)q[10]<<24); }
            *cur += 1 + L; return 0; }
        case MYSQL_TYPE_TIME: {
            if (*cur + 1 > end) return -1;
            uint8_t L = (*cur)[0]; const uint8_t *q = *cur + 1;
            if (q + L > end) return -1;
            v->kind = V_TIME; v->t.time_type = MYSQL_TIMESTAMP_TIME;
            if (L >= 8) { v->t.neg=q[0]; uint32_t days=(uint32_t)(q[1]|q[2]<<8|q[3]<<16|(uint32_t)q[4]<<24);
                          v->t.hour=days*24+q[5]; v->t.minute=q[6]; v->t.second=q[7]; }
            if (L >= 12){ v->t.second_part=(unsigned long)(q[8]|q[9]<<8|q[10]<<16|(uint32_t)q[11]<<24); }
            *cur += 1 + L; return 0; }
        default: {  /* unknown: treat as length-encoded string */
            int isn = 0; uint64_t n = lenenc_int(cur, end, &isn);
            if (*cur + n > end) return -1;
            v->kind = V_STR; v->str = *cur; v->slen = (size_t)n; *cur += n; return 0; }
    }
}

/* Format a decoded value as text into buf (for string-target conversion). */
static int cval_to_text(const cval *v, char *buf, size_t cap);
static unsigned long cval_display_len(const cval *v, enum enum_field_types ft) {
    char tmp[64];
    switch (v->kind) {
        case V_NULL: return 0;
        case V_INT: return v->is_unsigned ? (unsigned long)snprintf(tmp, sizeof(tmp), "%llu", (unsigned long long)v->i)
                                          : (unsigned long)snprintf(tmp, sizeof(tmp), "%lld", v->i);
        case V_DBL: return (unsigned long)snprintf(tmp, sizeof(tmp), "%.17g", v->d);
        case V_STR: return v->slen;
        case V_TIME: return (unsigned long)cval_to_text(v, tmp, sizeof(tmp));
        default: (void)ft; return 0;
    }
}
static int cval_to_text(const cval *v, char *buf, size_t cap) {
    switch (v->kind) {
        case V_INT: return v->is_unsigned ? snprintf(buf, cap, "%llu", (unsigned long long)v->i)
                                          : snprintf(buf, cap, "%lld", v->i);
        case V_DBL: return snprintf(buf, cap, "%.17g", v->d);
        case V_TIME:
            if (v->t.time_type == MYSQL_TIMESTAMP_DATE) {
                if (v->t.second_part)
                    return snprintf(buf, cap, "%04u-%02u-%02u.%06lu", v->t.year, v->t.month, v->t.day, v->t.second_part);
                return snprintf(buf, cap, "%04u-%02u-%02u", v->t.year, v->t.month, v->t.day);
            }
            if (v->t.time_type == MYSQL_TIMESTAMP_TIME) {
                if (v->t.second_part)
                    return snprintf(buf, cap, "%s%02u:%02u:%02u.%06lu", v->t.neg?"-":"", v->t.hour, v->t.minute, v->t.second, v->t.second_part);
                return snprintf(buf, cap, "%s%02u:%02u:%02u", v->t.neg?"-":"", v->t.hour, v->t.minute, v->t.second);
            }
            if (v->t.second_part)
                return snprintf(buf, cap, "%04u-%02u-%02u %02u:%02u:%02u.%06lu",
                                v->t.year, v->t.month, v->t.day, v->t.hour, v->t.minute, v->t.second, v->t.second_part);
            return snprintf(buf, cap, "%04u-%02u-%02u %02u:%02u:%02u",
                            v->t.year, v->t.month, v->t.day, v->t.hour, v->t.minute, v->t.second);
        default: return 0;
    }
}
static long long cval_as_int(const cval *v) {
    switch (v->kind) {
        case V_INT: return v->i;
        case V_DBL: return (long long)v->d;
        case V_STR: {
            if (v->is_bit) {
                uint64_t u = 0;
                for (size_t k = 0; k < v->slen && k < 8; k++) u = (u << 8) | v->str[k];
                return (long long)u;
            }
            size_t n = v->slen < 255 ? v->slen : 255; char tmp[256]; memcpy(tmp, v->str, n); tmp[n]=0; return strtoll(tmp, NULL, 10);
        }
        default: return 0;
    }
}
static double cval_as_dbl(const cval *v) {
    switch (v->kind) {
        case V_INT: return v->is_unsigned ? (double)(unsigned long long)v->i : (double)v->i;
        case V_DBL: return v->d;
        case V_STR: {
            if (v->is_bit) {
                uint64_t u = 0;
                for (size_t k = 0; k < v->slen && k < 8; k++) u = (u << 8) | v->str[k];
                return (double)u;
            }
            size_t n = v->slen < 255 ? v->slen : 255; char tmp[256]; memcpy(tmp, v->str, n); tmp[n]=0; return strtod(tmp, NULL);
        }
        default: return 0;
    }
}

/* Store a decoded value into a result bind, converting to bind->buffer_type. */
static void store_into_bind(const cval *v, MYSQL_BIND *b) {
    unsigned long *lenp = b->length ? b->length : &b->length_value;
    my_bool *nullp = b->is_null ? b->is_null : &b->is_null_value;
    my_bool *errp  = b->error   ? b->error   : &b->error_value;
    *nullp = 0; *errp = 0; *lenp = 0;
    if (v->kind == V_NULL) { *nullp = 1; return; }

    switch (b->buffer_type) {
        case MYSQL_TYPE_TINY:     if (b->buffer) {
                                      uint64_t uval; int64_t ival; int src_unsigned = (v->is_unsigned && v->kind == V_INT) || v->is_bit;
                                      if (v->is_bit) {
                                          uval = 0;
                                          for (size_t k = 0; k < v->slen && k < 8; k++) uval = (uval << 8) | v->str[k];
                                      } else if (src_unsigned) uval = (uint64_t)v->i; else ival = cval_as_int(v);
                                      if (b->is_unsigned) {
                                          uint8_t tv = src_unsigned ? (uint8_t)uval : (uint8_t)ival;
                                          *(uint8_t*)b->buffer = tv;
                                          if (src_unsigned ? (uval != tv) : ((uint64_t)ival != tv)) *errp=1;
                                      } else {
                                          if (src_unsigned) { if(uval > 127) *errp=1; *(int8_t*)b->buffer=(int8_t)(uint8_t)uval; }
                                          else { *(int8_t*)b->buffer=(int8_t)ival; if(ival!=((int64_t)(int8_t)ival)) *errp=1; }
                                      }
                                  } *lenp = 1; break;
        case MYSQL_TYPE_SHORT: case MYSQL_TYPE_YEAR:
                                  if (b->buffer) {
                                      uint64_t uval; int64_t ival; int src_unsigned = (v->is_unsigned && v->kind == V_INT) || v->is_bit;
                                      if (v->is_bit) {
                                          uval = 0;
                                          for (size_t k = 0; k < v->slen && k < 8; k++) uval = (uval << 8) | v->str[k];
                                      } else if (src_unsigned) uval = (uint64_t)v->i; else ival = cval_as_int(v);
                                      if (b->is_unsigned) {
                                          uint16_t tv = src_unsigned ? (uint16_t)uval : (uint16_t)ival;
                                          *(uint16_t*)b->buffer = tv;
                                          if (src_unsigned ? (uval != tv) : ((uint64_t)ival != tv)) *errp=1;
                                      } else {
                                          if (src_unsigned) { if(uval > 32767) *errp=1; *(int16_t*)b->buffer=(int16_t)(uint16_t)uval; }
                                          else { *(int16_t*)b->buffer=(int16_t)ival; if(ival!=((int64_t)(int16_t)ival)) *errp=1; }
                                      }
                                  } *lenp = 2; break;
        case MYSQL_TYPE_LONG: case MYSQL_TYPE_INT24:
                                  if (b->buffer) {
                                      uint64_t uval; int64_t ival; int src_unsigned = (v->is_unsigned && v->kind == V_INT) || v->is_bit;
                                      if (v->is_bit) {
                                          uval = 0;
                                          for (size_t k = 0; k < v->slen && k < 8; k++) uval = (uval << 8) | v->str[k];
                                      } else if (src_unsigned) uval = (uint64_t)v->i; else ival = cval_as_int(v);
                                      if (b->is_unsigned) {
                                          uint32_t tv = src_unsigned ? (uint32_t)uval : (uint32_t)ival;
                                          *(uint32_t*)b->buffer = tv;
                                          if (src_unsigned ? (uval != tv) : ((uint64_t)ival != tv)) *errp=1;
                                      } else {
                                          if (src_unsigned) { if(uval > 2147483647ULL) *errp=1; *(int32_t*)b->buffer=(int32_t)(uint32_t)uval; }
                                          else { *(int32_t*)b->buffer=(int32_t)ival; if(ival!=((int64_t)(int32_t)ival)) *errp=1; }
                                      }
                                  } *lenp = 4; break;
        case MYSQL_TYPE_LONGLONG: if (b->buffer) {
                                      int src_unsigned = (v->is_unsigned && v->kind == V_INT) || v->is_bit;
                                      if (v->is_bit) {
                                          uint64_t uval = 0;
                                          for (size_t k = 0; k < v->slen && k < 8; k++) uval = (uval << 8) | v->str[k];
                                          if (b->is_unsigned) { *(uint64_t*)b->buffer = uval; }
                                          else { if(uval > 9223372036854775807ULL) *errp=1; *(int64_t*)b->buffer = (int64_t)uval; }
                                      } else if (src_unsigned) {
                                          uint64_t uv = (uint64_t)v->i;
                                          if (b->is_unsigned) { *(uint64_t*)b->buffer = uv; }
                                          else { if(uv > 9223372036854775807ULL) *errp=1; *(int64_t*)b->buffer = (int64_t)uv; }
                                      } else {
                                          int64_t iv = cval_as_int(v);
                                          if (b->is_unsigned) { *(uint64_t*)b->buffer = (uint64_t)iv; }
                                          else { *(int64_t*)b->buffer = iv; }
                                      }
                                  } *lenp = 8; break;
        case MYSQL_TYPE_FLOAT:    if (b->buffer) {
                                      float fv = (float)cval_as_dbl(v);
                                      *(float*)b->buffer = fv;
                                      if (cval_as_dbl(v) != (double)fv) *errp = 1;
                                  } *lenp = 4; break;
        case MYSQL_TYPE_DOUBLE:   if (b->buffer) { *(double*)b->buffer = cval_as_dbl(v); } *lenp = 8; break;
        case MYSQL_TYPE_NULL:     *lenp = 0; break;
        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_DATE: case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP: case MYSQL_TYPE_TIME:
            if (b->buffer) {
                MYSQL_TIME mt; memset(&mt, 0, sizeof mt);
                if (v->kind == V_TIME) mt = v->t;
                else if (v->kind == V_STR) {
                    if (b->buffer_type == MYSQL_TYPE_TIME) {
                        int h=0,mi=0,se=0; unsigned long sp=0; char neg=0;
                        const char *s = (const char*)v->str; size_t sl = v->slen;
                        if (sl > 0 && s[0] == '-') { neg = 1; s++; sl--; }
                        int pos = 0;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { h = h*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && s[pos] == ':') pos++;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { mi = mi*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && s[pos] == ':') pos++;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { se = se*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && s[pos] == '.') { pos++;
                            unsigned long fac=1000000;
                            while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9' && fac > 0) {
                                sp += (unsigned long)(s[pos]-'0') * fac; fac /= 10; pos++;
                            }
                        }
                        mt.time_type = MYSQL_TIMESTAMP_TIME; mt.neg = neg;
                        mt.hour = h; mt.minute = mi; mt.second = se; mt.second_part = sp;
                    } else {
                        int y=0,mo=0,d=0,h=0,mi=0,se=0; unsigned long sp=0;
                        const char *s = (const char*)v->str; size_t sl = v->slen;
                        int pos = 0;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { y = y*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && s[pos] == '-') pos++;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { mo = mo*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && s[pos] == '-') pos++;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { d = d*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && (s[pos] == ' ' || s[pos] == 'T')) pos++;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { h = h*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && s[pos] == ':') pos++;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { mi = mi*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && s[pos] == ':') pos++;
                        while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9') { se = se*10 + (s[pos]-'0'); pos++; }
                        if (pos < (int)sl && s[pos] == '.') { pos++;
                            unsigned long fac=1000000;
                            while (pos < (int)sl && s[pos] >= '0' && s[pos] <= '9' && fac > 0) {
                                sp += (unsigned long)(s[pos]-'0') * fac; fac /= 10; pos++;
                            }
                        }
                        mt.year = y; mt.month = mo; mt.day = d;
                        mt.hour = h; mt.minute = mi; mt.second = se; mt.second_part = sp;
                        if (b->buffer_type == MYSQL_TYPE_DATE) mt.time_type = MYSQL_TIMESTAMP_DATE;
                        else mt.time_type = MYSQL_TIMESTAMP_DATETIME;
                    }
                } else if (v->kind == V_INT) {
                    if (b->buffer_type == MYSQL_TYPE_TIME) {
                        mt.time_type = MYSQL_TIMESTAMP_TIME; mt.second = (unsigned long)(v->i % 100);
                        mt.minute = (unsigned long)((v->i / 100) % 100); mt.hour = (unsigned long)(v->i / 10000);
                    } else {
                        mt.time_type = (b->buffer_type == MYSQL_TYPE_DATE) ? MYSQL_TIMESTAMP_DATE : MYSQL_TIMESTAMP_DATETIME;
                    }
                }
                memcpy(b->buffer, &mt, sizeof mt);
            }
            *lenp = sizeof(MYSQL_TIME); break;
        default: {  /* string-like target */
            const uint8_t *src; size_t srclen; char tmp[64];
            if (v->kind == V_STR) { src = v->str; srclen = v->slen; }
            else { int n = cval_to_text(v, tmp, sizeof tmp); src = (const uint8_t*)tmp; srclen = (n>0)?(size_t)n:0; }
            *lenp = (unsigned long)srclen;
            if (b->offset > 0 && b->offset < srclen) {
                src += b->offset;
                srclen -= b->offset;
            } else if (b->offset > 0 && srclen > 0) {
                srclen = 0;
            }
            if (b->buffer && b->buffer_length) {
                size_t copy = srclen < b->buffer_length ? srclen : b->buffer_length;
                memcpy(b->buffer, src, copy);
                if (copy < srclen) *errp = 1;
                else if (copy < b->buffer_length) ((char*)b->buffer)[copy] = 0;
            } else if (srclen > 0) {
                *errp = 1;
            }
            break;
        }
    }
}

/* ----------------------- buffered result set ----------------------- */
typedef struct {
    int          stored;
    int          wire_eof;
    int          cursor_open;
    uint8_t    **rows; size_t *lens;
    my_ulonglong count, cursor;
    uint8_t     *cur_row;
    size_t       cur_row_len;
    uint8_t    **fetch_cache; size_t *fetch_cache_lens;
    size_t       fetch_cache_count;
    size_t       fetch_cache_pos;
} stmt_result;

static void stmt_result_free(MYSQL_STMT *s) {
    stmt_result *r = (stmt_result*)s->internal_result;
    if (!r) return;
    for (my_ulonglong i = 0; i < r->count; i++) free(r->rows[i]);
    free(r->rows); free(r->lens);
    if (!r->stored) free(r->cur_row);
    for (size_t i = 0; i < r->fetch_cache_count; i++) free(r->fetch_cache[i]);
    free(r->fetch_cache); free(r->fetch_cache_lens);
    free(r);
    s->internal_result = NULL;
    s->row_count = 0;
}

static int stmt_send_fetch(MYSQL_STMT *s, unsigned long num_rows) {
    MYSQL *m = s->mysql;
    uint8_t buf[9];
    buf[0] = 0x1C;
    buf[1] = (uint8_t)(s->stmt_id);       buf[2] = (uint8_t)(s->stmt_id >> 8);
    buf[3] = (uint8_t)(s->stmt_id >> 16);  buf[4] = (uint8_t)(s->stmt_id >> 24);
    buf[5] = (uint8_t)(num_rows);          buf[6] = (uint8_t)(num_rows >> 8);
    buf[7] = (uint8_t)(num_rows >> 16);    buf[8] = (uint8_t)(num_rows >> 24);
    m->seq = 0;
    return write_compressed_packet(m, buf, 9, 0);
}

/* Read one row packet. Returns 1=row (payload in out/len), 0=EOF/OK, -1=error. */
static int stmt_read_row(MYSQL_STMT *s, uint8_t **out, size_t *len) {
    MYSQL *m = s->mysql;
    uint8_t *p = NULL; size_t l = 0;
    if (read_compressed_packet(m, &p, &l) < 0) { stmt_err_from_conn(s); return -1; }
    if (l > 0 && p[0] == 0xFF) { parse_err(m, p, l); stmt_err_from_conn(s); free(p); return -1; }
    if (ST(m)->deprecate_eof) {
        if (l > 0 && p[0] == 0xFE) {
            parse_ok(m, p, l);
            m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
            free(p); return 0;
        }
    } else {
        if (l > 0 && p[0] == 0xFE && l < 9) {
            if (l >= 5) { s->warning_count = (unsigned)(p[1]|p[2]<<8); m->server_status = (unsigned)(p[3]|p[4]<<8); }
            m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
            free(p); return 0;
        }
    }
    *out = p; *len = l; return 1;
}

/* Decode the current binary row into the bound result buffers. */
static int stmt_bind_row(MYSQL_STMT *s, const uint8_t *p, size_t len) {
    const uint8_t *cur = p, *end = p + len;
    if (cur >= end || *cur != 0x00) { stmt_err(s, 2013, "HY000", "Malformed binary row"); return 1; }
    cur++;
    unsigned int nf = s->field_count;
    size_t nbytes = (nf + 7 + 2) / 8;
    const uint8_t *nullmap = cur;
    if (cur + nbytes > end) { stmt_err(s, 2013, "HY000", "Truncated binary row"); return 1; }
    cur += nbytes;

    int no_bind = (!s->bind_result_done || !s->bind_results);
    int truncated = 0;
    for (unsigned int i = 0; i < nf; i++) {
        int is_null = (nullmap[(i + 2) / 8] >> ((i + 2) % 8)) & 1;
        cval v;
        if (is_null) { v.kind = V_NULL; }
        else if (decode_value(&cur, end, s->fields[i].type,
                              (s->fields[i].flags & UNSIGNED_FLAG) != 0, &v) < 0) {
            stmt_err(s, 2013, "HY000", "Truncated binary row value"); return 1;
        }
        if (!no_bind && i < s->bind_result_count) {
            MYSQL_BIND *b = &s->bind_results[i];
            store_into_bind(&v, b);
            my_bool *errp = b->error ? b->error : &b->error_value;
            if (*errp) truncated = 1;
        }
    }
    return truncated ? MYSQL_DATA_TRUNCATED : 0;
}

/* ----------------------- public: execute / fetch ----------------------- */

int mysql_stmt_execute(MYSQL_STMT *s) {
    if (!s || !s->mysql) return 1;
    MYSQL *m = s->mysql;
    if (m->fd < 0) { stmt_err(s, 2006, "HY000", "Server has gone away"); return 1; }
    if (s->param_count > 0 && (!s->bind_param_done || !s->bind_params)) {
        stmt_err(s, 2031, "HY000", "No parameter binds; call mysql_stmt_bind_param first");
        return 1;
    }
    stmt_clear_err(s);
    if (ST(m)->streaming_active && !s->internal_result) {
        stmt_err(s, 2014, "HY000", "Commands out of sync; you can't run this command now");
        return 1;
    }
    if (s->internal_result) {
        stmt_result *r = (stmt_result*)s->internal_result;
        if (r->cursor_open) {
            r->cursor_open = 0;
            ST(m)->streaming_active = 0;
            m->status = MYSQL_STATUS_READY;
        } else if (!r->stored && !r->wire_eof && m->fd >= 0) {
            for (;;) {
                uint8_t *dp = NULL; size_t dl = 0;
                if (read_compressed_packet(m, &dp, &dl) < 0) break;
                if (dl > 0 && dp[0] == 0xFF) { free(dp); break; }
                if (ST(m)->deprecate_eof) {
                    if (dl > 0 && dp[0] == 0xFE) { free(dp); break; }
                } else {
                    if (dl > 0 && dp[0] == 0xFE && dl < 9) { free(dp); break; }
                }
                free(dp);
            }
            ST(m)->streaming_active = 0;
            m->status = MYSQL_STATUS_READY;
        }
    }
    if (s->field_count > 0 && m->status != MYSQL_STATUS_READY && m->fd >= 0) {
        for (;;) {
            uint8_t *dp = NULL; size_t dl = 0;
            if (read_compressed_packet(m, &dp, &dl) < 0) break;
            if (dl > 0 && dp[0] == 0xFF) { free(dp); break; }
            if (ST(m)->deprecate_eof) {
                if (dl > 0 && dp[0] == 0xFE) { free(dp); break; }
            } else {
                if (dl > 0 && dp[0] == 0xFE && dl < 9) { free(dp); break; }
            }
            free(dp);
        }
        m->status = MYSQL_STATUS_READY;
    }
    stmt_result_free(s);

    bbuf b = {0};
    bb_u8(&b, COM_STMT_EXECUTE);
    bb_u32(&b, (uint32_t)s->stmt_id);
    bb_u8(&b, 0x00);                     /* flags: CURSOR_TYPE_NO_CURSOR */
    if (s->field_count > 0 && (s->flags & CURSOR_TYPE_READ_ONLY)) {
        b.p[5] = CURSOR_TYPE_READ_ONLY;  /* overwrite flags byte at offset 5 */
    }
    bb_u32(&b, 1);                       /* iteration count */

    unsigned int np = (unsigned int)s->param_count;
    if (np > 0) {
        size_t nb = (np + 7) / 8;
        size_t nullpos = b.n; for (size_t i = 0; i < nb; i++) bb_u8(&b, 0);  /* NULL bitmap */
        bb_u8(&b, 0x01);                 /* new-params-bound */
        for (unsigned int i = 0; i < np; i++) {
            MYSQL_BIND *p = &s->bind_params[i];
            bb_u8(&b, (uint8_t)p->buffer_type);
            bb_u8(&b, p->is_unsigned ? 0x80 : 0x00);
        }
        for (unsigned int i = 0; i < np; i++) {
            MYSQL_BIND *p = &s->bind_params[i];
            int is_null = (p->is_null && *p->is_null) || p->buffer_type == MYSQL_TYPE_NULL || !p->buffer;
            if (is_null) { b.p[nullpos + i/8] |= (uint8_t)(1 << (i%8)); continue; }
            if (p->long_data_used) continue;     /* value already streamed */
            unsigned long L = p->length ? *p->length : p->buffer_length;
            switch (p->buffer_type) {
                case MYSQL_TYPE_TINY:     bb_u8(&b, *(uint8_t*)p->buffer); break;
                case MYSQL_TYPE_SHORT: case MYSQL_TYPE_YEAR:
                                          bb_u16(&b, *(uint16_t*)p->buffer); break;
                case MYSQL_TYPE_LONG: case MYSQL_TYPE_INT24:
                                          bb_u32(&b, *(uint32_t*)p->buffer); break;
                case MYSQL_TYPE_LONGLONG: bb_u64(&b, *(uint64_t*)p->buffer); break;
                case MYSQL_TYPE_FLOAT:  { float f = *(float*)p->buffer; bb_put(&b, &f, 4); break; }
                case MYSQL_TYPE_DOUBLE: { double d = *(double*)p->buffer; bb_put(&b, &d, 8); break; }
                case MYSQL_TYPE_DATE: {
                    MYSQL_TIME *t = (MYSQL_TIME*)p->buffer;
                    bb_u8(&b, 4); bb_u16(&b, (uint16_t)t->year); bb_u8(&b,(uint8_t)t->month); bb_u8(&b,(uint8_t)t->day);
                    break; }
                case MYSQL_TYPE_DATETIME: case MYSQL_TYPE_TIMESTAMP: {
                    MYSQL_TIME *t = (MYSQL_TIME*)p->buffer;
                    if (t->second_part) {
                        bb_u8(&b, 11); bb_u16(&b, (uint16_t)t->year); bb_u8(&b,(uint8_t)t->month); bb_u8(&b,(uint8_t)t->day);
                        bb_u8(&b,(uint8_t)t->hour); bb_u8(&b,(uint8_t)t->minute); bb_u8(&b,(uint8_t)t->second);
                        bb_u32(&b, (uint32_t)t->second_part);
                    } else if (t->hour || t->minute || t->second) {
                        bb_u8(&b, 7); bb_u16(&b, (uint16_t)t->year); bb_u8(&b,(uint8_t)t->month); bb_u8(&b,(uint8_t)t->day);
                        bb_u8(&b,(uint8_t)t->hour); bb_u8(&b,(uint8_t)t->minute); bb_u8(&b,(uint8_t)t->second);
                    } else {
                        bb_u8(&b, 4); bb_u16(&b, (uint16_t)t->year); bb_u8(&b,(uint8_t)t->month); bb_u8(&b,(uint8_t)t->day);
                    }
                    break; }
                case MYSQL_TYPE_TIME: {
                    MYSQL_TIME *t = (MYSQL_TIME*)p->buffer;
                    if (!t->neg && !t->hour && !t->minute && !t->second && !t->second_part) {
                        bb_u8(&b, 0);
                    } else if (t->second_part) {
                        bb_u8(&b, 12); bb_u8(&b, t->neg?1:0); bb_u32(&b, t->hour/24);
                        bb_u8(&b,(uint8_t)(t->hour%24)); bb_u8(&b,(uint8_t)t->minute); bb_u8(&b,(uint8_t)t->second);
                        bb_u32(&b, (uint32_t)t->second_part);
                    } else {
                        bb_u8(&b, 8); bb_u8(&b, t->neg?1:0); bb_u32(&b, t->hour/24);
                        bb_u8(&b,(uint8_t)(t->hour%24)); bb_u8(&b,(uint8_t)t->minute); bb_u8(&b,(uint8_t)t->second);
                    }
                    break; }
                default: {  /* string-like */
                    if (!p->length && p->buffer_type >= MYSQL_TYPE_VAR_STRING) L = strlen((char*)p->buffer);
                    bb_lenenc(&b, L); bb_put(&b, p->buffer, L); break; }
            }
        }
    }
    if (b.oom) { free(b.p); stmt_err(s, 2008, "HY000", "Out of memory"); return 1; }

    m->seq = 0;
    int rc = write_compressed_packet(m, b.p, b.n, 0);
    free(b.p);
    if (rc < 0) { stmt_err_from_conn(s); return 1; }

    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) { stmt_err_from_conn(s); return 1; }
    if (len && p[0] == 0xFF) { parse_err(m, p, len); stmt_err_from_conn(s); free(p); return 1; }
    if (len >= 7 && p[0] == 0x00) {              /* OK: no result set */
        parse_ok(m, p, len);
        s->affected_rows = m->affected_rows; s->insert_id = m->insert_id;
        s->field_count = 0;
        m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
        m->status = MYSQL_STATUS_READY;
        free(p);
        return 0;
    }
    const uint8_t *c = p, *e = p + len;
    uint64_t cols = lenenc_int(&c, e, NULL);
    free(p);
    free_field_array(s->fields, s->field_count);
    s->fields = NULL; s->field_count = (unsigned int)cols;
    if (stmt_read_defs(m, &s->fields, s->field_count) < 0) { stmt_err_from_conn(s); return 1; }

    if (ST(m)->deprecate_eof) {
        if (s->flags & CURSOR_TYPE_READ_ONLY) {
            uint8_t *ep = NULL; size_t el = 0;
            if (read_compressed_packet(m, &ep, &el) < 0) { stmt_err_from_conn(s); return 1; }
            if (el > 0 && ep[0] == 0xFE) {
                parse_ok(m, ep, el);
                s->server_status = m->server_status;
            } else if (el > 0 && ep[0] == 0x00) {
                parse_ok(m, ep, el);
                s->server_status = m->server_status;
            }
            free(ep);
        }
    } else {
        if (s->flags & CURSOR_TYPE_READ_ONLY) {
            uint8_t *ep = NULL; size_t el = 0;
            if (read_compressed_packet(m, &ep, &el) < 0) { stmt_err_from_conn(s); return 1; }
            if (el > 0 && ep[0] == 0xFE && el < 9) {
                if (el >= 5) {
                    m->server_status = (unsigned int)(ep[3]) | ((unsigned int)(ep[4]) << 8);
                    s->server_status = m->server_status;
                }
            }
            free(ep);
        }
    }

    stmt_result *r = (stmt_result*)calloc(1, sizeof(stmt_result));
    if (!r) { stmt_err(s, 2008, "HY000", "Out of memory"); return 1; }
    s->internal_result = r;
    if (s->server_status & SERVER_STATUS_CURSOR_EXISTS) {
        r->cursor_open = 1;
    } else {
        ST(m)->streaming_active = 1;
        m->status = MYSQL_STATUS_STATEMENT_GET_RESULT;
    }
    s->state = MYSQL_STMT_EXECUTE_DONE;
    return 0;
}

int mysql_stmt_store_result(MYSQL_STMT *s) {
    if (!s || !s->mysql) return 1;
    stmt_result *r = (stmt_result*)s->internal_result;
    if (!r) { stmt_clear_err(s); return 0; }
    if (r->stored) return 0;
    size_t cap = r->count > 0 ? r->count * 2 : 64;
    if (!r->rows) r->rows = (uint8_t**)malloc(cap * sizeof(uint8_t*));
    if (!r->lens) r->lens = (size_t*)  malloc(cap * sizeof(size_t));
    if (!r->rows || !r->lens) { stmt_err(s,2008,"HY000","Out of memory"); return 1; }
    if (r->cursor_open) {
        if (stmt_send_fetch(s, 0xFFFFFFFF) < 0) { stmt_err_from_conn(s); return 1; }
        for (;;) {
            uint8_t *p = NULL; size_t l = 0;
            int k = stmt_read_row(s, &p, &l);
            if (k < 0) return 1;
            if (k == 0) {
                break;
            }
            if (r->count >= cap) {
                cap *= 2;
                uint8_t **nr = (uint8_t**)realloc(r->rows, cap * sizeof(uint8_t*));
                if (!nr) { free(p); stmt_err(s,2008,"HY000","Out of memory"); return 1; }
                r->rows = nr;
                size_t   *nl = (size_t*)  realloc(r->lens, cap * sizeof(size_t));
                if (!nl) { free(p); stmt_err(s,2008,"HY000","Out of memory"); return 1; }
                r->lens = nl;
            }
            r->rows[r->count] = p; r->lens[r->count] = l; r->count++;
        }
    } else {
        int row_num = 0;
        for (;;) {
            uint8_t *p = NULL; size_t l = 0;
            int k = stmt_read_row(s, &p, &l);
            row_num++;
            if (k < 0) return 1;
            if (k == 0) break;
            if (r->count >= cap) {
                cap *= 2;
                uint8_t **nr = (uint8_t**)realloc(r->rows, cap * sizeof(uint8_t*));
                size_t   *nl = (size_t*)  realloc(r->lens, cap * sizeof(size_t));
                if (!nr || !nl) {
                    if (nr) r->rows = nr;
                    if (nl) r->lens = nl;
                    free(p); stmt_err(s,2008,"HY000","Out of memory"); return 1;
                }
                r->rows = nr; r->lens = nl;
            }
            r->rows[r->count] = p; r->lens[r->count] = l; r->count++;
        }
    }
    r->stored = 1; r->wire_eof = 1; r->cursor = 0; r->cursor_open = 0;
    for (size_t i = 0; i < r->fetch_cache_count; i++) free(r->fetch_cache[i]);
    free(r->fetch_cache); free(r->fetch_cache_lens);
    r->fetch_cache = NULL; r->fetch_cache_lens = NULL;
    r->fetch_cache_count = 0; r->fetch_cache_pos = 0;
    s->row_count = r->count;
    ST(s->mysql)->streaming_active = 0;
    s->mysql->status = MYSQL_STATUS_READY;
    if (s->update_max_length && s->fields && r->count > 0) {
        unsigned int nf = s->field_count;
        for (unsigned int i = 0; i < nf; i++) s->fields[i].max_length = 0;
        for (my_ulonglong row = 0; row < r->count; row++) {
            const uint8_t *p = r->rows[row], *end = p + r->lens[row];
            if (p >= end || *p != 0x00) continue;
            p++;
            size_t nbytes = (nf + 7 + 2) / 8;
            const uint8_t *nullmap = p; p += nbytes;
            if (p > end) continue;
            for (unsigned int i = 0; i < nf; i++) {
                int is_null = (nullmap[(i + 2) / 8] >> ((i + 2) % 8)) & 1;
                if (is_null) continue;
                cval v;
                if (decode_value(&p, end, s->fields[i].type,
                                 (s->fields[i].flags & UNSIGNED_FLAG) != 0, &v) < 0) break;
                unsigned long vlen = cval_display_len(&v, s->fields[i].type);
                if (vlen > s->fields[i].max_length) s->fields[i].max_length = vlen;
            }
        }
    }
    return 0;
}

int mysql_stmt_fetch(MYSQL_STMT *s) {
    if (!s || !s->mysql) return 1;
    stmt_result *r = (stmt_result*)s->internal_result;
    if (!r) return MYSQL_NO_DATA;
    stmt_clear_err(s);
    if (r->stored) {
        if (r->cursor >= r->count) { s->state = MYSQL_STMT_FETCH_DONE; return MYSQL_NO_DATA; }
        const uint8_t *p = r->rows[r->cursor]; size_t l = r->lens[r->cursor];
        r->cursor++;
        r->cur_row = (uint8_t*)p; r->cur_row_len = l;
        return stmt_bind_row(s, p, l);
    }
    if (r->wire_eof) { s->state = MYSQL_STMT_FETCH_DONE; return MYSQL_NO_DATA; }
    if (r->cursor_open) {
        if (r->fetch_cache && r->fetch_cache_pos < r->fetch_cache_count) {
            const uint8_t *p = r->fetch_cache[r->fetch_cache_pos];
            size_t l = r->fetch_cache_lens[r->fetch_cache_pos];
            r->fetch_cache_pos++;
            free(r->cur_row);
            r->cur_row = (uint8_t*)malloc(l);
            if (r->cur_row) { memcpy(r->cur_row, p, l); r->cur_row_len = l; }
            int rc = stmt_bind_row(s, p, l);
            return rc;
        }
        for (size_t i = 0; i < r->fetch_cache_count; i++) free(r->fetch_cache[i]);
        r->fetch_cache_count = 0; r->fetch_cache_pos = 0;
        unsigned long prefetch = s->prefetch_rows > 0 ? s->prefetch_rows : 1;
        if (stmt_send_fetch(s, prefetch) < 0) {
            stmt_err_from_conn(s); return 1;
        }
        size_t cap = 8;
        r->fetch_cache = (uint8_t**)malloc(cap * sizeof(uint8_t*));
        r->fetch_cache_lens = (size_t*)malloc(cap * sizeof(size_t));
        if (!r->fetch_cache || !r->fetch_cache_lens) {
            free(r->fetch_cache); free(r->fetch_cache_lens);
            r->fetch_cache = NULL; r->fetch_cache_lens = NULL;
            stmt_err(s, 2008, "HY000", "Out of memory"); return 1;
        }
        for (;;) {
            uint8_t *p = NULL; size_t l = 0;
            int k = stmt_read_row(s, &p, &l);
            if (k < 0) {
                for (size_t i = 0; i < r->fetch_cache_count; i++) free(r->fetch_cache[i]);
                free(r->fetch_cache); free(r->fetch_cache_lens);
                r->fetch_cache = NULL; r->fetch_cache_lens = NULL;
                r->fetch_cache_count = 0;
                return 1;
            }
            if (k == 0) {
                if (r->fetch_cache_count == 0) {
                    free(r->fetch_cache); free(r->fetch_cache_lens);
                    r->fetch_cache = NULL; r->fetch_cache_lens = NULL;
                    r->wire_eof = 1; r->cursor_open = 0;
                    ST(s->mysql)->streaming_active = 0;
                    s->state = MYSQL_STMT_FETCH_DONE;
                    return MYSQL_NO_DATA;
                }
                break;
            }
            if (r->fetch_cache_count >= cap) {
                cap *= 2;
                uint8_t **np = (uint8_t**)realloc(r->fetch_cache, cap * sizeof(uint8_t*));
                size_t *nl = (size_t*)realloc(r->fetch_cache_lens, cap * sizeof(size_t));
                if (!np || !nl) {
                    for (size_t i = 0; i < r->fetch_cache_count; i++) free(r->fetch_cache[i]);
                    free(np ? np : r->fetch_cache); free(nl ? nl : r->fetch_cache_lens);
                    r->fetch_cache = NULL; r->fetch_cache_lens = NULL;
                    r->fetch_cache_count = 0;
                    stmt_err(s, 2008, "HY000", "Out of memory"); return 1;
                }
                r->fetch_cache = np; r->fetch_cache_lens = nl;
            }
            r->fetch_cache[r->fetch_cache_count] = p;
            r->fetch_cache_lens[r->fetch_cache_count] = l;
            r->fetch_cache_count++;
        }
        r->fetch_cache_pos = 0;
        const uint8_t *fp = r->fetch_cache[0];
        size_t fl = r->fetch_cache_lens[0];
        r->fetch_cache_pos = 1;
        free(r->cur_row);
        r->cur_row = (uint8_t*)malloc(fl);
        if (r->cur_row) { memcpy(r->cur_row, fp, fl); r->cur_row_len = fl; }
        return stmt_bind_row(s, fp, fl);
    }
    uint8_t *p = NULL; size_t l = 0;
    int k = stmt_read_row(s, &p, &l);
    if (k < 0) return 1;
    if (k == 0) {
        r->wire_eof = 1; r->cursor_open = 0;
        ST(s->mysql)->streaming_active = 0;
        s->state = MYSQL_STMT_FETCH_DONE; return MYSQL_NO_DATA;
    }
    r->cur_row = (uint8_t*)malloc(l);
    if (r->cur_row) { memcpy(r->cur_row, p, l); r->cur_row_len = l; }
    int rc = stmt_bind_row(s, p, l);
    free(p);
    return rc;
}

int mysql_stmt_fetch_column(MYSQL_STMT *s, MYSQL_BIND *b, unsigned int col, unsigned long off) {
    if (!s || !s->mysql) return 1;
    stmt_result *r = (stmt_result*)s->internal_result;
    if (!r || !r->cur_row || col >= s->field_count) {
        stmt_err(s, 2013, "HY000", "fetch_column requires a current row");
        return 1;
    }
    b->offset = off;
    const uint8_t *p = r->cur_row; size_t l = r->cur_row_len;
    const uint8_t *cur = p + 1, *end = p + l;
    size_t nbytes = (s->field_count + 7 + 2) / 8;
    const uint8_t *nullmap = cur; cur += nbytes;
    if (cur > end) { stmt_err(s, 2013, "HY000", "Truncated row data in fetch_column"); return 1; }
    for (unsigned int i = 0; i < s->field_count; i++) {
        int is_null = (nullmap[(i + 2) / 8] >> ((i + 2) % 8)) & 1;
        cval v;
        if (is_null) v.kind = V_NULL;
        else if (decode_value(&cur, end, s->fields[i].type,
                             (s->fields[i].flags & UNSIGNED_FLAG) != 0, &v) < 0) return 1;
        if (i == col) { store_into_bind(&v, b); return 0; }
    }
    return 1;
}

my_bool mysql_stmt_send_long_data(MYSQL_STMT *s, unsigned int param, const char *data, unsigned long length) {
    if (!s || !s->mysql || s->mysql->fd < 0) return 1;
    if (param >= s->param_count) { stmt_err(s, 2034, "HY000", "Parameter number out of range"); return 1; }
    if (!data && length > 0) { stmt_err(s, 2031, "HY000", "Invalid NULL data pointer with non-zero length"); return 1; }
    MYSQL *m = s->mysql;
    m->seq = 0;
    bbuf b = {0};
    bb_u8(&b, COM_STMT_SEND_LONG_DATA);
    bb_u32(&b, (uint32_t)s->stmt_id);
    bb_u16(&b, (uint16_t)param);
    bb_put(&b, data, length);
    if (b.oom) { free(b.p); stmt_err(s, 2008, "HY000", "Out of memory"); return 1; }
    int rc = write_compressed_packet(m, b.p, b.n, 0);       /* no reply */
    free(b.p);
    if (s->bind_params) s->bind_params[param].long_data_used = 1;
    return rc < 0 ? 1 : 0;
}

int mysql_stmt_reset(MYSQL_STMT *s) {
    if (!s || !s->mysql || s->mysql->fd < 0) { if (s) stmt_err(s, 2006, "HY000", "Server has gone away"); return 1; }
    MYSQL *m = s->mysql;
    if (s->internal_result) {
        stmt_result *r = (stmt_result*)s->internal_result;
        if (!r->stored && !r->wire_eof && m->fd >= 0) {
            for (;;) {
                uint8_t *dp = NULL; size_t dl = 0;
                if (read_compressed_packet(m, &dp, &dl) < 0) break;
                if (dl > 0 && dp[0] == 0xFF) { free(dp); break; }
                if (ST(m)->deprecate_eof) {
                    if (dl > 0 && dp[0] == 0xFE) { free(dp); break; }
                } else {
                    if (dl > 0 && dp[0] == 0xFE && dl < 9) { free(dp); break; }
                }
                free(dp);
            }
            ST(m)->streaming_active = 0;
        }
    }
    stmt_result_free(s);
    m->seq = 0;
    uint8_t buf[5] = { COM_STMT_RESET,
        (uint8_t)s->stmt_id, (uint8_t)(s->stmt_id>>8), (uint8_t)(s->stmt_id>>16), (uint8_t)(s->stmt_id>>24) };
    if (write_compressed_packet(m, buf, 5, 0) < 0) { stmt_err_from_conn(s); return 1; }
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) { stmt_err_from_conn(s); return 1; }
    int err = (len && p[0] == 0xFF);
    if (err) { parse_err(m, p, len); stmt_err_from_conn(s); }
    else { s->last_errno = 0; s->last_error[0] = '\0'; s->sqlstate[0] = '\0'; }
    free(p);
    if (s->bind_params) for (unsigned long i = 0; i < s->param_count; i++) s->bind_params[i].long_data_used = 0;
    s->state = MYSQL_STMT_PREPARE_DONE;
    return err ? 1 : 0;
}

void mysql_stmt_data_seek(MYSQL_STMT *s, my_ulonglong off) {
    if (!s) return;
    stmt_result *r = (stmt_result*)s->internal_result;
    if (r && r->stored) r->cursor = (off <= r->count) ? off : r->count;
}

MYSQL_RES *mysql_stmt_result_metadata(MYSQL_STMT *s) {
    if (!s || s->field_count == 0) return NULL;
    MYSQL_RES *res = (MYSQL_RES*)calloc(1, sizeof(MYSQL_RES));
    if (!res) { stmt_err(s, 2008, "HY000", "Out of memory"); return NULL; }
    res->field_count = s->field_count;
    res->fields = (MYSQL_FIELD*)calloc(s->field_count, sizeof(MYSQL_FIELD));
    if (!res->fields) { free(res); stmt_err(s, 2008, "HY000", "Out of memory"); return NULL; }
    for (unsigned int i = 0; i < s->field_count; i++) {
        MYSQL_FIELD *d = &res->fields[i], *src = &s->fields[i];
        *d = *src;     /* copy scalars, then deep-copy the strings we own */
        d->name=src->name?strdup(src->name):NULL; d->org_name=src->org_name?strdup(src->org_name):NULL;
        d->table=src->table?strdup(src->table):NULL; d->org_table=src->org_table?strdup(src->org_table):NULL;
        d->db=src->db?strdup(src->db):NULL; d->catalog=src->catalog?strdup(src->catalog):NULL;
        d->def=src->def?strdup(src->def):NULL;
    }
    res->handle = s->mysql;
    return res;
}

int mysql_stmt_free_result(MYSQL_STMT *s) {
    if (!s) return 0;
    MYSQL *m = s->mysql;
    if (m && m->fd >= 0 && s->internal_result) {
        stmt_result *r = (stmt_result*)s->internal_result;
        if (!r->stored && !r->wire_eof) {
            for (;;) {
                uint8_t *p = NULL; size_t l = 0;
                if (read_compressed_packet(m, &p, &l) < 0) break;
                if (l > 0 && p[0] == 0xFF) { free(p); break; }
                if (ST(m)->deprecate_eof) {
                    if (l > 0 && p[0] == 0xFE) { free(p); break; }
                } else {
                    if (l > 0 && p[0] == 0xFE && l < 9) { free(p); break; }
                }
                free(p);
            }
        }
    }
    if (m) ST(m)->streaming_active = 0;
    stmt_result_free(s);
    return 0;
}

int mysql_stmt_close(MYSQL_STMT *s) {
    if (!s) return 0;
    MYSQL *m = s->mysql;
    if (m && m->fd >= 0 && s->stmt_id) {
        if (s->internal_result) {
            stmt_result *r = (stmt_result*)s->internal_result;
            if (!r->stored && !r->wire_eof) {
                for (;;) {
                    uint8_t *p = NULL; size_t l = 0;
                    if (read_compressed_packet(m, &p, &l) < 0) break;
                    if (l > 0 && p[0] == 0xFF) { free(p); break; }
                    if (ST(m)->deprecate_eof) {
                        if (l > 0 && p[0] == 0xFE) { free(p); break; }
                    } else {
                        if (l > 0 && p[0] == 0xFE && l < 9) { free(p); break; }
                    }
                    free(p);
                }
                ST(m)->streaming_active = 0;
                m->status = MYSQL_STATUS_READY;
            }
        }
        mm_cache_remove((mm_stmt_cache*)m->stmt_cache, s->stmt_id);
        m->seq = 0;
        uint8_t buf[5];
        buf[0] = COM_STMT_CLOSE;
        buf[1] = (uint8_t)(s->stmt_id);       buf[2] = (uint8_t)(s->stmt_id>>8);
        buf[3] = (uint8_t)(s->stmt_id>>16);   buf[4] = (uint8_t)(s->stmt_id>>24);
        if (write_compressed_packet(m, buf, 5, 0) < 0) {
            set_err(m, CR_SERVER_LOST, "HY000", "Lost connection to server during close");
        }
    }
    stmt_result_free(s);
    stmt_free_metadata(s);
    free(s);
    return 0;
}

unsigned long mysql_stmt_param_count(MYSQL_STMT *s)  { return s ? s->param_count : 0; }
unsigned int  mysql_stmt_field_count(MYSQL_STMT *s)  { return s ? s->field_count : 0; }
my_ulonglong  mysql_stmt_affected_rows(MYSQL_STMT *s){ return s ? s->affected_rows : 0; }
my_ulonglong  mysql_stmt_insert_id(MYSQL_STMT *s)    { return s ? s->insert_id : 0; }
my_ulonglong  mysql_stmt_num_rows(MYSQL_STMT *s)     { return s ? s->row_count : 0; }
unsigned int  mysql_stmt_warning_count(MYSQL_STMT *s) { return s ? s->warning_count : 0; }

const char  *mysql_stmt_error(MYSQL_STMT *s)    { return s ? s->last_error : ""; }
unsigned int mysql_stmt_errno(MYSQL_STMT *s)    { return s ? s->last_errno : 0; }
const char  *mysql_stmt_sqlstate(MYSQL_STMT *s) { return s ? s->sqlstate : "00000"; }

/* ---- new API functions ---- */

MYSQL_ROW_OFFSET mysql_row_seek(MYSQL_RES *res, MYSQL_ROW_OFFSET offset) {
    if (!res || res->streaming) return NULL;
    MYSQL_ROW_OFFSET prev = (MYSQL_ROW_OFFSET)(uintptr_t)res->store_cursor;
    my_ulonglong off = (my_ulonglong)(uintptr_t)offset;
    res->store_cursor = (off < res->row_count) ? off : res->row_count;
    if (res->store_cursor >= res->row_count) {
        free(res->cur_lengths); res->cur_lengths = NULL;
    }
    return prev;
}

MYSQL_ROW_OFFSET mysql_row_tell(MYSQL_RES *res) {
    if (!res) return NULL;
    return (MYSQL_ROW_OFFSET)(uintptr_t)res->store_cursor;
}

MYSQL_FIELD_OFFSET mysql_field_tell(MYSQL_RES *res) {
    return res ? res->current_field : 0;
}

my_bool mysql_eof(MYSQL_RES *res) {
    if (!res) return 1;
    return res->eof ? 1 : 0;
}

enum enum_resultset_metadata mysql_result_metadata(MYSQL_RES *res) {
    if (!res) return RESULTSET_METADATA_NONE;
    return res->metadata;
}

const char *mysql_info(MYSQL *m) {
    return m ? m->info : NULL;
}

unsigned long mysql_get_client_version(void) {
    return 80045UL;
}

int mysql_reset_connection(MYSQL *m) {
    if (!m || m->fd < 0) return 1;
    mm_state *st = ST(m);
    if (st->streaming_active) {
        set_err(m, 2014, "HY000", "Commands out of sync; you can't run this command now");
        return 1;
    }
    clear_err(m);
    m->seq = 0;
    uint8_t cmd = 0x1F;
    if (write_compressed_packet(m, &cmd, 1, 0) < 0) return 1;
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) return 1;
    int rc = 0;
    if (len > 0 && p[0] == 0xFF) { parse_err(m, p, len); rc = 1; }
    else if (len > 0 && p[0] == 0x00) { parse_ok(m, p, len); }
    free(p);
    m->affected_rows = 0; m->insert_id = 0; m->warning_count = 0;
    m->field_count = 0; m->multi_status = 0;
    st->streaming_active = 0;
    mm_stmt_cache *cache = (mm_stmt_cache*)m->stmt_cache;
    if (cache) {
        for (unsigned int i = 0; i < cache->count; i++) free(cache->entries[i].sql);
        cache->count = 0;
    }
    for (int i = 0; i < 6; i++) {
        free(st->session_track[i]); st->session_track[i] = NULL;
        st->session_track_len[i] = 0; st->session_track_pos[i] = 0;
    }
    return rc;
}

unsigned long mysql_hex_string(char *to, const char *from, unsigned long length) {
    static const char hex[] = "0123456789ABCDEF";
    unsigned long out = 0;
    for (unsigned long i = 0; i < length; i++) {
        to[out++] = hex[((unsigned char)from[i]) >> 4];
        to[out++] = hex[((unsigned char)from[i]) & 0x0F];
    }
    to[out] = '\0';
    return out;
}

void mysql_get_character_set_info(MYSQL *m, MY_CHARSET_INFO *cs) {
    if (!m || !cs) return;
    memset(cs, 0, sizeof(*cs));
    unsigned int nr = ST(m) ? ST(m)->charsetnr : 45;
    cs->number = nr;
    cs->state  = 1;
    switch (nr) {
        case 45:
            cs->csname  = "utf8mb3"; cs->name = "utf8mb3_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 3; break;
        case 46:
            cs->csname  = "utf8mb3"; cs->name = "utf8mb3_bin";
            cs->mbminlen = 1; cs->mbmaxlen = 3; break;
        case 63:
            cs->csname  = "binary";  cs->name = "binary";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 255:
            cs->csname  = "utf8mb4"; cs->name = "utf8mb4_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 4; break;
        case 256:
            cs->csname  = "utf8mb4"; cs->name = "utf8mb4_bin";
            cs->mbminlen = 1; cs->mbmaxlen = 4; break;
        case 8: case 14: case 31: case 47:
            cs->csname  = "latin1";  cs->name = "latin1_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 33:
            cs->csname  = "utf8mb3"; cs->name = "utf8mb3_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 3; break;
        case 83:
            cs->csname  = "utf8mb3"; cs->name = "utf8mb3_unicode_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 3; break;
        case 192: case 193:
            cs->csname  = "gb2312";  cs->name = "gb2312_chinese_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 2; break;
        case 28:
            cs->csname  = "gbk";  cs->name = "gbk_chinese_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 2; break;
        case 1:
            cs->csname  = "big5";  cs->name = "big5_chinese_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 2; break;
        case 13:
            cs->csname  = "sjis";  cs->name = "sjis_japanese_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 2; break;
        case 19:
            cs->csname  = "euckr";  cs->name = "euckr_korean_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 2; break;
        case 12:
            cs->csname  = "ujis";  cs->name = "ujis_japanese_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 3; break;
        case 97:
            cs->csname  = "eucjpms";  cs->name = "eucjpms_japanese_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 3; break;
        case 248:
            cs->csname  = "gb18030";  cs->name = "gb18030_chinese_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 4; break;
        case 246:
            cs->csname  = "utf8mb4"; cs->name = "utf8mb4_0900_ai_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 4; break;
        case 247:
            cs->csname  = "utf8mb4"; cs->name = "utf8mb4_0900_as_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 4; break;
        case 249:
            cs->csname  = "utf8mb4"; cs->name = "utf8mb4_0900_as_cs";
            cs->mbminlen = 1; cs->mbmaxlen = 4; break;
        case 257: case 258:
            cs->csname  = "utf8mb4"; cs->name = "utf8mb4_unicode_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 4; break;
        case 5:
            cs->csname  = "koi8r"; cs->name = "koi8r_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 7:
            cs->csname  = "latin2"; cs->name = "latin2_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 2:
            cs->csname  = "latin2"; cs->name = "latin2_czech_cs";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 36:
            cs->csname  = "macroman"; cs->name = "macroman_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 39:
            cs->csname  = "cp1251"; cs->name = "cp1251_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 64:
            cs->csname  = "binary"; cs->name = "binary";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 11:
            cs->csname  = "ascii"; cs->name = "ascii_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 65:
            cs->csname  = "ascii"; cs->name = "ascii_bin";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 32:
            cs->csname  = "latin1"; cs->name = "latin1_swedish_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 48:
            cs->csname  = "latin1"; cs->name = "latin1_general_cs";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 15:
            cs->csname  = "hebrew"; cs->name = "hebrew_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 16:
            cs->csname  = "tis620"; cs->name = "tis620_thai_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 34:
            cs->csname  = "greek"; cs->name = "greek_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 35:
            cs->csname  = "cp1250"; cs->name = "cp1250_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 26:
            cs->csname  = "cp1257"; cs->name = "cp1257_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        case 57:
            cs->csname  = "cp1256"; cs->name = "cp1256_general_ci";
            cs->mbminlen = 1; cs->mbmaxlen = 1; break;
        default:
            cs->csname  = m->charset_name ? m->charset_name : "utf8mb4";
            cs->name    = "unknown";
            cs->mbminlen = 1; cs->mbmaxlen = 4; break;
    }
}

MYSQL_RES *mysql_list_dbs(MYSQL *m, const char *wild) {
    char q[512];
    int n;
    if (wild && *wild) {
        char esc[256];
        unsigned long el = mysql_real_escape_string_quote(m, esc, wild, (unsigned long)strlen(wild), '\'');
        n = snprintf(q, sizeof(q), "SHOW DATABASES LIKE '%.*s'", (int)el, esc);
    } else
        n = snprintf(q, sizeof(q), "SHOW DATABASES");
    if (mysql_real_query(m, q, (unsigned long)n) != 0) return NULL;
    return mysql_store_result(m);
}

MYSQL_RES *mysql_list_tables(MYSQL *m, const char *wild) {
    char q[1024];
    int n;
    if (wild && *wild) {
        char esc[256];
        unsigned long el = mysql_real_escape_string_quote(m, esc, wild, (unsigned long)strlen(wild), '\'');
        n = snprintf(q, sizeof(q), "SHOW TABLES LIKE '%.*s'", (int)el, esc);
    } else
        n = snprintf(q, sizeof(q), "SHOW TABLES");
    if (mysql_real_query(m, q, (unsigned long)n) != 0) return NULL;
    return mysql_store_result(m);
}

MYSQL_RES *mysql_list_fields(MYSQL *m, const char *table, const char *wild) {
    if (!m || m->fd < 0 || !table) return NULL;
    clear_err(m); m->seq = 0;
    size_t tl = strlen(table);
    size_t wl = wild ? strlen(wild) : 0;
    size_t buf_sz = 1 + tl + 1 + wl;
    uint8_t *buf = (uint8_t*)malloc(buf_sz);
    if (!buf) { set_err(m, 2008, "HY000", "Out of memory"); return NULL; }
    size_t o = 0;
    buf[o++] = 0x04;
    memcpy(buf + o, table, tl); o += tl; buf[o++] = 0;
    if (wild) { memcpy(buf + o, wild, wl); o += wl; }
    if (write_compressed_packet(m, buf, o, 0) < 0) { free(buf); return NULL; }
    free(buf);

    MYSQL_RES *res = (MYSQL_RES*)calloc(1, sizeof(MYSQL_RES));
    if (!res) return NULL;
    res->handle = m;
    unsigned int cap = 16;
    res->fields = (MYSQL_FIELD*)calloc(cap, sizeof(MYSQL_FIELD));
    if (!res->fields) { free(res); return NULL; }

    while (1) {
        uint8_t *p = NULL; size_t len = 0;
        if (read_compressed_packet(m, &p, &len) < 0) { mysql_free_result(res); return NULL; }
        if (len && p[0] == 0xFF) { parse_err(m, p, len); free(p); mysql_free_result(res); return NULL; }
        if (len > 0 && p[0] == 0xFE) { free(p); break; }
        if (len > 0 && p[0] == 0x00 && len >= 7 && res->field_count > 0) {
            const uint8_t *ck = p + 1; const uint8_t *ce = p + len;
            if (ck < ce) {
                uint64_t sl = lenenc_int(&ck, ce, NULL);
                if (sl < 0x1000000 && ck + sl <= ce) { free(p); break; }
            }
        }
        if (res->field_count >= cap) {
            cap *= 2;
            MYSQL_FIELD *nf = (MYSQL_FIELD*)realloc(res->fields, cap * sizeof(MYSQL_FIELD));
            if (!nf) { free(p); mysql_free_result(res); return NULL; }
            res->fields = nf;
        }
        parse_field(p, len, &res->fields[res->field_count++]);
        free(p);
    }
    res->eof = 1;
    return res;
}

MYSQL_RES *mysql_list_processes(MYSQL *m) {
    if (mysql_real_query(m, "SHOW PROCESSLIST", 16) != 0) return NULL;
    return mysql_store_result(m);
}

int mysql_set_server_option(MYSQL *m, enum enum_mysql_set_option option) {
    if (!m || m->fd < 0) return 1;
    clear_err(m);
    m->seq = 0;
    uint8_t buf[3];
    buf[0] = 0x1A;
    buf[1] = (uint8_t)option;
    buf[2] = 0;
    if (write_compressed_packet(m, buf, 3, 0) < 0) return 1;
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) return 1;
    int rc = 0;
    if (len > 0 && p[0] == 0xFF) { parse_err(m, p, len); rc = 1; }
    free(p);
    return rc;
}

int mysql_kill(MYSQL *m, unsigned long pid) {
    if (!m || m->fd < 0) return 1;
    char q[64];
    snprintf(q, sizeof(q), "KILL %lu", pid);
    return mysql_real_query(m, q, (unsigned long)strlen(q));
}

int mysql_shutdown(MYSQL *m, int shutdown_level) {
    if (!m || m->fd < 0) return 1;
    (void)shutdown_level;
    return mysql_real_query(m, "SHUTDOWN", (unsigned long)strlen("SHUTDOWN"));
}

int mysql_refresh(MYSQL *m, unsigned int refresh_options) {
    if (!m || m->fd < 0) return 1;
    if (ST(m)->streaming_active) {
        set_err(m, 2014, "HY000", "Commands out of sync; you can't run this command now");
        return 1;
    }
    clear_err(m);
    m->seq = 0;
    uint8_t buf[5];
    buf[0] = 0x07;
    buf[1] = (uint8_t)(refresh_options);       buf[2] = (uint8_t)(refresh_options>>8);
    buf[3] = (uint8_t)(refresh_options>>16);   buf[4] = (uint8_t)(refresh_options>>24);
    if (write_compressed_packet(m, buf, 5, 0) < 0) return 1;
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) return 1;
    int rc = 0;
    if (len > 0 && p[0] == 0xFF) { parse_err(m, p, len); rc = 1; }
    free(p);
    return rc;
}

int mysql_dump_debug_info(MYSQL *m) {
    if (!m || m->fd < 0) return 1;
    clear_err(m);
    m->seq = 0;
    uint8_t cmd = 0x0D;
    if (write_compressed_packet(m, &cmd, 1, 0) < 0) return 1;
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) return 1;
    int rc = 0;
    if (len > 0 && p[0] == 0xFF) { parse_err(m, p, len); rc = 1; }
    free(p);
    return rc;
}

const char *mysql_stat(MYSQL *m) {
    if (!m || m->fd < 0) return NULL;
    clear_err(m);
    m->seq = 0;
    uint8_t cmd = 0x09;
    if (write_compressed_packet(m, &cmd, 1, 0) < 0) return NULL;
    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) return NULL;
    if (len > 0 && p[0] == 0xFF) { parse_err(m, p, len); free(p); return NULL; }
    free(m->info);
    m->info = (char*)malloc(len + 1);
    if (!m->info) { free(p); return ""; }
    size_t cp = len < 511 ? len : 511;
    memcpy(m->info, p, cp); m->info[cp] = 0;
    free(p);
    return m->info;
}

int mysql_send_query(MYSQL *m, const char *q, unsigned long length) {
    return mysql_real_query(m, q, length);
}

my_bool mysql_read_query_result(MYSQL *m) {
    (void)m;
    return 0;
}

int mysql_options4(MYSQL *m, enum mysql_option option, const void *arg1, const void *arg2) {
    if (!m) return 1;
    mm_state *st = ST(m);
    switch (option) {
        case MYSQL_OPT_CONNECT_ATTR_ADD: {
            const char *key = (const char*)arg1;
            const char *val = (const char*)arg2;
            if (!key) break;
            unsigned int found = st->conn_attr_count;
            for (unsigned int i = 0; i < st->conn_attr_count; i++) {
                if (strcmp(st->conn_attr_keys[i], key) == 0) { found = i; break; }
            }
            if (found < st->conn_attr_count) {
                free(st->conn_attr_vals[found]);
                st->conn_attr_vals[found] = strdup(val ? val : "");
            } else {
                if (st->conn_attr_count >= st->conn_attr_cap) {
                    unsigned int nc = st->conn_attr_cap ? st->conn_attr_cap * 2 : 8;
                    char **nk = (char**)realloc(st->conn_attr_keys, nc * sizeof(char*));
                    if (!nk) return 1;
                    st->conn_attr_keys = nk;
                    char **nv = (char**)realloc(st->conn_attr_vals, nc * sizeof(char*));
                    if (!nv) return 1;
                    st->conn_attr_vals = nv;
                    st->conn_attr_cap = nc;
                }
                st->conn_attr_keys[st->conn_attr_count] = strdup(key);
                st->conn_attr_vals[st->conn_attr_count] = strdup(val ? val : "");
                st->conn_attr_count++;
            }
            break;
        }
        default: return mysql_options(m, option, arg1);
    }
    return 0;
}

int mysql_get_option(MYSQL *m, enum mysql_option option, const void *arg) {
    if (!m || !arg) return 1;
    mm_state *st = ST(m);
    switch (option) {
        case MYSQL_OPT_CONNECT_TIMEOUT: *(unsigned int*)arg = m->connect_timeout; break;
        case MYSQL_OPT_READ_TIMEOUT:    *(unsigned int*)arg = m->read_timeout; break;
        case MYSQL_OPT_WRITE_TIMEOUT:   *(unsigned int*)arg = m->write_timeout; break;
        case MYSQL_OPT_SSL_MODE:        *(unsigned int*)arg = (unsigned int)st->ssl_mode; break;
        case MYSQL_OPT_RECONNECT:       *(my_bool*)arg = st->reconnect; break;
        case MYSQL_OPT_PROTOCOL:        *(unsigned int*)arg = st->protocol; break;
        case MYSQL_OPT_LOCAL_INFILE:    *(unsigned int*)arg = st->local_infile; break;
        case MYSQL_REPORT_DATA_TRUNCATION: *(my_bool*)arg = st->report_data_truncation; break;
        case MYSQL_OPT_MAX_ALLOWED_PACKET: *(unsigned long*)arg = st->max_allowed_packet; break;
        case MYSQL_OPT_NET_BUFFER_LENGTH: *(unsigned long*)arg = st->net_buffer_length; break;
        case MYSQL_OPT_RETRY_COUNT:     *(unsigned int*)arg = st->retry_count; break;
        case MYSQL_OPT_SSL_FIPS_MODE:   *(int*)arg = st->ssl_fips_mode; break;
        case MYSQL_OPT_ZSTD_COMPRESSION_LEVEL: *(unsigned int*)arg = st->zstd_compression_level; break;
        case MYSQL_OPT_OPTIONAL_RESULTSET_METADATA: *(my_bool*)arg = st->optional_resultset_metadata; break;
        case MYSQL_ENABLE_CLEARTEXT_PLUGIN: *(my_bool*)arg = st->enable_cleartext_plugin; break;
        case MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS: *(my_bool*)arg = st->can_handle_expired_passwords; break;
        case MYSQL_SERVER_PUBLIC_KEY:     *(const char**)arg = st->server_public_key; break;
        case MYSQL_DEFAULT_AUTH:          *(const char**)arg = st->default_auth; break;
        case MYSQL_OPT_TLS_CIPHERSUITES:  *(const char**)arg = st->tls_ciphersuites; break;
        case MYSQL_INIT_COMMAND:          *(const char**)arg = st->init_command; break;
        case MYSQL_SET_CHARSET_NAME:      *(const char**)arg = m->charset_name; break;
        case MYSQL_OPT_SSL_KEY:           *(const char**)arg = st->ssl_key; break;
        case MYSQL_OPT_SSL_CERT:          *(const char**)arg = st->ssl_cert; break;
        case MYSQL_OPT_SSL_CA:            *(const char**)arg = st->ssl_ca; break;
        case MYSQL_OPT_SSL_CAPATH:        *(const char**)arg = st->ssl_capath; break;
        case MYSQL_OPT_SSL_CIPHER:        *(const char**)arg = st->ssl_cipher; break;
        case MYSQL_OPT_TLS_VERSION:       *(const char**)arg = st->tls_version; break;
        case MYSQL_OPT_COMPRESSION_ALGORITHMS: *(const char**)arg = st->compression_algorithms; break;
        case MYSQL_OPT_LOAD_DATA_LOCAL_DIR: *(const char**)arg = st->load_data_local_dir; break;
        case MYSQL_OPT_USE_RESULT:        *(my_bool*)arg = st->use_result; break;
        default: return 1;
    }
    return 0;
}

#define SESSION_TRACK_TYPE_MAX 6

int mysql_session_track_get_first(MYSQL *m, enum enum_session_state_type type,
                                   const char **data, size_t *length) {
    if (!m || !data || !length || (int)type < 0 || (int)type >= SESSION_TRACK_TYPE_MAX) return 1;
    if (!ST(m) || !ST(m)->session_track[type]) return 1;
    const uint8_t *p = ST(m)->session_track[type];
    const uint8_t *end = p + ST(m)->session_track_len[type];
    if (p >= end) return 1;
    size_t vlen = lenenc_int(&p, end, NULL);
    if (p + vlen > end) return 1;
    *data = (const char*)p;
    *length = vlen;
    ST(m)->session_track_pos[type] = (unsigned int)(p + vlen - ST(m)->session_track[type]);
    return 0;
}

int mysql_session_track_get_next(MYSQL *m, enum enum_session_state_type type,
                                  const char **data, size_t *length) {
    if (!m || !data || !length || (int)type < 0 || (int)type >= SESSION_TRACK_TYPE_MAX) return 1;
    if (!ST(m) || !ST(m)->session_track[type]) return 1;
    const uint8_t *p = ST(m)->session_track[type] + ST(m)->session_track_pos[type];
    const uint8_t *end = ST(m)->session_track[type] + ST(m)->session_track_len[type];
    if (p >= end) return 1;
    size_t vlen = lenenc_int(&p, end, NULL);
    if (p + vlen > end) return 1;
    *data = (const char*)p;
    *length = vlen;
    ST(m)->session_track_pos[type] = (unsigned int)(p + vlen - ST(m)->session_track[type]);
    return 0;
}

my_bool mysql_stmt_attr_set(MYSQL_STMT *s, enum enum_stmt_attr_type attr_type,
                             const void *attr) {
    if (!s) return 1;
    switch (attr_type) {
        case STMT_ATTR_UPDATE_MAX_LENGTH:
            s->update_max_length = attr ? *(const my_bool*)attr : 0;
            break;
        case STMT_ATTR_CURSOR_TYPE:
            s->flags = attr ? *(const unsigned long*)attr : 0;
            break;
        case STMT_ATTR_PREFETCH_ROWS:
            s->prefetch_rows = attr ? *(const unsigned long*)attr : 0;
            break;
        default: return 1;
    }
    return 0;
}

my_bool mysql_stmt_attr_get(MYSQL_STMT *s, enum enum_stmt_attr_type attr_type,
                             void *attr) {
    if (!s || !attr) return 1;
    switch (attr_type) {
        case STMT_ATTR_UPDATE_MAX_LENGTH:
            *(my_bool*)attr = s->update_max_length;
            break;
        case STMT_ATTR_CURSOR_TYPE:
            *(unsigned long*)attr = s->flags;
            break;
        case STMT_ATTR_PREFETCH_ROWS:
            *(unsigned long*)attr = s->prefetch_rows;
            break;
        default: return 1;
    }
    return 0;
}

MYSQL_ROW_OFFSET mysql_stmt_row_seek(MYSQL_STMT *s, MYSQL_ROW_OFFSET offset) {
    if (!s) return NULL;
    stmt_result *r = (stmt_result*)s->internal_result;
    if (!r || !r->stored) return NULL;
    MYSQL_ROW_OFFSET prev = (MYSQL_ROW_OFFSET)(uintptr_t)(r->cursor + 1);
    r->cursor = (my_ulonglong)(uintptr_t)offset - 1;
    return prev;
}

MYSQL_ROW_OFFSET mysql_stmt_row_tell(MYSQL_STMT *s) {
    if (!s) return NULL;
    stmt_result *r = (stmt_result*)s->internal_result;
    if (!r) return NULL;
    return (MYSQL_ROW_OFFSET)(uintptr_t)(r->cursor + 1);
}

MYSQL_RES *mysql_stmt_param_metadata(MYSQL_STMT *s) {
    if (!s || s->param_count == 0) return NULL;
    return NULL;
}

int mysql_stmt_next_result(MYSQL_STMT *s) {
    if (!s || !s->mysql) return -1;
    MYSQL *m = s->mysql;
    if (m->fd < 0) return -1;
    if (!m->multi_status) return -1;
    stmt_clear_err(s);

    if (m->status != MYSQL_STATUS_READY && s->internal_result) {
        stmt_result *r = (stmt_result*)s->internal_result;
        if (r->cursor_open) {
            r->cursor_open = 0;
            ST(m)->streaming_active = 0;
            m->status = MYSQL_STATUS_READY;
        } else if (!r->stored && !r->wire_eof && m->fd >= 0) {
            for (;;) {
                uint8_t *dp = NULL; size_t dl = 0;
                if (read_compressed_packet(m, &dp, &dl) < 0) break;
                if (dl > 0 && dp[0] == 0xFF) { free(dp); break; }
                if (ST(m)->deprecate_eof) {
                    if (dl > 0 && dp[0] == 0xFE) { free(dp); break; }
                } else {
                    if (dl > 0 && dp[0] == 0xFE && dl < 9) { free(dp); break; }
                }
                free(dp);
            }
            ST(m)->streaming_active = 0;
            m->status = MYSQL_STATUS_READY;
        }
    }
    stmt_result_free(s);

    uint8_t *p = NULL; size_t len = 0;
    if (read_compressed_packet(m, &p, &len) < 0) { stmt_err_from_conn(s); return 1; }
    int kind = packet_kind(m, p, len);
    if (kind == 2) { parse_err(m, p, len); stmt_err_from_conn(s); free(p); return 1; }
    if (kind == 1 || kind == 3) {
        parse_ok(m, p, len);
        s->affected_rows = m->affected_rows; s->insert_id = m->insert_id;
        s->field_count = 0;
        m->status = MYSQL_STATUS_READY;
        m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
        free(p);
        return 0;
    }
    const uint8_t *cur = p, *end = p + len;
    uint64_t cols = lenenc_int(&cur, end, NULL);
    free(p);

    free_field_array(s->fields, s->field_count);
    s->fields = NULL;
    s->field_count = (unsigned int)cols;
    if (stmt_read_defs(m, &s->fields, s->field_count) < 0) { stmt_err_from_conn(s); return 1; }

    stmt_result *r = (stmt_result*)calloc(1, sizeof(stmt_result));
    if (!r) { stmt_err(s, 2008, "HY000", "Out of memory"); return 1; }
    s->internal_result = r;
    ST(m)->streaming_active = 1;
    m->status = MYSQL_STATUS_STATEMENT_GET_RESULT;
    s->state = MYSQL_STMT_EXECUTE_DONE;
    m->multi_status = (m->server_status & SERVER_MORE_RESULTS_EXISTS) ? 1 : 0;
    return 0;
}

unsigned int mysql_thread_safe(void) { return 1; }
void mysql_debug(const char *debug) { (void)debug; }

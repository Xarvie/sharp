/*
 * mitmysql - transport implementations.
 *
 * Two transports implement mm_transport:
 *   - plaintext: a thin loop over recv()/send() on a TCP fd.
 *   - tls:       an mbedtls TLS 1.2+ session layered on the same fd.
 *
 * This file is the ONLY place that includes mbedtls. Everything above the
 * transport seam (framing, handshake, queries) is crypto-agnostic.
 */
#include "transport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/md.h>
#include <mbedtls/error.h>

/* ===================== plaintext ===================== */

typedef struct { int fd; } mm_plain;

static int plain_read(mm_transport *t, void *buf, size_t n) {
    mm_plain *s = (mm_plain *)t->ctx;
    uint8_t *p = (uint8_t *)buf;
    while (n) {
        ssize_t r = recv(s->fd, p, n, 0);
        if (r == 0) return 1;                         /* clean EOF */
        if (r < 0) {
            if (errno == EINTR) continue;
            t->errstr = strerror(errno);
            return -1;
        }
        p += r; n -= (size_t)r;
    }
    return 0;
}

static int plain_write(mm_transport *t, const void *buf, size_t n) {
    mm_plain *s = (mm_plain *)t->ctx;
    const uint8_t *p = (const uint8_t *)buf;
#ifdef MSG_NOSIGNAL
    int flags = MSG_NOSIGNAL;   /* never raise SIGPIPE in the host process */
#else
    int flags = 0;
#endif
    while (n) {
        ssize_t w = send(s->fd, p, n, flags);
        if (w < 0) {
            if (errno == EINTR) continue;
            t->errstr = strerror(errno);
            return -1;
        }
        p += w; n -= (size_t)w;
    }
    return 0;
}

static void plain_close(mm_transport *t) {
    mm_plain *s = (mm_plain *)t->ctx;
    if (s) { if (s->fd >= 0) close(s->fd); free(s); }
    free(t);
}

mm_transport *mm_transport_plain_new(int fd) {
    mm_transport *t = (mm_transport *)calloc(1, sizeof(*t));
    mm_plain     *s = (mm_plain *)calloc(1, sizeof(*s));
    if (!t || !s) { free(t); free(s); return NULL; }
    s->fd    = fd;
    t->ctx   = s;
    t->read  = plain_read;
    t->write = plain_write;
    t->close = plain_close;
    return t;
}

/* ===================== TLS (mbedtls) ===================== */

typedef struct {
    mbedtls_net_context      net;
    mbedtls_ssl_context      ssl;
    mbedtls_ssl_config       conf;
    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_x509_crt         cacert;
    mbedtls_x509_crt         clicert;
    mbedtls_pk_context       pkey;
    char                     errbuf[160];   /* backs mm_transport.errstr */
} mm_tls;

static void mm_strerr(mm_tls *s, int code, const char *what) {
    char det[112];
    mbedtls_strerror(code, det, sizeof(det));
    snprintf(s->errbuf, sizeof(s->errbuf), "%s: %s (-0x%04x)",
             what, det, (unsigned)(-code));
}

static int tls_read(mm_transport *t, void *buf, size_t n) {
    mm_tls *s = (mm_tls *)t->ctx;
    uint8_t *p = (uint8_t *)buf;
    while (n) {
        int r = mbedtls_ssl_read(&s->ssl, p, n);
        if (r == MBEDTLS_ERR_SSL_WANT_READ || r == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (r == 0 || r == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) return 1;   /* EOF */
        if (r < 0) { mm_strerr(s, r, "TLS read"); t->errstr = s->errbuf; return -1; }
        p += r; n -= (size_t)r;
    }
    return 0;
}

static int tls_write(mm_transport *t, const void *buf, size_t n) {
    mm_tls *s = (mm_tls *)t->ctx;
    const uint8_t *p = (const uint8_t *)buf;
    while (n) {
        int w = mbedtls_ssl_write(&s->ssl, p, n);
        if (w == MBEDTLS_ERR_SSL_WANT_READ || w == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (w < 0) { mm_strerr(s, w, "TLS write"); t->errstr = s->errbuf; return -1; }
        p += w; n -= (size_t)w;
    }
    return 0;
}

static void tls_close(mm_transport *t) {
    mm_tls *s = (mm_tls *)t->ctx;
    if (s) {
        mbedtls_ssl_close_notify(&s->ssl);
        mbedtls_ssl_free(&s->ssl);
        mbedtls_ssl_config_free(&s->conf);
        mbedtls_x509_crt_free(&s->cacert);
        mbedtls_x509_crt_free(&s->clicert);
        mbedtls_pk_free(&s->pkey);
        mbedtls_ctr_drbg_free(&s->drbg);
        mbedtls_entropy_free(&s->entropy);
        mbedtls_net_free(&s->net);   /* closes the fd */
        free(s);
    }
    free(t);
}

mm_transport *mm_transport_tls_upgrade(mm_transport *plain,
                                       const mm_ssl_opts *opts,
                                       const char *server_name,
                                       char *cipher_out, size_t cipher_len,
                                       char *errbuf, size_t errlen) {
    /* Extract the fd from the plaintext transport (we reuse the socket). */
    int fd = ((mm_plain *)plain->ctx)->fd;

    mm_transport *t = (mm_transport *)calloc(1, sizeof(*t));
    mm_tls       *s = (mm_tls *)calloc(1, sizeof(*s));
    if (!t || !s) { free(t); free(s); snprintf(errbuf, errlen, "out of memory"); return NULL; }

    mbedtls_net_init(&s->net);
    s->net.fd = fd;
    mbedtls_ssl_init(&s->ssl);
    mbedtls_ssl_config_init(&s->conf);
    mbedtls_entropy_init(&s->entropy);
    mbedtls_ctr_drbg_init(&s->drbg);
    mbedtls_x509_crt_init(&s->cacert);
    mbedtls_x509_crt_init(&s->clicert);
    mbedtls_pk_init(&s->pkey);

    int rc;
    const char *pers = "mitmysql";
    rc = mbedtls_ctr_drbg_seed(&s->drbg, mbedtls_entropy_func, &s->entropy,
                               (const unsigned char *)pers, strlen(pers));
    if (rc) { mm_strerr(s, rc, "RNG seed"); goto fail; }

    rc = mbedtls_ssl_config_defaults(&s->conf, MBEDTLS_SSL_IS_CLIENT,
                                     MBEDTLS_SSL_TRANSPORT_STREAM,
                                     MBEDTLS_SSL_PRESET_DEFAULT);
    if (rc) { mm_strerr(s, rc, "TLS config"); goto fail; }

    /* Refuse anything below TLS 1.2. */
    mbedtls_ssl_conf_min_version(&s->conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                 MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_rng(&s->conf, mbedtls_ctr_drbg_random, &s->drbg);

    /* Server authentication. */
    if (opts->verify >= MM_TLS_VERIFY_CA) {
        int loaded = 0;
        if (opts->ca && *opts->ca) {
            rc = mbedtls_x509_crt_parse_file(&s->cacert, opts->ca);
            if (rc < 0) { mm_strerr(s, rc, "load CA file"); goto fail; }
            loaded = 1;
        }
        if (opts->capath && *opts->capath) {
            rc = mbedtls_x509_crt_parse_path(&s->cacert, opts->capath);
            if (rc < 0) { mm_strerr(s, rc, "load CA path"); goto fail; }
            loaded = 1;
        }
        if (!loaded) {
            snprintf(errbuf, errlen,
                     "TLS verification requested but no CA configured "
                     "(set MYSQL_OPT_SSL_CA or MYSQL_OPT_SSL_CAPATH)");
            goto fail_noerr;
        }
        mbedtls_ssl_conf_ca_chain(&s->conf, &s->cacert, NULL);
        mbedtls_ssl_conf_authmode(&s->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    } else {
        /* PREFERRED / REQUIRED: encrypt without authenticating the peer. */
        mbedtls_ssl_conf_authmode(&s->conf, MBEDTLS_SSL_VERIFY_NONE);
    }

    /* Optional client certificate (mutual TLS). */
    if (opts->cert && *opts->cert && opts->key && *opts->key) {
        rc = mbedtls_x509_crt_parse_file(&s->clicert, opts->cert);
        if (rc < 0) { mm_strerr(s, rc, "load client cert"); goto fail; }
        rc = mbedtls_pk_parse_keyfile(&s->pkey, opts->key, NULL);
        if (rc != 0) { mm_strerr(s, rc, "load client key"); goto fail; }
        rc = mbedtls_ssl_conf_own_cert(&s->conf, &s->clicert, &s->pkey);
        if (rc != 0) { mm_strerr(s, rc, "set client cert"); goto fail; }
    }

    rc = mbedtls_ssl_setup(&s->ssl, &s->conf);
    if (rc) { mm_strerr(s, rc, "TLS setup"); goto fail; }

    /* SNI; also the name checked against the cert when verify == IDENTITY. */
    if (opts->verify == MM_TLS_VERIFY_IDENTITY && server_name && *server_name) {
        rc = mbedtls_ssl_set_hostname(&s->ssl, server_name);
        if (rc) { mm_strerr(s, rc, "set hostname"); goto fail; }
    } else if (server_name && *server_name) {
        mbedtls_ssl_set_hostname(&s->ssl, server_name);  /* SNI only */
    }

    mbedtls_ssl_set_bio(&s->ssl, &s->net, mbedtls_net_send, mbedtls_net_recv, NULL);

    do { rc = mbedtls_ssl_handshake(&s->ssl); }
    while (rc == MBEDTLS_ERR_SSL_WANT_READ || rc == MBEDTLS_ERR_SSL_WANT_WRITE);
    if (rc != 0) { mm_strerr(s, rc, "TLS handshake"); goto fail; }

    if (opts->verify >= MM_TLS_VERIFY_CA) {
        uint32_t f = mbedtls_ssl_get_verify_result(&s->ssl);
        if (f != 0) {
            char vb[128];
            mbedtls_x509_crt_verify_info(vb, sizeof(vb), "", f);
            char *nl = strchr(vb, '\n'); if (nl) *nl = 0;
            snprintf(errbuf, errlen, "server certificate rejected: %s", vb);
            goto fail_noerr;
        }
    }

    if (cipher_out && cipher_len) {
        const char *cs = mbedtls_ssl_get_ciphersuite(&s->ssl);
        snprintf(cipher_out, cipher_len, "%s", cs ? cs : "");
    }

    /* Success: TLS now owns the fd. Free the plaintext wrapper only. */
    free(plain->ctx);
    free(plain);

    t->ctx   = s;
    t->read  = tls_read;
    t->write = tls_write;
    t->close = tls_close;
    return t;

fail:
    snprintf(errbuf, errlen, "%s", s->errbuf);
fail_noerr:
    s->net.fd = -1;   /* leave the fd to the caller's plaintext transport */
    mbedtls_ssl_free(&s->ssl);
    mbedtls_ssl_config_free(&s->conf);
    mbedtls_x509_crt_free(&s->cacert);
    mbedtls_x509_crt_free(&s->clicert);
    mbedtls_pk_free(&s->pkey);
    mbedtls_ctr_drbg_free(&s->drbg);
    mbedtls_entropy_free(&s->entropy);
    free(s);
    free(t);
    return NULL;
}

/* ===================== RSA full-auth seal (caching_sha2 over plaintext) ===== */

int mm_rsa_seal_password(const unsigned char *pubkey_pem, size_t pem_len,
                         const char *password, const unsigned char nonce[20],
                         unsigned char **out, size_t *outlen,
                         char *errbuf, size_t errlen) {
    *out = NULL; *outlen = 0;
    int ret = -1, rc;
    unsigned char *xored = NULL, *ct = NULL;
    mbedtls_pk_context pk;          mbedtls_pk_init(&pk);
    mbedtls_entropy_context  ent;   mbedtls_entropy_init(&ent);
    mbedtls_ctr_drbg_context drbg;  mbedtls_ctr_drbg_init(&drbg);

    /* mbedtls_pk_parse_public_key wants a NUL-terminated PEM buffer. */
    unsigned char *pem = (unsigned char *)malloc(pem_len + 1);
    if (!pem) { snprintf(errbuf, errlen, "out of memory"); goto done; }
    memcpy(pem, pubkey_pem, pem_len);
    pem[pem_len] = 0;

    rc = mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &ent,
                               (const unsigned char *)"mitmysql-rsa", 12);
    if (rc) { snprintf(errbuf, errlen, "RNG seed failed"); goto done; }

    rc = mbedtls_pk_parse_public_key(&pk, pem, pem_len + 1);
    if (rc != 0) {
        char d[100]; mbedtls_strerror(rc, d, sizeof d);
        snprintf(errbuf, errlen, "parse server public key: %s", d);
        goto done;
    }
    if (mbedtls_pk_get_type(&pk) != MBEDTLS_PK_RSA) {
        snprintf(errbuf, errlen, "server public key is not RSA");
        goto done;
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
    mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA1);

    /* obfuscate: (password + trailing NUL) XOR cyclic(nonce[20]) */
    size_t plen = (password ? strlen(password) : 0) + 1;   /* include NUL */
    xored = (unsigned char *)malloc(plen);
    if (!xored) { snprintf(errbuf, errlen, "out of memory"); goto done; }
    for (size_t i = 0; i < plen; i++) {
        unsigned char c = (i + 1 < plen) ? (unsigned char)password[i] : 0;
        xored[i] = (unsigned char)(c ^ nonce[i % 20]);
    }

    size_t klen = mbedtls_rsa_get_len(rsa);
    if (plen > klen - 42) {           /* OAEP-SHA1 limit: k - 2*hLen - 2 */
        snprintf(errbuf, errlen, "password too long for server RSA key");
        goto done;
    }
    ct = (unsigned char *)malloc(klen);
    if (!ct) { snprintf(errbuf, errlen, "out of memory"); goto done; }

    rc = mbedtls_rsa_rsaes_oaep_encrypt(rsa, mbedtls_ctr_drbg_random, &drbg,
                                        MBEDTLS_RSA_PUBLIC, NULL, 0,
                                        plen, xored, ct);
    if (rc != 0) {
        char d[100]; mbedtls_strerror(rc, d, sizeof d);
        snprintf(errbuf, errlen, "RSA-OAEP encrypt: %s", d);
        goto done;
    }

    *out = ct; *outlen = klen; ct = NULL;   /* hand ownership to caller */
    ret = 0;

done:
    if (xored) { memset(xored, 0, (password ? strlen(password) : 0) + 1); free(xored); }
    free(ct);
    free(pem);
    mbedtls_pk_free(&pk);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&ent);
    return ret;
}

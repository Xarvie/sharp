/*
 * mitmysql - byte-stream transport abstraction.
 *
 * The MySQL wire protocol is framed on top of an ordered, reliable byte
 * stream. That stream can be a bare TCP socket or a TLS session; the protocol
 * code above does not care which. This vtable is the single seam between the
 * two: plaintext and TLS implement the same three operations, so the handshake
 * / packet / query layers are written once and never mention sockets or
 * mbedtls directly.
 *
 * All mbedtls usage in the whole library lives behind this interface, in
 * transport.c. Clean-room: protocol code stays crypto-agnostic.
 */
#ifndef MITMYSQL_TRANSPORT_H
#define MITMYSQL_TRANSPORT_H

#include <stddef.h>

/* read/write are blocking and complete: they transfer exactly `n` bytes or
 * report why they could not. Return value:
 *   0  -> all n bytes transferred
 *   1  -> peer closed the connection cleanly before n bytes
 *  -1  -> error (errstr is set to a human-readable reason)
 */
typedef struct mm_transport {
    int  (*read )(struct mm_transport *t, void *buf, size_t n);
    int  (*write)(struct mm_transport *t, const void *buf, size_t n);
    void (*close)(struct mm_transport *t);   /* closes fd/session and frees t */
    const char *errstr;                      /* last failure reason, or NULL */
    void *ctx;                               /* implementation-private state  */
} mm_transport;

/* TLS verification depth, derived from the connection's ssl_mode. */
enum mm_tls_verify {
    MM_TLS_VERIFY_NONE     = 0,  /* encrypt only (PREFERRED / REQUIRED)        */
    MM_TLS_VERIFY_CA       = 1,  /* verify server chain against CA             */
    MM_TLS_VERIFY_IDENTITY = 2   /* verify chain + that hostname matches cert  */
};

typedef struct {
    int         verify;     /* enum mm_tls_verify                              */
    const char *ca;         /* CA bundle file, or NULL                         */
    const char *capath;     /* CA directory, or NULL                           */
    const char *cert;       /* client certificate file (mutual TLS), or NULL   */
    const char *key;        /* client private key file, or NULL                */
    const char *cipher;     /* reserved for an explicit cipher list (Stage 1+) */
} mm_ssl_opts;

/* Wrap an already-connected fd in a plaintext transport. Takes ownership of
 * the fd (its close() will close the fd). Returns NULL on allocation failure. */
mm_transport *mm_transport_plain_new(int fd);

/* Upgrade a plaintext transport to TLS, performing the handshake on its fd.
 *
 * On success: returns a new TLS transport that owns the fd, and consumes
 *   `plain` (frees the wrapper without closing the fd). The negotiated cipher
 *   name is copied into cipher_out (if non-NULL).
 * On failure: returns NULL, fills errbuf, and leaves `plain` untouched so the
 *   caller can still close it.
 *
 * server_name is used for SNI and (when verify == IDENTITY) hostname checking.
 */
mm_transport *mm_transport_tls_upgrade(mm_transport *plain,
                                       const mm_ssl_opts *opts,
                                       const char *server_name,
                                       char *cipher_out, size_t cipher_len,
                                       char *errbuf, size_t errlen);

/* caching_sha2_password / sha256_password full authentication over a PLAINTEXT
 * channel. Obfuscates the password as (password + NUL) XOR cyclic(nonce[20]),
 * then RSA-OAEP(SHA-1) encrypts it with the server's PEM public key — the same
 * scheme libmysqlclient uses. On success returns 0 and sets *out (malloc'd,
 * caller frees) and *outlen; on failure returns -1 and fills errbuf.
 *
 * This is the one cryptographic operation the protocol layer needs that is not
 * a transport, so it lives here to keep all mbedtls usage in one file. */
int mm_rsa_seal_password(const unsigned char *pubkey_pem, size_t pem_len,
                         const char *password, const unsigned char nonce[20],
                         unsigned char **out, size_t *outlen,
                         char *errbuf, size_t errlen);

#endif /* MITMYSQL_TRANSPORT_H */

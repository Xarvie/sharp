/* Clean-room SHA-1 / SHA-256, written from FIPS 180-4. Public domain. */
#ifndef MITMYSQL_SHA_H
#define MITMYSQL_SHA_H

#include <stddef.h>
#include <stdint.h>

#define MM_SHA1_DIGEST_LEN   20
#define MM_SHA256_DIGEST_LEN 32

void mm_sha1(const void *data, size_t len, uint8_t out[20]);
void mm_sha256(const void *data, size_t len, uint8_t out[32]);

/* SHA-256 of two concatenated buffers without an intermediate copy. */
void mm_sha256_2(const void *a, size_t la, const void *b, size_t lb, uint8_t out[32]);
void mm_sha1_2(const void *a, size_t la, const void *b, size_t lb, uint8_t out[20]);

#endif

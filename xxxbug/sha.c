/* Clean-room SHA-1 / SHA-256 from FIPS 180-4. Public domain / MIT. */
#include "sha.h"
#include <string.h>

/* ============================ SHA-1 ============================ */
typedef struct {
    uint32_t h[5];
    uint64_t len;
    uint8_t  buf[64];
    size_t   n;
} sha1_ctx;

#define ROL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void sha1_block(sha1_ctx *c, const uint8_t *p) {
    uint32_t w[80], a, b, d, e, f, k, t;
    uint32_t cc;
    int i;
    for (i = 0; i < 16; i++)
        w[i] = (uint32_t)p[i*4] << 24 | (uint32_t)p[i*4+1] << 16 |
               (uint32_t)p[i*4+2] << 8 | (uint32_t)p[i*4+3];
    for (i = 16; i < 80; i++)
        w[i] = ROL32(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    a = c->h[0]; b = c->h[1]; cc = c->h[2]; d = c->h[3]; e = c->h[4];
    for (i = 0; i < 80; i++) {
        if (i < 20)      { f = (b & cc) | ((~b) & d);            k = 0x5A827999; }
        else if (i < 40) { f = b ^ cc ^ d;                       k = 0x6ED9EBA1; }
        else if (i < 60) { f = (b & cc) | (b & d) | (cc & d);    k = 0x8F1BBCDC; }
        else             { f = b ^ cc ^ d;                       k = 0xCA62C1D6; }
        t = ROL32(a, 5) + f + e + k + w[i];
        e = d; d = cc; cc = ROL32(b, 30); b = a; a = t;
    }
    c->h[0] += a; c->h[1] += b; c->h[2] += cc; c->h[3] += d; c->h[4] += e;
}

static void sha1_init(sha1_ctx *c) {
    c->h[0] = 0x67452301; c->h[1] = 0xEFCDAB89; c->h[2] = 0x98BADCFE;
    c->h[3] = 0x10325476; c->h[4] = 0xC3D2E1F0;
    c->len = 0; c->n = 0;
}
static void sha1_update(sha1_ctx *c, const uint8_t *p, size_t len) {
    c->len += len;
    while (len) {
        size_t take = 64 - c->n;
        if (take > len) take = len;
        memcpy(c->buf + c->n, p, take);
        c->n += take; p += take; len -= take;
        if (c->n == 64) { sha1_block(c, c->buf); c->n = 0; }
    }
}
static void sha1_final(sha1_ctx *c, uint8_t out[20]) {
    uint64_t bits = c->len * 8;
    uint8_t pad = 0x80;
    int i;
    sha1_update(c, &pad, 1);
    pad = 0;
    while (c->n != 56) sha1_update(c, &pad, 1);
    for (i = 7; i >= 0; i--) { uint8_t b = (uint8_t)(bits >> (i*8)); sha1_update(c, &b, 1); }
    for (i = 0; i < 5; i++) {
        out[i*4]   = (uint8_t)(c->h[i] >> 24);
        out[i*4+1] = (uint8_t)(c->h[i] >> 16);
        out[i*4+2] = (uint8_t)(c->h[i] >> 8);
        out[i*4+3] = (uint8_t)(c->h[i]);
    }
}

void mm_sha1(const void *data, size_t len, uint8_t out[20]) {
    sha1_ctx c; sha1_init(&c); sha1_update(&c, (const uint8_t*)data, len); sha1_final(&c, out);
}
void mm_sha1_2(const void *a, size_t la, const void *b, size_t lb, uint8_t out[20]) {
    sha1_ctx c; sha1_init(&c);
    sha1_update(&c, (const uint8_t*)a, la);
    sha1_update(&c, (const uint8_t*)b, lb);
    sha1_final(&c, out);
}

/* ============================ SHA-256 ============================ */
typedef struct {
    uint32_t h[8];
    uint64_t len;
    uint8_t  buf[64];
    size_t   n;
} sha256_ctx;

static const uint32_t K256[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROR32(x,n) (((x) >> (n)) | ((x) << (32-(n))))

static void sha256_block(sha256_ctx *c, const uint8_t *p) {
    uint32_t w[64], a,b,cc,d,e,f,g,hh,t1,t2,s0,s1,ch,maj;
    int i;
    for (i = 0; i < 16; i++)
        w[i] = (uint32_t)p[i*4] << 24 | (uint32_t)p[i*4+1] << 16 |
               (uint32_t)p[i*4+2] << 8 | (uint32_t)p[i*4+3];
    for (i = 16; i < 64; i++) {
        s0 = ROR32(w[i-15],7) ^ ROR32(w[i-15],18) ^ (w[i-15] >> 3);
        s1 = ROR32(w[i-2],17) ^ ROR32(w[i-2],19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    a=c->h[0];b=c->h[1];cc=c->h[2];d=c->h[3];e=c->h[4];f=c->h[5];g=c->h[6];hh=c->h[7];
    for (i = 0; i < 64; i++) {
        s1 = ROR32(e,6) ^ ROR32(e,11) ^ ROR32(e,25);
        ch = (e & f) ^ ((~e) & g);
        t1 = hh + s1 + ch + K256[i] + w[i];
        s0 = ROR32(a,2) ^ ROR32(a,13) ^ ROR32(a,22);
        maj = (a & b) ^ (a & cc) ^ (b & cc);
        t2 = s0 + maj;
        hh=g; g=f; f=e; e=d+t1; d=cc; cc=b; b=a; a=t1+t2;
    }
    c->h[0]+=a;c->h[1]+=b;c->h[2]+=cc;c->h[3]+=d;c->h[4]+=e;c->h[5]+=f;c->h[6]+=g;c->h[7]+=hh;
}

static void sha256_init(sha256_ctx *c) {
    c->h[0]=0x6a09e667;c->h[1]=0xbb67ae85;c->h[2]=0x3c6ef372;c->h[3]=0xa54ff53a;
    c->h[4]=0x510e527f;c->h[5]=0x9b05688c;c->h[6]=0x1f83d9ab;c->h[7]=0x5be0cd19;
    c->len=0;c->n=0;
}
static void sha256_update(sha256_ctx *c, const uint8_t *p, size_t len) {
    c->len += len;
    while (len) {
        size_t take = 64 - c->n;
        if (take > len) take = len;
        memcpy(c->buf + c->n, p, take);
        c->n += take; p += take; len -= take;
        if (c->n == 64) { sha256_block(c, c->buf); c->n = 0; }
    }
}
static void sha256_final(sha256_ctx *c, uint8_t out[32]) {
    uint64_t bits = c->len * 8;
    uint8_t pad = 0x80;
    int i;
    sha256_update(c, &pad, 1);
    pad = 0;
    while (c->n != 56) sha256_update(c, &pad, 1);
    for (i = 7; i >= 0; i--) { uint8_t b = (uint8_t)(bits >> (i*8)); sha256_update(c, &b, 1); }
    for (i = 0; i < 8; i++) {
        out[i*4]   = (uint8_t)(c->h[i] >> 24);
        out[i*4+1] = (uint8_t)(c->h[i] >> 16);
        out[i*4+2] = (uint8_t)(c->h[i] >> 8);
        out[i*4+3] = (uint8_t)(c->h[i]);
    }
}

void mm_sha256(const void *data, size_t len, uint8_t out[32]) {
    sha256_ctx c; sha256_init(&c); sha256_update(&c, (const uint8_t*)data, len); sha256_final(&c, out);
}
void mm_sha256_2(const void *a, size_t la, const void *b, size_t lb, uint8_t out[32]) {
    sha256_ctx c; sha256_init(&c);
    sha256_update(&c, (const uint8_t*)a, la);
    sha256_update(&c, (const uint8_t*)b, lb);
    sha256_final(&c, out);
}

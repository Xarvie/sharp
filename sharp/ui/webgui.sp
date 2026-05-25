/*
 * webgui.c — Single-threaded, non-blocking implementation of WebGUI.
 *
 * Design notes:
 *   - One thread runs everything: HTTP/WS protocol, frame parsing,
 *     event dispatch, timers, user callbacks.  No background threads.
 *   - The event loop is one poll(2) over: the listen socket, every
 *     connected client socket, and a self-pipe used to wake the loop
 *     from wg_post() on other threads.
 *   - Tree state needs no locks — only the event-loop thread touches it.
 *   - Cross-thread wg_post is the only synchronized path: a tiny mutex
 *     plus a one-byte self-pipe wake.
 *   - Per-client send buffers absorb sends; the loop drains them on
 *     POLLOUT.  ws_broadcast and per-client ws_send_text both just
 *     append bytes — never blocking on socket writes.
 *
 * Portability: POSIX (Linux / macOS) via <poll.h>, <sys/socket.h>.
 *              Windows via WSAPoll + WSAEventSelect + SRWLOCK.
 */

/* Feature-test for clock_gettime(CLOCK_MONOTONIC) etc. */
#if !defined(_WIN32) && !defined(_WIN64)
  #define _POSIX_C_SOURCE 200809L
#endif

#include "webgui.sph"

#include "cjson.sph"

#include "webgui_html.sph"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

/* ========================================================================
 * Platform abstraction
 * ======================================================================== */

#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>
  #pragma comment(lib, "ws2_32.lib")

  typedef SOCKET wg__socket_t;
  #define WG__INVALID_SOCKET INVALID_SOCKET

  static int wg__posix_errno(void) {
    int e = WSAGetLastError();
    switch (e) {
      case WSAEWOULDBLOCK: return 35;       /* EAGAIN */
      case WSAEINTR:       return 4;        /* EINTR  */
      case WSAECONNRESET:  return 104;      /* ECONNRESET */
      case WSAENOTCONN:    return 107;      /* ENOTCONN */
      default:             return e;
    }
  }
  #define WG__EAGAIN  35
  #define WG__EINTR   4

  /* Non-blocking via ioctlsocket */
  static int wg__set_nonblocking_socket(wg__socket_t s) {
    u_long mode = 1;
    return ioctlsocket(s, FIONBIO, &mode);
  }

  /* Windows: use WSAPoll instead of poll */
  #define poll    WSAPoll

  /* Windows: use SRWLOCK instead of pthread_mutex */
  typedef struct { SRWLOCK lock; } wg__mutex_t;
  #define WG__MUTEX_INIT    { SRWLOCK_INIT }
  static void wg__mutex_init(wg__mutex_t *m) { InitializeSRWLock(&m->lock); }
  static void wg__mutex_lock(wg__mutex_t *m) { AcquireSRWLockExclusive(&m->lock); }
  static void wg__mutex_unlock(wg__mutex_t *m) { ReleaseSRWLockExclusive(&m->lock); }
  static void wg__mutex_destroy(wg__mutex_t *m) { (void)m; }

  /* Self-pipe replacement: TCP socketpair for cross-thread wakeup.
   * We create a listening socket on loopback, connect to it, then
   * discard the listener.  The two connected sockets work as a pipe. */
  static int wg__wakeup_init(int *p) {
    /* Find a free port by binding to 0. */
    SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET) return -1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        closesocket(listener); return -1;
    }
    int alen = sizeof(addr);
    if (getsockname(listener, (struct sockaddr*)&addr, &alen) < 0) {
        closesocket(listener); return -1;
    }
    if (listen(listener, 1) < 0) { closesocket(listener); return -1; }

    /* Connect client side. */
    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET) { closesocket(listener); return -1; }
    if (connect(client, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        closesocket(client); closesocket(listener); return -1;
    }

    /* Accept server side. */
    SOCKET server = accept(listener, NULL, NULL);
    closesocket(listener);
    if (server == INVALID_SOCKET) { closesocket(client); return -1; }

    /* Both non-blocking. */
    u_long mode = 1;
    ioctlsocket(client, FIONBIO, &mode);
    ioctlsocket(server, FIONBIO, &mode);

    p[0] = (int)server;  /* read end */
    p[1] = (int)client;  /* write end */
    return 0;
  }
  static void wg__wakeup_signal(int w) {
    uint8_t one = 1;
    send(w, (const char*)&one, 1, 0);
  }
  static void wg__wakeup_drain(int r) {
    uint8_t buf[256];
    for (;;) {
        int n = recv(r, (char*)buf, sizeof(buf), 0);
        if (n > 0) continue;
        break;
    }
  }
  static void wg__wakeup_close(int r) { closesocket((SOCKET)r); }

  /* close() -> closesocket() */
  #define close(fd) closesocket(fd)

  /* gettimeofday fallback */
  #if defined(_MSC_VER)
    #define WG__THREAD_LOCAL __declspec(thread)
  #else
    #define WG__THREAD_LOCAL __thread
#endif

#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <pthread.h>
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <poll.h>
  #include <signal.h>

  typedef int wg__socket_t;
  #define WG__INVALID_SOCKET (-1)
  #define WG__EAGAIN  EAGAIN
  #define WG__EINTR   EINTR

  static int wg__set_nonblocking_socket(int s) {
    int fl = fcntl(s, F_GETFL, 0);
    if (fl < 0) return -1;
    return fcntl(s, F_SETFL, fl | O_NONBLOCK);
  }

  typedef pthread_mutex_t wg__mutex_t;
  static void wg__mutex_init(wg__mutex_t *m) { pthread_mutex_init(m, NULL); }
  static void wg__mutex_lock(wg__mutex_t *m) { pthread_mutex_lock(m); }
  static void wg__mutex_unlock(wg__mutex_t *m) { pthread_mutex_unlock(m); }
  static void wg__mutex_destroy(wg__mutex_t *m) { pthread_mutex_destroy(m); }

  static int wg__wakeup_init(int p[2]) { return pipe(p); }
  static void wg__wakeup_signal(int w) {
    uint8_t one = 1;
    ssize_t n; do { n = write(w, &one, 1); } while (n < 0 && errno == EINTR);
  }
  static void wg__wakeup_drain(int r) {
    uint8_t buf[256];
    for (;;) {
      ssize_t n = read(r, buf, sizeof(buf));
      if (n > 0) continue;
      if (n < 0 && errno == EINTR) continue;
      break;
    }
  }
  static void wg__wakeup_close(int r) { (void)r; }

  #define WG__THREAD_LOCAL __thread
#endif

/* ========================================================================
 * Arena types — must be before any static function that references them,
 * because sharpc emits forward declarations for static functions at the
 * top of the generated C output.
 * ======================================================================== */

#define WG__ARENA_PAGE_SIZE (64 * 1024)

typedef struct wg__arena_page {
    struct wg__arena_page *next;
    size_t                 cap;
    size_t                 used;
} wg__arena_page_t;

typedef struct {
    wg__arena_page_t *pages;
    size_t            total_alloc;
} wg_arena_t;

/* Monotonic time */
static int64_t wg__now_ms(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq;
    static int init = 0;
    if (!init) { QueryPerformanceFrequency(&freq); init = 1; }
    LARGE_INTEGER now; QueryPerformanceCounter(&now);
    return (int64_t)(now.QuadPart * 1000 / freq.QuadPart);
#elif defined(CLOCK_MONOTONIC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
    struct timeval tv; gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

/* ========================================================================
 * Tunables
 * ======================================================================== */

#define WG__MAX_HTTP_HEADER     (16 * 1024)
#define WG__MAX_MSG_SIZE        (4  * 1024 * 1024)
#define WG__BACKLOG             32

/* ========================================================================
 * errno
 * ======================================================================== */

static WG__THREAD_LOCAL int wg__errno_val = WG_OK;

int  wg_errno(void)         { return wg__errno_val; }
static int wg__set_err(int e) { wg__errno_val = e; return e; }

const char *wg_strerror(int err) {
    switch (err) {
        case WG_OK:      return "ok";
        case WG_ENOMEM:  return "out of memory";
        case WG_ENOENT:  return "not found";
        case WG_EINVAL:  return "invalid argument";
        case WG_ENOTSUP: return "not supported";
        case WG_EIO:     return "I/O error";
        case WG_EAGAIN:  return "try again";
        case WG_EEXIST:  return "already exists";
        default:         return "unknown error";
    }
}

/* ========================================================================
 * SHA1 — reference impl for the WebSocket handshake.
 * ======================================================================== */

typedef struct { uint32_t h[5]; uint64_t n; uint8_t buf[64]; size_t buflen; } wg__sha1_t;

static void wg__sha1_init(wg__sha1_t *s) {
    s->h[0]=0x67452301; s->h[1]=0xEFCDAB89; s->h[2]=0x98BADCFE;
    s->h[3]=0x10325476; s->h[4]=0xC3D2E1F0;
    s->n=0; s->buflen=0;
}
static uint32_t wg__rol(uint32_t v, int n) { return (v<<n) | (v>>(32-n)); }
static void wg__sha1_block(wg__sha1_t *s, const uint8_t *p) {
    uint32_t w[80];
    for (int i=0;i<16;i++)
        w[i]=((uint32_t)p[i*4]<<24)|((uint32_t)p[i*4+1]<<16)
            |((uint32_t)p[i*4+2]<<8)|p[i*4+3];
    for (int i=16;i<80;i++) w[i]=wg__rol(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
    uint32_t a=s->h[0],b=s->h[1],c=s->h[2],d=s->h[3],e=s->h[4];
    for (int i=0;i<80;i++) {
        uint32_t f, k;
        if      (i < 20) { f=(b&c)|((~b)&d);   k=0x5A827999; }
        else if (i < 40) { f=b^c^d;             k=0x6ED9EBA1; }
        else if (i < 60) { f=(b&c)|(b&d)|(c&d); k=0x8F1BBCDC; }
        else             { f=b^c^d;             k=0xCA62C1D6; }
        uint32_t t = wg__rol(a,5)+f+e+k+w[i];
        e=d; d=c; c=wg__rol(b,30); b=a; a=t;
    }
    s->h[0]+=a; s->h[1]+=b; s->h[2]+=c; s->h[3]+=d; s->h[4]+=e;
}
static void wg__sha1_update(wg__sha1_t *s, const void *data, size_t len) {
    const uint8_t *p = (const uint8_t*)data;
    s->n += (uint64_t)len * 8;
    while (len) {
        size_t k = 64 - s->buflen;
        if (k > len) k = len;
        memcpy(s->buf+s->buflen, p, k);
        s->buflen += k; p += k; len -= k;
        if (s->buflen == 64) { wg__sha1_block(s, s->buf); s->buflen = 0; }
    }
}
static void wg__sha1_final(wg__sha1_t *s, uint8_t out[20]) {
    s->buf[s->buflen++] = 0x80;
    if (s->buflen > 56) {
        while (s->buflen < 64) s->buf[s->buflen++] = 0;
        wg__sha1_block(s, s->buf); s->buflen = 0;
    }
    while (s->buflen < 56) s->buf[s->buflen++] = 0;
    for (int i = 7; i >= 0; i--)
        s->buf[s->buflen++] = (uint8_t)(s->n >> (i*8));
    wg__sha1_block(s, s->buf);
    for (int i = 0; i < 5; i++) {
        out[i*4  ] = (uint8_t)(s->h[i] >> 24);
        out[i*4+1] = (uint8_t)(s->h[i] >> 16);
        out[i*4+2] = (uint8_t)(s->h[i] >>  8);
        out[i*4+3] = (uint8_t) s->h[i];
    }
}

/* Base64-encode exactly 20 bytes into a 28-char + NUL buffer. */
static void wg__b64_20(const uint8_t in[20], char out[29]) {
    static const char tab[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int o = 0;
    for (int i = 0; i < 18; i += 3) {
        uint32_t v = ((uint32_t)in[i] << 16) | ((uint32_t)in[i+1] << 8) | in[i+2];
        out[o++] = tab[(v >> 18) & 0x3F];
        out[o++] = tab[(v >> 12) & 0x3F];
        out[o++] = tab[(v >>  6) & 0x3F];
        out[o++] = tab[(v      ) & 0x3F];
    }
    uint32_t v = ((uint32_t)in[18] << 16) | ((uint32_t)in[19] << 8);
    out[o++] = tab[(v >> 18) & 0x3F];
    out[o++] = tab[(v >> 12) & 0x3F];
    out[o++] = tab[(v >>  6) & 0x3F];
    out[o++] = '=';
    out[o]   = '\0';
}

static void wg__ws_accept_hash(const char *key, char out[29]) {
    static const char guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    wg__sha1_t s; wg__sha1_init(&s);
    wg__sha1_update(&s, key, strlen(key));
    wg__sha1_update(&s, guid, sizeof(guid) - 1);
    uint8_t digest[20];
    wg__sha1_final(&s, digest);
    wg__b64_20(digest, out);
}

/* ========================================================================
 * String helpers
 * ======================================================================== */

static char *wg__strdup(const char *s) {
    if (!s) s = "";
    size_t n = strlen(s);
    char *d = (char*)malloc(n + 1);
    if (!d) return NULL;
    memcpy(d, s, n + 1);
    return d;
}

static int wg__str_set(char **slot, const char *s) {
    char *d = wg__strdup(s);
    if (!d) return WG_ENOMEM;
    free(*slot);
    *slot = d;
    return WG_OK;
}

/* Arena version: does NOT free the old string (arena can't individual-free).
 * The old string is reclaimed on arena_free_all. */
static int wg__str_set_arena(char **slot, const char *s) {
    wg_arena_t *arena = wg__current_app ? &wg__current_app->arena : NULL;
    if (arena) {
        char *d = wg__arena_strdup(arena, s ? s : "");
        if (!d) return WG_ENOMEM;
        *slot = d;
    } else {
        int rc = wg__str_set(slot, s);
        if (rc != WG_OK) return rc;
    }
    return WG_OK;
}

static int wg__strieq(const char *a, const char *b) {
    while (*a && *b) {
        char x = (*a >= 'A' && *a <= 'Z') ? (char)(*a + 32) : *a;
        char y = (*b >= 'A' && *b <= 'Z') ? (char)(*b + 32) : *b;
        if (x != y) return 0;
        a++; b++;
    }
    return *a == *b;
}

/* Case-insensitive equality between the first `n` chars of `s` and the
 * lowercase ASCII literal `lit`.  Used to match HTTP header tokens that
 * aren't NUL-terminated (slices of comma-separated lists). */
static int wg__strnieq_lit(const char *s, size_t n, const char *lit) {
    if (strlen(lit) != n) return 0;
    for (size_t i = 0; i < n; i++) {
        char x = (s[i] >= 'A' && s[i] <= 'Z') ? (char)(s[i] + 32) : s[i];
        if (x != lit[i]) return 0;
    }
    return 1;
}

/* ========================================================================
 * Arena functions
 * ======================================================================== */

static wg__arena_page_t *wg__arena_page_new(size_t cap) {
    wg__arena_page_t *p = (wg__arena_page_t*)malloc(sizeof(*p) + cap);
    if (!p) return NULL;
    p->next = NULL;
    p->cap = cap;
    p->used = 0;
    return p;
}

static void *wg__arena_alloc(void *arena, size_t sz) {
    wg_arena_t *a = (wg_arena_t*)arena;
    if (sz == 0) return NULL;
    /* align to 8 bytes */
    sz = (sz + 7) & ~(size_t)7;
    /* try current page */
    wg__arena_page_t *p = a->pages;
    if (p && p->used + sz <= p->cap) {
        void *ptr = (char*)(p + 1) + p->used;
        p->used += sz;
        a->total_alloc += sz;
        return ptr;
    }
    /* need new page */
    size_t new_cap = p ? p->cap * 2 : WG__ARENA_PAGE_SIZE;
    if (new_cap < sz) new_cap = sz;
    p = wg__arena_page_new(new_cap);
    if (!p) return NULL;
    p->next = a->pages;
    a->pages = p;
    void *ptr = (char*)(p + 1) + p->used;
    p->used += sz;
    a->total_alloc += sz;
    return ptr;
}

static void *wg__arena_alloc0(void *arena, size_t sz) {
    void *p = wg__arena_alloc(arena, sz);
    if (p) memset(p, 0, sz);
    return p;
}

static char *wg__arena_strdup(void *arena, const char *s) {
    if (!s) {
        char *r = (char*)wg__arena_alloc(arena, 1);
        if (r) *r = '\0';
        return r;
    }
    size_t n = strlen(s) + 1;
    char *r = (char*)wg__arena_alloc(arena, n);
    if (r) memcpy(r, s, n);
    return r;
}

static void wg__arena_free_all(void *arena) {
    wg_arena_t *a = (wg_arena_t*)arena;
    wg__arena_page_t *p = a->pages;
    while (p) {
        wg__arena_page_t *next = p->next;
        free(p);
        p = next;
    }
    a->pages = NULL;
    a->total_alloc = 0;
}

/* ========================================================================
 * Internal types
 * ======================================================================== */

typedef enum {
    WG__K_CONTAINER = 0, WG__K_TEXT, WG__K_HEADING, WG__K_BUTTON,
    WG__K_INPUT, WG__K_CHECKBOX, WG__K_SELECT, WG__K_IMAGE,
    WG__K_SEPARATOR, WG__K_FORM,
} wg__kind_t;

/* Forward declarations for custom component registry */
typedef struct wg__comp_type {
    char *name;
    void (*on_destroy)(wg_node_t *n);
} wg__comp_type_t;

static wg__comp_type_t *wg__comp_types;
static size_t wg__comp_types_n;

static const char *wg__kind_name(int k) {
    static const char *names[] = {
        "container","text","heading","button","input","checkbox",
        "select","image","separator","form"
    };
    if (k >= WG__K_CONTAINER && k <= WG__K_FORM) return names[k];
    if (k < WG__K_CONTAINER) {
        int idx = WG__K_CONTAINER - 1 - k;
        if (idx >= 0 && idx < (int)wg__comp_types_n)
            return wg__comp_types[idx].name;
    }
    return "container";
}

typedef struct { char *key; char *value; } wg__attr_t;

struct wg_node {
    char        *id;
    int          kind;
    int          level;             /* heading only */
    char        *text;              /* may be NULL */
    char        *value;             /* may be NULL */
    int          disabled;
    int          visible;
    char       **options;
    size_t       options_n;
    wg__attr_t  *attrs;
    size_t       attrs_n, attrs_cap;
    struct wg_node  *parent;
    struct wg_node **children;
    size_t       children_n, children_cap;
    wg_app_t    *app;
    int          _arena;            /* 1 = arena-allocated (do NOT free individually) */
    /* lifecycle */
    wg_lifecycle_fn on_destroy_fn;
    void           *on_destroy_ud;
};

typedef struct {
    char            *id;
    char            *event;
    wg_callback_fn   cb;
    void            *ud;
} wg__cb_t;

typedef struct {
    int          id;
    int64_t      next_fire_ms;
    int          delay_ms;
    int          repeat;
    int          alive;
    wg_timer_fn  fn;
    void        *ud;
} wg__timer_t;

typedef struct wg__post {
    wg_post_fn       fn;
    void            *ud;
    struct wg__post *next;
} wg__post_t;

/* ---- WebSocket client ---- */

typedef enum {
    /* Reading HTTP request bytes, waiting for end-of-headers. */
    WG__CS_HANDSHAKE = 0,
    /* WebSocket open: parsing/dispatching frames. */
    WG__CS_OPEN,
    /* Socket already closed, struct pending reap at end of tick. */
    WG__CS_DEAD,
} wg__cstate_t;
/* want_close = "drain tx, then close" — works in any state.
 * The on_readable path treats want_close as "ignore further reads". */

typedef struct wg_client {
    wg__socket_t    fd;
    char              ip[64];
    wg__cstate_t      state;
    wg_app_t         *app;

    /* Receive buffer for raw bytes coming off the socket. */
    uint8_t          *rx; size_t rx_len, rx_cap;

    /* Send buffer for bytes queued to write. */
    uint8_t          *tx; size_t tx_len, tx_cap;

    /* Per-message reassembly for fragmented data frames. */
    uint8_t          *msg;
    size_t            msg_len, msg_cap;
    int               msg_op;            /* opcode of first frame in message */
    int               msg_in_progress;

    /* Want-close: when set, after tx drains we close the socket. */
    int               want_close;

    /* vDOM: per-client snapshot of what was last sent */
    cJSON            *sent_tree;     /* JSON snapshot of last sent tree */
    uint32_t          sent_rev;      /* revision counter */
    int               dirty;         /* tree has unsent changes */
    /* Meta snapshot: track what title/icon was last sent */
    char             *sent_meta_title;
    char             *sent_meta_icon;

    struct wg_client *next;
} wg_client_t;

struct wg_app {
    wg_arena_t   arena;       /* application-level memory pool */
    wg_node_t   *root;
    /* idmap (parallel-array hash-free table; linear lookup is fine for
     * the small node counts typical of GUIs). */
    wg_node_t  **idx_nodes;
    size_t       idx_n, idx_cap;

    /* event handlers */
    wg__cb_t    *cbs;
    size_t       cbs_n, cbs_cap;

    /* timers */
    wg__timer_t *timers;
    size_t       timers_n, timers_cap;
    int          next_timer_id;

    /* config */
    char        *title;
    char        *icon_url;
    int          debug;

    /* network — listen_fd >= 0 iff bound */
    int          listen_fd;

    /* connected clients (linked list) */
    wg_client_t *clients;

    /* loop control — only touched on the EL thread */
    int          running;
    int          stop_requested;

    /* cross-thread wakeup: wg_post enqueues onto post_head (under
     * post_mtx) and signals wake_evt; the EL drains both ends
     * inside its poll loop. */
    int              wake_r, wake_w;
    int              wake_evt;          /* Windows: WSAEvent */
    wg__mutex_t      post_mtx;
    wg__post_t      *post_head;

    /* custom frontend HTML (NULL → use built-in) */
    char            *custom_html;
    size_t           custom_html_len;
};

/* ========================================================================
 * Safe cJSON helpers — cJSON_AddItemToObject has two sharp edges: it does
 * not copy the key string (expects caller to manage), and on its own
 * failure it does not free the value.  These wrap the construct-and-attach
 * pattern.
 * ======================================================================== */

static Str s(const char *s) { Str r = {0}; r.ptr = s; r.len = (s ? (isize)strlen(s) : 0); return r; }

static cJSON_bool wg__obj_take(cJSON *obj, const char *key, cJSON *val) {
    if (!obj || !key) { if (val) cJSON_Delete(val); return false; }
    if (!val) return false;
    if (!cJSON_AddItemToObject(obj, s(key), val)) { cJSON_Delete(val); return false; }
    return true;
}
static cJSON_bool wg__obj_set_str(cJSON *o, const char *k, const char *v) {
    return wg__obj_take(o, k, cJSON_CreateString(s(v ? v : "")));
}
static cJSON_bool wg__obj_set_bool(cJSON *o, const char *k, int v) {
    return wg__obj_take(o, k, cJSON_CreateBool(v ? true : false));
}
static cJSON_bool wg__obj_set_int(cJSON *o, const char *k, int64_t v) {
    return wg__obj_take(o, k, cJSON_CreateNumber((double)(v)));
}
static cJSON_bool wg__arr_take(cJSON *arr, cJSON *item) {
    if (!arr) { if (item) cJSON_Delete(item); return false; }
    if (!item) return false;
    if (!cJSON_AddItemToArray(arr, item)) { cJSON_Delete(item); return false; }
    return true;
}

/* cJSON object helpers: count members and access by index (needed for
 * form-data iteration where we used cson_len/cson_obj_iter). */
static int wg__cjson_obj_count(cJSON *obj) {
    if (!obj || !cJSON_IsObject(obj)) return 0;
    int n = 0;
    cJSON *item;
    for (item = obj->child; item; item = item->next) n++;
    return n;
}
static cJSON *wg__cjson_obj_item_at(cJSON *obj, int index) {
    if (!obj || !cJSON_IsObject(obj) || index < 0) return NULL;
    cJSON *item;
    for (item = obj->child; item; item = item->next) {
        if (index-- == 0) return item;
    }
    return NULL;
}

/* ========================================================================
 * Forward decls
 * ======================================================================== */

static int      wg__idx_add   (wg_app_t *app, wg_node_t *n);
static void     wg__idx_remove(wg_app_t *app, const wg_node_t *n);
static int      wg__mount     (wg_app_t *app, wg_node_t *n);
static void     wg__unmount   (wg_node_t *n);
static cJSON  *wg__node_json (const wg_node_t *n);
static cJSON  *wg__render    (wg_app_t *app);
static void     wg__broadcast (wg_app_t *app, cJSON *obj);
static void     wg__mark_dirty(wg_app_t *app);
static void     wg__send_obj  (wg_client_t *c, cJSON *obj);
static void     wg__ws_send_text(wg_client_t *c, const char *data, size_t len);
static void     wg__client_close(wg_client_t *c);

/* ========================================================================
 * Node constructors
 * ======================================================================== */

/* Thread-local current app — set by wg_app_create, used by node
 * constructors to allocate from the app's arena. */
static __thread wg_app_t *wg__current_app;

static wg_node_t *wg__node_alloc(int kind, const char *id) {
    if (!id || !*id) { wg__set_err(WG_EINVAL); return NULL; }
    wg_arena_t *arena = wg__current_app ? &wg__current_app->arena : NULL;
    wg_node_t *n;
    if (arena) {
        n = (wg_node_t*)wg__arena_alloc0(arena, sizeof(*n));
        if (!n) { wg__set_err(WG_ENOMEM); return NULL; }
        n->id = wg__arena_strdup(arena, id);
        n->_arena = 1;
    } else {
        /* fallback: unmounted node, use calloc */
        n = (wg_node_t*)calloc(1, sizeof(*n));
        if (!n) { wg__set_err(WG_ENOMEM); return NULL; }
        n->id = wg__strdup(id);
        n->_arena = 0;
    }
    if (!n->id) { wg__set_err(WG_ENOMEM); return NULL; }
    n->kind = kind;
    n->visible = 1;
    return n;
}

static char *wg__node_strdup(const char *s) {
    if (!s) return NULL;
    wg_arena_t *arena = wg__current_app ? &wg__current_app->arena : NULL;
    if (arena) return wg__arena_strdup(arena, s);
    return wg__strdup(s);
}

wg_node_t *wg_container(const char *id) { return wg__node_alloc(WG__K_CONTAINER, id); }
wg_node_t *wg_separator(const char *id) { return wg__node_alloc(WG__K_SEPARATOR, id); }
wg_node_t *wg_form     (const char *id) { return wg__node_alloc(WG__K_FORM, id); }

/* Custom components */
int wg_register_type(const char *type_name, const wg_component_ops_t *ops) {
    if (!type_name) return WG_EINVAL;
    for (size_t i = 0; i < wg__comp_types_n; i++) {
        if (strcmp(wg__comp_types[i].name, type_name) == 0)
            return WG_EEXIST;
    }
    size_t cap = wg__comp_types_n + 1;
    wg__comp_type_t *p = (wg__comp_type_t*)realloc(wg__comp_types, cap * sizeof(*p));
    if (!p) return WG_ENOMEM;
    wg__comp_types = p;
    wg__comp_types[wg__comp_types_n].name = wg__strdup(type_name);
    wg__comp_types[wg__comp_types_n].on_destroy = ops ? ops->on_destroy : NULL;
    wg__comp_types_n++;
    return (int)(wg__comp_types_n - 1);
}

wg_node_t *wg_create(const char *type_name, const char *id) {
    if (!type_name || !id || !*id) { wg__set_err(WG_EINVAL); return NULL; }
    int type_idx = -1;
    for (size_t i = 0; i < wg__comp_types_n; i++) {
        if (strcmp(wg__comp_types[i].name, type_name) == 0) {
            type_idx = (int)i;
            break;
        }
    }
    if (type_idx < 0) { wg__set_err(WG_ENOENT); return NULL; }
    return wg__node_alloc(WG__K_CONTAINER - 1 - type_idx, id);
}

wg_node_t *wg_input    (const char *id) { return wg__node_alloc(WG__K_INPUT, id); }

wg_node_t *wg_text(const char *id, const char *text) {
    wg_node_t *n = wg__node_alloc(WG__K_TEXT, id);
    if (!n) return NULL;
    if ((n->text = wg__node_strdup(text)) == NULL) { wg_free(n); wg__set_err(WG_ENOMEM); return NULL; }
    return n;
}
wg_node_t *wg_heading(const char *id, int level, const char *text) {
    wg_node_t *n = wg__node_alloc(WG__K_HEADING, id);
    if (!n) return NULL;
    if (level < 1) level = 1;
    if (level > 6) level = 6;
    n->level = level;
    if ((n->text = wg__node_strdup(text)) == NULL) { wg_free(n); wg__set_err(WG_ENOMEM); return NULL; }
    return n;
}
wg_node_t *wg_button(const char *id, const char *text) {
    wg_node_t *n = wg__node_alloc(WG__K_BUTTON, id);
    if (!n) return NULL;
    if ((n->text = wg__node_strdup(text)) == NULL) { wg_free(n); wg__set_err(WG_ENOMEM); return NULL; }
    return n;
}
wg_node_t *wg_checkbox(const char *id, const char *label) {
    wg_node_t *n = wg__node_alloc(WG__K_CHECKBOX, id);
    if (!n) return NULL;
    if ((n->text = wg__node_strdup(label)) == NULL) { wg_free(n); wg__set_err(WG_ENOMEM); return NULL; }
    return n;
}
wg_node_t *wg_select(const char *id, const char *const *options, size_t n_opt) {
    wg_node_t *n = wg__node_alloc(WG__K_SELECT, id);
    if (!n) return NULL;
    if (n_opt > 0 && options) {
        wg_arena_t *arena = wg__current_app ? &wg__current_app->arena : NULL;
        if (arena) {
            n->options = (char**)wg__arena_alloc0(arena, n_opt * sizeof(char*));
        } else {
            n->options = (char**)calloc(n_opt, sizeof(char*));
        }
        if (!n->options) { wg_free(n); wg__set_err(WG_ENOMEM); return NULL; }
        for (size_t i = 0; i < n_opt; i++) {
            n->options[i] = wg__node_strdup(options[i] ? options[i] : "");
            if (!n->options[i]) { wg_free(n); wg__set_err(WG_ENOMEM); return NULL; }
            n->options_n = i + 1;
        }
    }
    return n;
}
wg_node_t *wg_image(const char *id, const char *src) {
    wg_node_t *n = wg__node_alloc(WG__K_IMAGE, id);
    if (!n) return NULL;
    if (src && *src && wg_set_attr(n, "src", src) != WG_OK) {
        wg_free(n); wg__set_err(WG_ENOMEM); return NULL;
    }
    return n;
}

/* ========================================================================
 * idmap
 * ======================================================================== */

static int wg__idx_add(wg_app_t *app, wg_node_t *n) {
    if (app->idx_n + 1 > app->idx_cap) {
        size_t cap = app->idx_cap ? app->idx_cap * 2 : 16;
        wg_node_t **p = (wg_node_t**)realloc(app->idx_nodes, cap * sizeof(*p));
        if (!p) return WG_ENOMEM;
        app->idx_nodes = p;
        app->idx_cap   = cap;
    }
    app->idx_nodes[app->idx_n++] = n;
    return WG_OK;
}

static void wg__idx_remove(wg_app_t *app, const wg_node_t *n) {
    for (size_t i = 0; i < app->idx_n; i++) {
        if (app->idx_nodes[i] == n) {
            app->idx_nodes[i] = app->idx_nodes[--app->idx_n];
            return;
        }
    }
}

static wg_node_t *wg__idx_find(const wg_app_t *app, const char *id) {
    if (!app || !id) return NULL;
    for (size_t i = 0; i < app->idx_n; i++) {
        if (strcmp(app->idx_nodes[i]->id, id) == 0) return app->idx_nodes[i];
    }
    return NULL;
}

wg_node_t *wg_find(wg_app_t *app, const char *id) {
    return wg__idx_find(app, id);
}

/* ========================================================================
 * Mount / unmount — atomic w.r.t. duplicate-id and OOM failures.
 * ======================================================================== */

static int wg__mount(wg_app_t *app, wg_node_t *n) {
    if (wg__idx_find(app, n->id)) return WG_EEXIST;
    int rc = wg__idx_add(app, n);
    if (rc != WG_OK) return rc;
    n->app = app;
    size_t mounted = 0;
    for (size_t i = 0; i < n->children_n; i++) {
        rc = wg__mount(app, n->children[i]);
        if (rc != WG_OK) {
            for (size_t j = 0; j < mounted; j++) wg__unmount(n->children[j]);
            wg__idx_remove(app, n);
            n->app = NULL;
            return rc;
        }
        mounted++;
    }
    return WG_OK;
}

static void wg__unmount(wg_node_t *n) {
    if (!n->app) return;
    for (size_t i = 0; i < n->children_n; i++) wg__unmount(n->children[i]);
    wg__idx_remove(n->app, n);
    n->app = NULL;
}

/* ========================================================================
 * Attrs
 * ======================================================================== */

static wg__attr_t *wg__attr_find(const wg_node_t *n, const char *key) {
    for (size_t i = 0; i < n->attrs_n; i++) {
        if (strcmp(n->attrs[i].key, key) == 0) return (wg__attr_t*)&n->attrs[i];
    }
    return NULL;
}

static int wg__attr_set(wg_node_t *n, const char *key, const char *value) {
    if (!key || !*key) return WG_EINVAL;
    wg__attr_t *a = wg__attr_find(n, key);
    wg_arena_t *arena = wg__current_app ? &wg__current_app->arena : NULL;
    if (a) {
        if (arena) {
            char *v = wg__arena_strdup(arena, value);
            if (!v) return WG_ENOMEM;
            a->value = v;
        } else {
            char *v = wg__strdup(value);
            if (!v) return WG_ENOMEM;
            free(a->value); a->value = v;
        }
        return WG_OK;
    }
    if (n->attrs_n + 1 > n->attrs_cap) {
        size_t cap = n->attrs_cap ? n->attrs_cap * 2 : 4;
        if (arena) {
            wg__attr_t *p = (wg__attr_t*)wg__arena_alloc(arena, cap * sizeof(*p));
            if (!p) return WG_ENOMEM;
            if (n->attrs) memcpy(p, n->attrs, n->attrs_n * sizeof(*p));
            n->attrs = p;
        } else {
            wg__attr_t *p = (wg__attr_t*)realloc(n->attrs, cap * sizeof(*p));
            if (!p) return WG_ENOMEM;
            n->attrs = p;
        }
        n->attrs_cap = cap;
    }
    char *k = arena ? wg__arena_strdup(arena, key) : wg__strdup(key);
    char *v = arena ? wg__arena_strdup(arena, value) : wg__strdup(value);
    if (!k || !v) { if (!arena) { free(k); free(v); } return WG_ENOMEM; }
    n->attrs[n->attrs_n].key   = k;
    n->attrs[n->attrs_n].value = v;
    n->attrs_n++;
    return WG_OK;
}

const char *wg_get_attr(const wg_node_t *n, const char *key) {
    if (!n || !key) return NULL;
    const wg__attr_t *a = wg__attr_find(n, key);
    return a ? a->value : NULL;
}

/* ========================================================================
 * wg_free — three entry paths:
 *   (1) mounted with parent: act like wg_remove then free.
 *   (2) has parent but unmounted: detach silently.
 *   (3) mounted without parent (root via wg_app_destroy): unmount silent.
 * ======================================================================== */

void wg_free(wg_node_t *n) {
    if (!n) return;

    /* Arena-allocated nodes: only fire on_destroy callbacks.
     * Memory is reclaimed on wg_app_destroy via wg__arena_free_all. */
    if (n->_arena) {
        /* Call on_destroy for all children first (depth-first), then self. */
        for (size_t i = 0; i < n->children_n; i++) {
            n->children[i]->parent = NULL;
            wg_free(n->children[i]);
        }
        if (n->kind < WG__K_CONTAINER) {
            int idx = WG__K_CONTAINER - 1 - n->kind;
            if (idx >= 0 && idx < (int)wg__comp_types_n && wg__comp_types[idx].on_destroy) {
                wg__comp_types[idx].on_destroy(n);
            }
        }
        if (n->on_destroy_fn) {
            n->on_destroy_fn(n, n->on_destroy_ud);
        }
        return;
    }

    /* Non-arena (calloc'd) node: full teardown. */
    if (n->parent && n->app) {
        wg_remove(n->parent, n);
    } else if (n->parent) {
        wg_node_t *p = n->parent;
        for (size_t i = 0; i < p->children_n; i++) {
            if (p->children[i] == n) {
                memmove(&p->children[i], &p->children[i+1],
                        (p->children_n - i - 1) * sizeof(p->children[0]));
                p->children_n--;
                break;
            }
        }
        n->parent = NULL;
    } else if (n->app) {
        wg__unmount(n);
    }
    for (size_t i = 0; i < n->children_n; i++) {
        n->children[i]->parent = NULL;
        wg_free(n->children[i]);
    }
    free(n->children);
    for (size_t i = 0; i < n->attrs_n; i++) {
        free(n->attrs[i].key); free(n->attrs[i].value);
    }
    free(n->attrs);
    for (size_t i = 0; i < n->options_n; i++) free(n->options[i]);
    free(n->options);
    if (n->kind < WG__K_CONTAINER) {
        int idx = WG__K_CONTAINER - 1 - n->kind;
        if (idx >= 0 && idx < (int)wg__comp_types_n && wg__comp_types[idx].on_destroy) {
            wg__comp_types[idx].on_destroy(n);
        }
    }
    if (n->on_destroy_fn) {
        n->on_destroy_fn(n, n->on_destroy_ud);
    }
    free(n->id);
    free(n->text);
    free(n->value);
    free(n);
}

int wg_on_destroy(wg_node_t *n, wg_lifecycle_fn fn, void *ud) {
    if (!n || !fn) return WG_EINVAL;
    n->on_destroy_fn = fn;
    n->on_destroy_ud = ud;
    return WG_OK;
}

/* ========================================================================
 * JSON serialization
 * ======================================================================== */

static cJSON *wg__node_json(const wg_node_t *n) {
    cJSON *o = cJSON_CreateObject();
    if (!o) return NULL;
    if (wg__obj_set_str(o, "id",   n->id) != true) goto fail;
    if (wg__obj_set_str(o, "kind", wg__kind_name(n->kind)) != true) goto fail;
    if (n->kind == WG__K_HEADING &&
        wg__obj_set_int(o, "level", n->level) != true) goto fail;
    /* text / value are always-present (even if NULL) so the JS side
     * doesn't get undefined when it checks node.text / node.value. */
    if (wg__obj_take(o, "text",  cJSON_CreateString(s(n->text  ? n->text  : ""))) != true) goto fail;
    if (wg__obj_take(o, "value", cJSON_CreateString(s(n->value ? n->value : ""))) != true) goto fail;
    if (n->disabled && wg__obj_set_bool(o, "disabled", 1) != true) goto fail;
    if (!n->visible && wg__obj_set_bool(o, "visible",  0) != true) goto fail;
    if (n->options_n > 0) {
        cJSON *arr = cJSON_CreateArray();
        if (!arr) goto fail;
        for (size_t i = 0; i < n->options_n; i++) {
            if (wg__arr_take(arr, cJSON_CreateString(s(n->options[i] ? n->options[i] : ""))) != true) {
                cJSON_Delete(arr); goto fail;
            }
        }
        if (wg__obj_take(o, "options", arr) != true) goto fail;
    }
    if (n->attrs_n > 0) {
        cJSON *a = cJSON_CreateObject();
        if (!a) goto fail;
        for (size_t i = 0; i < n->attrs_n; i++) {
            if (wg__obj_set_str(a, n->attrs[i].key, n->attrs[i].value) != true) {
                cJSON_Delete(a); goto fail;
            }
        }
        if (wg__obj_take(o, "attrs", a) != true) goto fail;
    }
    if (n->children_n > 0) {
        cJSON *ch = cJSON_CreateArray();
        if (!ch) goto fail;
        for (size_t i = 0; i < n->children_n; i++) {
            cJSON *c = wg__node_json(n->children[i]);
            if (wg__arr_take(ch, c) != true) { cJSON_Delete(ch); goto fail; }
        }
        if (wg__obj_take(o, "children", ch) != true) goto fail;
    }
    return o;
fail:
    cJSON_Delete(o);
    return NULL;
}

static cJSON *wg__render(wg_app_t *app) {
    cJSON *o = cJSON_CreateObject();
    if (!o) return NULL;
    if (wg__obj_set_str(o, "type", "render") != true) goto fail;
    if (app->title    && wg__obj_set_str(o, "title", app->title)    != true) goto fail;
    if (app->icon_url && wg__obj_set_str(o, "icon",  app->icon_url) != true) goto fail;
    if (app->root) {
        cJSON *t = wg__node_json(app->root);
        if (wg__obj_take(o, "tree", t) != true) goto fail;
    }
    return o;
fail:
    cJSON_Delete(o);
    return NULL;
}

/* ========================================================================
 * Tree ops
 * ======================================================================== */

static int wg__children_reserve(wg_node_t *p, size_t add) {
    if (p->children_n + add <= p->children_cap) return WG_OK;
    size_t cap = p->children_cap ? p->children_cap * 2 : 4;
    while (cap < p->children_n + add) cap *= 2;
    wg_arena_t *arena = p->app ? &p->app->arena : NULL;
    wg_node_t **a;
    if (arena) {
        a = (wg_node_t**)wg__arena_alloc(arena, cap * sizeof(*a));
        if (!a) return WG_ENOMEM;
        if (p->children) memcpy(a, p->children, p->children_n * sizeof(*a));
    } else {
        a = (wg_node_t**)realloc(p->children, cap * sizeof(*a));
        if (!a) return WG_ENOMEM;
    }
    p->children = a;
    p->children_cap = cap;
    return WG_OK;
}

static void wg__send_subtree_patch(wg_app_t *app, const char *type,
                                   const char *parent_id, int index,
                                   const wg_node_t *node) {
    (void)type; (void)parent_id; (void)index; (void)node;
    /* vDOM: diff will handle this on next render */
    wg__mark_dirty(app);
}

int wg_insert(wg_node_t *parent, wg_node_t *child, int index) {
    if (!parent || !child) return WG_EINVAL;
    if (child == parent || child->parent) return WG_EINVAL;
    for (wg_node_t *p = parent; p; p = p->parent)
        if (p == child) return WG_EINVAL;

    int rc = wg__children_reserve(parent, 1);
    if (rc != WG_OK) return rc;
    if (parent->app) {
        rc = wg__mount(parent->app, child);
        if (rc != WG_OK) return rc;
    }
    int n = (int)parent->children_n;
    if (index < 0 || index > n) index = n;
    memmove(&parent->children[index + 1], &parent->children[index],
            (n - index) * sizeof(parent->children[0]));
    parent->children[index] = child;
    parent->children_n++;
    child->parent = parent;
    if (parent->app)
        wg__send_subtree_patch(parent->app, "insert", parent->id, index, child);
    return WG_OK;
}

int wg_append(wg_node_t *parent, wg_node_t *child) {
    return wg_insert(parent, child, -1);
}

int wg_append_n(wg_node_t *parent, wg_node_t *const *children, size_t n) {
    if (!parent || (!children && n)) return WG_EINVAL;
    for (size_t i = 0; i < n; i++) {
        int rc = wg_append(parent, children[i]);
        if (rc != WG_OK) return rc;
    }
    return WG_OK;
}

int wg_remove(wg_node_t *parent, wg_node_t *child) {
    if (!parent || !child || child->parent != parent) return WG_EINVAL;
    size_t idx = (size_t)-1;
    for (size_t i = 0; i < parent->children_n; i++)
        if (parent->children[i] == child) { idx = i; break; }
    if (idx == (size_t)-1) return WG_ENOENT;
    if (parent->app) {
        wg__unmount(child);
        wg__mark_dirty(parent->app);
    }
    memmove(&parent->children[idx], &parent->children[idx + 1],
            (parent->children_n - idx - 1) * sizeof(parent->children[0]));
    parent->children_n--;
    child->parent = NULL;
    return WG_OK;
}

void wg_remove_self(wg_node_t *n) {
    if (!n) return;
    wg_node_t *p = n->parent;
    if (p) {
        wg_remove(p, n);
    } else if (n->app) {
        /* Root node: unmount and leave orphaned; caller should not use it. */
        wg__unmount(n);
    }
}

int wg_clear_children(wg_node_t *parent) {
    if (!parent) return WG_EINVAL;
    while (parent->children_n > 0) {
        wg_remove(parent, parent->children[0]);
    }
    return WG_OK;
}

typedef struct { const char *id; int idx; int seen; } wg__scmap_t;
typedef struct { int inserted; int index; } wg__curinfo_t;

/* ========================================================================
 * vDOM — dirty marking + diff + incremental render
 * ======================================================================== */

static void wg__mark_dirty(wg_app_t *app) {
    if (!app) return;
    for (wg_client_t *c = app->clients; c; c = c->next) {
        if (c->state == WG__CS_OPEN) c->dirty = 1;
    }
}

/* Diff current node against sent JSON snapshot, generate patch ops */
static void wg__diff_node(const wg_node_t *n, cJSON *sent, cJSON *ops) {
    if (!n || !sent) return;

    /* text */
    cJSON *stj = cJSON_GetObjectItem(sent, s("text"));
    const char *st = (stj && cJSON_IsString(stj)) ? stj->valuestring : NULL;
    const char *nt = n->text ? n->text : "";
    if (!st || strcmp(st, nt) != 0) {
        cJSON *op = cJSON_CreateObject();
        wg__obj_set_str(op, "op", "update");
        wg__obj_set_str(op, "id", n->id);
        wg__obj_set_str(op, "prop", "text");
        wg__obj_set_str(op, "value", nt);
        wg__arr_take(ops, op);
    }

    /* value */
    cJSON *svj = cJSON_GetObjectItem(sent, s("value"));
    const char *sv = (svj && cJSON_IsString(svj)) ? svj->valuestring : NULL;
    const char *nv = n->value ? n->value : "";
    if (!sv || strcmp(sv, nv) != 0) {
        cJSON *op = cJSON_CreateObject();
        wg__obj_set_str(op, "op", "update");
        wg__obj_set_str(op, "id", n->id);
        wg__obj_set_str(op, "prop", "value");
        wg__obj_set_str(op, "value", nv);
        wg__arr_take(ops, op);
    }

    /* disabled */
    cJSON *sdj = cJSON_GetObjectItem(sent, s("disabled"));
    int sd = (sdj && cJSON_IsBool(sdj)) ? cJSON_IsTrue(sdj) : 0;
    if (sd != n->disabled) {
        cJSON *op = cJSON_CreateObject();
        wg__obj_set_str(op, "op", "update");
        wg__obj_set_str(op, "id", n->id);
        wg__obj_set_str(op, "prop", "disabled");
        cJSON_AddItemToObject(op, s("value"), cJSON_CreateBool(n->disabled ? true : false));
        wg__arr_take(ops, op);
    }

    /* visible */
    cJSON *svij = cJSON_GetObjectItem(sent, s("visible"));
    int svi = (svij && cJSON_IsBool(svij)) ? cJSON_IsTrue(svij) : 1;
    if (svi != n->visible) {
        cJSON *op = cJSON_CreateObject();
        wg__obj_set_str(op, "op", "update");
        wg__obj_set_str(op, "id", n->id);
        wg__obj_set_str(op, "prop", "visible");
        cJSON_AddItemToObject(op, s("value"), cJSON_CreateBool(n->visible ? true : false));
        wg__arr_take(ops, op);
    }

    /* level (heading only) */
    if (n->kind == WG__K_HEADING) {
        cJSON *slj = cJSON_GetObjectItem(sent, s("level"));
        int sl = (slj && cJSON_IsNumber(slj)) ? (int)slj->valuedouble : 0;
        if (sl != n->level) {
            cJSON *op = cJSON_CreateObject();
            wg__obj_set_str(op, "op", "update");
            wg__obj_set_str(op, "id", n->id);
            wg__obj_set_str(op, "prop", "level");
            wg__obj_set_int(op, "value", n->level);
            wg__arr_take(ops, op);
        }
    }

    /* options (select only) */
    if (n->kind == WG__K_SELECT) {
        cJSON *so = cJSON_GetObjectItem(sent, s("options"));
        int so_n = so ? cJSON_GetArraySize(so) : 0;
        if ((int)n->options_n != so_n) {
            /* options count changed → send full options array */
            cJSON *op = cJSON_CreateObject();
            wg__obj_set_str(op, "op", "options");
            wg__obj_set_str(op, "id", n->id);
            cJSON *arr = cJSON_CreateArray();
            for (size_t i = 0; i < n->options_n; i++) {
                wg__arr_take(arr, cJSON_CreateString(s(n->options[i] ? n->options[i] : "")));
            }
            wg__obj_take(op, "options", arr);
            wg__arr_take(ops, op);
        } else {
            /* same count → compare each option */
            for (size_t i = 0; i < n->options_n; i++) {
                cJSON *soi = cJSON_GetArrayItem(so, (int)i);
                const char *sov = (soi && cJSON_IsString(soi)) ? soi->valuestring : NULL;
                const char *nov = n->options[i] ? n->options[i] : "";
                if (!sov || strcmp(sov, nov) != 0) {
                    /* Send full options array on any change for simplicity */
                    cJSON *op = cJSON_CreateObject();
                    wg__obj_set_str(op, "op", "options");
                    wg__obj_set_str(op, "id", n->id);
                    cJSON *arr = cJSON_CreateArray();
                    for (size_t j = 0; j < n->options_n; j++) {
                        wg__arr_take(arr, cJSON_CreateString(s(n->options[j] ? n->options[j] : "")));
                    }
                    wg__obj_take(op, "options", arr);
                    wg__arr_take(ops, op);
                    break;
                }
            }
        }
    }

    /* attrs */
    if (n->attrs_n > 0) {
        cJSON *sa = cJSON_GetObjectItem(sent, s("attrs"));
        for (size_t i = 0; i < n->attrs_n; i++) {
            const char *av = NULL;
            if (sa && cJSON_IsObject(sa)) {
                cJSON *avj = cJSON_GetObjectItem(sa, s(n->attrs[i].key));
                if (avj && cJSON_IsString(avj)) av = avj->valuestring;
            }
            if (!av || strcmp(av, n->attrs[i].value) != 0) {
                cJSON *op = cJSON_CreateObject();
                wg__obj_set_str(op, "op", "attr");
                wg__obj_set_str(op, "id", n->id);
                wg__obj_set_str(op, "key", n->attrs[i].key);
                wg__obj_set_str(op, "value", n->attrs[i].value);
                wg__arr_take(ops, op);
            }
        }
    }

    /* children diff: emit insert/remove for individual changes.
     * Compare children by ID to detect insertions, deletions, and reorders. */
    cJSON *sc = cJSON_GetObjectItem(sent, s("children"));
    int sc_n = sc ? cJSON_GetArraySize(sc) : 0;

    if (n->children_n == 0 && sc_n == 0) {
        /* both empty → nothing to diff */
    } else if (sc_n == 0) {
        /* new children, none previously sent → insert all */
        for (size_t i = 0; i < n->children_n; i++) {
            cJSON *op = cJSON_CreateObject();
            wg__obj_set_str(op, "op", "insert");
            wg__obj_set_str(op, "id", n->id);
            wg__obj_set_int(op, "index", (int)i);
            cJSON *cj = wg__node_json(n->children[i]);
            wg__obj_take(op, "node", cj);
            wg__arr_take(ops, op);
        }
    } else if (n->children_n == 0) {
        /* all children removed → send remove for each */
        for (int i = 0; i < sc_n; i++) {
            cJSON *sci = cJSON_GetArrayItem(sc, i);
            if (!sci || !cJSON_IsObject(sci)) continue;
            cJSON *sid = cJSON_GetObjectItem(sci, s("id"));
            if (!sid || !cJSON_IsString(sid)) continue;
            cJSON *op = cJSON_CreateObject();
            wg__obj_set_str(op, "op", "remove");
            wg__obj_set_str(op, "id", sid->valuestring);
            wg__arr_take(ops, op);
        }
    } else {
        /* General case: find insertions and deletions by ID comparison. */
        /* Build a set of sent child IDs. */
        wg__scmap_t *sent_map = (wg__scmap_t*)malloc((size_t)sc_n * sizeof(*sent_map));
        if (!sent_map) goto __children_fallback;
        for (int i = 0; i < sc_n; i++) {
            cJSON *sci = cJSON_GetArrayItem(sc, i);
            cJSON *sid = sci ? cJSON_GetObjectItem(sci, s("id")) : NULL;
            sent_map[i].id = (sid && cJSON_IsString(sid)) ? sid->valuestring : NULL;
            sent_map[i].idx = i;
            sent_map[i].seen = 0;
        }

        /* For each current child, find matching sent child by ID. */
        wg__curinfo_t *cur_info = (wg__curinfo_t*)malloc(n->children_n * sizeof(*cur_info));
        if (!cur_info) { free(sent_map); goto __children_fallback; }
        for (size_t i = 0; i < n->children_n; i++) {
            cur_info[i].inserted = 1;
            cur_info[i].index = -1;
            for (int j = 0; j < sc_n; j++) {
                if (sent_map[j].seen) continue;
                if (sent_map[j].id && strcmp(sent_map[j].id, n->children[i]->id) == 0) {
                    cur_info[i].inserted = 0;
                    cur_info[i].index = j;
                    sent_map[j].seen = 1;
                    break;
                }
            }
        }

        /* Emit remove ops for deleted children (in reverse order to preserve indices). */
        for (int j = sc_n - 1; j >= 0; j--) {
            if (!sent_map[j].seen) {
                cJSON *sci = cJSON_GetArrayItem(sc, j);
                cJSON *sid = sci ? cJSON_GetObjectItem(sci, s("id")) : NULL;
                if (sid && cJSON_IsString(sid)) {
                    cJSON *op = cJSON_CreateObject();
                    wg__obj_set_str(op, "op", "remove");
                    wg__obj_set_str(op, "id", sid->valuestring);
                    wg__arr_take(ops, op);
                }
            }
        }

        /* Emit insert ops for new children. */
        for (size_t i = 0; i < n->children_n; i++) {
            if (cur_info[i].inserted) {
                cJSON *op = cJSON_CreateObject();
                wg__obj_set_str(op, "op", "insert");
                wg__obj_set_str(op, "id", n->id);
                wg__obj_set_int(op, "index", (int)i);
                cJSON *cj = wg__node_json(n->children[i]);
                wg__obj_take(op, "node", cj);
                wg__arr_take(ops, op);
            }
        }

        /* Recursive diff for children that exist in both. */
        for (size_t i = 0; i < n->children_n; i++) {
            if (!cur_info[i].inserted && cur_info[i].index >= 0) {
                cJSON *sci = cJSON_GetArrayItem(sc, cur_info[i].index);
                wg__diff_node(n->children[i], sci, ops);
            }
        }

        free(sent_map);
        free(cur_info);
    }
    return;

__children_fallback: {
    /* Fallback: send full children array on alloc failure. */
    cJSON *op = cJSON_CreateObject();
    wg__obj_set_str(op, "op", "children");
    wg__obj_set_str(op, "id", n->id);
    cJSON *arr = cJSON_CreateArray();
    for (size_t i = 0; i < n->children_n; i++) {
        cJSON *cj = wg__node_json(n->children[i]);
        wg__arr_take(arr, cj);
    }
    wg__obj_take(op, "children", arr);
    wg__arr_take(ops, op);
    }
}

int wg_render(wg_app_t *app) {
    if (!app || !app->root) return WG_OK;

    for (wg_client_t *c = app->clients; c; c = c->next) {
        if (c->state != WG__CS_OPEN || !c->dirty) continue;
        if (!c->sent_tree) {
            /* sent_tree not yet initialized — skip.
             * wg__send_init should have set this on connect. */
            c->dirty = 0;
            continue;
        }

        cJSON *ops = cJSON_CreateArray();
        if (!ops) continue;
        wg__diff_node(app->root, c->sent_tree, ops);

        int has_ops = cJSON_GetArraySize(ops) > 0;

        /* Check meta (title/icon) change against what was last sent.
         * We track these in a tiny per-client snapshot to avoid serializing
         * the whole tree just to compare two strings. */
        int title_changed = 0;
        int icon_changed = 0;
        const char *st = c->sent_meta_title;
        const char *si = c->sent_meta_icon;
        if (!st || !app->title || strcmp(st, app->title) != 0) title_changed = 1;
        if (!si || !app->icon_url || strcmp(si, app->icon_url) != 0) icon_changed = 1;

        int meta_changed = title_changed || icon_changed;
        if (!has_ops && !meta_changed) {
            /* No changes at all — skip serialization entirely. */
            cJSON_Delete(ops);
            c->dirty = 0;
            continue;
        }

        /* Tree changed: send patch ops. */
        if (has_ops) {
            cJSON *msg = cJSON_CreateObject();
            if (msg) {
                wg__obj_set_str(msg, "type", "patch");
                wg__obj_set_int(msg, "rev", ++c->sent_rev);
                wg__obj_take(msg, "ops", ops);  /* transfers ownership */
                char *json = cJSON_PrintUnformatted(msg);
                if (json) {
                    wg__ws_send_text(c, json, strlen(json));
                    cJSON_free(json);
                }
                cJSON_Delete(msg);
            }
        } else {
            cJSON_Delete(ops);
        }

        /* Meta changed: send meta message (title/icon only, no tree). */
        if (meta_changed) {
            cJSON *meta = cJSON_CreateObject();
            if (meta) {
                wg__obj_set_str(meta, "type", "meta");
                if (app->title)    wg__obj_set_str(meta, "title", app->title);
                if (app->icon_url) wg__obj_set_str(meta, "icon",  app->icon_url);
                char *json = cJSON_PrintUnformatted(meta);
                if (json) {
                    wg__ws_send_text(c, json, strlen(json));
                    cJSON_free(json);
                }
                cJSON_Delete(meta);
            }
        }

        /* Update sent_tree snapshot. */
        cJSON_Delete(c->sent_tree);
        c->sent_tree = wg__node_json(app->root);

        /* Update meta snapshot. */
        free(c->sent_meta_title);
        c->sent_meta_title = wg__strdup(app->title);
        free(c->sent_meta_icon);
        c->sent_meta_icon = wg__strdup(app->icon_url);
        c->dirty = 0;
    }
    return WG_OK;
}

/* ========================================================================
 * Patch helpers (vDOM: just mark dirty, render handles the push)
 * ======================================================================== */

static void wg__patch_str(wg_node_t *n, const char *type, const char *value) {
    if (!n->app) return;
    (void)type; (void)value;  /* vDOM diff reads from node state */
    wg__mark_dirty(n->app);
}

static void wg__patch_bool(wg_node_t *n, const char *type, int value) {
    if (!n->app) return;
    (void)type; (void)value;
    wg__mark_dirty(n->app);
}

/* ========================================================================
 * Setters / getters
 * ======================================================================== */

int wg_set_text(wg_node_t *n, const char *text) {
    if (!n) return WG_EINVAL;
    int rc = wg__str_set_arena(&n->text, text);
    if (rc == WG_OK) wg__patch_str(n, "set_text", n->text);
    return rc;
}
int wg_set_value(wg_node_t *n, const char *value) {
    if (!n) return WG_EINVAL;
    int rc = wg__str_set_arena(&n->value, value);
    if (rc == WG_OK) wg__patch_str(n, "set_value", n->value);
    return rc;
}
int wg_set_disabled(wg_node_t *n, int disabled) {
    if (!n) return WG_EINVAL;
    n->disabled = disabled ? 1 : 0;
    wg__patch_bool(n, "set_disabled", n->disabled);
    return WG_OK;
}
int wg_set_visible(wg_node_t *n, int visible) {
    if (!n) return WG_EINVAL;
    n->visible = visible ? 1 : 0;
    wg__patch_bool(n, "set_visible", n->visible);
    return WG_OK;
}
int wg_set_attr(wg_node_t *n, const char *key, const char *value) {
    if (!n || !key) return WG_EINVAL;
    int rc = wg__attr_set(n, key, value ? value : "");
    if (rc == WG_OK) wg__mark_dirty(n->app);
    return rc;
}

const char *wg_get_text    (const wg_node_t *n) { return (n && n->text)  ? n->text  : ""; }
const char *wg_get_value   (const wg_node_t *n) { return (n && n->value) ? n->value : ""; }
const char *wg_get_id      (const wg_node_t *n) { return (n && n->id)    ? n->id    : ""; }
int         wg_get_disabled(const wg_node_t *n) { return n ? n->disabled : 0; }
int         wg_get_visible (const wg_node_t *n) { return n ? n->visible  : 0; }

/* ========================================================================
 * Per-client buffer helpers
 *
 * One reserve helper for rx / tx / msg.  All three share the same growth
 * policy and a hard cap of WG__MAX_MSG_SIZE — a client that's pathologically
 * behind on its socket would otherwise let us grow tx without bound.
 * ======================================================================== */

static int wg__buf_reserve(uint8_t **buf, size_t *cap, size_t cur, size_t need) {
    if (cur + need <= *cap) return 0;
    size_t c = *cap ? *cap : 1024;
    while (c < cur + need) {
        if (c > WG__MAX_MSG_SIZE) return -1;
        c *= 2;
    }
    uint8_t *p = (uint8_t*)realloc(*buf, c);
    if (!p) return -1;
    *buf = p; *cap = c;
    return 0;
}

/* Append raw bytes to tx, advancing tx_len.  Returns 0 / -1 (OOM or
 * client too far behind — caller should mark want_close). */
static int wg__tx_append(wg_client_t *c, const void *data, size_t len) {
    if (wg__buf_reserve(&c->tx, &c->tx_cap, c->tx_len, len) < 0) return -1;
    memcpy(c->tx + c->tx_len, data, len);
    c->tx_len += len;
    return 0;
}

/* ========================================================================
 * WebSocket frame builder
 *
 * Server-to-client frames are unmasked.  We always send single-frame
 * text messages (FIN=1, opcode=0x1).  Header overhead: 2 bytes for
 * payloads <126, 4 for <=65535, 10 for larger.
 * ======================================================================== */

static int wg__frame_text(wg_client_t *c, const char *data, size_t len) {
    uint8_t hdr[10]; size_t h = 0;
    hdr[h++] = 0x81;                              /* FIN | text */
    if (len < 126) {
        hdr[h++] = (uint8_t)len;
    } else if (len <= 0xFFFF) {
        hdr[h++] = 126;
        hdr[h++] = (uint8_t)(len >> 8);
        hdr[h++] = (uint8_t)(len     );
    } else {
        hdr[h++] = 127;
        uint64_t v = len;
        for (int i = 7; i >= 0; i--) hdr[h++] = (uint8_t)(v >> (i*8));
    }
    if (wg__tx_append(c, hdr, h) < 0) return -1;
    if (len > 0 && wg__tx_append(c, data, len) < 0) return -1;
    return 0;
}

static void wg__ws_send_text(wg_client_t *c, const char *data, size_t len) {
    if (!c || c->state != WG__CS_OPEN) return;
    if (wg__frame_text(c, data, len) < 0) c->want_close = 1; /* OOM */
}

static void wg__send_obj(wg_client_t *c, cJSON *obj) {
    if (!c || !obj) return;
    char *s = cJSON_PrintUnformatted(obj);
    if (!s) return;
    wg__ws_send_text(c, s, strlen(s));
    cJSON_free(s);
}

static void wg__broadcast(wg_app_t *app, cJSON *obj) {
    if (!app || !obj) return;
    char *s = cJSON_PrintUnformatted(obj);
    if (!s) return;
    size_t len = strlen(s);
    for (wg_client_t *c = app->clients; c; c = c->next) {
        if (c->state == WG__CS_OPEN) wg__ws_send_text(c, s, len);
    }
    cJSON_free(s);
}

static void wg__send_init(wg_client_t *c) {
    if (!c || !c->app || !c->app->root) return;
    cJSON *tree = wg__node_json(c->app->root);
    if (!tree) return;
    cJSON *obj = cJSON_CreateObject();
    if (!obj) { cJSON_Delete(tree); return; }
    wg__obj_set_str(obj, "type", "render");
    if (c->app->title) wg__obj_set_str(obj, "title", c->app->title);
    if (c->app->icon_url) wg__obj_set_str(obj, "icon", c->app->icon_url);
    cJSON_AddItemToObject(obj, s("tree"), cJSON_Duplicate(tree, 1));
    wg__send_obj(c, obj);
    cJSON_Delete(obj);

    /* initialize vDOM snapshot — reuse the tree we already serialized. */
    cJSON_Delete(c->sent_tree);
    c->sent_tree = tree;
    c->dirty = 0;
    free(c->sent_meta_title);
    c->sent_meta_title = wg__strdup(c->app->title);
    free(c->sent_meta_icon);
    c->sent_meta_icon = wg__strdup(c->app->icon_url);
}

/* ========================================================================
 * WebSocket frame parser
 *
 * Reads from c->rx and emits complete messages by calling out into the
 * dispatch path.  Returns:
 *   0  — fed some bytes but need more for the next frame
 *   1  — a complete frame was consumed (caller should call again)
 *  -1  — protocol error or OOM, client should be closed
 *
 * On success, consumed bytes are removed from the head of c->rx.
 * ======================================================================== */

static void wg__handle_message(wg_client_t *c, int opcode,
                               const uint8_t *payload, size_t plen);

static int wg__try_parse_frame(wg_client_t *c) {
    if (c->rx_len < 2) return 0;
    uint8_t b0 = c->rx[0], b1 = c->rx[1];
    int fin    = (b0 & 0x80) != 0;
    int opcode = b0 & 0x0F;
    int masked = (b1 & 0x80) != 0;
    uint64_t plen = b1 & 0x7F;
    size_t hdr = 2;

    if (plen == 126) {
        if (c->rx_len < hdr + 2) return 0;
        plen = ((uint64_t)c->rx[hdr] << 8) | c->rx[hdr+1];
        hdr += 2;
    } else if (plen == 127) {
        if (c->rx_len < hdr + 8) return 0;
        plen = 0;
        for (int i = 0; i < 8; i++) plen = (plen << 8) | c->rx[hdr + i];
        hdr += 8;
    }
    /* Client-to-server frames MUST be masked. */
    if (!masked) return -1;
    if (plen > WG__MAX_MSG_SIZE) return -1;
    if (c->rx_len < hdr + 4) return 0;
    uint8_t mask[4] = { c->rx[hdr], c->rx[hdr+1], c->rx[hdr+2], c->rx[hdr+3] };
    hdr += 4;
    if (c->rx_len < hdr + plen) return 0;

    uint8_t *payload = c->rx + hdr;
    for (size_t i = 0; i < plen; i++) payload[i] ^= mask[i & 3];

    /* Control frames: opcode >= 8 — these must be fin=1 and <= 125 bytes. */
    if (opcode & 0x08) {
        if (!fin || plen > 125) return -1;
        switch (opcode) {
            case 0x8: {  /* close — echo back, schedule socket close */
                uint8_t f[4] = { 0x88, 0x02, 0x03, 0xE8 };   /* code 1000 */
                wg__tx_append(c, f, 4);
                c->want_close = 1;
                break;
            }
            case 0x9: {  /* ping → pong */
                uint8_t hdr2[2] = { 0x8A, (uint8_t)plen };
                if (wg__tx_append(c, hdr2, 2) < 0 ||
                    (plen && wg__tx_append(c, payload, plen) < 0))
                    return -1;
                break;
            }
            case 0xA: /* pong */ break;
            default:  return -1;
        }
        size_t consumed = hdr + plen;
        memmove(c->rx, c->rx + consumed, c->rx_len - consumed);
        c->rx_len -= consumed;
        return 1;
    }

    /* Data frames: text/binary/continuation. */
    if (opcode == 0x1 || opcode == 0x2) {
        if (c->msg_in_progress) return -1;     /* must continue, not start */
        c->msg_in_progress = 1;
        c->msg_op = opcode;
        c->msg_len = 0;
    } else if (opcode == 0x0) {
        if (!c->msg_in_progress) return -1;
    } else {
        return -1;
    }

    if (wg__buf_reserve(&c->msg, &c->msg_cap, c->msg_len, (size_t)plen) < 0) return -1;
    memcpy(c->msg + c->msg_len, payload, (size_t)plen);
    c->msg_len += plen;

    size_t consumed = hdr + plen;
    memmove(c->rx, c->rx + consumed, c->rx_len - consumed);
    c->rx_len -= consumed;

    if (fin) {
        int op = c->msg_op;
        c->msg_in_progress = 0;
        wg__handle_message(c, op, c->msg, c->msg_len);
        c->msg_len = 0;
    }
    return 1;
}

/* ========================================================================
 * Callbacks (id × event → fn)
 * ======================================================================== */

static wg__cb_t *wg__cb_find(wg_app_t *app, const char *id, const char *event) {
    for (size_t i = 0; i < app->cbs_n; i++) {
        if (strcmp(app->cbs[i].id, id)    == 0 &&
            strcmp(app->cbs[i].event, event) == 0)
            return &app->cbs[i];
    }
    return NULL;
}

int wg_on(wg_app_t *app, const char *id, const char *event,
          wg_callback_fn cb, void *userdata) {
    if (!app || !id || !event || !cb) return WG_EINVAL;
    wg__cb_t *e = wg__cb_find(app, id, event);
    if (e) { e->cb = cb; e->ud = userdata; return WG_OK; }
    if (app->cbs_n + 1 > app->cbs_cap) {
        size_t cap = app->cbs_cap ? app->cbs_cap * 2 : 8;
        wg__cb_t *p = (wg__cb_t*)realloc(app->cbs, cap * sizeof(*p));
        if (!p) return WG_ENOMEM;
        app->cbs = p; app->cbs_cap = cap;
    }
    char *id_dup = wg__strdup(id);
    char *ev_dup = wg__strdup(event);
    if (!id_dup || !ev_dup) { free(id_dup); free(ev_dup); return WG_ENOMEM; }
    app->cbs[app->cbs_n].id    = id_dup;
    app->cbs[app->cbs_n].event = ev_dup;
    app->cbs[app->cbs_n].cb    = cb;
    app->cbs[app->cbs_n].ud    = userdata;
    app->cbs_n++;
    return WG_OK;
}

int wg_off(wg_app_t *app, const char *id, const char *event) {
    if (!app || !id || !event) return WG_EINVAL;
    for (size_t i = 0; i < app->cbs_n; i++) {
        if (strcmp(app->cbs[i].id, id) == 0 &&
            strcmp(app->cbs[i].event, event) == 0) {
            free(app->cbs[i].id);
            free(app->cbs[i].event);
            app->cbs[i] = app->cbs[--app->cbs_n];
            return WG_OK;
        }
    }
    return WG_ENOENT;
}

int wg_emit(wg_app_t *app, const char *id, const char *event,
            const char *value) {
    if (!app || !id || !event) return WG_EINVAL;
    wg__cb_t *cb = wg__cb_find(app, id, event);
    if (!cb) return WG_ENOENT;

    /* Sync node value if provided (same echo-prevention as real events). */
    if (value) {
        wg_node_t *n = wg__idx_find(app, id);
        if (n) {
            wg_app_t *saved = n->app; n->app = NULL;
            wg__str_set_arena(&n->value, value);
            n->app = saved;
        }
    }

    wg__event_full_t full;
    full.pub.app      = app;
    full.pub.id       = id;
    full.pub.event    = event;
    full.pub.value    = value;
    full.pub.userdata = cb->ud;
    full.form = NULL;  /* no form_data for programmatically emitted events */
    cb->cb(&full.pub);
    return WG_OK;
}

/* ========================================================================
 * Form-data accessors (private internal type carries form payload)
 * ======================================================================== */

typedef struct {
    wg_event_t  pub;
    cJSON        *form;
} wg__event_full_t;

const char *wg_form_get(const wg_event_t *ev, const char *key) {
    if (!ev || !key) return NULL;
    const wg__event_full_t *f = (const wg__event_full_t *)ev;
    if (!f->form) return NULL;
    cJSON *v = cJSON_GetObjectItem(f->form, s(key));
    return (v && cJSON_IsString(v)) ? v->valuestring : NULL;
}

int wg_form_iter(const wg_event_t *ev, size_t *cursor,
                 const char **key, const char **value) {
    if (!ev || !cursor || !key || !value) return 0;
    const wg__event_full_t *f = (const wg__event_full_t *)ev;
    if (!f->form) return 0;
    int n = wg__cjson_obj_count(f->form);
    if (n < 0 || (size_t)n <= *cursor) return 0;
    cJSON *v = wg__cjson_obj_item_at(f->form, (int)*cursor);
    if (!v) return 0;
    *key = v->string;
    *value = (cJSON_IsString(v)) ? v->valuestring : NULL;
    (*cursor)++;
    return 1;
}

/* ========================================================================
 * Event dispatch (on the event-loop thread, no locking needed)
 * ======================================================================== */

static void wg__dispatch_event(wg_app_t *app, cJSON *msg) {
    cJSON *id_n  = cJSON_GetObjectItem(msg, s("id"));
    cJSON *ev_n  = cJSON_GetObjectItem(msg, s("event"));
    cJSON *val_n = cJSON_GetObjectItem(msg, s("value"));
    cJSON *frm_n = cJSON_GetObjectItem(msg, s("form_data"));
    const char *id  = (id_n  && cJSON_IsString(id_n))  ? id_n->valuestring  : NULL;
    const char *ev  = (ev_n  && cJSON_IsString(ev_n))  ? ev_n->valuestring  : NULL;
    const char *val = (val_n && cJSON_IsString(val_n)) ? val_n->valuestring : NULL;
    if (!id || !ev) return;

    /* Echo-prevention: sync the node's local state from incoming value
     * without firing another patch.  Detach the app pointer briefly. */
    if (val) {
        wg_node_t *n = wg__idx_find(app, id);
        if (n) {
            wg_app_t *saved = n->app; n->app = NULL;
            wg__str_set_arena(&n->value, val);
            n->app = saved;
        }
    }
    if (frm_n && cJSON_IsObject(frm_n)) {
        int n = wg__cjson_obj_count(frm_n);
        for (int i = 0; i < n; i++) {
            cJSON *v = wg__cjson_obj_item_at(frm_n, i);
            const char *k = v ? v->string : NULL;
            if (!k) continue;
            const char *vs = (v && cJSON_IsString(v)) ? v->valuestring : NULL;
            if (!vs) continue;
            wg_node_t *cn = wg__idx_find(app, k);
            if (cn) {
                wg_app_t *saved = cn->app; cn->app = NULL;
                wg__str_set_arena(&cn->value, vs);
                cn->app = saved;
            }
        }
    }

    wg__cb_t *cb = wg__cb_find(app, id, ev);
    if (!cb) return;

    wg__event_full_t full;
    full.pub.app      = app;
    full.pub.id       = id;
    full.pub.event    = ev;
    full.pub.value    = val;
    full.pub.userdata = cb->ud;
    full.form = (frm_n && cJSON_IsObject(frm_n)) ? frm_n : NULL;
    cb->cb(&full.pub);
}

/* Called when a complete data frame's message is assembled. */
static void wg__handle_message(wg_client_t *c, int opcode,
                               const uint8_t *payload, size_t plen) {
    if (opcode != 0x1) return;       /* ignore binary */
    Str ps = {0}; ps.ptr = (const char *)payload; ps.len = (isize)plen;
    cJSON *msg = cJSON_ParseWithLength(ps, (isize)plen);
    if (!msg) return;
    cJSON *t = cJSON_GetObjectItem(msg, s("type"));
    const char *ts = (t && cJSON_IsString(t)) ? t->valuestring : NULL;
    if (ts && strcmp(ts, "event") == 0) {
        wg__dispatch_event(c->app, msg);
    }
    cJSON_Delete(msg);
}

/* ========================================================================
 * HTTP handshake
 * ======================================================================== */

/* Parse the HTTP request from c->rx (consumed up to end-of-headers).
 * Returns:
 *   0  — incomplete, need more bytes
 *   1  — parsed; passes back method, path, headers pointer/len
 *  -1  — malformed / over-size
 *
 * The returned pointers are into c->rx and remain valid until c->rx is
 * compacted.  *consumed reports how many bytes from the head of rx
 * comprise the full request (i.e. up to and including \r\n\r\n).
 */
static int wg__http_parse(wg_client_t *c,
                          char **method, char **path,
                          char *ws_key,            /* out, 64 bytes */
                          int *is_ws,
                          size_t *consumed) {
    /* find \r\n\r\n */
    size_t end = 0;
    int found = 0;
    if (c->rx_len > WG__MAX_HTTP_HEADER) return -1;
    for (size_t i = 3; i < c->rx_len; i++) {
        if (c->rx[i-3]=='\r' && c->rx[i-2]=='\n' &&
            c->rx[i-1]=='\r' && c->rx[i]   =='\n') {
            end = i + 1; found = 1; break;
        }
    }
    if (!found) return 0;
    *consumed = end;

    /* Operate on a NUL-terminated copy of the headers. */
    char *hdr = (char*)c->rx;
    char saved = hdr[end];
    hdr[end] = '\0';

    /* Parse request line. */
    char *line = hdr;
    char *eol  = strstr(line, "\r\n");
    if (!eol) { hdr[end] = saved; return -1; }
    *eol = '\0';
    char *sp1 = strchr(line, ' ');
    if (!sp1) { *eol = '\r'; hdr[end] = saved; return -1; }
    *sp1 = '\0';
    char *sp2 = strchr(sp1 + 1, ' ');
    if (!sp2) { *sp1 = ' '; *eol = '\r'; hdr[end] = saved; return -1; }
    *sp2 = '\0';
    *method = line;
    *path   = sp1 + 1;

    *is_ws = 0;
    ws_key[0] = '\0';
    int has_upgrade = 0, has_connection_upgrade = 0;

    char *h = eol + 2;
    while (h < hdr + end - 2) {
        char *next = strstr(h, "\r\n");
        if (!next) break;
        *next = '\0';
        char *colon = strchr(h, ':');
        if (colon) {
            *colon = '\0';
            char *v = colon + 1;
            while (*v == ' ' || *v == '\t') v++;
            if (wg__strieq(h, "Upgrade") && wg__strieq(v, "websocket"))
                has_upgrade = 1;
            else if (wg__strieq(h, "Connection")) {
                /* Connection is a comma-separated list of tokens.  We
                 * want to see "Upgrade" as one of them — regardless of
                 * position or surrounding whitespace. */
                const char *p = v;
                while (*p) {
                    while (*p == ' ' || *p == '\t' || *p == ',') p++;
                    const char *tok = p;
                    while (*p && *p != ',' && *p != ' ' && *p != '\t') p++;
                    if (wg__strnieq_lit(tok, (size_t)(p - tok), "upgrade")) {
                        has_connection_upgrade = 1;
                        break;
                    }
                }
            }
            else if (wg__strieq(h, "Sec-WebSocket-Key")) {
                size_t vl = strlen(v);
                if (vl < 64) { memcpy(ws_key, v, vl); ws_key[vl] = '\0'; }
            }
            *colon = ':';
        }
        h = next + 2;
    }
    *is_ws = (has_upgrade && has_connection_upgrade && ws_key[0]);

    hdr[end] = saved;
    return 1;
}

static int wg__send_http_html(wg_client_t *c, int head_only) {
    const char *body;
    size_t body_len;

    if (c->app->custom_html) {
        body = c->app->custom_html;
        body_len = c->app->custom_html_len;
    } else {
        body = wg__html_page;
        body_len = sizeof(wg__html_page) - 1;
    }

    char hdr[256];
    int n = snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n"
        "\r\n", body_len);
    if (n <= 0) return -1;
    if (wg__tx_append(c, hdr, (size_t)n) < 0) return -1;
    if (!head_only && wg__tx_append(c, (const uint8_t*)body, body_len) < 0) return -1;
    return 0;
}

static int wg__send_http_404(wg_client_t *c) {
    static const char r[] =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 0\r\nConnection: close\r\n\r\n";
    return wg__tx_append(c, r, sizeof(r) - 1);
}

static int wg__handle_handshake(wg_client_t *c) {
    char *method, *path;
    char ws_key[64];
    int is_ws = 0;
    size_t consumed = 0;

    int r = wg__http_parse(c, &method, &path, ws_key, &is_ws, &consumed);
    if (r == 0) return 0;
    if (r < 0) { c->want_close = 1; return -1; }

    /* Snapshot method/path because we're about to compact rx. */
    char method_copy[16] = {0}, path_copy[256] = {0};
    snprintf(method_copy, sizeof(method_copy), "%s", method);
    snprintf(path_copy,   sizeof(path_copy),   "%s", path);

    if (is_ws) {
        char accept_b64[29];
        wg__ws_accept_hash(ws_key, accept_b64);
        char resp[256];
        int n = snprintf(resp, sizeof(resp),
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: %s\r\n\r\n", accept_b64);
        if (n <= 0 || wg__tx_append(c, resp, (size_t)n) < 0) {
            c->want_close = 1; return -1;
        }
        /* Consume the request from rx, transition to OPEN. */
        memmove(c->rx, c->rx + consumed, c->rx_len - consumed);
        c->rx_len -= consumed;
        c->state = WG__CS_OPEN;
        /* Send the initial render and initialize vDOM snapshots. */
        wg__send_init(c);
        return 0;
    }

    /* HTTP fallback — serve the embedded page only for "/" with GET/HEAD. */
    int head_only = (strcmp(method_copy, "HEAD") == 0);
    int is_get    = (strcmp(method_copy, "GET")  == 0);
    int is_root   = (strcmp(path_copy, "/") == 0);
    int rc;
    if ((is_get || head_only) && is_root) rc = wg__send_http_html(c, head_only);
    else                                  rc = wg__send_http_404(c);
    /* Done with rx for this request. */
    memmove(c->rx, c->rx + consumed, c->rx_len - consumed);
    c->rx_len -= consumed;
    /* No state change needed: state stays HANDSHAKE but want_close gates
     * any further reads, and on_writable drains the response and closes. */
    c->want_close = 1;
    return rc;
}

/* ========================================================================
 * Per-client read/write drivers
 * ======================================================================== */

static void wg__client_close(wg_client_t *c) {
    if (c->fd != WG__INVALID_SOCKET) { close((int)c->fd); c->fd = WG__INVALID_SOCKET; }
    c->state = WG__CS_DEAD;
}

static void wg__client_on_readable(wg_client_t *c) {
    if (c->want_close) return;            /* draining tx only */
    /* Read everything available into rx (until EAGAIN). */
    for (;;) {
        if (wg__buf_reserve(&c->rx, &c->rx_cap, c->rx_len, 4096) < 0) {
            c->want_close = 1; break;
        }
        int r = recv(c->fd, (char*)c->rx + c->rx_len, (int)(c->rx_cap - c->rx_len), 0);
        if (r > 0)      { c->rx_len += (size_t)r; continue; }
        if (r == 0)     { c->want_close = 1; break; }
#ifdef _WIN32
        { int e = wg__posix_errno();
          if (e == WG__EINTR) continue;
          if (e == WG__EAGAIN) break; }
#else
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
#endif
        c->want_close = 1; break;
    }
    /* Drive whatever state we're in. */
    if (c->state == WG__CS_HANDSHAKE) {
        if (wg__handle_handshake(c) < 0) c->want_close = 1;
    }
    if (c->state == WG__CS_OPEN && !c->want_close) {
        for (;;) {
            int r = wg__try_parse_frame(c);
            if (r == 0) break;
            if (r < 0) { c->want_close = 1; break; }
        }
    }
}

static void wg__client_on_writable(wg_client_t *c) {
    while (c->tx_len > 0) {
        int w = send(c->fd, (const char*)c->tx, (int)c->tx_len, 0);
        if (w > 0) {
            memmove(c->tx, c->tx + w, c->tx_len - (size_t)w);
            c->tx_len -= (size_t)w;
            continue;
        }
        if (w < 0) {
#ifdef _WIN32
            int e = wg__posix_errno();
            if (e == WG__EINTR) continue;
            if (e == WG__EAGAIN) return;
#else
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
#endif
        }
        c->want_close = 1; return;
    }
    /* Buffer drained. If we wanted to close, do it now. */
    if (c->want_close) wg__client_close(c);
}

/* ========================================================================
 * Accept & listen
 * ======================================================================== */

static int wg__set_nonblocking(int fd) {
    return wg__set_nonblocking_socket((wg__socket_t)fd);
}

static void wg__accept_one(wg_app_t *app) {
    for (;;) {
        struct sockaddr_in peer;
        socklen_t plen = sizeof(peer);
        wg__socket_t s = accept(app->listen_fd, (struct sockaddr*)&peer, &plen);
        int fd = (int)s;
        if (s == WG__INVALID_SOCKET) {
#ifdef _WIN32
            int e = wg__posix_errno();
            if (e == WG__EAGAIN) return;
            if (e == WG__EINTR) continue;
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            if (errno == EINTR) continue;
#endif
            return;
        }
        if (wg__set_nonblocking(fd) < 0) { close(fd); continue; }
        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&one, (int)sizeof(one));

        wg_client_t *c = (wg_client_t*)calloc(1, sizeof(*c));
        if (!c) { close(fd); continue; }
        c->fd  = (wg__socket_t)fd;
        c->app = app;
        c->state = WG__CS_HANDSHAKE;
        inet_ntop(AF_INET, &peer.sin_addr, c->ip, sizeof(c->ip));
        c->next = app->clients;
        app->clients = c;
    }
}

static void wg__reap_dead(wg_app_t *app) {
    wg_client_t **link = &app->clients;
    while (*link) {
        wg_client_t *c = *link;
        if (c->state == WG__CS_DEAD) {
            *link = c->next;
            free(c->rx); free(c->tx); free(c->msg);
            free(c->sent_meta_title); free(c->sent_meta_icon);
            cJSON_Delete(c->sent_tree);
            free(c);
        } else {
            link = &c->next;
        }
    }
}

/* ========================================================================
 * Timers
 * ======================================================================== */

int wg_set_timer(wg_app_t *app, int delay_ms, int repeat,
                 wg_timer_fn fn, void *userdata) {
    if (!app || !fn || delay_ms < 0) return WG_EINVAL;
    if (app->timers_n + 1 > app->timers_cap) {
        size_t cap = app->timers_cap ? app->timers_cap * 2 : 4;
        wg__timer_t *p = (wg__timer_t*)realloc(app->timers, cap * sizeof(*p));
        if (!p) return WG_ENOMEM;
        app->timers = p; app->timers_cap = cap;
    }
    int tid = ++app->next_timer_id;
    wg__timer_t *t = &app->timers[app->timers_n++];
    t->id = tid; t->delay_ms = delay_ms; t->repeat = repeat ? 1 : 0;
    t->fn = fn; t->ud = userdata; t->alive = 1;
    t->next_fire_ms = wg__now_ms() + delay_ms;
    return tid;
}

int wg_clear_timer(wg_app_t *app, int timer_id) {
    if (!app) return WG_EINVAL;
    for (size_t i = 0; i < app->timers_n; i++) {
        if (app->timers[i].id == timer_id && app->timers[i].alive) {
            app->timers[i].alive = 0;
            return WG_OK;
        }
    }
    return WG_ENOENT;
}

static void wg__timers_compact(wg_app_t *app) {
    size_t w = 0;
    for (size_t r = 0; r < app->timers_n; r++) {
        if (app->timers[r].alive) {
            if (w != r) app->timers[w] = app->timers[r];
            w++;
        }
    }
    app->timers_n = w;
}

static int64_t wg__next_timer_due(wg_app_t *app) {
    int64_t best = -1;
    for (size_t i = 0; i < app->timers_n; i++) {
        if (!app->timers[i].alive) continue;
        if (best < 0 || app->timers[i].next_fire_ms < best)
            best = app->timers[i].next_fire_ms;
    }
    return best;
}

static void wg__fire_due_timers(wg_app_t *app) {
    int64_t now = wg__now_ms();
    /* Snapshot ids that are due right now.  Anything created during fn
     * isn't fired this pass even with delay=0. */
    int *due = NULL; size_t n_due = 0;
    for (size_t i = 0; i < app->timers_n; i++) {
        if (!app->timers[i].alive) continue;
        if (app->timers[i].next_fire_ms > now) continue;
        int *p = (int*)realloc(due, (n_due + 1) * sizeof(int));
        if (!p) break;
        due = p; due[n_due++] = app->timers[i].id;
    }
    for (size_t k = 0; k < n_due; k++) {
        size_t idx = (size_t)-1;
        for (size_t i = 0; i < app->timers_n; i++) {
            if (app->timers[i].id == due[k]) { idx = i; break; }
        }
        if (idx == (size_t)-1) continue;
        if (!app->timers[idx].alive) continue;
        wg__timer_t *t = &app->timers[idx];
        wg_timer_fn fn = t->fn;
        void *ud = t->ud;
        if (t->repeat) t->next_fire_ms = now + t->delay_ms;
        else           t->alive = 0;
        fn(app, ud);
    }
    free(due);
    if (n_due > 0) wg__timers_compact(app);
}

/* ========================================================================
 * Cross-thread wg_post
 * ======================================================================== */

int wg_post(wg_app_t *app, wg_post_fn fn, void *userdata) {
    if (!app || !fn) return WG_EINVAL;
    wg__post_t *p = (wg__post_t*)malloc(sizeof(*p));
    if (!p) return WG_ENOMEM;
    p->fn = fn; p->ud = userdata;
    wg__mutex_lock(&app->post_mtx);
    p->next = app->post_head;
    app->post_head = p;
    wg__mutex_unlock(&app->post_mtx);
    /* Wake the loop. */
    wg__wakeup_signal(app->wake_evt);
    return WG_OK;
}

static void wg__drain_posts(wg_app_t *app) {
    /* First, drain the wakeup signal. */
    wg__wakeup_drain(app->wake_evt);
    /* Snatch the head, reverse to FIFO, run. */
    wg__mutex_lock(&app->post_mtx);
    wg__post_t *head = app->post_head;
    app->post_head = NULL;
    wg__mutex_unlock(&app->post_mtx);
    wg__post_t *rev = NULL;
    while (head) {
        wg__post_t *next = head->next;
        head->next = rev;
        rev = head;
        head = next;
    }
    while (rev) {
        wg__post_t *next = rev->next;
        rev->fn(app, rev->ud);
        free(rev);
        rev = next;
    }
}

/* ========================================================================
 * App lifecycle
 * ======================================================================== */

wg_app_t *wg_app_create(wg_node_t *root) {
    if (!root) { wg__set_err(WG_EINVAL); return NULL; }
    if (root->parent || root->app) { wg__set_err(WG_EINVAL); return NULL; }
#ifdef _WIN32
    {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            wg__set_err(WG_EIO); return NULL;
        }
    }
#endif
    wg_app_t *app = (wg_app_t*)calloc(1, sizeof(*app));
    if (!app) { wg__set_err(WG_ENOMEM); return NULL; }
    app->listen_fd = (int)WG__INVALID_SOCKET;
    app->wake_r = app->wake_w = -1;
    app->wake_evt = -1;
    wg__mutex_init(&app->post_mtx);
    /* Cross-thread wakeup */
    {
      int pipes[2];
      if (wg__wakeup_init(pipes) < 0) {
          wg__mutex_destroy(&app->post_mtx);
          free(app); wg__set_err(WG_EIO); return NULL;
      }
      app->wake_r = pipes[0];
      app->wake_w = pipes[1];
      app->wake_evt = pipes[1]; /* for signal/drain */
    }
    app->root = root;
    wg__current_app = app;  /* subsequent node constructors use app->arena */
    int rc = wg__mount(app, root);
    if (rc != WG_OK) {
        free(app->idx_nodes);
#ifdef _WIN32
        wg__wakeup_close(app->wake_evt);
#else
        close(app->wake_r); close(app->wake_w);
#endif
        wg__mutex_destroy(&app->post_mtx);
        free(app);
        wg__set_err(rc);
        return NULL;
    }
    return app;
}

int wg_bind_tcp(wg_app_t *app, const char *host, uint16_t port) {
    if (!app) return WG_EINVAL;
    if (app->listen_fd != (int)WG__INVALID_SOCKET) return WG_EINVAL;     /* already bound */

    wg__socket_t s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == WG__INVALID_SOCKET) return WG_EIO;
    int one = 1;
    setsockopt((int)s, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, (int)sizeof(one));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (host && *host) {
        if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
            close((int)s); return WG_EINVAL;
        }
    } else {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    if (bind((int)s,   (struct sockaddr*)&addr, (int)sizeof(addr)) < 0) { close((int)s); return WG_EIO; }
    if (listen((int)s, WG__BACKLOG)                              < 0) { close((int)s); return WG_EIO; }
    if (wg__set_nonblocking((int)s)                              < 0) { close((int)s); return WG_EIO; }

    app->listen_fd = (int)s;
    return WG_OK;
}

int wg_set_title(wg_app_t *app, const char *title) {
    if (!app) return WG_EINVAL;
    int rc = wg__str_set(&app->title, title ? title : "");
    if (rc != WG_OK) return rc;
    wg__mark_dirty(app);
    return WG_OK;
}

int wg_set_icon(wg_app_t *app, const char *url) {
    if (!app) return WG_EINVAL;
    int rc = wg__str_set(&app->icon_url, url ? url : "");
    if (rc != WG_OK) return rc;
    wg__mark_dirty(app);
    return WG_OK;
}

int wg_set_debug(wg_app_t *app, int enabled) {
    if (!app) return WG_EINVAL;
    app->debug = enabled ? 1 : 0;
    return WG_OK;
}

int wg_set_html(wg_app_t *app, const char *html) {
    if (!app) return WG_EINVAL;
    char *dup = wg__strdup(html);
    if (!dup) return WG_ENOMEM;
    free(app->custom_html);
    app->custom_html = dup;
    app->custom_html_len = strlen(html);
    return WG_OK;
}

int wg_set_html_file(wg_app_t *app, const char *path) {
    if (!app || !path) return WG_EINVAL;
    FILE *f = fopen(path, "r");
    if (!f) return WG_EIO;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return WG_EIO; }
    fseek(f, 0, SEEK_SET);
    char *buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return WG_ENOMEM; }
    size_t r = fread(buf, 1, (size_t)sz, f);
    buf[r] = '\0';
    fclose(f);
    int rc = wg_set_html(app, buf);
    free(buf);
    return rc;
}

/* wg_stop dispatches a "set the stop flag" post.  This keeps stop_requested
 * a purely single-threaded EL field — no volatile, no atomics, no separate
 * wake path. */
static void wg__do_stop(wg_app_t *app, void *ud) { (void)ud; app->stop_requested = 1; }

void wg_stop(wg_app_t *app) {
    if (!app) return;
    wg_post(app, wg__do_stop, NULL);
}

void wg_app_destroy(wg_app_t *app) {
    if (!app) return;
    /* Close listen socket and all clients. */
    if (app->listen_fd != (int)WG__INVALID_SOCKET) {
        close(app->listen_fd); app->listen_fd = (int)WG__INVALID_SOCKET;
    }
    while (app->clients) {
        wg_client_t *c = app->clients;
        app->clients = c->next;
        if (c->fd != WG__INVALID_SOCKET) close((int)c->fd);
        free(c->rx); free(c->tx); free(c->msg);
        free(c->sent_meta_title); free(c->sent_meta_icon);
        cJSON_Delete(c->sent_tree);
        free(c);
    }
    /* Drain any queued posts (they'll never run). */
    wg__mutex_lock(&app->post_mtx);
    wg__post_t *head = app->post_head;
    app->post_head = NULL;
    wg__mutex_unlock(&app->post_mtx);
    while (head) { wg__post_t *n = head->next; free(head); head = n; }

    wg__wakeup_close(app->wake_evt);
    wg__mutex_destroy(&app->post_mtx);

    /* Fire on_destroy for all arena nodes (depth-first via wg_free),
     * then reclaim all arena memory in one shot. */
    if (app->root) wg_free(app->root);

    /* Free arena-managed memory in one shot (nodes, node strings,
     * attrs, children arrays, etc). */
    wg__arena_free_all(&app->arena);

    wg__current_app = NULL;

    /* Free non-arena allocations (callback registry, timers,
     * title/icon/custom_html use regular malloc). */
    for (size_t i = 0; i < app->cbs_n; i++) {
        free(app->cbs[i].id); free(app->cbs[i].event);
    }
    free(app->cbs);
    free(app->timers);
    free(app->title);
    free(app->icon_url);
    free(app->custom_html);
    free(app->idx_nodes);
    free(app);
}

/* ========================================================================
 * Main event loop — one poll() over everything
 * ======================================================================== */

int wg_run(wg_app_t *app) {
    if (!app) return WG_EINVAL;
    if (app->running) return WG_EINVAL;

    if (app->listen_fd == (int)WG__INVALID_SOCKET) return WG_EINVAL;   /* must wg_bind_tcp first */

#ifndef _WIN32
    /* Ignore SIGPIPE; we check write() errors instead. */
    signal(SIGPIPE, SIG_IGN);
#endif

    app->running = 1;
    app->stop_requested = 0;

    /* Reusable pollfd buffer. */
    struct pollfd *pfds = NULL;
    size_t pfd_cap = 0;

    while (!app->stop_requested) {
        /* Count fds: listen + wakeup + each client. */
        size_t n_clients = 0;
        for (wg_client_t *c = app->clients; c; c = c->next) n_clients++;
        size_t need = 2 + n_clients;
        if (need > pfd_cap) {
            size_t cap = pfd_cap ? pfd_cap : 8;
            while (cap < need) cap *= 2;
            struct pollfd *p = (struct pollfd*)realloc(pfds, cap * sizeof(*p));
            if (!p) { free(pfds); app->running = 0; return WG_ENOMEM; }
            pfds = p; pfd_cap = cap;
        }

        pfds[0].fd = app->listen_fd; pfds[0].events = POLLIN; pfds[0].revents = 0;
#ifdef _WIN32
        /* On Windows, wake_evt is a socketpair so we use it directly via pfds. */
        pfds[1].fd = app->wake_r; pfds[1].events = POLLIN; pfds[1].revents = 0;
#else
        pfds[1].fd = app->wake_r;    pfds[1].events = POLLIN; pfds[1].revents = 0;
#endif

        /* Map index -> client for after poll. */
        wg_client_t **slot = (wg_client_t**)calloc(n_clients, sizeof(*slot));
        if (n_clients && !slot) { free(pfds); app->running = 0; return WG_ENOMEM; }
        size_t i = 2;
        for (wg_client_t *c = app->clients; c; c = c->next) {
            pfds[i].fd = (int)c->fd;
            short ev = 0;
            if (c->state != WG__CS_DEAD) ev |= POLLIN;
            if (c->tx_len > 0)            ev |= POLLOUT;
            pfds[i].events = ev;
            pfds[i].revents = 0;
            slot[i - 2] = c;
            i++;
        }

        int64_t due = wg__next_timer_due(app);
        int timeout_ms;
        if (due < 0) timeout_ms = -1;
        else {
            int64_t d = due - wg__now_ms();
            if (d < 0) d = 0;
            timeout_ms = (d > 60000) ? 60000 : (int)d;
        }

        int pr = poll(pfds, (unsigned int)need, timeout_ms);
        if (pr < 0) {
#ifdef _WIN32
            int e = wg__posix_errno();
            if (e == WG__EINTR) { free(slot); continue; }
#else
            if (errno == EINTR) { free(slot); continue; }
#endif
            free(slot); free(pfds); app->running = 0; return WG_EIO;
        }

        if (pfds[0].revents & POLLIN) wg__accept_one(app);
        wg__drain_posts(app);

        for (size_t k = 0; k < n_clients; k++) {
            wg_client_t *c = slot[k];
            short re = pfds[k + 2].revents;
            if (re & (POLLERR | POLLHUP | POLLNVAL)) c->want_close = 1;
            if ((re & POLLIN) && c->state != WG__CS_DEAD) {
                wg__client_on_readable(c);
            }
            if ((re & POLLOUT) || c->tx_len > 0) {
                wg__client_on_writable(c);
            }
            /* If wanted close and tx drained, close. */
            if (c->state != WG__CS_DEAD && c->want_close && c->tx_len == 0) {
                wg__client_close(c);
            }
        }

        /* Process any clients that were accepted after this poll cycle
         * started (they weren't in pfds[] so we can't rely on revents).
         * Try to read whatever is already in the socket buffer. */
        for (wg_client_t *c = app->clients; c; c = c->next) {
            /* Clients that were in slot[] have already been handled above.
             * Newly accepted clients have rx_len == 0 and state == INIT or HANDSHAKE.
             * Skip clients that were already handled (they may have rx_len > 0
             * from this cycle, but that's fine — we only target new ones). */
            if (c->state == WG__CS_DEAD) continue;
            /* Check if this client was in the poll set by seeing if
             * its fd matches any slot entry. We handle all clients
             * that weren't in slot[] by trying a non-blocking read. */
            int was_in_slot = 0;
            for (size_t k = 0; k < n_clients; k++) {
                if (slot[k] == c) { was_in_slot = 1; break; }
            }
            if (!was_in_slot) {
                /* Newly accepted — try to read any pending data. */
                wg__client_on_readable(c);
                if (c->tx_len > 0) wg__client_on_writable(c);
                if (c->want_close && c->tx_len == 0) wg__client_close(c);
            }
        }
        free(slot);

        wg__fire_due_timers(app);
        wg_render(app);          /* auto diff + incremental push */
        wg__reap_dead(app);
    }

    free(pfds);
    app->running = 0;
    return WG_OK;
}

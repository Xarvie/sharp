/*
 * tcp.c — TCP listener + stream (基于 socket)
 *
 * 设计:
 *   - listener/stream 均为栈上 32B 不透明结构体, 内部仅含 socket_t
 *   - 创建 socket 时自动设置 nonblock + reuseaddr
 *   - tcp_dial INPROGRESS: 返回有效 stream (内部持有已发起的 socket)
 *   - 地址查询 (peer/local/listener) 通过 memcpy 绕过 strict-aliasing
 */

#include "tcp.sph"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

/* ========================================================================
 * Opaque layout + static assert
 * ======================================================================== */

typedef struct {
    socket_t sock;
} real_tcp_listener_t;

typedef struct {
    socket_t sock;
} real_tcp_stream_t;

_Static_assert(sizeof(real_tcp_listener_t) <= sizeof(tcp_listener_t),
               "real_tcp_listener_t exceeds tcp_listener_t opaque limit");
_Static_assert(sizeof(real_tcp_stream_t) <= sizeof(tcp_stream_t),
               "real_tcp_stream_t exceeds tcp_stream_t opaque limit");

/* Opaque access via memcpy — avoids strict-aliasing UB */
static inline socket_t get_sock(const uint64_t opaque[4]) {
    socket_t sock;
    memcpy(&sock, opaque, sizeof(socket_t));
    return sock;
}
static inline void set_sock(uint64_t opaque[4], socket_t sock) {
    memcpy(opaque, &sock, sizeof(socket_t));
}

/* ========================================================================
 * Helpers
 * ======================================================================== */

static socket_t make_sock(int family, int* out_err) {
    int err;
    socket_t s = socket_create(family, SOCKET_STREAM, 0, &err);
    if (s == SOCKET_INVALID) { if (out_err) *out_err = err; return SOCKET_INVALID; }
    socket_set_reuseaddr(s, 1);
    if (socket_set_nonblock(s, 1) != SOCKET_OK) {
        err = socket_last_error();
        socket_close(s);
        if (out_err) *out_err = err;
        return SOCKET_INVALID;
    }
    socket_set_nodelay(s, 1);  /* TCP_NODELAY for low-latency */
    if (out_err) *out_err = SOCKET_OK;
    return s;
}

static void addr_to_out(const struct sockaddr_storage* ss,
                        socket_addr_t* out_addr) {
    /* socket_addr_t is 128B, sockaddr_storage is 128B. memcpy is safe. */
    memcpy(out_addr, ss, sizeof(*ss));
}

#ifdef _WIN32
static int get_addr_from_fd_win(socket_t sock,
                                int (*fn)(SOCKET, struct sockaddr*, int*),
                                socket_addr_t* out_addr) {
    struct sockaddr_storage ss;
    int slen = sizeof(ss);
    SOCKET fd = (SOCKET)(intptr_t)sock;
    if (fn(fd, (struct sockaddr*)&ss, &slen) < 0)
        return socket_last_error();
    addr_to_out(&ss, out_addr);
    return SOCKET_OK;
}
#else
static int get_addr_from_fd(socket_t sock, int (*fn)(int, struct sockaddr*, socklen_t*),
                            socket_addr_t* out_addr) {
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = (int)(intptr_t)sock;
    if (fn(fd, (struct sockaddr*)&ss, &slen) < 0)
        return socket_last_error();
    addr_to_out(&ss, out_addr);
    return SOCKET_OK;
}
#endif

/* ========================================================================
 * Listener
 * ======================================================================== */

int tcp_listen(tcp_listener_t* ln, const char* ip, size_t ip_len,
               uint16_t port, int backlog, int family) {
    *ln = TCP_LISTENER_INIT;

    socket_addr_t addr;
    int err;

    if (ip && ip_len > 0) {
        err = socket_addr_resolve(&addr, ip, ip_len, port, family);
    } else {
        if (family == 0) family = SOCKET_AF_INET;
        err = socket_addr_set_any(&addr, family, port);
    }
    if (err) return err;

    /* Use the resolved family to create socket */
    int af = socket_addr_get_family(&addr);
    socket_t sock = make_sock(af, &err);
    if (err) return err;

    err = socket_bind(sock, &addr);
    if (err) { socket_close(sock); return err; }

    err = socket_listen(sock, backlog);
    if (err) { socket_close(sock); return err; }

    set_sock(ln->opaque, sock);
    return SOCKET_OK;
}

socket_t tcp_listener_socket(const tcp_listener_t* ln) {
    return get_sock(ln->opaque);
}

int tcp_listener_addr(const tcp_listener_t* ln, socket_addr_t* out_addr) {
#ifdef _WIN32
    return get_addr_from_fd_win(get_sock(ln->opaque), getsockname, out_addr);
#else
    return get_addr_from_fd(get_sock(ln->opaque), getsockname, out_addr);
#endif
}

void tcp_listener_close(tcp_listener_t* ln) {
    socket_t s = get_sock(ln->opaque);
    if (s != SOCKET_INVALID) {
        socket_close(s);
        *ln = TCP_LISTENER_INIT;
    }
}

/* ========================================================================
 * Accept
 * ======================================================================== */

tcp_stream_t tcp_accept(tcp_listener_t* ln, socket_addr_t* peer, int* out_err) {
    tcp_stream_t s = TCP_STREAM_INIT;

    socket_t client = socket_accept(get_sock(ln->opaque), peer, out_err);
    if (client == SOCKET_INVALID) return s;

    socket_set_nonblock(client, 1);
    socket_set_nodelay(client, 1);

    set_sock(s.opaque, client);
    return s;
}

/* ========================================================================
 * Dial
 * ======================================================================== */

tcp_stream_t tcp_dial(const char* host, size_t host_len,
                      uint16_t port, int family, int* out_err) {
    tcp_stream_t s = TCP_STREAM_INIT;

    socket_addr_t addr;
    int err = socket_addr_resolve(&addr, host, host_len, port, family);
    if (err) { if (out_err) *out_err = err; return s; }

    return tcp_dial_addr(&addr, out_err);
}

tcp_stream_t tcp_dial_addr(const socket_addr_t* addr, int* out_err) {
    tcp_stream_t s = TCP_STREAM_INIT;

    int af = socket_addr_get_family(addr);
    socket_t sock = make_sock(af, out_err);
    if (sock == SOCKET_INVALID) return s;

    int err = socket_connect(sock, addr);

    /* On non-blocking connect:
     *   SOCKET_OK (0)         → connect completed immediately (rare)
     *   SOCKET_ERR_INPROGRESS → in progress (POSIX), poll POLLER_OUT
     *   SOCKET_ERR_AGAIN      → in progress (Windows WSAEWOULDBLOCK), poll POLLER_OUT
     *   other                 → fatal, close socket and return INIT
     */
    if (err == SOCKET_OK || err == SOCKET_ERR_INPROGRESS || err == SOCKET_ERR_AGAIN) {
        set_sock(s.opaque, sock);
        if (out_err) *out_err = err;
        return s;
    }

    socket_close(sock);
    if (out_err) *out_err = err;
    return s;
}

/* ========================================================================
 * Stream I/O
 * ======================================================================== */

socket_t tcp_stream_socket(const tcp_stream_t* s) {
    return get_sock(s->opaque);
}

socket_io_result_t tcp_recv(tcp_stream_t* s, void* buf, size_t len) {
    return socket_recv(get_sock(s->opaque), buf, len, 0);
}

socket_io_result_t tcp_send(const tcp_stream_t* s, const void* buf, size_t len) {
    return socket_send(get_sock(s->opaque), buf, len, 0);
}

ptrdiff_t tcp_drain(tcp_stream_t* s, ringbuf_t* rb, int* drained) {
    if (!s || !rb) return -1;
    if (drained) *drained = 1;

    ptrdiff_t total = 0;
    for (;;) {
        void* wr;
        size_t space = ringbuf_space(rb);
        if (space == 0) {
            if (drained) *drained = 0;
            break;
        }

        size_t rn = ringbuf_reserve(rb, &wr, space < 4096 ? space : 4096);
        socket_io_result_t r = tcp_recv(s, wr, rn);

        if (r.nbytes > 0) {
            ringbuf_commit(rb, (size_t)r.nbytes);
            total += r.nbytes;
        } else if (r.err == SOCKET_ERR_AGAIN) {
            break; /* fully drained */
        } else if (r.nbytes == 0) {
            break; /* peer closed */
        } else {
            if (total == 0) return (ptrdiff_t)(-(int)r.err);
            break;
        }
    }
    return total;
}

int tcp_shutdown_write(tcp_stream_t* s) {
    return socket_shutdown(get_sock(s->opaque), SOCKET_SHUT_WR);
}

int tcp_set_keepalive(const tcp_stream_t* s, int keepalive) {
    return socket_set_keepalive(get_sock(s->opaque), keepalive);
}

int tcp_set_nodelay(const tcp_stream_t* s, int nodelay) {
    return socket_set_nodelay(get_sock(s->opaque), nodelay);
}

int tcp_set_defer_accept(const tcp_stream_t* s, int enable) {
    return socket_set_defer_accept(get_sock(s->opaque), enable);
}

#ifndef _WIN32
/* ========================================================================
 * Unix domain socket listener
 * ======================================================================== */

int tcp_listen_unix(tcp_listener_t* ln,
                    const char* path, size_t path_len,
                    int backlog) {
    *ln = TCP_LISTENER_INIT;

    if (!path || path_len == 0) return SOCKET_ERR_INVAL;

    /* Unlink any existing socket file */
    char pathz[108];
    size_t n = path_len < sizeof(pathz) - 1 ? path_len : sizeof(pathz) - 1;
    memcpy(pathz, path, n);
    pathz[n] = '\0';
    unlink(pathz);

    int err;
    socket_t sock = socket_create(SOCKET_AF_UNIX, SOCKET_STREAM, 0, &err);
    if (err) return err;

    socket_set_reuseaddr(sock, 1);
    if (socket_set_nonblock(sock, 1) != SOCKET_OK) {
        err = socket_last_error();
        socket_close(sock);
        return err;
    }

    socket_addr_t addr;
    err = socket_addr_set_unix(&addr, path, path_len);
    if (err) { socket_close(sock); return err; }

    err = socket_bind(sock, &addr);
    if (err) { socket_close(sock); return err; }

    err = socket_listen(sock, backlog);
    if (err) { socket_close(sock); return err; }

    set_sock(ln->opaque, sock);
    return SOCKET_OK;
}
#endif /* !_WIN32 */

/* ========================================================================
 * Stream addresses
 * ======================================================================== */

int tcp_peer_addr(const tcp_stream_t* s, socket_addr_t* out_addr) {
#ifdef _WIN32
    return get_addr_from_fd_win(get_sock(s->opaque), getpeername, out_addr);
#else
    return get_addr_from_fd(get_sock(s->opaque), getpeername, out_addr);
#endif
}

int tcp_local_addr(const tcp_stream_t* s, socket_addr_t* out_addr) {
#ifdef _WIN32
    return get_addr_from_fd_win(get_sock(s->opaque), getsockname, out_addr);
#else
    return get_addr_from_fd(get_sock(s->opaque), getsockname, out_addr);
#endif
}

/* ========================================================================
 * Stream close
 * ======================================================================== */

void tcp_stream_close(tcp_stream_t* s) {
    socket_t sock = get_sock(s->opaque);
    if (sock != SOCKET_INVALID) {
        socket_close(sock);
        *s = TCP_STREAM_INIT;
    }
}
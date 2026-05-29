/*
 * socket.c — libsocket 平台适配实现
 *
 * 策略:
 *   - #ifdef _WIN32 切换 WinSock2 / BSD sockets
 *   - 所有字符串入参从 ptr+len 转为临时 \0 栈缓冲再调系统 API
 *   - sockaddr_storage 与 socket_addr_t 通过 memcpy 互转, 不依赖 strict-aliasing
 *   - 错误码统一映射到 SOCKET_ERR_* 命名空间
 */

/* POSIX 平台启用扩展: getaddrinfo / struct timeval / etc. */
#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include "socket.sph"

#include <string.h> /* memcpy, strlen */
#include <stdio.h>  /* snprintf */

/* ========================================================================
 * Platform: Windows
 * ======================================================================== */
#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET fd_t;
#define FD_INVALID INVALID_SOCKET

#define CLOSE_FD(fd)          closesocket(fd)
#define GET_ERROR()           WSAGetLastError()
#define IOCTL_SOCKET(fd, ...) ioctlsocket(fd, __VA_ARGS__)

/* ---- WinSock lifecycle (reference-counted) ---- */
/* Contract: socket_init must be called from a single thread during startup
   before any concurrent socket access. wsa_refcount is not atomically guarded. */
static int wsa_refcount = 0;

int socket_init(void) {
    if (wsa_refcount == 0) {
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            return SOCKET_ERR_UNKNOWN;
        }
    }
    wsa_refcount++;
    return SOCKET_OK;
}

int socket_cleanup(void) {
    if (wsa_refcount <= 0) return SOCKET_OK;
    wsa_refcount--;
    if (wsa_refcount == 0) {
        WSACleanup();
    }
    return SOCKET_OK;
}

/* ---- Error mapping ---- */
static int map_error(int syserr) {
    switch (syserr) {
    case 0:                return SOCKET_OK;
    case WSAEWOULDBLOCK:   return SOCKET_ERR_AGAIN;
    case WSAEINTR:         return SOCKET_ERR_INTR;
    case WSAEBADF:         return SOCKET_ERR_BADF;
    case WSAECONNRESET:    return SOCKET_ERR_CONNRESET;
    case WSAETIMEDOUT:     return SOCKET_ERR_TIMEDOUT;
    case WSAECONNREFUSED:  return SOCKET_ERR_CONNREFUSED;
    case WSAENOBUFS:       return SOCKET_ERR_NOMEM;
    case WSAEINVAL:        return SOCKET_ERR_INVAL;
    case WSAENOTCONN:      return SOCKET_ERR_NOTCONN;
    case WSAEISCONN:       return SOCKET_ERR_ISCONN;
    case WSAEADDRINUSE:    return SOCKET_ERR_ADDRINUSE;
    case WSAEADDRNOTAVAIL: return SOCKET_ERR_ADDRNOTAVAIL;
    case WSAENETDOWN:      return SOCKET_ERR_NETDOWN;
    case WSAENETUNREACH:   return SOCKET_ERR_NETUNREACH;
    case WSAEHOSTUNREACH:  return SOCKET_ERR_HOSTUNREACH;
    case WSAEACCES:        return SOCKET_ERR_ACCESS;
    case WSAEMFILE:        return SOCKET_ERR_MFILE;
    case WSAENOTSOCK:      return SOCKET_ERR_NOTSOCK;
    case WSAEPROTONOSUPPORT: return SOCKET_ERR_PROTONOSUPPORT;
    case WSAEOPNOTSUPP:    return SOCKET_ERR_OPNOTSUPP;
    case WSAEAFNOSUPPORT:  return SOCKET_ERR_AFNOSUPPORT;
    case WSAEINPROGRESS:   return SOCKET_ERR_INPROGRESS;
    case WSAEALREADY:      return SOCKET_ERR_ALREADY;
    case WSAEMSGSIZE:      return SOCKET_ERR_MSGSIZE;
    case WSAEHOSTDOWN:     return SOCKET_ERR_HOSTDOWN;
    case WSAECONNABORTED:  return SOCKET_ERR_CONNABORTED;
    case WSAENETRESET:     return SOCKET_ERR_NETRESET;
    default:               return SOCKET_ERR_UNKNOWN;
    }
}

/* ---- Shutdown mode mapping ---- */
static int map_shut(int how) {
    switch (how) {
    case 0:  return SD_RECEIVE;
    case 1:  return SD_SEND;
    default: return SD_BOTH;
    }
}

#else /* ================================================================
       * Platform: POSIX (Linux / macOS / BSD)
       * ================================================================ */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <errno.h>

typedef int fd_t;
#define FD_INVALID (-1)

#define CLOSE_FD(fd)          close(fd)
#define GET_ERROR()           errno
#define IOCTL_SOCKET(fd, ...) ioctl(fd, __VA_ARGS__)

/* ---- No-op lifecycle ---- */
int socket_init(void)    { return SOCKET_OK; }
int socket_cleanup(void) { return SOCKET_OK; }

/* ---- Error mapping ---- */
static int map_error(int syserr) {
    switch (syserr) {
    case 0:                 return SOCKET_OK;
#if EAGAIN != EWOULDBLOCK
    case EWOULDBLOCK:
#endif
    case EAGAIN:            return SOCKET_ERR_AGAIN;
    case EINTR:             return SOCKET_ERR_INTR;
    case EBADF:             return SOCKET_ERR_BADF;
    case ECONNRESET:        return SOCKET_ERR_CONNRESET;
    case ETIMEDOUT:         return SOCKET_ERR_TIMEDOUT;
    case ECONNREFUSED:      return SOCKET_ERR_CONNREFUSED;
    case ENOMEM:            return SOCKET_ERR_NOMEM;
    case EINVAL:            return SOCKET_ERR_INVAL;
    case ENOTCONN:          return SOCKET_ERR_NOTCONN;
    case EISCONN:           return SOCKET_ERR_ISCONN;
    case EADDRINUSE:        return SOCKET_ERR_ADDRINUSE;
    case EADDRNOTAVAIL:     return SOCKET_ERR_ADDRNOTAVAIL;
    case ENETDOWN:          return SOCKET_ERR_NETDOWN;
    case ENETUNREACH:       return SOCKET_ERR_NETUNREACH;
    case EHOSTUNREACH:      return SOCKET_ERR_HOSTUNREACH;
    case EACCES:            return SOCKET_ERR_ACCESS;
    case EMFILE:
    case ENFILE:            return SOCKET_ERR_MFILE;
    case ENOTSOCK:          return SOCKET_ERR_NOTSOCK;
    case EPROTONOSUPPORT:   return SOCKET_ERR_PROTONOSUPPORT;
    case EOPNOTSUPP:        return SOCKET_ERR_OPNOTSUPP;
    case EAFNOSUPPORT:      return SOCKET_ERR_AFNOSUPPORT;
    case EINPROGRESS:       return SOCKET_ERR_INPROGRESS;
    case EALREADY:          return SOCKET_ERR_ALREADY;
    case EMSGSIZE:          return SOCKET_ERR_MSGSIZE;
    case EHOSTDOWN:         return SOCKET_ERR_HOSTDOWN;
    case ECONNABORTED:      return SOCKET_ERR_CONNABORTED;
    case EPIPE:             return SOCKET_ERR_PIPE;
    case ENETRESET:         return SOCKET_ERR_NETRESET;
    default:                return SOCKET_ERR_UNKNOWN;
    }
}

static int map_shut(int how) {
    switch (how) {
    case 0:  return SHUT_RD;
    case 1:  return SHUT_WR;
    default: return SHUT_RDWR;
    }
}

#endif /* _WIN32 */

/* ---- Cross-platform setsockopt optval cast ---- */
#ifdef _WIN32
#define SETSOCK_OPTVAL(ptr) ((const char*)(ptr))
#else
#define SETSOCK_OPTVAL(ptr) (ptr)
#endif

/* ========================================================================
 * Common: family / type / flags mapping
 * ======================================================================== */

static int map_family(int f) {
    switch (f) {
    case SOCKET_AF_INET:  return AF_INET;
    case SOCKET_AF_INET6: return AF_INET6;
#ifndef _WIN32
    case SOCKET_AF_UNIX:  return AF_UNIX;
#endif
    default:              return -1;
    }
}

static int unmap_family(int af) {
    switch (af) {
    case AF_INET:  return SOCKET_AF_INET;
    case AF_INET6: return SOCKET_AF_INET6;
    default:       return SOCKET_ERR_AFNOSUPPORT;
    }
}

static int map_socktype(int t) {
    switch (t) {
    case SOCKET_STREAM: return SOCK_STREAM;
    case SOCKET_DGRAM:  return SOCK_DGRAM;
    default:            return -1;
    }
}

static int map_msg_flags(int f) {
    int r = 0;
    if (f & SOCKET_MSG_PEEK)    r |= MSG_PEEK;
    if (f & SOCKET_MSG_OOB)     r |= MSG_OOB;
    if (f & SOCKET_MSG_WAITALL) r |= MSG_WAITALL;
    return r;
}

/* ========================================================================
 * Common: socket_addr_t <-> sockaddr_storage (memcpy, strict-aliasing safe)
 * ======================================================================== */

/* Compile-time guard: socket_addr_t must match sockaddr_storage size */
typedef char _guard_addr_size
    [sizeof(socket_addr_t) == sizeof(struct sockaddr_storage) ? 1 : -1];

/** 从 socket_addr_t 读取为 sockaddr_storage (memcpy, 零 UB). */
static struct sockaddr_storage addr_to_ss(const socket_addr_t* a) {
    struct sockaddr_storage ss;
    memcpy(&ss, a, sizeof(ss));
    return ss;
}

/** 将 sockaddr_storage 写回 socket_addr_t (memcpy, 零 UB). */
static void ss_to_addr(socket_addr_t* a, const struct sockaddr_storage* ss) {
    memcpy(a, ss, sizeof(*ss));
}

/* ---- ptr+len → 临时 \0 字符串 (栈分配, SOCKET_ADDR_STR_LEN 上限) ---- */
static int str_to_cstr(const char* src, size_t len, char* dst, size_t dstcap) {
    if (len >= dstcap) return 0;
    memcpy(dst, src, len);
    dst[len] = '\0';
    return 1;
}

/* ========================================================================
 * Address
 * ======================================================================== */

int socket_addr_set_ipv4(socket_addr_t* addr, const char* ip, size_t ip_len,
                         uint16_t port) {
    char tmp[SOCKET_ADDR_STR_LEN];
    if (!str_to_cstr(ip, ip_len, tmp, sizeof(tmp))) return SOCKET_ERR_INVAL;

    struct sockaddr_storage ss;
    memset(&ss, 0, sizeof(ss));
    struct sockaddr_in* sa = (struct sockaddr_in*)&ss;
    sa->sin_family = AF_INET;
    sa->sin_port   = htons(port);

    if (inet_pton(AF_INET, tmp, &sa->sin_addr) != 1) return SOCKET_ERR_INVAL;
    ss_to_addr(addr, &ss);
    return SOCKET_OK;
}

int socket_addr_set_ipv6(socket_addr_t* addr, const char* ip, size_t ip_len,
                         uint16_t port) {
    char tmp[SOCKET_ADDR_STR_LEN];
    if (!str_to_cstr(ip, ip_len, tmp, sizeof(tmp))) return SOCKET_ERR_INVAL;

    struct sockaddr_storage ss;
    memset(&ss, 0, sizeof(ss));
    struct sockaddr_in6* sa = (struct sockaddr_in6*)&ss;
    sa->sin6_family = AF_INET6;
    sa->sin6_port   = htons(port);

    if (inet_pton(AF_INET6, tmp, &sa->sin6_addr) != 1) return SOCKET_ERR_INVAL;
    ss_to_addr(addr, &ss);
    return SOCKET_OK;
}

int socket_addr_set_any(socket_addr_t* addr, int family, uint16_t port) {
    int af = map_family(family);
    if (af < 0) return SOCKET_ERR_AFNOSUPPORT;

    struct sockaddr_storage ss;
    memset(&ss, 0, sizeof(ss));

    if (af == AF_INET) {
        struct sockaddr_in* sa = (struct sockaddr_in*)&ss;
        sa->sin_family      = AF_INET;
        sa->sin_port        = htons(port);
        sa->sin_addr.s_addr = INADDR_ANY;
    } else {
        struct sockaddr_in6* sa = (struct sockaddr_in6*)&ss;
        sa->sin6_family = AF_INET6;
        sa->sin6_port   = htons(port);
        sa->sin6_addr   = in6addr_any;
    }
    ss_to_addr(addr, &ss);
    return SOCKET_OK;
}

int socket_addr_resolve(socket_addr_t* addr, const char* host, size_t host_len,
                        uint16_t port, int family) {
    char tmp[SOCKET_ADDR_STR_LEN];
    const char* node = NULL;

    if (host && host_len > 0) {
        if (host_len >= sizeof(tmp)) return SOCKET_ERR_INVAL;
        memcpy(tmp, host, host_len);
        tmp[host_len] = '\0';
        node = tmp;
    }
    /* host == NULL or host_len==0: node stays NULL → getaddrinfo uses loopback */

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = (family == 0) ? AF_UNSPEC : map_family(family);
    if (hints.ai_family < 0) return SOCKET_ERR_AFNOSUPPORT;
    hints.ai_socktype = SOCK_STREAM; /* don't care, just resolve */

    char port_str[8];
    /* snprintf for u16 port into small buffer */
    {
        int n = 0;
        unsigned p = port;
        if (p == 0) { port_str[0] = '0'; port_str[1] = '\0'; n = 1; }
        else {
            char rev[6]; int i = 0;
            while (p > 0) { rev[i++] = '0' + (p % 10); p /= 10; }
            while (i > 0) { port_str[n++] = rev[--i]; }
            port_str[n] = '\0';
        }
    }

    int rc = getaddrinfo(node, port_str, &hints, &res);
    if (rc != 0) {
        return SOCKET_ERR_RESOLVE;
    }

    /* Take the first result — zero first to avoid stale stack data in padding */
    memset(addr, 0, sizeof(*addr));
    memcpy(addr, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    return SOCKET_OK;
}

int socket_addr_from_ip_port(socket_addr_t* addr,
                             const char* ip, size_t ip_len,
                             uint16_t port, int family) {
    memset(addr, 0, sizeof(*addr));

    /* Copy IP string to a null-terminated buffer for inet_pton */
    char ipz[46]; /* max INET6_ADDRSTRLEN + 1 */
    size_t n = ip_len < sizeof(ipz) - 1 ? ip_len : sizeof(ipz) - 1;
    memcpy(ipz, ip, n);
    ipz[n] = '\0';

    if (family == SOCKET_AF_INET) {
        struct sockaddr_in* sin = (struct sockaddr_in*)addr;
        sin->sin_family = AF_INET;
        sin->sin_port   = htons(port);
        if (inet_pton(AF_INET, ipz, &sin->sin_addr) != 1)
            return SOCKET_ERR_RESOLVE;
        return SOCKET_OK;
    } else if (family == SOCKET_AF_INET6) {
        struct sockaddr_in6* sin6 = (struct sockaddr_in6*)addr;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port   = htons(port);
        if (inet_pton(AF_INET6, ipz, &sin6->sin6_addr) != 1)
            return SOCKET_ERR_RESOLVE;
        return SOCKET_OK;
    }
    return SOCKET_ERR_INVAL;
}

int socket_addr_set_unix(socket_addr_t* addr,
                          const char* path, size_t path_len) {
#ifdef _WIN32
    (void)addr; (void)path; (void)path_len;
    return SOCKET_ERR_AFNOSUPPORT;
#else
    memset(addr, 0, sizeof(*addr));
    struct sockaddr_un* sun = (struct sockaddr_un*)addr;
    sun->sun_family = AF_UNIX;
    size_t max = sizeof(sun->sun_path) - 1;
    if (path_len > max) return SOCKET_ERR_MSGSIZE;
    memcpy(sun->sun_path, path, path_len);
    sun->sun_path[path_len] = '\0';
    return SOCKET_OK;
#endif
}

ptrdiff_t socket_addr_get_ip(const socket_addr_t* addr, char* buf, size_t len) {
    struct sockaddr_storage ss = addr_to_ss(addr);
    char tmp[SOCKET_ADDR_STR_LEN];
    const char* result = NULL;

    if (ss.ss_family == AF_INET) {
        result = inet_ntop(AF_INET,
                           &((const struct sockaddr_in*)&ss)->sin_addr,
                           tmp, sizeof(tmp));
    } else if (ss.ss_family == AF_INET6) {
        result = inet_ntop(AF_INET6,
                           &((const struct sockaddr_in6*)&ss)->sin6_addr,
                           tmp, sizeof(tmp));
    } else {
        return (ptrdiff_t)(-SOCKET_ERR_AFNOSUPPORT);
    }

    if (!result) return (ptrdiff_t)(-map_error(GET_ERROR()));

    size_t n = strlen(tmp); /* tmp 是 inet_ntop 输出, 必然带 \0 */
    if (n > len) return (ptrdiff_t)(-SOCKET_ERR_INVAL);

    memcpy(buf, tmp, n); /* 不拷贝 \0 */
    return (ptrdiff_t)n;
}

uint16_t socket_addr_get_port(const socket_addr_t* addr) {
    struct sockaddr_storage ss = addr_to_ss(addr);
    if (ss.ss_family == AF_INET)
        return ntohs(((const struct sockaddr_in*)&ss)->sin_port);
    if (ss.ss_family == AF_INET6)
        return ntohs(((const struct sockaddr_in6*)&ss)->sin6_port);
    return 0;
}

int socket_addr_get_family(const socket_addr_t* addr) {
    return unmap_family(addr_to_ss(addr).ss_family);
}

/* ========================================================================
 * Socket lifecycle
 * ======================================================================== */

socket_t socket_create(int family, int type, int protocol, int* out_err) {
    int af  = map_family(family);
    int tp  = map_socktype(type);
    if (af < 0 || tp < 0) {
        if (out_err) *out_err = SOCKET_ERR_INVAL;
        return SOCKET_INVALID;
    }

    fd_t fd = socket(af, tp, protocol);
    if (fd == FD_INVALID) {
        int e = map_error(GET_ERROR());
        if (out_err) *out_err = e;
        return SOCKET_INVALID;
    }

    if (out_err) *out_err = SOCKET_OK;
    return (socket_t)(intptr_t)fd;
}

int socket_close(socket_t sock) {
    if (sock == SOCKET_INVALID) return SOCKET_OK;
    fd_t fd = (fd_t)(intptr_t)sock;
    if (CLOSE_FD(fd) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

/* ========================================================================
 * Operations
 * ======================================================================== */

/* get_addrlen: return correct sockaddr size for any address family */
static socklen_t get_addrlen(const struct sockaddr_storage* ss) {
    switch (ss->ss_family) {
    case AF_INET:  return sizeof(struct sockaddr_in);
    case AF_INET6: return sizeof(struct sockaddr_in6);
#ifndef _WIN32
    case AF_UNIX:  return sizeof(struct sockaddr_un);
#endif
    default:       return sizeof(struct sockaddr_storage);
    }
}

int socket_bind(socket_t sock, const socket_addr_t* addr) {
    fd_t fd = (fd_t)(intptr_t)sock;
    struct sockaddr_storage ss = addr_to_ss(addr);
    socklen_t addrlen = get_addrlen(&ss);

    if (bind(fd, (const struct sockaddr*)&ss, addrlen) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_listen(socket_t sock, int backlog) {
    fd_t fd = (fd_t)(intptr_t)sock;
    if (listen(fd, backlog) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

socket_t socket_accept(socket_t sock, socket_addr_t* peer, int* out_err) {
    fd_t fd = (fd_t)(intptr_t)sock;

    if (peer) {
        struct sockaddr_storage ss;
        memset(&ss, 0, sizeof(ss));
        socklen_t addrlen = sizeof(ss);
        fd_t client = accept(fd, (struct sockaddr*)&ss, &addrlen);
        if (client == FD_INVALID) {
            if (out_err) *out_err = map_error(GET_ERROR());
            return SOCKET_INVALID;
        }
        ss_to_addr(peer, &ss);
        if (out_err) *out_err = SOCKET_OK;
        return (socket_t)(intptr_t)client;
    } else {
        fd_t client = accept(fd, NULL, NULL);
        if (client == FD_INVALID) {
            if (out_err) *out_err = map_error(GET_ERROR());
            return SOCKET_INVALID;
        }
        if (out_err) *out_err = SOCKET_OK;
        return (socket_t)(intptr_t)client;
    }
}

int socket_connect(socket_t sock, const socket_addr_t* addr) {
    fd_t fd = (fd_t)(intptr_t)sock;
    struct sockaddr_storage ss = addr_to_ss(addr);
    socklen_t addrlen = get_addrlen(&ss);

    if (connect(fd, (const struct sockaddr*)&ss, addrlen) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_shutdown(socket_t sock, int how) {
    fd_t fd = (fd_t)(intptr_t)sock;
    if (shutdown(fd, map_shut(how)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

/* ========================================================================
 * I/O
 * ======================================================================== */

static socket_io_result_t make_io(ptrdiff_t n, int syserr) {
    socket_io_result_t r;
    if (n >= 0) {
        r.nbytes = n;
        r.err    = SOCKET_OK;
    } else {
        r.nbytes = -1;
        r.err    = map_error(syserr);
    }
    return r;
}

socket_io_result_t socket_recv(socket_t sock, void* buf, size_t len,
                               int flags) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int    sysflags = map_msg_flags(flags);
#ifdef _WIN32
    int wlen = len > 0x7FFFFFFF ? 0x7FFFFFFF : (int)len;
    int n = recv(fd, (char*)buf, wlen, sysflags);
    return make_io(n >= 0 ? n : -1, n < 0 ? GET_ERROR() : 0);
#else
    ssize_t n = recv(fd, buf, len, sysflags);
    return make_io(n >= 0 ? n : -1, n < 0 ? GET_ERROR() : 0);
#endif
}

socket_io_result_t socket_send(socket_t sock, const void* buf, size_t len,
                               int flags) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int    sysflags = map_msg_flags(flags);
#ifdef _WIN32
    int wlen = len > 0x7FFFFFFF ? 0x7FFFFFFF : (int)len;
    int n = send(fd, (const char*)buf, wlen, sysflags);
    return make_io(n >= 0 ? n : -1, n < 0 ? GET_ERROR() : 0);
#else
    ssize_t n = send(fd, buf, len, sysflags);
    return make_io(n >= 0 ? n : -1, n < 0 ? GET_ERROR() : 0);
#endif
}

socket_io_result_t socket_recvfrom(socket_t sock, void* buf, size_t len,
                                   int flags, socket_addr_t* from) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int    sysflags = map_msg_flags(flags);
    struct sockaddr_storage ss;
    memset(&ss, 0, sizeof(ss));
    socklen_t addrlen = from ? sizeof(ss) : 0;

#ifdef _WIN32
    int wlen = len > 0x7FFFFFFF ? 0x7FFFFFFF : (int)len;
    int n = recvfrom(fd, (char*)buf, wlen, sysflags,
                     from ? (struct sockaddr*)&ss : NULL,
                     from ? &addrlen : NULL);
    if (n >= 0 && from) ss_to_addr(from, &ss);
    return make_io(n >= 0 ? n : -1, n < 0 ? GET_ERROR() : 0);
#else
    ssize_t n = recvfrom(fd, buf, len, sysflags,
                         from ? (struct sockaddr*)&ss : NULL,
                         from ? &addrlen : NULL);
    if (n >= 0 && from) ss_to_addr(from, &ss);
    return make_io(n >= 0 ? n : -1, n < 0 ? GET_ERROR() : 0);
#endif
}

socket_io_result_t socket_sendto(socket_t sock, const void* buf, size_t len,
                                 int flags, const socket_addr_t* to) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int sysflags = map_msg_flags(flags);
    struct sockaddr_storage ss = addr_to_ss(to);
    socklen_t addrlen = get_addrlen(&ss);

#ifdef _WIN32
    int wlen = len > 0x7FFFFFFF ? 0x7FFFFFFF : (int)len;
    int n = sendto(fd, (const char*)buf, wlen, sysflags,
                   (const struct sockaddr*)&ss, addrlen);
    return make_io(n >= 0 ? n : -1, n < 0 ? GET_ERROR() : 0);
#else
    ssize_t n = sendto(fd, buf, len, sysflags,
                       (const struct sockaddr*)&ss, addrlen);
    return make_io(n >= 0 ? n : -1, n < 0 ? GET_ERROR() : 0);
#endif
}

/* ========================================================================
 * Socket options
 * ======================================================================== */

int socket_set_reuseaddr(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = enable ? 1 : 0;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_set_reuseport(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = enable ? 1 : 0;
#ifdef SO_REUSEPORT
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
                   (const char*)&val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
#else
    (void)fd; (void)val;
    return SOCKET_ERR_OPNOTSUPP;
#endif
}

int socket_set_keepalive(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = enable ? 1 : 0;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                   (const char*)&val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_set_nodelay(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = enable ? 1 : 0;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                   (const char*)&val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_set_nonblock(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
#ifdef _WIN32
    u_long mode = enable ? 1 : 0;
    if (ioctlsocket(fd, FIONBIO, &mode) != 0)
        return map_error(GET_ERROR());
#else
    /* POSIX: fcntl is canonical; ioctl(FIONBIO) expects int* not u_long* */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return map_error(GET_ERROR());
    if (enable) flags |= O_NONBLOCK;
    else        flags &= ~O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) != 0)
        return map_error(GET_ERROR());
#endif
    return SOCKET_OK;
}

int socket_set_ipv6only(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = enable ? 1 : 0;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY,
                   (const char*)&val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_set_recv_timeout(socket_t sock, int ms) {
    fd_t fd = (fd_t)(intptr_t)sock;
#ifdef _WIN32
    DWORD val = (DWORD)ms;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
                   (const char*)&val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
#else
    struct timeval tv;
    tv.tv_sec  = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, SETSOCK_OPTVAL(&tv), sizeof(tv)) != 0)
        return map_error(GET_ERROR());
#endif
    return SOCKET_OK;
}

int socket_set_send_timeout(socket_t sock, int ms) {
    fd_t fd = (fd_t)(intptr_t)sock;
#ifdef _WIN32
    DWORD val = (DWORD)ms;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
                   (const char*)&val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
#else
    struct timeval tv;
    tv.tv_sec  = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, SETSOCK_OPTVAL(&tv), sizeof(tv)) != 0)
        return map_error(GET_ERROR());
#endif
    return SOCKET_OK;
}

int socket_set_rcvbuf(socket_t sock, int bytes) {
    fd_t fd = (fd_t)(intptr_t)sock;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
                   (const char*)&bytes, sizeof(bytes)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_set_sndbuf(socket_t sock, int bytes) {
    fd_t fd = (fd_t)(intptr_t)sock;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                   (const char*)&bytes, sizeof(bytes)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_set_defer_accept(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = enable ? 1 : 0;
#ifdef TCP_DEFER_ACCEPT
    if (setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT,
                   &val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
#elif defined(SO_ACCEPTFILTER)
    struct accept_filter_arg af = {0};
    if (enable) {
        memcpy(af.af_name, "dataready", sizeof("dataready"));
        if (setsockopt(fd, SOL_SOCKET, SO_ACCEPTFILTER,
                       &af, sizeof(af)) != 0)
            return map_error(GET_ERROR());
    } else {
        /* Remove filter by setting empty */
        if (setsockopt(fd, SOL_SOCKET, SO_ACCEPTFILTER,
                       &af, sizeof(af)) != 0)
            return map_error(GET_ERROR());
    }
    return SOCKET_OK;
#else
    (void)fd; (void)val;
    return SOCKET_ERR_OPNOTSUPP;
#endif
}

int socket_addr_equal(const socket_addr_t* a, const socket_addr_t* b) {
    if (!a || !b) return 0;
    return memcmp(a->opaque, b->opaque, sizeof(a->opaque)) == 0 ? 1 : 0;
}

ptrdiff_t socket_addr_to_string(const socket_addr_t* addr, char* buf, size_t len) {
    if (!addr || !buf || len == 0) return -SOCKET_ERR_INVAL;

    char ip[SOCKET_ADDR_STR_LEN];
    ptrdiff_t ip_len = socket_addr_get_ip(addr, ip, sizeof(ip));
    if (ip_len <= 0) return ip_len; /* propagate error */

    uint16_t port = socket_addr_get_port(addr);
    int family = socket_addr_get_family(addr);

    /* IPv4: "addr:port", IPv6: "[addr]:port" */
    int n;
    if (family == SOCKET_AF_INET6) {
        n = snprintf(buf, len, "[%.*s]:%u", (int)ip_len, ip, (unsigned)port);
    } else {
        n = snprintf(buf, len, "%.*s:%u", (int)ip_len, ip, (unsigned)port);
    }
    if (n < 0) return -SOCKET_ERR_UNKNOWN;
    if ((size_t)n >= len) return -SOCKET_ERR_MSGSIZE; /* truncated */
    return (ptrdiff_t)n;
}

/* ========================================================================
 * Address 分类
 * ======================================================================== */

int socket_addr_is_loopback(const socket_addr_t* addr) {
    struct sockaddr_storage ss = addr_to_ss(addr);
    if (ss.ss_family == AF_INET) {
        const struct sockaddr_in* sin = (const struct sockaddr_in*)&ss;
        return (ntohl(sin->sin_addr.s_addr) >> 24) == 127 ? 1 : 0;
    }
    if (ss.ss_family == AF_INET6) {
        const struct sockaddr_in6* sin6 = (const struct sockaddr_in6*)&ss;
        return IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr) ? 1 : 0;
    }
    return 0;
}

int socket_addr_is_multicast(const socket_addr_t* addr) {
    struct sockaddr_storage ss = addr_to_ss(addr);
    if (ss.ss_family == AF_INET) {
        const struct sockaddr_in* sin = (const struct sockaddr_in*)&ss;
        uint32_t h = ntohl(sin->sin_addr.s_addr);
        return (h >= 0xE0000000 && h <= 0xEFFFFFFF) ? 1 : 0;
    }
    if (ss.ss_family == AF_INET6) {
        const struct sockaddr_in6* sin6 = (const struct sockaddr_in6*)&ss;
        return IN6_IS_ADDR_MULTICAST(&sin6->sin6_addr) ? 1 : 0;
    }
    return 0;
}

int socket_addr_is_unspecified(const socket_addr_t* addr) {
    struct sockaddr_storage ss = addr_to_ss(addr);
    if (ss.ss_family == AF_INET) {
        const struct sockaddr_in* sin = (const struct sockaddr_in*)&ss;
        return sin->sin_addr.s_addr == INADDR_ANY ? 1 : 0;
    }
    if (ss.ss_family == AF_INET6) {
        const struct sockaddr_in6* sin6 = (const struct sockaddr_in6*)&ss;
        return IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr) ? 1 : 0;
    }
    return 0;
}

int socket_addr_is_linklocal(const socket_addr_t* addr) {
    struct sockaddr_storage ss = addr_to_ss(addr);
    if (ss.ss_family == AF_INET) {
        const struct sockaddr_in* sin = (const struct sockaddr_in*)&ss;
        uint32_t h = ntohl(sin->sin_addr.s_addr);
        return (h >= 0xA9FE0000 && h <= 0xA9FEFFFF) ? 1 : 0;  /* 169.254.0.0/16 */
    }
    if (ss.ss_family == AF_INET6) {
        const struct sockaddr_in6* sin6 = (const struct sockaddr_in6*)&ss;
        return IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr) ? 1 : 0;
    }
    return 0;
}

int socket_addr_is_private(const socket_addr_t* addr) {
    struct sockaddr_storage ss = addr_to_ss(addr);
    if (ss.ss_family == AF_INET) {
        const struct sockaddr_in* sin = (const struct sockaddr_in*)&ss;
        uint32_t h = ntohl(sin->sin_addr.s_addr);
        /* 10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16 */
        if ((h & 0xFF000000) == 0x0A000000) return 1;
        if ((h & 0xFFF00000) == 0xAC100000) return 1;
        if ((h & 0xFFFF0000) == 0xC0A80000) return 1;
        return 0;
    }
    if (ss.ss_family == AF_INET6) {
        const struct sockaddr_in6* sin6 = (const struct sockaddr_in6*)&ss;
        /* fc00::/7 = unique local */
        return (sin6->sin6_addr.s6_addr[0] == 0xFC ||
                sin6->sin6_addr.s6_addr[0] == 0xFD) ? 1 : 0;
    }
    return 0;
}

/* ========================================================================
 * Address hash — FNV-1a 64-bit
 * ======================================================================== */

uint64_t socket_addr_hash(const socket_addr_t* addr) {
    if (!addr) return 0;
    uint64_t h = 14695981039346656037ULL;  /* FNV offset basis */
    const uint8_t* p = (const uint8_t*)addr->opaque;
    for (size_t i = 0; i < sizeof(addr->opaque); i++) {
        h ^= (uint64_t)p[i];
        h *= 1099511628211ULL;  /* FNV prime */
    }
    return h;
}

/* ========================================================================
 * Address 解析 — "host:port" 一键构造
 * ======================================================================== */

int socket_addr_parse(socket_addr_t* addr, const char* str, size_t len,
                      int family) {
    if (!addr || !str || len == 0) return SOCKET_ERR_INVAL;
    memset(addr, 0, sizeof(*addr));

    /* Find the last colon for port (handle IPv6 [addr]:port) */
    const char* port_start = NULL;
    size_t host_len = 0;

    /* Check for IPv6 bracket notation: [::1]:8080 */
    if (len > 0 && str[0] == '[') {
        const char* closing = NULL;
        for (size_t i = 1; i < len; i++) {
            if (str[i] == ']') { closing = &str[i]; break; }
        }
        if (!closing) return SOCKET_ERR_RESOLVE;
        host_len = (size_t)(closing - str - 1);
        /* After ']', expect ':port' */
        if ((size_t)(closing - str + 1) < len && closing[1] == ':')
            port_start = closing + 2;
        else
            port_start = NULL;
    } else {
        /* Find last ':' for host:port */
        const char* last_colon = NULL;
        for (size_t i = len; i > 0; i--) {
            if (str[i - 1] == ':') { last_colon = &str[i - 1]; break; }
        }
        if (last_colon) {
            host_len = (size_t)(last_colon - str);
            port_start = last_colon + 1;
        } else {
            host_len = len;
            port_start = NULL;
        }
    }

    /* Parse port */
    uint16_t port = 0;
    if (port_start) {
        size_t port_len = (size_t)(str + len - port_start);
        unsigned p = 0;
        for (size_t i = 0; i < port_len; i++) {
            char c = port_start[i];
            if (c < '0' || c > '9') return SOCKET_ERR_RESOLVE;
            p = p * 10 + (unsigned)(c - '0');
            if (p > 65535) return SOCKET_ERR_RESOLVE;
        }
        port = (uint16_t)p;
    }

    /* Resolve host (plus port) */
    if (host_len == 0) return SOCKET_ERR_RESOLVE;
    return socket_addr_resolve(addr, str + (str[0] == '[' ? 1 : 0),
                               host_len, port, family);
}

/* ========================================================================
 * Address 解析 — DNS 所有结果
 * ======================================================================== */

int socket_addr_resolve_all(socket_addr_t* addrs, size_t max_addrs,
                            size_t* out_count,
                            const char* host, size_t host_len,
                            uint16_t port, int family) {
    if (!addrs || max_addrs == 0 || !out_count) return SOCKET_ERR_INVAL;
    *out_count = 0;

    char tmp[SOCKET_ADDR_STR_LEN];
    const char* node = NULL;

    if (host && host_len > 0) {
        if (host_len >= sizeof(tmp)) return SOCKET_ERR_INVAL;
        memcpy(tmp, host, host_len);
        tmp[host_len] = '\0';
        node = tmp;
    }

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = (family == 0) ? AF_UNSPEC : map_family(family);
    if (hints.ai_family < 0) return SOCKET_ERR_AFNOSUPPORT;
    hints.ai_socktype = SOCK_STREAM;

    /* Convert port to string */
    char port_str[8];
    {
        int n = 0;
        unsigned p = port;
        if (p == 0) { port_str[0] = '0'; port_str[1] = '\0'; n = 1; }
        else {
            char rev[6]; int i = 0;
            while (p > 0) { rev[i++] = '0' + (p % 10); p /= 10; }
            while (i > 0) { port_str[n++] = rev[--i]; }
            port_str[n] = '\0';
        }
    }

    int rc = getaddrinfo(node, port_str, &hints, &res);
    if (rc != 0) return SOCKET_ERR_RESOLVE;

    /* Walk linked list and collect all results */
    size_t count = 0;
    for (struct addrinfo* rp = res; rp && count < max_addrs; rp = rp->ai_next) {
        memset(&addrs[count], 0, sizeof(addrs[count]));
        memcpy(&addrs[count], rp->ai_addr, rp->ai_addrlen);
        count++;
    }
    freeaddrinfo(res);
    *out_count = count;
    return SOCKET_OK;
}

/* ========================================================================
 * Socket diagnostic / graceful close
 * ======================================================================== */

int socket_get_error(socket_t sock) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len) != 0)
        return map_error(GET_ERROR());
    return err == 0 ? SOCKET_OK : map_error(err);
}

int socket_last_error(void) {
    return map_error(GET_ERROR());
}

int socket_set_linger(socket_t sock, int onoff, int seconds) {
    fd_t fd = (fd_t)(intptr_t)sock;
    struct linger l;
    l.l_onoff  = onoff;
    l.l_linger = (unsigned short)(seconds > 65535 ? 65535 : seconds);
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(l)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

/* ========================================================================
 * Extended socket options — broadcast, cork, keepalive tuning, QoS
 * ======================================================================== */

int socket_set_broadcast(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = enable ? 1 : 0;
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
                   (const char*)&val, sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
}

int socket_set_cork(socket_t sock, int enable) {
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = enable ? 1 : 0;
#ifdef TCP_CORK
    if (setsockopt(fd, IPPROTO_TCP, TCP_CORK, SETSOCK_OPTVAL(&val), sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
#elif defined(TCP_NOPUSH)
    if (setsockopt(fd, IPPROTO_TCP, TCP_NOPUSH, SETSOCK_OPTVAL(&val), sizeof(val)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
#else
    (void)fd; (void)val;
    return SOCKET_ERR_OPNOTSUPP;
#endif
}

int socket_set_keepalive_idle(socket_t sock, int seconds) {
    fd_t fd = (fd_t)(intptr_t)sock;
#ifdef TCP_KEEPIDLE
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, SETSOCK_OPTVAL(&seconds), sizeof(seconds)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
#elif defined(TCP_KEEPALIVE)
    /* macOS uses TCP_KEEPALIVE (also seconds, despite confusing name) */
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, SETSOCK_OPTVAL(&seconds), sizeof(seconds)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
#else
    (void)fd; (void)seconds;
    return SOCKET_ERR_OPNOTSUPP;
#endif
}

int socket_set_keepalive_interval(socket_t sock, int seconds) {
    fd_t fd = (fd_t)(intptr_t)sock;
#ifdef TCP_KEEPINTVL
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, SETSOCK_OPTVAL(&seconds), sizeof(seconds)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
#else
    (void)fd; (void)seconds;
    return SOCKET_ERR_OPNOTSUPP;
#endif
}

int socket_set_keepalive_count(socket_t sock, int count) {
    fd_t fd = (fd_t)(intptr_t)sock;
#ifdef TCP_KEEPCNT
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, SETSOCK_OPTVAL(&count), sizeof(count)) != 0)
        return map_error(GET_ERROR());
    return SOCKET_OK;
#else
    (void)fd; (void)count;
    return SOCKET_ERR_OPNOTSUPP;
#endif
}

int socket_set_tos(socket_t sock, int tos) {
    fd_t fd = (fd_t)(intptr_t)sock;
    struct sockaddr_storage ss;
    socklen_t sslen = sizeof(ss);
    if (getsockname(fd, (struct sockaddr*)&ss, &sslen) == 0 &&
        ss.ss_family == AF_INET6) {
        return setsockopt(fd, IPPROTO_IPV6, IPV6_TCLASS,
                          (const char*)&tos, sizeof(tos)) == 0
            ? SOCKET_OK : map_error(GET_ERROR());
    }
    /* AF_INET or unbound — use IPv4 by default */
    int val = tos;
    return setsockopt(fd, IPPROTO_IP, IP_TOS,
                      (const char*)&val, sizeof(val)) == 0
        ? SOCKET_OK : map_error(GET_ERROR());
}

int socket_set_ttl(socket_t sock, int ttl) {
    fd_t fd = (fd_t)(intptr_t)sock;
    struct sockaddr_storage ss;
    socklen_t sslen = sizeof(ss);
    if (getsockname(fd, (struct sockaddr*)&ss, &sslen) == 0 &&
        ss.ss_family == AF_INET6) {
        int hops = ttl;
        return setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                          SETSOCK_OPTVAL(&hops), sizeof(hops)) == 0
            ? SOCKET_OK : map_error(GET_ERROR());
    }
    int val = ttl;
    return setsockopt(fd, IPPROTO_IP, IP_TTL,
                      SETSOCK_OPTVAL(&val), sizeof(val)) == 0
        ? SOCKET_OK : map_error(GET_ERROR());
}

/* ========================================================================
 * Socket option getters
 * ======================================================================== */

int socket_get_recv_timeout(socket_t sock, int* out_ms) {
    if (!out_ms) return SOCKET_ERR_INVAL;
    fd_t fd = (fd_t)(intptr_t)sock;
#ifdef _WIN32
    DWORD val = 0;
    int vlen = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&val, &vlen) != 0)
        return map_error(GET_ERROR());
    *out_ms = (int)val;
#else
    struct timeval tv;
    socklen_t len = sizeof(tv);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, &len) != 0)
        return map_error(GET_ERROR());
    *out_ms = (int)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
    return SOCKET_OK;
}

int socket_get_send_timeout(socket_t sock, int* out_ms) {
    if (!out_ms) return SOCKET_ERR_INVAL;
    fd_t fd = (fd_t)(intptr_t)sock;
#ifdef _WIN32
    DWORD val = 0;
    int vlen = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&val, &vlen) != 0)
        return map_error(GET_ERROR());
    *out_ms = (int)val;
#else
    struct timeval tv;
    socklen_t len = sizeof(tv);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, &len) != 0)
        return map_error(GET_ERROR());
    *out_ms = (int)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
    return SOCKET_OK;
}

int socket_get_rcvbuf(socket_t sock, int* out_bytes) {
    if (!out_bytes) return SOCKET_ERR_INVAL;
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = 0;
    socklen_t len = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&val, &len) != 0)
        return map_error(GET_ERROR());
    *out_bytes = val;
    return SOCKET_OK;
}

int socket_get_sndbuf(socket_t sock, int* out_bytes) {
    if (!out_bytes) return SOCKET_ERR_INVAL;
    fd_t fd = (fd_t)(intptr_t)sock;
    int val = 0;
    socklen_t len = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&val, &len) != 0)
        return map_error(GET_ERROR());
    *out_bytes = val;
    return SOCKET_OK;
}

/* ========================================================================
 * Error strings
 * ======================================================================== */

const char* socket_strerror(int err) {
    switch (err) {
    case SOCKET_OK:               return "ok";
    case SOCKET_ERR_UNKNOWN:      return "unknown error";
    case SOCKET_ERR_AGAIN:        return "try again / would block";
    case SOCKET_ERR_INTR:         return "interrupted";
    case SOCKET_ERR_BADF:         return "bad file descriptor";
    case SOCKET_ERR_CONNRESET:    return "connection reset";
    case SOCKET_ERR_TIMEDOUT:     return "timed out";
    case SOCKET_ERR_CONNREFUSED:   return "connection refused";
    case SOCKET_ERR_NOMEM:        return "out of memory";
    case SOCKET_ERR_INVAL:        return "invalid argument";
    case SOCKET_ERR_NOTCONN:      return "not connected";
    case SOCKET_ERR_ISCONN:       return "already connected";
    case SOCKET_ERR_ADDRINUSE:    return "address in use";
    case SOCKET_ERR_ADDRNOTAVAIL: return "address not available";
    case SOCKET_ERR_NETDOWN:      return "network down";
    case SOCKET_ERR_NETUNREACH:   return "network unreachable";
    case SOCKET_ERR_HOSTUNREACH:  return "host unreachable";
    case SOCKET_ERR_ACCESS:       return "permission denied";
    case SOCKET_ERR_MFILE:        return "too many open files";
    case SOCKET_ERR_NOTSOCK:      return "not a socket";
    case SOCKET_ERR_PROTONOSUPPORT: return "protocol not supported";
    case SOCKET_ERR_OPNOTSUPP:    return "operation not supported";
    case SOCKET_ERR_AFNOSUPPORT:  return "address family not supported";
    case SOCKET_ERR_INPROGRESS:   return "operation in progress";
    case SOCKET_ERR_ALREADY:      return "already in progress";
    case SOCKET_ERR_MSGSIZE:      return "message too long";
    case SOCKET_ERR_HOSTDOWN:     return "host down";
    case SOCKET_ERR_RESOLVE:      return "name resolution failed";
    case SOCKET_ERR_CONNABORTED:  return "connection aborted";
    case SOCKET_ERR_PIPE:         return "broken pipe";
    case SOCKET_ERR_NETRESET:     return "network dropped - reset";
    case SOCKET_ERR_CANCELED:     return "operation canceled";
    default:                      return "unknown error";
    }
}
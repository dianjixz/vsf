/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/

#include "shell/sys/linux/vsf_linux_cfg.h"

#if     VSF_USE_LINUX == ENABLED && VSF_LINUX_USE_SOCKET == ENABLED             \
    &&  VSF_LINUX_SOCKET_USE_INET == ENABLED                                    \
    && (defined(__WIN__) || defined(__LINUX__) || defined(__linux__))

#if VSF_LINUX_CFG_RELATIVE_PATH == ENABLED
#   include "../../../include/errno.h"
#   include "../../../include/sys/types.h"
#else
#   include <errno.h>
#   include <sys/types.h>
#endif

#define __VSF_LINUX_SOCKET_CLASS_IMPLEMENT
#include "utilities/ooc_class.h"

struct dirent {
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char *d_name;
};

#define __VSF_LINUX_FS_CLASS_INHERIT__
#include "../../fs/vsf_linux_fs.h"

#if defined(__WIN__)
#   include <WinSock2.h>
#   pragma comment (lib, "ws2_32.lib")
#elif defined(__LINUX__) || defined(__linux__)
// TODO: include linux raw socket headers
#endif

/*============================ MACROS ========================================*/

#if VSF_LINUX_SOCKET_CFG_WRAPPER != ENABLED
#   error VSF_LINUX_SOCKET_CFG_WRAPPER MUST be enabled to avoid conflicts with host socket APIs
#endif

#define VSF_LINUX_SOCKET_WRAPPER(__api)     VSF_SHELL_WRAPPER(vsf_linux_socket, __api)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

// need to sync types/constants below with the real definitions in vsf
// from time.h
#ifndef __SUSECONDS_T
#   define __SUSECONDS_T    long int
#endif
typedef __SUSECONDS_T       suseconds_t;
struct vsf_linux_timeval {
    time_t          tv_sec;
    suseconds_t     tv_usec;
};

// from socket.h
#define VSF_LINUX_SOCKET_INVALID_SOCKET -1
#define VSF_LINUX_SOCKET_SOCKET_ERROR   -1
#define VSF_LINUX_SOCKET_AF_INET        2
#define VSF_LINUX_SOCKET_SOCK_STREAM    1
#define VSF_LINUX_SOCKET_SOCK_DGRAM     2
enum {
    VSF_LINUX_SOCKET_IPPROTO_TCP        = 6,
    VSF_LINUX_SOCKET_IPPROTO_UDP        = 17,
};
#define VSF_LINUX_SOCKET_SOL_SOCKET     0xFFFF
#define VSF_LINUX_SOCKET_SO_DEBUG       1
#define VSF_LINUX_SOCKET_SO_REUSEADDR   2
#define VSF_LINUX_SOCKET_SO_ACCEPTCONN  3
#define VSF_LINUX_SOCKET_SO_KEEPALIVE   4
#define VSF_LINUX_SOCKET_SO_BROADCAST   5
#define VSF_LINUX_SOCKET_SO_TYPE        6
#define VSF_LINUX_SOCKET_SO_ERROR       7
#define VSF_LINUX_SOCKET_SO_SNDBUF      8
#define VSF_LINUX_SOCKET_SO_RCVBUF      9
#define VSF_LINUX_SOCKET_SO_RCVTIMEO    10
#define VSF_LINUX_SOCKET_SO_SNDTIMEO    11
#define VSF_LINUX_SOCKET_SO_NONBLOCK    12
typedef uint32_t socklen_t;
typedef uint16_t vsf_linux_socket_sa_family_t;
struct vsf_linux_socket_sockaddr {
    vsf_linux_socket_sa_family_t sa_family;
    char sa_data[14];
};

// from netinet/in.h
typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;
struct vsf_linux_socket_in_addr {
    // s_addr maybe MACRO
    in_addr_t __s_addr;
};
struct vsf_linux_socket_sockaddr_in {
    vsf_linux_socket_sa_family_t sin_family;
    in_port_t sin_port;
    struct vsf_linux_socket_in_addr sin_addr;
    char sin_zero[8];
};

// from netinet/tcp.h
#define VSF_LINUX_SOCKET_TCP_NODELAY    1
#define VSF_LINUX_SOCKET_TCP_KEEPALIVE  2

// from vsf_linux_socket.h
vsf_dcl_class(vsf_linux_socket_priv_t)

typedef struct vsf_linux_socket_op_t {
    vsf_linux_fd_op_t fdop;

    int (*fn_init)(vsf_linux_fd_t *sfd);
    int (*fn_fini)(vsf_linux_socket_priv_t *priv, int how);
    int (*fn_connect)(vsf_linux_socket_priv_t *priv, const struct vsf_linux_socket_sockaddr *addr, socklen_t addrlen);
    int (*fn_listen)(vsf_linux_socket_priv_t *priv, int backlog);
    int (*fn_accept)(vsf_linux_socket_priv_t *priv, struct vsf_linux_socket_sockaddr *addr, socklen_t *addr_len);
    int (*fn_bind)(vsf_linux_socket_priv_t *priv, const struct vsf_linux_socket_sockaddr *addr, socklen_t addrlen);

    int (*fn_getsockopt)(vsf_linux_socket_priv_t *priv, int level, int optname, void *optval, socklen_t *optlen);
    int (*fn_setsockopt)(vsf_linux_socket_priv_t *priv, int level, int optname,const void *optval, socklen_t optlen);

    int (*fn_getpeername)(vsf_linux_socket_priv_t *socket_priv, struct vsf_linux_socket_sockaddr *addr, socklen_t *addrlen);
    int (*fn_getsockname)(vsf_linux_socket_priv_t *socket_priv, struct vsf_linux_socket_sockaddr *addr, socklen_t *addrlen);
} vsf_linux_socket_op_t;

vsf_class(vsf_linux_socket_priv_t) {
    public_member(
        implement(vsf_linux_fd_priv_t)

        int domain;
        int type;
        int protocol;
        const vsf_linux_socket_op_t *sockop;
    )
};

// hostsock private
typedef struct vsf_linux_socket_inet_priv_t {
    implement(vsf_linux_socket_priv_t)

#ifdef __WIN__
    SOCKET hostsock;
#else
    int hostsock;
#endif

    int hdomain;
    int htype;
    int hprotocol;
} vsf_linux_socket_inet_priv_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

int __vsf_linux_socket_inet_fcntl(vsf_linux_fd_t *sfd, int cmd, long arg);
static ssize_t __vsf_linux_socket_inet_read(vsf_linux_fd_t *sfd, void *buf, size_t count);
static ssize_t __vsf_linux_socket_inet_write(vsf_linux_fd_t *sfd, const void *buf, size_t count);
static int __vsf_linux_socket_inet_close(vsf_linux_fd_t *sfd);

static int __vsf_linux_socket_inet_init(vsf_linux_fd_t *sfd);
static int __vsf_linux_socket_inet_fini(vsf_linux_socket_priv_t *socket_priv, int how);
static int __vsf_linux_socket_inet_connect(vsf_linux_socket_priv_t *socket_priv, const struct vsf_linux_socket_sockaddr *addr, socklen_t addrlen);
static int __vsf_linux_socket_inet_listen(vsf_linux_socket_priv_t *socket_priv, int backlog);
static int __vsf_linux_socket_inet_accept(vsf_linux_socket_priv_t *socket_priv, struct vsf_linux_socket_sockaddr *addr, socklen_t *addr_len);
static int __vsf_linux_socket_inet_bind(vsf_linux_socket_priv_t *socket_priv, const struct vsf_linux_socket_sockaddr *addr, socklen_t addrlen);
static int __vsf_linux_socket_inet_getsockopt(vsf_linux_socket_priv_t *socket_priv, int level, int optname, void *optval, socklen_t *optlen);
static int __vsf_linux_socket_inet_setsockopt(vsf_linux_socket_priv_t *socket_priv, int level, int optname,const void *optval, socklen_t optlen);
static int __vsf_linux_socket_inet_getpeername(vsf_linux_socket_priv_t *socket_priv, struct vsf_linux_socket_sockaddr *addr, socklen_t *addrlen);
static int __vsf_linux_socket_inet_getsockname(vsf_linux_socket_priv_t *socket_priv, struct vsf_linux_socket_sockaddr *addr, socklen_t *addrlen);

/*============================ LOCAL VARIABLES ===============================*/

#ifdef __WIN__
struct vsf_linux_socket_inet_hostsock_t {
    WSADATA wsaData;
    bool is_inited;
} __vsf_linux_hostsock = {
    .is_inited = false,
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/

const vsf_linux_socket_op_t vsf_linux_socket_inet_op = {
    .fdop               = {
        .priv_size      = sizeof(vsf_linux_socket_inet_priv_t),
        .fn_fcntl       = __vsf_linux_socket_inet_fcntl,
        .fn_read        = __vsf_linux_socket_inet_read,
        .fn_write       = __vsf_linux_socket_inet_write,
        .fn_close       = __vsf_linux_socket_inet_close,
    },

    .fn_init            = __vsf_linux_socket_inet_init,
    .fn_fini            = __vsf_linux_socket_inet_fini,
    .fn_connect         = __vsf_linux_socket_inet_connect,
    .fn_listen          = __vsf_linux_socket_inet_listen,
    .fn_accept          = __vsf_linux_socket_inet_accept,
    .fn_bind            = __vsf_linux_socket_inet_bind,
    .fn_getsockopt      = __vsf_linux_socket_inet_getsockopt,
    .fn_setsockopt      = __vsf_linux_socket_inet_setsockopt,
    .fn_getpeername     = __vsf_linux_socket_inet_getpeername,
    .fn_getsockname     = __vsf_linux_socket_inet_getsockname,
};

/*============================ IMPLEMENTATION ================================*/

static void __vsf_linux_hostsock_init(void)
{
#ifdef __WIN__
    if (!__vsf_linux_hostsock.is_inited) {
        __vsf_linux_hostsock.is_inited = true;
        WSAStartup(MAKEWORD(2, 2), &__vsf_linux_hostsock.wsaData);
    }
#endif
}

static void __vsf_linux_sockaddr2host(const struct vsf_linux_socket_sockaddr *sockaddr,
                    struct sockaddr *hsockaddr)
{
    struct vsf_linux_socket_sockaddr_in *sockaddr_in = (struct vsf_linux_socket_sockaddr_in *)sockaddr;
    struct sockaddr_in *hsockaddr_in = (struct sockaddr_in *)hsockaddr;

    switch (sockaddr_in->sin_family) {
    case 0:
    case VSF_LINUX_SOCKET_AF_INET:  hsockaddr_in->sin_family = AF_INET; break;
    default:                        VSF_LINUX_ASSERT(false);
    }
    hsockaddr_in->sin_port = sockaddr_in->sin_port;
    hsockaddr_in->sin_addr.s_addr = sockaddr_in->sin_addr.__s_addr;
}

static void __vsf_linux_sockaddr2vsf(const struct sockaddr *hsockaddr,
                    struct vsf_linux_socket_sockaddr *sockaddr)
{
    struct vsf_linux_socket_sockaddr_in *sockaddr_in = (struct vsf_linux_socket_sockaddr_in *)sockaddr;
    struct sockaddr_in *hsockaddr_in = (struct sockaddr_in *)hsockaddr;

    switch (hsockaddr_in->sin_family) {
    case 0:
    case AF_INET:   sockaddr_in->sin_family = VSF_LINUX_SOCKET_AF_INET; break;
    default:        VSF_LINUX_ASSERT(false);
    }
    sockaddr_in->sin_port = hsockaddr_in->sin_port;
    sockaddr_in->sin_addr.__s_addr = hsockaddr_in->sin_addr.s_addr;
}

static unsigned long __vsf_linux_timeval_to_ms(const struct vsf_linux_timeval *t)
{
    return t->tv_sec * 1000 + t->tv_usec / 1000;
}

static void __vsf_linux_ms_to_timeval(struct vsf_linux_timeval *t, unsigned long ms)
{
    t->tv_sec = ms / 1000;
    t->tv_usec = (ms % 1000) * 1000;
}

static int __vsf_linux_sockflag2host(int flags)
{
    return 0;
}

static int __vsf_linux_socket_inet_init(vsf_linux_fd_t *sfd)
{
    vsf_linux_socket_priv_t *socket_priv = (vsf_linux_socket_priv_t *)sfd->priv;
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;

    __vsf_linux_hostsock_init();

    switch (socket_priv->domain) {
    case VSF_LINUX_SOCKET_AF_INET:      priv->hdomain = AF_INET;        break;
    default:                            goto assert_fail;
    }
    switch (socket_priv->type) {
    case VSF_LINUX_SOCKET_SOCK_STREAM:  priv->htype = SOCK_STREAM;      break;
    case VSF_LINUX_SOCKET_SOCK_DGRAM:   priv->htype = SOCK_DGRAM;       break;
    default:                            goto assert_fail;
    }
    switch (socket_priv->protocol) {
    case 0:                             priv->hprotocol = 0;            break;
    case VSF_LINUX_SOCKET_IPPROTO_TCP:  priv->hprotocol = IPPROTO_TCP;  break;
    case VSF_LINUX_SOCKET_IPPROTO_UDP:  priv->hprotocol = IPPROTO_UDP;  break;
    default:                            goto assert_fail;
    }

    priv->hostsock = socket(priv->hdomain, priv->htype, priv->hprotocol);
    if (INVALID_SOCKET == priv->hostsock) {
        return -1;
    }
    return 0;
assert_fail:
    VSF_LINUX_ASSERT(false);
    return VSF_LINUX_SOCKET_INVALID_SOCKET;
}

static int __vsf_linux_socket_inet_fini(vsf_linux_socket_priv_t *socket_priv, int how)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    shutdown(priv->hostsock, SD_BOTH);
    return 0;
}

static int __vsf_linux_socket_inet_setsockopt(vsf_linux_socket_priv_t *socket_priv,
                    int level, int optname, const void *optval, socklen_t optlen)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    int ret;

    switch (level) {
    case VSF_LINUX_SOCKET_SOL_SOCKET:
        level = SOL_SOCKET;
        switch (optname) {
        case VSF_LINUX_SOCKET_SO_BROADCAST:     optname = SO_BROADCAST;     break;
        case VSF_LINUX_SOCKET_SO_KEEPALIVE:     optname = SO_KEEPALIVE;     break;
        case VSF_LINUX_SOCKET_SO_RCVBUF:        optname = SO_RCVBUF;        break;
        case VSF_LINUX_SOCKET_SO_REUSEADDR:     optname = SO_REUSEADDR;     break;
        case VSF_LINUX_SOCKET_SO_RCVTIMEO: {
                unsigned long ms = __vsf_linux_timeval_to_ms((const struct vsf_linux_timeval *)optval);
                optname = SO_RCVTIMEO;
                ret = setsockopt(priv->hostsock, level, optname, (const char *)&ms, sizeof(ms));
                goto __return;
            }
            break;
        case VSF_LINUX_SOCKET_SO_SNDTIMEO: {
                unsigned long ms = __vsf_linux_timeval_to_ms((const struct vsf_linux_timeval *)optval);
                optname = SO_SNDTIMEO;
                ret = setsockopt(priv->hostsock, level, optname, (const char *)&ms, sizeof(ms));
                goto __return;
            }
            break;
        case VSF_LINUX_SOCKET_SO_NONBLOCK: {
                u_long optval_ulong = (u_long)(int *)optval;
                ret = ioctlsocket(priv->hostsock, FIONBIO, &optval_ulong);
                goto __return;
            }
        default:                                VSF_LINUX_ASSERT(false);    break;
        }
        break;
    case VSF_LINUX_SOCKET_IPPROTO_TCP:
        level = IPPROTO_TCP;
        switch (optname) {
        case VSF_LINUX_SOCKET_TCP_NODELAY:      optname = TCP_NODELAY;      break;
        case VSF_LINUX_SOCKET_TCP_KEEPALIVE:    optname = SO_KEEPALIVE;     break;
        default:                                VSF_LINUX_ASSERT(false);    break;
        }
        break;
    default:
        VSF_LINUX_ASSERT(false);
        break;
    }
    ret = setsockopt(priv->hostsock, level, optname, optval, optlen);
__return:
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

static int __vsf_linux_socket_inet_getsockopt(vsf_linux_socket_priv_t *socket_priv,
                    int level, int optname, void *optval, socklen_t *optlen)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    int ret;

    switch (level) {
    case VSF_LINUX_SOCKET_SOL_SOCKET:
        switch (optname) {
        case VSF_LINUX_SOCKET_SO_BROADCAST:     optname = SO_BROADCAST;     break;
        case VSF_LINUX_SOCKET_SO_KEEPALIVE:     optname = SO_KEEPALIVE;     break;
        case VSF_LINUX_SOCKET_SO_RCVBUF:        optname = SO_RCVBUF;        break;
        case VSF_LINUX_SOCKET_SO_REUSEADDR:     optname = SO_REUSEADDR;     break;
        case VSF_LINUX_SOCKET_SO_RCVTIMEO: {
                unsigned long ms;
                int hoptlen = sizeof(ms);
                optname = SO_RCVTIMEO;
                ret = getsockopt(priv->hostsock, level, optname, (char *)&ms, &hoptlen);
                __vsf_linux_ms_to_timeval(optval, ms);
            }
            break;
        case VSF_LINUX_SOCKET_SO_SNDTIMEO: {
                unsigned long ms;
                int hoptlen = sizeof(ms);
                optname = SO_SNDTIMEO;
                ret = getsockopt(priv->hostsock, level, optname, (char *)&ms, &hoptlen);
                __vsf_linux_ms_to_timeval(optval, ms);
            }
            break;
        case VSF_LINUX_SOCKET_SO_NONBLOCK:      VSF_LINUX_ASSERT(false);    break;
        default:                                VSF_LINUX_ASSERT(false);    break;
        }
        break;
    case VSF_LINUX_SOCKET_IPPROTO_TCP:
        switch (optname) {
        case VSF_LINUX_SOCKET_TCP_NODELAY:      optname = TCP_NODELAY;      break;
        case VSF_LINUX_SOCKET_TCP_KEEPALIVE:    optname = SO_KEEPALIVE;     break;
        default:                                VSF_LINUX_ASSERT(false);    break;
        }
        break;
    default:
        VSF_LINUX_ASSERT(false);
        break;
    }
    ret = getsockopt(priv->hostsock, level, optname, optval, (int *)optlen);
__return:
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

static int __vsf_linux_socket_inet_getpeername(vsf_linux_socket_priv_t *socket_priv,
                    struct vsf_linux_socket_sockaddr *addr, socklen_t *addrlen)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    struct sockaddr hsockaddr = { 0 };
    int hsockaddr_len = sizeof(hsockaddr);
    int ret = getpeername(priv->hostsock, &hsockaddr, &hsockaddr_len);
    __vsf_linux_sockaddr2vsf(&hsockaddr, addr);
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

static int __vsf_linux_socket_inet_getsockname(vsf_linux_socket_priv_t *socket_priv,
                    struct vsf_linux_socket_sockaddr *addr, socklen_t *addrlen)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    struct sockaddr hsockaddr = { 0 };
    int hsockaddr_len = sizeof(hsockaddr);
    int ret = getsockname(priv->hostsock, &hsockaddr, &hsockaddr_len);
    __vsf_linux_sockaddr2vsf(&hsockaddr, addr);
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

static int __vsf_linux_socket_inet_accept(vsf_linux_socket_priv_t *socket_priv,
                    struct vsf_linux_socket_sockaddr *addr, socklen_t *addrlen)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    struct sockaddr hsockaddr;
    int hsockaddr_len = sizeof(hsockaddr);

    int hnewsock = accept(priv->hostsock, &hsockaddr, &hsockaddr_len);
    if (INVALID_SOCKET == hnewsock) {
        return VSF_LINUX_SOCKET_INVALID_SOCKET;
    }

    int newsock = VSF_LINUX_SOCKET_WRAPPER(socket)(socket_priv->domain, socket_priv->type, socket_priv->protocol);
    if (newsock < 0) {
        closesocket(newsock);
        return VSF_LINUX_SOCKET_INVALID_SOCKET;
    }

    vsf_linux_fd_t *sfd = vsf_linux_fd_get(newsock);
    vsf_linux_socket_inet_priv_t *newpriv = (vsf_linux_socket_inet_priv_t *)sfd->priv;
    *newpriv = *priv;
    newpriv->hostsock = hnewsock;
    return newsock;
}

static int __vsf_linux_socket_inet_bind(vsf_linux_socket_priv_t *socket_priv,
                    const struct vsf_linux_socket_sockaddr *addr, socklen_t addrlen)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    struct sockaddr hsockaddr = { 0 };
    __vsf_linux_sockaddr2host(addr, &hsockaddr);
    int ret = bind(priv->hostsock, (const struct sockaddr *)&hsockaddr, sizeof(hsockaddr));
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

static int __vsf_linux_socket_inet_connect(vsf_linux_socket_priv_t *socket_priv,
                    const struct vsf_linux_socket_sockaddr *addr, socklen_t addrlen)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    struct sockaddr hsockaddr = { 0 };
    __vsf_linux_sockaddr2host(addr, &hsockaddr);
    int ret = connect(priv->hostsock, &hsockaddr, sizeof(hsockaddr));
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

static int __vsf_linux_socket_inet_listen(vsf_linux_socket_priv_t *socket_priv, int backlog)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)socket_priv;
    int ret = listen(priv->hostsock, backlog);
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

static ssize_t __vsf_linux_socket_inet_send(vsf_linux_socket_inet_priv_t *priv, const void *buffer, size_t size, int flags,
                    const struct vsf_linux_socket_sockaddr *dst_addr, socklen_t addrlen)
{
    int ret;
    if (dst_addr != NULL) {
        struct sockaddr hsockaddr = { 0 };
        __vsf_linux_sockaddr2host(dst_addr, &hsockaddr);
        ret = sendto(priv->hostsock, buffer, size, __vsf_linux_sockflag2host(flags), &hsockaddr, sizeof(hsockaddr));
    } else {
        ret = send(priv->hostsock, buffer, size, __vsf_linux_sockflag2host(flags));
    }
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

static ssize_t __vsf_linux_socket_inet_recv(vsf_linux_socket_inet_priv_t *priv, void *buffer, size_t size, int flags,
                    struct vsf_linux_socket_sockaddr *src_addr, socklen_t *addrlen)
{
    int ret;
    if (src_addr != NULL) {
        struct sockaddr hsockaddr = { 0 };
        int hsockaddr_len = sizeof(hsockaddr);
        ret = recvfrom(priv->hostsock, buffer, size, __vsf_linux_sockflag2host(flags), &hsockaddr, &hsockaddr_len);
        __vsf_linux_sockaddr2vsf(&hsockaddr, src_addr);
    } else {
        ret = send(priv->hostsock, buffer, size, __vsf_linux_sockflag2host(flags));
    }
    return SOCKET_ERROR == ret ? VSF_LINUX_SOCKET_SOCKET_ERROR : ret;
}

// socket fd
static ssize_t __vsf_linux_socket_inet_read(vsf_linux_fd_t *sfd, void *buf, size_t count)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)sfd->priv;
    return __vsf_linux_socket_inet_recv(priv, buf, count, 0, NULL, NULL);
}

static ssize_t __vsf_linux_socket_inet_write(vsf_linux_fd_t *sfd, const void *buf, size_t count)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)sfd->priv;
    return __vsf_linux_socket_inet_send(priv, buf, count, 0, NULL, 0);
}

static int __vsf_linux_socket_inet_close(vsf_linux_fd_t *sfd)
{
    vsf_linux_socket_inet_priv_t *priv = (vsf_linux_socket_inet_priv_t *)sfd->priv;
    closesocket(priv->hostsock);
    return 0;
}

// sendto & recvfrom
ssize_t VSF_LINUX_SOCKET_WRAPPER(sendto)(int sockfd, const void *buffer, size_t size, int flags,
                    const struct vsf_linux_socket_sockaddr *dst_addr, socklen_t addrlen)
{
    vsf_linux_fd_t *sfd = vsf_linux_fd_get(sockfd);
	if (!sfd) {
		return -1;
	}

    return __vsf_linux_socket_inet_send((vsf_linux_socket_inet_priv_t *)sfd->priv,
                    buffer, size, flags, dst_addr, addrlen);
}

ssize_t VSF_LINUX_SOCKET_WRAPPER(recvfrom)(int sockfd, void *buffer, size_t size, int flags,
                    struct vsf_linux_socket_sockaddr *src_addr, socklen_t *addrlen)
{
    vsf_linux_fd_t *sfd = vsf_linux_fd_get(sockfd);
    if (!sfd) {
        return -1;
    }

    return __vsf_linux_socket_inet_recv((vsf_linux_socket_inet_priv_t *)sfd->priv,
                    buffer, size, flags, src_addr, addrlen);
}

// netdb.h
// none thread safty
int __inet_gethostbyname(const char *name, in_addr_t *addr)
{
    __vsf_linux_hostsock_init();

    struct hostent *host = gethostbyname(name);
    if (NULL == host) {
        return -1;
    }
    if (addr != NULL) {
        *addr = *(in_addr_t *)host->h_addr;
    }
    return 0;
}

#endif
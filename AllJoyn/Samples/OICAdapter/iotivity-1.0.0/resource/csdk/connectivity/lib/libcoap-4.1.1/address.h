/* address.h -- representation of network addresses
 *
 * Copyright (C) 2010,2011 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

/**
 * @file address.h
 * @brief representation of network addresses
 */

#ifndef _COAP_ADDRESS_H_
#define _COAP_ADDRESS_H_

#include "config.h"

#ifdef HAVE_ASSERT_H
#include <assert.h>
#else
#ifndef assert
#warning "assertions are disabled"
#  define assert(x)
#endif
#endif

#include <string.h>
#include <stdint.h>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <sys/socket.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#include <Ws2ipdef.h>
#include <Ws2tcpip.h>
#endif

#ifdef WITH_ARDUINO
#define DEV_ADDR_SIZE_MAX (16)
#endif

#ifdef WITH_LWIP
#include <lwip/ip_addr.h>

typedef struct coap_address_t
{
    uint16_t port;
    ip_addr_t addr;
}coap_address_t;

/* FIXME oversimplification: just assuming it's an ipv4 address instead of
 * looking up the appropraite lwip function */

#define _coap_address_equals_impl(A, B) ((A)->addr.addr == (B)->addr.addr && A->port == B->port)

/* FIXME sure there is something in lwip */

#define _coap_is_mcast_impl(Address) 0

#endif /* WITH_LWIP */
#ifdef WITH_CONTIKI
#include "uip.h"

typedef struct coap_address_t
{
    unsigned char size;
    uip_ipaddr_t addr;
    unsigned short port;
}coap_address_t;

#define _coap_address_equals_impl(A,B)				\
  ((A)->size == (B)->size					\
   && (A)->port == (B)->port					\
   && uip_ipaddr_cmp(&((A)->addr),&((B)->addr)))

#define _coap_is_mcast_impl(Address) uip_is_addr_mcast(&((Address)->addr))
#endif /* WITH_CONTIKI */
#if defined(WITH_POSIX) || defined(WIN32)

/** multi-purpose address abstraction */
typedef struct coap_address_t
{
    socklen_t size; /**< size of addr */
    union
    {
        struct sockaddr sa;
        struct sockaddr_storage st;
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
    }addr;
}coap_address_t;

static inline int
_coap_address_equals_impl(const coap_address_t *a,
        const coap_address_t *b)
{
    if (a->size != b->size || a->addr.sa.sa_family != b->addr.sa.sa_family)
    return 0;

    /* need to compare only relevant parts of sockaddr_in6 */
    switch (a->addr.sa.sa_family)
    {
        case AF_INET:
        return
        a->addr.sin.sin_port == b->addr.sin.sin_port &&
        memcmp(&a->addr.sin.sin_addr, &b->addr.sin.sin_addr,
                sizeof(struct in_addr)) == 0;
        case AF_INET6:
        return a->addr.sin6.sin6_port == b->addr.sin6.sin6_port &&
        memcmp(&a->addr.sin6.sin6_addr, &b->addr.sin6.sin6_addr,
                sizeof(struct in6_addr)) == 0;
        default: /* fall through and signal error */
        ;
    }
    return 0;
}

static inline int
_coap_is_mcast_impl(const coap_address_t *a)
{
    if (!a)
    {
        return 0;
    }
    switch (a->addr.sa.sa_family)
    {
        case AF_INET:
        return IN_MULTICAST(a->addr.sin.sin_addr.s_addr);
        case AF_INET6:
        return IN6_IS_ADDR_MULTICAST(&a->addr.sin6.sin6_addr);
        default: /* fall through and signal error */
        ;
    }
    return 0;
}
#endif /* WITH_POSIX */

#ifdef WITH_ARDUINO
typedef struct coap_address_t
{
    uint32_t     size;                    /**< length of the address stored in addr field. */
    uint8_t      addr[DEV_ADDR_SIZE_MAX]; /**< device address. */
} coap_address_t;

static inline int
_coap_address_equals_impl(const coap_address_t *a,
                          const coap_address_t *b)
{
    uint32_t i;

    if ((a == NULL) || (b == NULL))
        return 0;

    if (a->size != b->size)
        return 0;

    for (i = 0; i < a->size; i++)
    {
        if (a->addr[i] != b->addr[i])
            return 0;
    }
    return 1;
}

static inline int
_coap_is_mcast_impl(const coap_address_t *a)
{
    if (!a)
        return 0;

    /* TODO */
    return 0;
}

#endif /* WITH_ARDUINO */

/**
 * Resets the given coap_address_t object @p addr to its default
 * values.  In particular, the member size must be initialized to the
 * available size for storing addresses.
 *
 * @param addr The coap_address_t object to initialize.
 */
static inline void coap_address_init(coap_address_t *addr)
{
    assert(addr);
    memset(addr, 0, sizeof(coap_address_t));
#ifndef WITH_LWIP
    /* lwip has constandt address sizes and doesn't need the .size part */
    addr->size = sizeof(addr->addr);
#endif
}

/**
 * Compares given address objects @p a and @p b. This function returns
 * @c 1 if addresses are equal, @c 0 otherwise. The parameters @p a
 * and @p b must not be @c NULL;
 */
static inline int coap_address_equals(const coap_address_t *a, const coap_address_t *b)
{
    assert(a);
    assert(b);
    return _coap_address_equals_impl(a, b);
}

/**
 * Checks if given address @p a denotes a multicast address.  This
 * function returns @c 1 if @p a is multicast, @c 0 otherwise.
 */
static inline int coap_is_mcast(const coap_address_t *a)
{
    return a && _coap_is_mcast_impl(a);
}

#endif /* _COAP_ADDRESS_H_ */

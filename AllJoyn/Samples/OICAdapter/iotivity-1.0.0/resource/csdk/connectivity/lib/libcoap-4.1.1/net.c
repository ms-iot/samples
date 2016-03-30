/* net.c -- CoAP network interface
 *
 * Copyright (C) 2010--2014 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif HAVE_SYS_UNISTD_H
#include <sys/unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef WIN32
#include <winsock2.h>
typedef int ssize_t;
static int close(SOCKET sd) { return closesocket((sd)); }
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef WITH_LWIP
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/timers.h>
#endif

#include "debug.h"
#include "mem.h"
#include "str.h"
#include "async.h"
#include "resource.h"
#include "option.h"
#include "encode.h"
#include "block.h"
#include "net.h"

#if defined(WITH_POSIX) || defined(WITH_ARDUINO) || defined(WIN32)

time_t clock_offset=0;

static inline coap_queue_t *
coap_malloc_node()
{
    return (coap_queue_t *)coap_malloc(sizeof(coap_queue_t));
}

static inline void
coap_free_node(coap_queue_t *node)
{
    coap_free(node);
}
#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_LWIP

#include <lwip/memp.h>

static void coap_retransmittimer_execute(void *arg);
static void coap_retransmittimer_restart(coap_context_t *ctx);

static inline coap_queue_t *
coap_malloc_node()
{
    return (coap_queue_t *)memp_malloc(MEMP_COAP_NODE);
}

static inline void
coap_free_node(coap_queue_t *node)
{
    memp_free(MEMP_COAP_NODE, node);
}

#endif /* WITH_LWIP */
#ifdef WITH_CONTIKI
# ifndef DEBUG
#  define DEBUG DEBUG_PRINT
# endif /* DEBUG */

#include "memb.h"
#include "net/uip-debug.h"

clock_time_t clock_offset;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[UIP_LLIPH_LEN])

void coap_resources_init();
void coap_pdu_resources_init();

unsigned char initialized = 0;
coap_context_t the_coap_context;

MEMB(node_storage, coap_queue_t, COAP_PDU_MAXCNT);

PROCESS(coap_retransmit_process, "message retransmit process");

static inline coap_queue_t *
coap_malloc_node()
{
    return (coap_queue_t *)memb_alloc(&node_storage);
}

static inline void
coap_free_node(coap_queue_t *node)
{
    memb_free(&node_storage, node);
}
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP

/** Callback to udp_recv when using lwIP. Gets called by lwIP on arriving
 * packages, places a reference in context->pending_package, and calls
 * coap_read to process the package. Thus, coap_read needs not be called in
 * lwIP main loops. (When modifying this for thread-like operation, ie. if you
 * remove the coap_read call from this, make sure that coap_read gets a chance
 * to run before this callback is entered the next time.)
 */
static void received_package(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
    struct coap_context_t *context = (coap_context_t *)arg;

    LWIP_ASSERT("pending_package was not cleared.", context->pending_package == NULL);

    context->pending_package = p; /* we don't free it, coap_read has to do that */
    context->pending_address.addr = addr->addr; /* FIXME: this has to become address-type independent, probably there'll be an lwip function for that */
    context->pending_port = port;

    coap_read(context);
}

#endif /* WITH_LWIP */

int print_wellknown(coap_context_t *, unsigned char *, size_t *, size_t, coap_opt_t *);

void coap_handle_failed_notify(coap_context_t *, const coap_address_t *, const str *);

unsigned int coap_adjust_basetime(coap_context_t *ctx, coap_tick_t now)
{
    unsigned int result = 0;
    coap_tick_diff_t delta = now - ctx->sendqueue_basetime;

    if (ctx->sendqueue)
    {
        /* delta < 0 means that the new time stamp is before the old. */
        if (delta <= 0)
        {
            ctx->sendqueue->t -= delta;
        }
        else
        {
            /* This case is more complex: The time must be advanced forward,
             * thus possibly leading to timed out elements at the queue's
             * start. For every element that has timed out, its relative
             * time is set to zero and the result counter is increased. */

            coap_queue_t *q = ctx->sendqueue;
            coap_tick_t t = 0;
            while (q && (t + q->t < (coap_tick_t) delta))
            {
                t += q->t;
                q->t = 0;
                result++;
                q = q->next;
            }

            /* finally adjust the first element that has not expired */
            if (q)
            {
                q->t = (coap_tick_t) delta - t;
            }
        }
    }

    /* adjust basetime */
    ctx->sendqueue_basetime += delta;

    return result;
}

int coap_insert_node(coap_queue_t **queue, coap_queue_t *node)
{
    coap_queue_t *p, *q;
    if (!queue || !node)
        return 0;

    /* set queue head if empty */
    if (!*queue)
    {
        *queue = node;
        return 1;
    }

    /* replace queue head if PDU's time is less than head's time */
    q = *queue;
    if (node->t < q->t)
    {
        node->next = q;
        *queue = node;
        q->t -= node->t; /* make q->t relative to node->t */
        return 1;
    }

    /* search for right place to insert */
    do
    {
        node->t -= q->t; /* make node-> relative to q->t */
        p = q;
        q = q->next;
    } while (q && q->t <= node->t);

    /* insert new item */
    if (q)
    {
        q->t -= node->t; /* make q->t relative to node->t */
    }
    node->next = q;
    p->next = node;
    return 1;
}

int coap_delete_node(coap_queue_t *node)
{
    if (!node)
        return 0;

    coap_delete_pdu(node->pdu);
    coap_free_node(node);

    return 1;
}

void coap_delete_all(coap_queue_t *queue)
{
    if (!queue)
        return;

    coap_delete_all(queue->next);
    coap_delete_node(queue);
}

coap_queue_t *
coap_new_node()
{
    coap_queue_t *node;
    node = coap_malloc_node();

    if (!node)
    {
#ifndef NDEBUG
        coap_log(LOG_WARNING, "coap_new_node: malloc\n");
#endif
        return NULL;
    }

    memset(node, 0, sizeof *node);
    return node;
}

coap_queue_t *
coap_peek_next(coap_context_t *context)
{
    if (!context || !context->sendqueue)
        return NULL;

    return context->sendqueue;
}

coap_queue_t *
coap_pop_next(coap_context_t *context)
{
    coap_queue_t *next;

    if (!context || !context->sendqueue)
        return NULL;

    next = context->sendqueue;
    context->sendqueue = context->sendqueue->next;
    if (context->sendqueue)
    {
        context->sendqueue->t += next->t;
    }
    next->next = NULL;
    return next;
}

#ifdef COAP_DEFAULT_WKC_HASHKEY
/** Checks if @p Key is equal to the pre-defined hash key for.well-known/core. */
#define is_wkc(Key)                         \
  (memcmp((Key), COAP_DEFAULT_WKC_HASHKEY, sizeof(coap_key_t)) == 0)
#else
/* Implements a singleton to store a hash key for the .wellknown/core
 * resources. */
int
is_wkc(coap_key_t k)
{
    static coap_key_t wkc;
    static unsigned char _initialized = 0;
    if (!_initialized)
    {
        _initialized = coap_hash_path((unsigned char *)COAP_DEFAULT_URI_WELLKNOWN,
                sizeof(COAP_DEFAULT_URI_WELLKNOWN) - 1, wkc);
    }
    return memcmp(k, wkc, sizeof(coap_key_t)) == 0;
}
#endif


#ifndef WITH_ARDUINO
coap_context_t *
coap_new_context(const coap_address_t *listen_addr)
{
#if defined(WITH_POSIX) || defined(WIN32)
    coap_context_t *c = coap_malloc( sizeof( coap_context_t ) );
    int reuse = 1;
#endif /* WITH_POSIX */
#ifdef WITH_LWIP
    coap_context_t *c = memp_malloc(MEMP_COAP_CONTEXT);
#endif /* WITH_LWIP */
#ifdef WITH_CONTIKI
    coap_context_t *c;

    if (initialized)
    return NULL;
#endif /* WITH_CONTIKI */

    if (!listen_addr)
    {
        coap_log(LOG_EMERG, "no listen address specified\n");
        return NULL;
    }

    coap_clock_init();
#ifdef WITH_LWIP
    prng_init(LWIP_RAND());
#else /* WITH_LWIP */
    prng_init((unsigned long)listen_addr ^ clock_offset);
#endif /* WITH_LWIP */

#ifndef WITH_CONTIKI
    if (!c)
    {
#ifndef NDEBUG
        coap_log(LOG_EMERG, "coap_init: malloc:\n");
#endif
        return NULL;
    }
#endif /* not WITH_CONTIKI */
#ifdef WITH_CONTIKI
    coap_resources_init();
    coap_pdu_resources_init();

    c = &the_coap_context;
    initialized = 1;
#endif /* WITH_CONTIKI */

    memset(c, 0, sizeof(coap_context_t));

    /* initialize message id */
    prng((unsigned char *)&c->message_id, sizeof(unsigned short));

    /* register the critical options that we know */
    coap_register_option(c, COAP_OPTION_IF_MATCH);
    coap_register_option(c, COAP_OPTION_URI_HOST);
    coap_register_option(c, COAP_OPTION_IF_NONE_MATCH);
    coap_register_option(c, COAP_OPTION_URI_PORT);
    coap_register_option(c, COAP_OPTION_URI_PATH);
    coap_register_option(c, COAP_OPTION_URI_QUERY);
    coap_register_option(c, COAP_OPTION_ACCEPT);
    coap_register_option(c, COAP_OPTION_PROXY_URI);
    coap_register_option(c, COAP_OPTION_PROXY_SCHEME);
    coap_register_option(c, COAP_OPTION_BLOCK2);
    coap_register_option(c, COAP_OPTION_BLOCK1);

#if defined(WITH_POSIX) || defined(WITH_ARDUINO) || defined(WIN32)
    c->sockfd = socket(listen_addr->addr.sa.sa_family, SOCK_DGRAM, 0);
    if ( c->sockfd < 0 )
    {
#ifndef NDEBUG
        coap_log(LOG_EMERG, "coap_new_context: socket\n");
#endif /* WITH_POSIX */
        goto onerror;
    }

    if ( setsockopt( c->sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse) ) < 0 )
    {
#ifndef NDEBUG
        coap_log(LOG_WARNING, "setsockopt SO_REUSEADDR\n");
#endif
    }

    if (bind(c->sockfd, &listen_addr->addr.sa, listen_addr->size) < 0)
    {
#ifndef NDEBUG
        coap_log(LOG_EMERG, "coap_new_context: bind\n");
#endif
        goto onerror;
    }

    return c;

    onerror:
    if ( c->sockfd >= 0 )
    close ( c->sockfd );
    coap_free( c );
    return NULL;

#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_CONTIKI
    c->conn = udp_new(NULL, 0, NULL);
    udp_bind(c->conn, listen_addr->port);

    process_start(&coap_retransmit_process, (char *)c);

    PROCESS_CONTEXT_BEGIN(&coap_retransmit_process);
#ifndef WITHOUT_OBSERVE
    etimer_set(&c->notify_timer, COAP_RESOURCE_CHECK_TIME * COAP_TICKS_PER_SECOND);
#endif /* WITHOUT_OBSERVE */
    /* the retransmit timer must be initialized to some large value */
    etimer_set(&the_coap_context.retransmit_timer, 0xFFFF);
    PROCESS_CONTEXT_END(&coap_retransmit_process);
    return c;
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP
    c->pcb = udp_new();
    /* hard assert: this is not expected to fail dynamically */
    LWIP_ASSERT("Failed to allocate PCB for CoAP", c->pcb != NULL);

    udp_recv(c->pcb, received_package, (void*)c);
    udp_bind(c->pcb, &listen_addr->addr, listen_addr->port);

    c->timer_configured = 0;

    return c;
#endif
}

void coap_free_context(coap_context_t *context)
{
#if defined(WITH_POSIX) || defined(WITH_LWIP) || defined(WIN32)
    coap_resource_t *res;
#ifndef COAP_RESOURCES_NOHASH
    coap_resource_t *rtmp;
#endif
#endif /* WITH_POSIX || WITH_LWIP */
    if (!context)
        return;

    coap_delete_all(context->recvqueue);
    coap_delete_all(context->sendqueue);

#ifdef WITH_LWIP
    context->sendqueue = NULL;
    coap_retransmittimer_restart(context);
#endif

#if defined(WITH_POSIX) || defined(WITH_LWIP) || defined(WITH_ARDUINO) || defined(WIN32)
#ifdef COAP_RESOURCES_NOHASH
    LL_FOREACH(context->resources, res)
    {
#else
        HASH_ITER(hh, context->resources, res, rtmp)
        {
#endif
            coap_delete_resource(context, res->key);
        }
#endif /* WITH_POSIX || WITH_LWIP */

#if defined(WITH_POSIX) || defined(WIN32)
    /* coap_delete_list(context->subscriptions); */
    close( context->sockfd );
    coap_free( context );
#endif
#ifdef WITH_LWIP
    udp_remove(context->pcb);
    memp_free(MEMP_COAP_CONTEXT, context);
#endif
#ifdef WITH_CONTIKI
    memset(&the_coap_context, 0, sizeof(coap_context_t));
    initialized = 0;
#endif /* WITH_CONTIKI */
}
#endif //ifndef WITH_ARDUINO
int coap_option_check_critical(coap_context_t *ctx, coap_pdu_t *pdu, coap_opt_filter_t unknown)
{

    coap_opt_iterator_t opt_iter;
    int ok = 1;

    coap_option_iterator_init(pdu, &opt_iter, COAP_OPT_ALL, coap_udp);

    while (coap_option_next(&opt_iter))
    {

        /* The following condition makes use of the fact that
         * coap_option_getb() returns -1 if type exceeds the bit-vector
         * filter. As the vector is supposed to be large enough to hold
         * the largest known option, we know that everything beyond is
         * bad.
         */
        if (opt_iter.type & 0x01 && coap_option_getb(ctx->known_options, opt_iter.type) < 1)
        {
            debug("unknown critical option %d\n", opt_iter.type);

            ok = 0;

            /* When opt_iter.type is beyond our known option range,
             * coap_option_setb() will return -1 and we are safe to leave
             * this loop. */
            if (coap_option_setb(unknown, opt_iter.type) == -1)
                break;
        }
    }

    return ok;
}

void coap_transaction_id(const coap_address_t *peer, const coap_pdu_t *pdu, coap_tid_t *id)
{
    coap_key_t h;

    memset(h, 0, sizeof(coap_key_t));

    /* Compare the complete address structure in case of IPv4. For IPv6,
     * we need to look at the transport address only. */

#if defined(WITH_POSIX) || defined(WIN32)
    switch (peer->addr.sa.sa_family)
    {
        case AF_INET:
        coap_hash((const unsigned char *)&peer->addr.sa, peer->size, h);
        break;
        case AF_INET6:
        coap_hash((const unsigned char *)&peer->addr.sin6.sin6_port,
                sizeof(peer->addr.sin6.sin6_port), h);
        coap_hash((const unsigned char *)&peer->addr.sin6.sin6_addr,
                sizeof(peer->addr.sin6.sin6_addr), h);
        break;
        default:
        return;
    }
#endif

#ifdef WITH_ARDUINO
    coap_hash((const unsigned char *)peer->addr, peer->size, h);
#endif /* WITH_ARDUINO */

#if defined(WITH_LWIP) || defined(WITH_CONTIKI)
    /* FIXME: with lwip, we can do better */
    coap_hash((const unsigned char *)&peer->port, sizeof(peer->port), h);
    coap_hash((const unsigned char *)&peer->addr, sizeof(peer->addr), h);
#endif /* WITH_LWIP || WITH_CONTIKI */

    coap_hash((const unsigned char *)&pdu->hdr->coap_hdr_udp_t.id, sizeof(unsigned short), h);

    *id = ((h[0] << 8) | h[1]) ^ ((h[2] << 8) | h[3]);
}

coap_tid_t coap_send_ack(coap_context_t *context, const coap_address_t *dst, coap_pdu_t *request)
{
    coap_pdu_t *response;
    coap_tid_t result = COAP_INVALID_TID;

    if (request && request->hdr->coap_hdr_udp_t.type == COAP_MESSAGE_CON)
    {
        response = coap_pdu_init(COAP_MESSAGE_ACK, 0, request->hdr->coap_hdr_udp_t.id,
                                 sizeof(coap_pdu_t), coap_udp);
        if (response)
        {
            result = coap_send(context, dst, response);
            coap_delete_pdu(response);
        }
    }
    return result;
}

#if defined(WITH_ARDUINO)
coap_tid_t
coap_send_impl(coap_context_t *context,
               const coap_address_t *dst,
               coap_pdu_t *pdu)
{
    return 0;
}
#endif

#if defined(WITH_POSIX) || defined(WIN32)
/* releases space allocated by PDU if free_pdu is set */
coap_tid_t
coap_send_impl(coap_context_t *context,
        const coap_address_t *dst,
        coap_pdu_t *pdu)
{
    ssize_t bytes_written;
    coap_tid_t id = COAP_INVALID_TID;

    if ( !context || !dst || !pdu )
    return id;

    bytes_written = sendto( context->sockfd, pdu->hdr, pdu->length, 0,
            &dst->addr.sa, dst->size);

    if (bytes_written >= 0)
    {
        coap_transaction_id(dst, pdu, &id);
    }
    else
    {
        coap_log(LOG_CRIT, "coap_send: sendto\n");
    }

    return id;
}
#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_CONTIKI
/* releases space allocated by PDU if free_pdu is set */
coap_tid_t
coap_send_impl(coap_context_t *context,
        const coap_address_t *dst,
        coap_pdu_t *pdu)
{
    coap_tid_t id = COAP_INVALID_TID;

    if ( !context || !dst || !pdu )
    return id;

    /* FIXME: is there a way to check if send was successful? */
    uip_udp_packet_sendto(context->conn, pdu->hdr, pdu->length,
            &dst->addr, dst->port);

    coap_transaction_id(dst, pdu, &id);

    return id;
}
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP
coap_tid_t
coap_send_impl(coap_context_t *context,
        const coap_address_t *dst,
        coap_pdu_t *pdu)
{
    coap_tid_t id = COAP_INVALID_TID;
    struct pbuf *p;
    uint8_t err;
    char *data_backup;

    if ( !context || !dst || !pdu )
    {
        return id;
    }

    data_backup = pdu->data;

    /* FIXME: we can't check this here with the existing infrastructure, but we
     * should actually check that the pdu is not held by anyone but us. the
     * respective pbuf is already exclusively owned by the pdu. */

    p = pdu->pbuf;
    LWIP_ASSERT("The PDU header is not where it is expected", pdu->hdr == p->payload + sizeof(coap_pdu_t));

    err = pbuf_header(p, -sizeof(coap_pdu_t));
    if (err)
    {
        debug("coap_send_impl: pbuf_header failed\n");
        pbuf_free(p);
        return id;
    }

    coap_transaction_id(dst, pdu, &id);

    pbuf_realloc(p, pdu->length);

    udp_sendto(context->pcb, p,
            &dst->addr, dst->port);

    pbuf_header(p, -(ptrdiff_t)((uint8_t*)pdu - (uint8_t*)p->payload) - sizeof(coap_pdu_t)); /* FIXME hack around udp_sendto not restoring; see http://lists.gnu.org/archive/html/lwip-users/2013-06/msg00008.html. for udp over ip over ethernet, this was -42; as we're doing ppp too, this has to be calculated generically */

    err = pbuf_header(p, sizeof(coap_pdu_t));
    LWIP_ASSERT("Cannot undo pbuf_header", err == 0);

    /* restore destroyed pdu data */
    LWIP_ASSERT("PDU not restored", p->payload == pdu);
    pdu->max_size = p->tot_len - sizeof(coap_pdu_t); /* reduced after pbuf_realloc */
    pdu->hdr = p->payload + sizeof(coap_pdu_t);
    pdu->max_delta = 0; /* won't be used any more */
    pdu->length = pdu->max_size;
    pdu->data = data_backup;
    pdu->pbuf = p;

    return id;
}
#endif /* WITH_LWIP */

coap_tid_t coap_send(coap_context_t *context, const coap_address_t *dst, coap_pdu_t *pdu)
{
#ifndef WITH_ARDUINO
    return coap_send_impl(context, dst, pdu);
#endif
}

coap_tid_t coap_send_error(coap_context_t *context, coap_pdu_t *request, const coap_address_t *dst,
        unsigned char code, coap_opt_filter_t opts)
{
    coap_pdu_t *response;
    coap_tid_t result = COAP_INVALID_TID;

    assert(request);
    assert(dst);

    response = coap_new_error_response(request, code, opts);
    if (response)
    {
        result = coap_send(context, dst, response);
        coap_delete_pdu(response);
    }

    return result;
}

coap_tid_t coap_send_message_type(coap_context_t *context, const coap_address_t *dst,
        coap_pdu_t *request, unsigned char type)
{
    coap_pdu_t *response;
    coap_tid_t result = COAP_INVALID_TID;

    if (request)
    {
        response = coap_pdu_init(type, 0, request->hdr->coap_hdr_udp_t.id,
                                 sizeof(coap_pdu_t), coap_udp);
        if (response)
        {
            result = coap_send(context, dst, response);
            coap_delete_pdu(response);
        }
    }
    return result;
}

coap_tid_t coap_send_confirmed(coap_context_t *context, const coap_address_t *dst, coap_pdu_t *pdu)
{
    coap_queue_t *node;
    coap_tick_t now;
    int r;

    node = coap_new_node();
    if (!node)
    {
        debug("coap_send_confirmed: insufficient memory\n");
        return COAP_INVALID_TID;
    }

    node->id = coap_send_impl(context, dst, pdu);
    if (COAP_INVALID_TID == node->id)
    {
        debug("coap_send_confirmed: error sending pdu\n");
        coap_free_node(node);
        return COAP_INVALID_TID;
    }

    prng((unsigned char *)&r, sizeof(r));

    /* add randomized RESPONSE_TIMEOUT to determine retransmission timeout */
    node->timeout = COAP_DEFAULT_RESPONSE_TIMEOUT * COAP_TICKS_PER_SECOND
            + (COAP_DEFAULT_RESPONSE_TIMEOUT >> 1) * ((COAP_TICKS_PER_SECOND * (r & 0xFF)) >> 8);

    memcpy(&node->remote, dst, sizeof(coap_address_t));
    node->pdu = pdu;

    /* Set timer for pdu retransmission. If this is the first element in
     * the retransmission queue, the base time is set to the current
     * time and the retransmission time is node->timeout. If there is
     * already an entry in the sendqueue, we must check if this node is
     * to be retransmitted earlier. Therefore, node->timeout is first
     * normalized to the base time and then inserted into the queue with
     * an adjusted relative time.
     */coap_ticks(&now);
    if (context->sendqueue == NULL)
    {
        node->t = node->timeout;
        context->sendqueue_basetime = now;
    }
    else
    {
        /* make node->t relative to context->sendqueue_basetime */
        node->t = (now - context->sendqueue_basetime) + node->timeout;
    }

    coap_insert_node(&context->sendqueue, node);

#ifdef WITH_LWIP
    if (node == context->sendqueue) /* don't bother with timer stuff if there are earlier retransmits */
    coap_retransmittimer_restart(context);
#endif

#ifdef WITH_CONTIKI
    { /* (re-)initialize retransmission timer */
        coap_queue_t *nextpdu;

        nextpdu = coap_peek_next(context);
        assert(nextpdu); /* we have just inserted a node */

        /* must set timer within the context of the retransmit process */
        PROCESS_CONTEXT_BEGIN(&coap_retransmit_process);
        etimer_set(&context->retransmit_timer, nextpdu->t);
        PROCESS_CONTEXT_END(&coap_retransmit_process);
    }
#endif /* WITH_CONTIKI */

    return node->id;
}

coap_tid_t coap_retransmit(coap_context_t *context, coap_queue_t *node)
{
    if (!context || !node)
        return COAP_INVALID_TID;

    /* re-initialize timeout when maximum number of retransmissions are not reached yet */
    if (node->retransmit_cnt < COAP_DEFAULT_MAX_RETRANSMIT)
    {
        node->retransmit_cnt++;
        node->t = node->timeout << node->retransmit_cnt;
        coap_insert_node(&context->sendqueue, node);
#ifdef WITH_LWIP
        /* don't bother with timer stuff if there are earlier retransmits */
        if (node == context->sendqueue)
        coap_retransmittimer_restart(context);
#endif

        debug(
                "** retransmission #%d of transaction %d\n", node->retransmit_cnt,
                ntohs(node->pdu->hdr->coap_hdr_udp_t.id));

        node->id = coap_send_impl(context, &node->remote, node->pdu);
        return node->id;
    }

    /* no more retransmissions, remove node from system */

#if !defined(WITH_CONTIKI) && !defined(WITH_ARDUINO)
    debug("** removed transaction %d\n", ntohs(node->id));
#endif

#ifndef WITHOUT_OBSERVE
    /* Check if subscriptions exist that should be canceled after
     COAP_MAX_NOTIFY_FAILURES */
    if (node->pdu->hdr->coap_hdr_udp_t.code >= 64)
    {
        str token =
        { 0, NULL };

        token.length = node->pdu->hdr->coap_hdr_udp_t.token_length;
        token.s = node->pdu->hdr->coap_hdr_udp_t.token;

        coap_handle_failed_notify(context, &node->remote, &token);
    }
#endif /* WITHOUT_OBSERVE */

    /* And finally delete the node */
    coap_delete_node(node);
    return COAP_INVALID_TID;
}

/**
 * Checks if @p opt fits into the message that ends with @p maxpos.
 * This function returns @c 1 on success, or @c 0 if the option @p opt
 * would exceed @p maxpos.
 */
static inline int check_opt_size(coap_opt_t *opt, unsigned char *maxpos)
{
    if (opt && opt < maxpos)
    {
        if (((*opt & 0x0f) < 0x0f) || (opt + 1 < maxpos))
            return opt + COAP_OPT_SIZE(opt) < maxpos;
    }
    return 0;
}

#ifndef WITH_ARDUINO
int coap_read(coap_context_t *ctx)
{
#if defined(WITH_POSIX) || defined(WIN32)
    static char buf[COAP_MAX_PDU_SIZE];
#endif
#if defined(WITH_LWIP) || defined(WITH_CONTIKI)
    char *buf;
#endif
    coap_hdr_t *pdu;
    ssize_t bytes_read = -1;
    coap_address_t src, dst;
    coap_queue_t *node;

#ifdef WITH_CONTIKI
    buf = uip_appdata;
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP
    LWIP_ASSERT("No package pending", ctx->pending_package != NULL);
    LWIP_ASSERT("Can only deal with contiguous PBUFs to read the initial details", ctx->pending_package->tot_len == ctx->pending_package->len);
    buf = ctx->pending_package->payload;
#endif /* WITH_LWIP */

    pdu = (coap_hdr_t *) buf;

    coap_address_init(&src);

#if defined(WITH_POSIX) || defined(WIN32)
    bytes_read = recvfrom(ctx->sockfd, buf, sizeof(buf), 0, &src.addr.sa, &src.size);

#endif /* WITH_POSIX || WITH_ARDUINO */
#ifdef WITH_CONTIKI
    if(uip_newdata())
    {
        uip_ipaddr_copy(&src.addr, &UIP_IP_BUF->srcipaddr);
        src.port = UIP_UDP_BUF->srcport;
        uip_ipaddr_copy(&dst.addr, &UIP_IP_BUF->destipaddr);
        dst.port = UIP_UDP_BUF->destport;

        bytes_read = uip_datalen();
        ((char *)uip_appdata)[bytes_read] = 0;
        PRINTF("Server received %d bytes from [", (int)bytes_read);
        PRINT6ADDR(&src.addr);
        PRINTF("]:%d\n", uip_ntohs(src.port));
    }
#endif /* WITH_CONTIKI */
#ifdef WITH_LWIP
    /* FIXME: use lwip address operation functions */
    src.addr.addr = ctx->pending_address.addr;
    src.port = ctx->pending_port;
    bytes_read = ctx->pending_package->tot_len;
#endif /* WITH_LWIP */

    if (bytes_read < 0)
    {
        warn("coap_read: recvfrom");
        goto error_early;
    }

    if ((size_t) bytes_read < sizeof(coap_hdr_t))
    {
        debug("coap_read: discarded invalid frame\n");
        goto error_early;
    }

    if (pdu->coap_hdr_udp_t.version != COAP_DEFAULT_VERSION)
    {
        debug("coap_read: unknown protocol version\n");
        goto error_early;
    }

    node = coap_new_node();
    if (!node)
        goto error_early;

#ifdef WITH_LWIP
    node->pdu = coap_pdu_from_pbuf(ctx->pending_package);
    ctx->pending_package = NULL;
#else
    node->pdu = coap_pdu_init(0, 0, 0, bytes_read, coap_udp);
#endif
    if (!node->pdu)
        goto error;

    coap_ticks(&node->t);
    memcpy(&node->local, &dst, sizeof(coap_address_t));
    memcpy(&node->remote, &src, sizeof(coap_address_t));

    if (!coap_pdu_parse((unsigned char *) buf, bytes_read, node->pdu, coap_udp))
    {
        warn("discard malformed PDU");
        goto error;
    }

    /* and add new node to receive queue */
    coap_transaction_id(&node->remote, node->pdu, &node->id);
    coap_insert_node(&ctx->recvqueue, node);

#ifndef NDEBUG
    if (LOG_DEBUG <= coap_get_log_level())
    {
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 40
#endif
        unsigned char addr[INET6_ADDRSTRLEN + 8];

        if (coap_print_addr(&src, addr, INET6_ADDRSTRLEN + 8))
            debug("** received %d bytes from %s:\n", (int)bytes_read, addr);

        coap_show_pdu(node->pdu);
    }
#endif

    return 0;

    error:
    /* FIXME: send back RST? */
    coap_delete_node(node);
    return -1;
    error_early:
#ifdef WITH_LWIP
    /* even if there was an error, clean up */
    pbuf_free(ctx->pending_package);
    ctx->pending_package = NULL;
#endif
    return -1;
}
#endif //#ifndef WITH_ARDUINO

int coap_remove_from_queue(coap_queue_t **queue, coap_tid_t id, coap_queue_t **node)
{
    coap_queue_t *p, *q;

    if (!queue || !*queue)
        return 0;

    /* replace queue head if PDU's time is less than head's time */

    if (id == (*queue)->id)
    { /* found transaction */
        *node = *queue;
        *queue = (*queue)->next;
        if (*queue)
        { /* adjust relative time of new queue head */
            (*queue)->t += (*node)->t;
        }
        (*node)->next = NULL;
        /* coap_delete_node( q ); */
        debug("*** removed transaction %u\n", id);
        return 1;
    }

    /* search transaction to remove (only first occurence will be removed) */
    q = *queue;
    do
    {
        p = q;
        q = q->next;
    } while (q && id != q->id);

    if (q)
    { /* found transaction */
        p->next = q->next;
        if (p->next)
        { /* must update relative time of p->next */
            p->next->t += q->t;
        }
        q->next = NULL;
        *node = q;
        /* coap_delete_node( q ); */
        debug("*** removed transaction %u\n", id);
        return 1;
    }

    return 0;

}

static inline int token_match(const unsigned char *a, size_t alen, const unsigned char *b,
        size_t blen)
{
    return alen == blen && (alen == 0 || memcmp(a, b, alen) == 0);
}

void coap_cancel_all_messages(coap_context_t *context, const coap_address_t *dst,
        const unsigned char *token, size_t token_length)
{
    /* cancel all messages in sendqueue that are for dst
     * and use the specified token */
    coap_queue_t *p, *q;

    debug("cancel_all_messages\n");
    while (context->sendqueue && coap_address_equals(dst, &context->sendqueue->remote)
            && token_match(token, token_length, context->sendqueue->pdu->hdr->coap_hdr_udp_t.token,
                    context->sendqueue->pdu->hdr->coap_hdr_udp_t.token_length))
    {
        q = context->sendqueue;
        context->sendqueue = q->next;
        debug("**** removed transaction %d\n", ntohs(q->pdu->hdr->coap_hdr_udp_t.id));
        coap_delete_node(q);
    }

    if (!context->sendqueue)
        return;

    p = context->sendqueue;
    q = p->next;

    /* when q is not NULL, it does not match (dst, token), so we can skip it */
    while (q)
    {
        if (coap_address_equals(dst, &q->remote)
                && token_match(token, token_length, q->pdu->hdr->coap_hdr_udp_t.token,
                               q->pdu->hdr->coap_hdr_udp_t.token_length))
        {
            p->next = q->next;
            debug("**** removed transaction %d\n", ntohs(q->pdu->hdr->coap_hdr_udp_t.id));
            coap_delete_node(q);
            q = p->next;
        }
        else
        {
            p = q;
            q = q->next;
        }
    }
}

coap_queue_t *
coap_find_transaction(coap_queue_t *queue, coap_tid_t id)
{
    while (queue && queue->id != id)
        queue = queue->next;

    return queue;
}

coap_pdu_t *
coap_new_error_response(coap_pdu_t *request, unsigned char code, coap_opt_filter_t opts)
{
    coap_opt_iterator_t opt_iter;
    coap_pdu_t *response;
    size_t size = sizeof(coap_hdr_t) + request->hdr->coap_hdr_udp_t.token_length;
    int type;
    coap_opt_t *option;
    unsigned short opt_type = 0; /* used for calculating delta-storage */

#if COAP_ERROR_PHRASE_LENGTH > 0
    char *phrase = coap_response_phrase(code);

    /* Need some more space for the error phrase and payload start marker */
    if (phrase)
        size += strlen(phrase) + 1;
#endif

    assert(request);

    /* cannot send ACK if original request was not confirmable */
    type = request->hdr->coap_hdr_udp_t.type == COAP_MESSAGE_CON ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON;

    /* Estimate how much space we need for options to copy from
     * request. We always need the Token, for 4.02 the unknown critical
     * options must be included as well. */
    coap_option_clrb(opts, COAP_OPTION_CONTENT_TYPE); /* we do not want this */

    coap_option_iterator_init(request, &opt_iter, opts, coap_udp);

    /* Add size of each unknown critical option. As known critical
     options as well as elective options are not copied, the delta
     value might grow.
     */
    while ((option = coap_option_next(&opt_iter)))
    {
        unsigned short delta = opt_iter.type - opt_type;
        /* calculate space required to encode (opt_iter.type - opt_type) */
        if (delta < 13)
        {
            size++;
        }
        else if (delta < 269)
        {
            size += 2;
        }
        else
        {
            size += 3;
        }

        /* add coap_opt_length(option) and the number of additional bytes
         * required to encode the option length */

        size += coap_opt_length(option);
        switch (*option & 0x0f)
        {
            case 0x0e:
                size++;
                /* fall through */
            case 0x0d:
                size++;
                break;
            default:
                ;
        }

        opt_type = opt_iter.type;
    }

    /* Now create the response and fill with options and payload data. */
    response = coap_pdu_init(type, code, request->hdr->coap_hdr_udp_t.id, size, coap_udp);
    if (response)
    {
        /* copy token */
        if (!coap_add_token(response, request->hdr->coap_hdr_udp_t.token_length,
                            request->hdr->coap_hdr_udp_t.token, coap_udp))
        {
            debug("cannot add token to error response\n");
            coap_delete_pdu(response);
            return NULL;
        }

        /* copy all options */
        coap_option_iterator_init(request, &opt_iter, opts, coap_udp);
        while ((option = coap_option_next(&opt_iter)))
            coap_add_option(response, opt_iter.type, COAP_OPT_LENGTH(option),
                    COAP_OPT_VALUE(option), coap_udp);

#if COAP_ERROR_PHRASE_LENGTH > 0
        /* note that diagnostic messages do not need a Content-Format option. */
        if (phrase)
            coap_add_data(response, strlen(phrase), (unsigned char *) phrase);
#endif
    }

    return response;
}

/**
 * Quick hack to determine the size of the resource description for
 * .well-known/core.
 */
static inline size_t get_wkc_len(coap_context_t *context, coap_opt_t *query_filter)
{
    unsigned char buf[1];
    size_t len = 0;

    if (print_wellknown(context, buf, &len, UINT_MAX, query_filter) & COAP_PRINT_STATUS_ERROR)
    {
        warn("cannot determine length of /.well-known/core\n");
        return 0;
    }

    debug("get_wkc_len: print_wellknown() returned %zu\n", len);

    return len;
}

#define SZX_TO_BYTES(SZX) ((size_t)(1 << ((SZX) + 4)))

coap_pdu_t *
wellknown_response(coap_context_t *context, coap_pdu_t *request)
{
    coap_pdu_t *resp;
    coap_opt_iterator_t opt_iter;
    size_t len, wkc_len;
    unsigned char buf[2];
    int result = 0;
    int need_block2 = 0; /* set to 1 if Block2 option is required */
    coap_block_t block;
    coap_opt_t *query_filter;
    size_t offset = 0;

    resp = coap_pdu_init(
            request->hdr->coap_hdr_udp_t.type == COAP_MESSAGE_CON ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON,
            COAP_RESPONSE_CODE(205),
            request->hdr->coap_hdr_udp_t.id, COAP_MAX_PDU_SIZE, coap_udp);
    if (!resp)
    {
        debug("wellknown_response: cannot create PDU\n");
        return NULL;
    }

    if (!coap_add_token(resp, request->hdr->coap_hdr_udp_t.token_length,
                        request->hdr->coap_hdr_udp_t.token, coap_udp))
    {
        debug("wellknown_response: cannot add token\n");
        goto error;
    }

    query_filter = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
    wkc_len = get_wkc_len(context, query_filter);

    /* check whether the request contains the Block2 option */
    if (coap_get_block(request, COAP_OPTION_BLOCK2, &block))
    {
        offset = block.num << (block.szx + 4);
        if (block.szx > 6)
        { /* invalid, MUST lead to 4.00 Bad Request */
            resp->hdr->coap_hdr_udp_t.code = COAP_RESPONSE_CODE(400);
            return resp;
        }
        else if (block.szx > COAP_MAX_BLOCK_SZX)
        {
            block.szx = COAP_MAX_BLOCK_SZX;
            block.num = offset >> (block.szx + 4);
        }

        need_block2 = 1;
    }

    /* Check if there is sufficient space to add Content-Format option
     * and data. We do this before adding the Content-Format option to
     * avoid sending error responses with that option but no actual
     * content. */
    if (resp->max_size <= (size_t) resp->length + 3)
    {
        debug("wellknown_response: insufficient storage space\n");
        goto error;
    }

    /* Add Content-Format. As we have checked for available storage,
     * nothing should go wrong here. */
    assert(coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_LINK_FORMAT) == 1);
    coap_add_option(resp, COAP_OPTION_CONTENT_FORMAT,
            coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_LINK_FORMAT), buf, coap_udp);

    /* check if Block2 option is required even if not requested */
    if (!need_block2 && (resp->max_size - (size_t) resp->length < wkc_len))
    {
        assert(resp->length <= resp->max_size);
        const size_t payloadlen = resp->max_size - resp->length;
        /* yes, need block-wise transfer */
        block.num = 0;
        block.m = 0; /* the M bit is set by coap_write_block_opt() */
        block.szx = COAP_MAX_BLOCK_SZX;
        while (payloadlen < SZX_TO_BYTES(block.szx))
        {
            if (block.szx == 0)
            {
                debug("wellknown_response: message to small even for szx == 0\n");
                goto error;
            }
            else
            {
                block.szx--;
            }
        }

        need_block2 = 1;
    }

    /* write Block2 option if necessary */
    if (need_block2)
    {
        if (coap_write_block_opt(&block, COAP_OPTION_BLOCK2, resp, wkc_len) < 0)
        {
            debug("wellknown_response: cannot add Block2 option\n");
            goto error;
        }
    }

    /* Manually set payload of response to let print_wellknown() write,
     * into our buffer without copying data. */

    resp->data = (unsigned char *) resp->hdr + resp->length;
    *resp->data = COAP_PAYLOAD_START;
    resp->data++;
    resp->length++;
    len = need_block2 ? SZX_TO_BYTES(block.szx) : resp->max_size - resp->length;

    result = print_wellknown(context, resp->data, &len, offset, query_filter);
    if ((result & COAP_PRINT_STATUS_ERROR) != 0)
    {
        debug("print_wellknown failed\n");
        goto error;
    }

    resp->length += COAP_PRINT_OUTPUT_LENGTH(result);
    return resp;

    error:
    /* set error code 5.03 and remove all options and data from response */
    resp->hdr->coap_hdr_udp_t.code = COAP_RESPONSE_CODE(503);
    resp->length = sizeof(coap_hdr_t) + resp->hdr->coap_hdr_udp_t.token_length;
    return resp;
}

#define WANT_WKC(Pdu,Key)                   \
  (((Pdu)->hdr->coap_hdr_udp_t.code == COAP_REQUEST_GET) && is_wkc(Key))

void handle_request(coap_context_t *context, coap_queue_t *node, const char* responseData)
{

    coap_method_handler_t h = NULL;
    coap_pdu_t *response = NULL;
    coap_opt_filter_t opt_filter;
    coap_resource_t *resource;
    coap_key_t key;

    coap_option_filter_clear(opt_filter);

    /* try to find the resource from the request URI */
    coap_hash_request_uri(node->pdu, key);
    resource = coap_get_resource_from_key(context, key);

    if (!resource)
    {
        /* The resource was not found. Check if the request URI happens to
         * be the well-known URI. In that case, we generate a default
         * response, otherwise, we return 4.04 */

        switch (node->pdu->hdr->coap_hdr_udp_t.code)
        {

            case COAP_REQUEST_GET:
                if (is_wkc(key))
                { /* GET request for .well-known/core */
                    info("create default response for %s\n", COAP_DEFAULT_URI_WELLKNOWN);

                    response = wellknown_response(context, node->pdu);

                }
                else
                { /* GET request for any another resource, return 4.04 */

                    debug(
                            "GET for unknown resource 0x%02x%02x%02x%02x, return 4.04\n", key[0], key[1], key[2], key[3]);
                    response = coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(404),
                            opt_filter);
                }
                break;

            default: /* any other request type */

                debug(
                        "unhandled request for unknown resource 0x%02x%02x%02x%02x\r\n", key[0], key[1], key[2], key[3]);
                if (!coap_is_mcast(&node->local))
                    response = coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(405),
                            opt_filter);
        }

        if (response && coap_send(context, &node->remote, response) == COAP_INVALID_TID)
        {
            warn("cannot send response for transaction %u\n", node->id);
        }
        coap_delete_pdu(response);

        return;
    }

    /* the resource was found, check if there is a registered handler */
    if ((size_t) node->pdu->hdr->coap_hdr_udp_t.code - 1
            < sizeof(resource->handler) / sizeof(coap_method_handler_t))
        h = resource->handler[node->pdu->hdr->coap_hdr_udp_t.code - 1];

    if (h)
    {
        debug(
                "call custom handler for resource 0x%02x%02x%02x%02x\n", key[0], key[1], key[2], key[3]);
        response = coap_pdu_init(
                node->pdu->hdr->coap_hdr_udp_t.type ==
                        COAP_MESSAGE_CON ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON,
                0, node->pdu->hdr->coap_hdr_udp_t.id, COAP_MAX_PDU_SIZE, coap_udp);

        /* Implementation detail: coap_add_token() immediately returns 0
         if response == NULL */
        if (coap_add_token(response, node->pdu->hdr->coap_hdr_udp_t.token_length,
                           node->pdu->hdr->coap_hdr_udp_t.token, coap_udp))
        {
            str token =
            { node->pdu->hdr->coap_hdr_udp_t.token_length, node->pdu->hdr->coap_hdr_udp_t.token };

            h(context, resource, &node->remote, node->pdu, &token, response);

            unsigned char buf[3];
            response->hdr->coap_hdr_udp_t.code = COAP_RESPONSE_CODE(205);
            coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                    coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf, coap_udp);
            coap_add_option(response, COAP_OPTION_MAXAGE, coap_encode_var_bytes(buf, 0x2ffff), buf, coap_udp);
            coap_add_data(response, strlen(responseData), (unsigned char *) responseData);

            if (response->hdr->coap_hdr_udp_t.type != COAP_MESSAGE_NON
                    || (response->hdr->coap_hdr_udp_t.code >= 64 && !coap_is_mcast(&node->local)))
            {

                if (coap_send(context, &node->remote, response) == COAP_INVALID_TID)
                {
                    debug("cannot send response for message %d\n", node->pdu->hdr->coap_hdr_udp_t.id);
                }
            }

            coap_delete_pdu(response);
        }
        else
        {
            warn("cannot generate response\r\n");
        }
    }
    else
    {
        if (WANT_WKC(node->pdu, key))
        {
            debug("create default response for %s\n", COAP_DEFAULT_URI_WELLKNOWN);
            response = wellknown_response(context, node->pdu);
        }
        else
            response = coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(405), opt_filter);

        if (!response || (coap_send(context, &node->remote, response) == COAP_INVALID_TID))
        {
            debug("cannot send response for transaction %u\n", node->id);
        }
        coap_delete_pdu(response);
    }
}

static inline void handle_response(coap_context_t *context, coap_queue_t *sent, coap_queue_t *rcvd)
{

    /* Call application-specific reponse handler when available.  If
     * not, we must acknowledge confirmable messages. */
    if (context->response_handler)
    {

        context->response_handler(context, &rcvd->remote, sent ? sent->pdu : NULL, rcvd->pdu,
                rcvd->id);
    }
    else
    {
        /* send ACK if rcvd is confirmable (i.e. a separate response) */
        coap_send_ack(context, &rcvd->remote, rcvd->pdu);
    }
}

static inline int
#ifdef __GNUC__
handle_locally(coap_context_t *context __attribute__ ((unused)),
        coap_queue_t *node __attribute__ ((unused)))
{
#else /* not a GCC */
    handle_locally(coap_context_t *context, coap_queue_t *node)
    {
#endif /* GCC */
        /* this function can be used to check if node->pdu is really for us */
        return 1;
    }

    /**
     * This function handles RST messages received for the message passed
     * in @p sent.
     */
    static void coap_handle_rst(coap_context_t *context, const coap_queue_t *sent)
    {
#ifndef WITHOUT_OBSERVE
        coap_resource_t *r;
#ifndef COAP_RESOURCES_NOHASH
        coap_resource_t *tmp;
#endif
        str token =
        { 0, NULL };

        /* remove observer for this resource, if any
         * get token from sent and try to find a matching resource. Uh!
         */

        COAP_SET_STR(&token, sent->pdu->hdr->coap_hdr_udp_t.token_length,
                     sent->pdu->hdr->coap_hdr_udp_t.token);

#ifndef WITH_CONTIKI
#ifdef COAP_RESOURCES_NOHASH
        LL_FOREACH(context->resources, r)
        {
#else
        HASH_ITER(hh, context->resources, r, tmp)
        {
#endif
            coap_delete_observer(r, &sent->remote, &token);
            coap_cancel_all_messages(context, &sent->remote, token.s, token.length);
        }
#else /* WITH_CONTIKI */
        r = (coap_resource_t *)resource_storage.mem;
        for (i = 0; i < resource_storage.num; ++i, ++r)
        {
            if (resource_storage.count[i])
            {
                coap_delete_observer(r, &sent->remote, &token);
                coap_cancel_all_messages(context, &sent->remote, token.s, token.length);
            }
        }
#endif /* WITH_CONTIKI */
#endif /* WITOUT_OBSERVE */
    }

    void coap_dispatch(coap_context_t *context, const char* responseData)
    {
        coap_queue_t *rcvd = NULL, *sent = NULL;
        coap_pdu_t *response;
        coap_opt_filter_t opt_filter;

        if (!context)
            return;

        memset(opt_filter, 0, sizeof(coap_opt_filter_t));

        while (context->recvqueue)
        {
            rcvd = context->recvqueue;

            /* remove node from recvqueue */
            context->recvqueue = context->recvqueue->next;
            rcvd->next = NULL;

            if (rcvd->pdu->hdr->coap_hdr_udp_t.version != COAP_DEFAULT_VERSION)
            {
                debug("dropped packet with unknown version %u\n", rcvd->pdu->hdr->coap_hdr_udp_t.version);
                goto cleanup;
            }

            switch (rcvd->pdu->hdr->coap_hdr_udp_t.type)
            {
                case COAP_MESSAGE_ACK:
                    /* find transaction in sendqueue to stop retransmission */
                    coap_remove_from_queue(&context->sendqueue, rcvd->id, &sent);

                    if (rcvd->pdu->hdr->coap_hdr_udp_t.code == 0)
                        goto cleanup;

                    /* FIXME: if sent code was >= 64 the message might have been a
                     * notification. Then, we must flag the observer to be alive
                     * by setting obs->fail_cnt = 0. */
                    if (sent && COAP_RESPONSE_CLASS(sent->pdu->hdr->coap_hdr_udp_t.code) == 2)
                    {
                        const str token =
                        { sent->pdu->hdr->coap_hdr_udp_t.token_length,
                          sent->pdu->hdr->coap_hdr_udp_t.token };

                        coap_touch_observer(context, &sent->remote, &token);
                    }
                    break;

                case COAP_MESSAGE_RST:
                    /* We have sent something the receiver disliked, so we remove
                     * not only the transaction but also the subscriptions we might
                     * have. */

                    coap_log(LOG_ALERT, "got RST for message %u\n",
                             ntohs(rcvd->pdu->hdr->coap_hdr_udp_t.id));

                    /* find transaction in sendqueue to stop retransmission */
                    coap_remove_from_queue(&context->sendqueue, rcvd->id, &sent);

                    if (sent)
                        coap_handle_rst(context, sent);
                    goto cleanup;

                case COAP_MESSAGE_NON: /* check for unknown critical options */
                    if (coap_option_check_critical(context, rcvd->pdu, opt_filter) == 0)
                        goto cleanup;
                    break;

                case COAP_MESSAGE_CON: /* check for unknown critical options */
                    if (coap_option_check_critical(context, rcvd->pdu, opt_filter) == 0)
                    {

                        /* FIXME: send response only if we have received a request. Otherwise,
                         * send RST. */
                        response = coap_new_error_response(rcvd->pdu, COAP_RESPONSE_CODE(402),
                                opt_filter);

                        if (!response)
                            warn("coap_dispatch: cannot create error reponse\n");
                        else
                        {
                            if (coap_send(context, &rcvd->remote, response) == COAP_INVALID_TID)
                            {
                                warn("coap_dispatch: error sending reponse\n");
                            }
                            coap_delete_pdu(response);
                        }

                        goto cleanup;
                    }
                    break;
            }

            /* Pass message to upper layer if a specific handler was
             * registered for a request that should be handled locally. */
            if (handle_locally(context, rcvd))
            {
                if (COAP_MESSAGE_IS_REQUEST(rcvd->pdu->hdr->coap_hdr_udp_t))
                    handle_request(context, rcvd, responseData);
                else if (COAP_MESSAGE_IS_RESPONSE(rcvd->pdu->hdr->coap_hdr_udp_t))
                    handle_response(context, sent, rcvd);
                else
                {
                    debug("dropped message with invalid code\n");
                    coap_send_message_type(context, &rcvd->remote, rcvd->pdu, COAP_MESSAGE_RST);
                }
            }

            cleanup: coap_delete_node(sent);
            coap_delete_node(rcvd);
        }
    }

    int coap_can_exit(coap_context_t *context)
    {
        return !context || (context->recvqueue == NULL && context->sendqueue == NULL);
    }

#ifdef WITH_CONTIKI

    /*---------------------------------------------------------------------------*/
    /* CoAP message retransmission */
    /*---------------------------------------------------------------------------*/
    PROCESS_THREAD(coap_retransmit_process, ev, data)
    {
        coap_tick_t now;
        coap_queue_t *nextpdu;

        PROCESS_BEGIN();

        debug("Started retransmit process\r\n");

        while(1)
        {
            PROCESS_YIELD();
            if (ev == PROCESS_EVENT_TIMER)
            {
                if (etimer_expired(&the_coap_context.retransmit_timer))
                {

                    nextpdu = coap_peek_next(&the_coap_context);

                    coap_ticks(&now);
                    while (nextpdu && nextpdu->t <= now)
                    {
                        coap_retransmit(&the_coap_context, coap_pop_next(&the_coap_context));
                        nextpdu = coap_peek_next(&the_coap_context);
                    }

                    /* need to set timer to some value even if no nextpdu is available */
                    etimer_set(&the_coap_context.retransmit_timer,
                            nextpdu ? nextpdu->t - now : 0xFFFF);
                }
#ifndef WITHOUT_OBSERVE
                if (etimer_expired(&the_coap_context.notify_timer))
                {
                    coap_check_notify(&the_coap_context);
                    etimer_reset(&the_coap_context.notify_timer);
                }
#endif /* WITHOUT_OBSERVE */
            }
        }

        PROCESS_END();
    }
    /*---------------------------------------------------------------------------*/

#endif /* WITH_CONTIKI */

#ifdef WITH_LWIP
    /* FIXME: retransmits that are not required any more due to incoming packages
     * do *not* get cleared at the moment, the wakeup when the transmission is due
     * is silently accepted. this is mainly due to the fact that the required
     * checks are similar in two places in the code (when receiving ACK and RST)
     * and that they cause more than one patch chunk, as it must be first checked
     * whether the sendqueue item to be dropped is the next one pending, and later
     * the restart function has to be called. nothing insurmountable, but it can
     * also be implemented when things have stabilized, and the performance
     * penality is minimal
     *
     * also, this completely ignores COAP_RESOURCE_CHECK_TIME.
     * */

    static void coap_retransmittimer_execute(void *arg)
    {
        coap_context_t *ctx = (coap_context_t*)arg;
        coap_tick_t now;
        coap_tick_t elapsed;
        coap_queue_t *nextinqueue;

        ctx->timer_configured = 0;

        coap_ticks(&now);

        elapsed = now - ctx->sendqueue_basetime; /* that's positive for sure, and unless we haven't been called for a complete wrapping cycle, did not wrap */

        nextinqueue = coap_peek_next(ctx);
        while (nextinqueue != NULL)
        {
            if (nextinqueue->t > elapsed)
            {
                nextinqueue->t -= elapsed;
                break;
            }
            else
            {
                elapsed -= nextinqueue->t;
                coap_retransmit(ctx, coap_pop_next(ctx));
                nextinqueue = coap_peek_next(ctx);
            }
        }

        ctx->sendqueue_basetime = now;

        coap_retransmittimer_restart(ctx);
    }

    static void coap_retransmittimer_restart(coap_context_t *ctx)
    {
        coap_tick_t now, elapsed, delay;

        if (ctx->timer_configured)
        {
            printf("clearing\n");
            sys_untimeout(coap_retransmittimer_execute, (void*)ctx);
            ctx->timer_configured = 0;
        }
        if (ctx->sendqueue != NULL)
        {
            coap_ticks(&now);
            elapsed = now - ctx->sendqueue_basetime;
            if (ctx->sendqueue->t >= elapsed)
            {
                delay = ctx->sendqueue->t - elapsed;
            }
            else
            {
                /* a strange situation, but not completely impossible.
                 *
                 * this happens, for example, right after
                 * coap_retransmittimer_execute, when a retransmission
                 * was *just not yet* due, and the clock ticked before
                 * our coap_ticks was called.
                 *
                 * not trying to retransmit anything now, as it might
                 * cause uncontrollable recursion; let's just try again
                 * with the next main loop run.
                 * */
                delay = 0;
            }

            printf("scheduling for %d ticks\n", delay);
            sys_timeout(delay, coap_retransmittimer_execute, (void*)ctx);
            ctx->timer_configured = 1;
        }
    }
#endif

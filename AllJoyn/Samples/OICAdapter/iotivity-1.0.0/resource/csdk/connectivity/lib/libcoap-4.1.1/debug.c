/* debug.c -- debug utilities
 *
 * Copyright (C) 2010--2012 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "config.h"

#if defined(HAVE_ASSERT_H) && !defined(assert)
# include <assert.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include "debug.h"
#include "net.h"

#ifdef WITH_CONTIKI
# ifndef DEBUG
#  define DEBUG DEBUG_PRINT
# endif /* DEBUG */
#include "net/uip-debug.h"
#endif

static coap_log_t maxlog = LOG_WARNING; /* default maximum log level */

coap_log_t coap_get_log_level()
{
    return maxlog;
}

void coap_set_log_level(coap_log_t level)
{
    maxlog = level;
}

/* this array has the same order as the type log_t */
static char *loglevels[] =
{ "EMRG", "ALRT", "CRIT", "ERR", "WARN", "NOTE", "INFO", "DEBG" };

#ifdef HAVE_TIME_H

static inline size_t print_timestamp(char *s, size_t len, coap_tick_t t)
{
    struct tm *tmp;
    time_t now = clock_offset + (t / COAP_TICKS_PER_SECOND);
    tmp = localtime(&now);
    return strftime(s, len, "%b %d %H:%M:%S", tmp);
}

#else /* alternative implementation: just print the timestamp */

static inline size_t
print_timestamp(char *s, size_t len, coap_tick_t t)
{
#ifdef HAVE_SNPRINTF
    return snprintf(s, len, "%u.%03u",
            (unsigned int)(clock_offset + (t / COAP_TICKS_PER_SECOND)),
            (unsigned int)(t % COAP_TICKS_PER_SECOND));
#else /* HAVE_SNPRINTF */
    /* @todo do manual conversion of timestamp */
    return 0;
#endif /* HAVE_SNPRINTF */
}

#endif /* HAVE_TIME_H */

#ifndef NDEBUG

#ifndef HAVE_STRNLEN
/**
 * A length-safe strlen() fake.
 *
 * @param s      The string to count characters != 0.
 * @param maxlen The maximum length of @p s.
 *
 * @return The length of @p s.
 */
static inline size_t
strnlen(const char *s, size_t maxlen)
{
    size_t n = 0;
    while(*s++ && n < maxlen)
    ++n;
    return n;
}
#endif /* HAVE_STRNLEN */

unsigned int print_readable(const unsigned char *data, unsigned int len, unsigned char *result,
        unsigned int buflen, int encode_always)
{
    const unsigned char hex[] = "0123456789ABCDEF";
    unsigned int cnt = 0;
    assert(data || len == 0);

    if (buflen == 0 || len == 0)
        return 0;

    while (len)
    {
        if (!encode_always && isprint(*data))
        {
            if (cnt == buflen)
                break;
            *result++ = *data;
            ++cnt;
        }
        else
        {
            if (cnt + 4 < buflen)
            {
                *result++ = '\\';
                *result++ = 'x';
                *result++ = hex[(*data & 0xf0) >> 4];
                *result++ = hex[*data & 0x0f];
                cnt += 4;
            }
            else
                break;
        }

        ++data;
        --len;
    }

    *result = '\0';
    return cnt;
}

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

size_t coap_print_addr(const struct coap_address_t *addr, unsigned char *buf, size_t len)
{
#if defined (HAVE_ARPA_INET_H) || defined(WIN32)
    const void *addrptr = NULL;
#if defined(__ANDROID__)
    __uint16_t port;
#elif defined(WIN32)
    uint16_t port;
#else
    in_port_t port;
#endif
    unsigned char *p = buf;

    switch (addr->addr.sa.sa_family)
    {
        case AF_INET:
            addrptr = &addr->addr.sin.sin_addr;
            port = ntohs(addr->addr.sin.sin_port);
            break;
        case AF_INET6:
            if (len < 7) /* do not proceed if buffer is even too short for [::]:0 */
                return 0;

            *p++ = '[';

            addrptr = &addr->addr.sin6.sin6_addr;
            port = ntohs(addr->addr.sin6.sin6_port);

            break;
        default:
            memcpy(buf, "(unknown address type)", min(22, len));
            return min(22, len);
    }

    if (inet_ntop(addr->addr.sa.sa_family, addrptr, (char *) p, len) == 0)
    {
        perror("coap_print_addr");
        return 0;
    }

    p += strnlen((char *) p, len);

    if (addr->addr.sa.sa_family == AF_INET6)
    {
        if (p < buf + len)
        {
            *p++ = ']';
        }
        else
            return 0;
    }

    p += snprintf((char *) p, buf + len - p + 1, ":%d", port);

    return buf + len - p;
#else /* HAVE_ARPA_INET_H */
# if WITH_CONTIKI
    unsigned char *p = buf;
    uint8_t i;
#  if WITH_UIP6
    const unsigned char hex[] = "0123456789ABCDEF";

    if (len < 41)
    return 0;

    *p++ = '[';

    for (i=0; i < 16; i += 2)
    {
        if (i)
        {
            *p++ = ':';
        }
        *p++ = hex[(addr->addr.u8[i] & 0xf0) >> 4];
        *p++ = hex[(addr->addr.u8[i] & 0x0f)];
        *p++ = hex[(addr->addr.u8[i+1] & 0xf0) >> 4];
        *p++ = hex[(addr->addr.u8[i+1] & 0x0f)];
    }
    *p++ = ']';
#  else /* WITH_UIP6 */
#   warning "IPv4 network addresses will not be included in debug output"

    if (len < 21)
    return 0;
#  endif /* WITH_UIP6 */
    if (buf + len - p < 6)
    return 0;

#ifdef HAVE_SNPRINTF
    p += snprintf((char *)p, buf + len - p + 1, ":%d", uip_htons(addr->port));
#else /* HAVE_SNPRINTF */
    /* @todo manual conversion of port number */
#endif /* HAVE_SNPRINTF */

    return p - buf;
# else /* WITH_CONTIKI */
    /* TODO: output addresses manually */
#   warning "inet_ntop() not available, network addresses will not be included in debug output"
# endif /* WITH_CONTIKI */
    return 0;
#endif
}

#ifndef WITH_CONTIKI
void coap_show_pdu(const coap_pdu_t *pdu)
{
#ifndef WITH_TCP
    unsigned char buf[COAP_MAX_PDU_SIZE]; /* need some space for output creation */
#else
    unsigned char buf[COAP_TCP_LENGTH_LIMIT_32_BIT]; /* need some space for output creation */
#endif
    int encode = 0, have_options = 0;
    coap_opt_iterator_t opt_iter;
    coap_opt_t *option;

    fprintf(COAP_DEBUG_FD, "v:%d t:%d tkl:%d c:%d id:%u", pdu->hdr->coap_hdr_udp_t.version,
            pdu->hdr->coap_hdr_udp_t.type, pdu->hdr->coap_hdr_udp_t.token_length,
            pdu->hdr->coap_hdr_udp_t.code, ntohs(pdu->hdr->coap_hdr_udp_t.id));

    /* show options, if any */
    coap_option_iterator_init((coap_pdu_t *) pdu, &opt_iter, COAP_OPT_ALL, coap_udp);

    while ((option = coap_option_next(&opt_iter)))
    {
        if (!have_options)
        {
            have_options = 1;
            fprintf(COAP_DEBUG_FD, " o: [");
        }
        else
        {
            fprintf(COAP_DEBUG_FD, ",");
        }

        if (opt_iter.type == COAP_OPTION_URI_PATH || opt_iter.type == COAP_OPTION_PROXY_URI
                || opt_iter.type == COAP_OPTION_URI_HOST
                || opt_iter.type == COAP_OPTION_LOCATION_PATH
                || opt_iter.type == COAP_OPTION_LOCATION_QUERY
                || opt_iter.type == COAP_OPTION_URI_PATH || opt_iter.type == COAP_OPTION_URI_QUERY)
        {
            encode = 0;
        }
        else
        {
            encode = 1;
        }

        if (print_readable(COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option), buf, sizeof(buf),
                encode))
            fprintf(COAP_DEBUG_FD, " %d:'%s'", opt_iter.type, buf);
    }

    if (have_options)
        fprintf(COAP_DEBUG_FD, " ]");

    if (pdu->data)
    {
        assert(pdu->data < (unsigned char *) pdu->hdr + pdu->length);
        print_readable(pdu->data, (unsigned char *) pdu->hdr + pdu->length - pdu->data, buf,
                sizeof(buf), 0);
        fprintf(COAP_DEBUG_FD, " d:%s", buf);
    }
    fprintf(COAP_DEBUG_FD, "\n");
    fflush(COAP_DEBUG_FD);
}

#else /* WITH_CONTIKI */

void
coap_show_pdu(const coap_pdu_t *pdu)
{
    unsigned char buf[80]; /* need some space for output creation */

    PRINTF("v:%d t:%d oc:%d c:%d id:%u",
            pdu->hdr->coap_hdr_udp_t.version, pdu->hdr->coap_hdr_udp_t.type,
            pdu->hdr->coap_hdr_udp_t.optcnt, pdu->hdr->coap_hdr_udp_t.code,
            uip_ntohs(pdu->hdr->coap_hdr_udp_t.id));

    /* show options, if any */
    if (pdu->hdr->coap_hdr_udp_t.optcnt)
    {
        coap_opt_iterator_t opt_iter;
        coap_opt_t *option;
        coap_option_iterator_init((coap_pdu_t *)pdu, &opt_iter, COAP_OPT_ALL, coap_udp);

        PRINTF(" o:");
        while ((option = coap_option_next(&opt_iter)))
        {

            if (print_readable(COAP_OPT_VALUE(option),
                            COAP_OPT_LENGTH(option),
                            buf, sizeof(buf), 0))
            PRINTF(" %d:%s", opt_iter.type, buf);
        }
    }

    if (pdu->data < (unsigned char *)pdu->hdr + pdu->length)
    {
        print_readable(pdu->data,
                (unsigned char *)pdu->hdr + pdu->length - pdu->data,
                buf, sizeof(buf), 0 );
        PRINTF(" d:%s", buf);
    }
    PRINTF("\r\n");
}
#endif /* WITH_CONTIKI */

#endif /* NDEBUG */

#ifdef WITH_ARDUINO
void coap_log_impl(coap_log_t level, const char *format, ...)
{
    //TODO: Implement logging functionalities for Arduino
}
#endif

#ifndef WITH_ARDUINO
#ifndef WITH_CONTIKI
void coap_log_impl(coap_log_t level, const char *format, ...)
{
    char timebuf[32];
    coap_tick_t now;
    va_list ap;
    FILE *log_fd;

    if (maxlog < level)
        return;

    log_fd = level <= LOG_CRIT ? COAP_ERR_FD : COAP_DEBUG_FD;

    coap_ticks(&now);
    if (print_timestamp(timebuf, sizeof(timebuf), now))
        fprintf(log_fd, "%s ", timebuf);

    if (level <= LOG_DEBUG)
        fprintf(log_fd, "%s ", loglevels[level]);

    va_start(ap, format);
    vfprintf(log_fd, format, ap);
    va_end(ap);
    fflush(log_fd);
}
#else /* WITH_CONTIKI */
void
coap_log_impl(coap_log_t level, const char *format, ...)
{
    char timebuf[32];
    coap_tick_t now;
    va_list ap;

    if (maxlog < level)
    return;

    coap_ticks(&now);
    if (print_timestamp(timebuf,sizeof(timebuf), now))
    PRINTF("%s ", timebuf);

    if (level <= LOG_DEBUG)
    PRINTF("%s ", loglevels[level]);

    va_start(ap, format);
    PRINTF(format, ap);
    va_end(ap);
}
#endif /* WITH_CONTIKI */
#endif

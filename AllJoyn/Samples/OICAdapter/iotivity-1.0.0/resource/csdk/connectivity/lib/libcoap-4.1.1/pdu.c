/* pdu.c -- CoAP message structure
 *
 * Copyright (C) 2010,2011 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "config.h"

#if defined(HAVE_ASSERT_H) && !defined(assert)
# include <assert.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "debug.h"
#include "pdu.h"
#include "option.h"
#include "encode.h"

#ifdef WITH_ARDUINO
#include "util.h"
#endif

#ifdef WIN32
#include <WinSock2.h>
#endif

#ifdef WITH_CONTIKI
#include "memb.h"

#ifndef WITH_TCP
typedef unsigned char _pdu[sizeof(coap_pdu_t) + COAP_MAX_PDU_SIZE];

MEMB(pdu_storage, _pdu, COAP_PDU_MAXCNT);
#endif

void
coap_pdu_resources_init()
{
    memb_init(&pdu_storage);
}
#else /* WITH_CONTIKI */
#include "mem.h"
#endif /* WITH_CONTIKI */

void coap_pdu_clear(coap_pdu_t *pdu, size_t size, coap_transport_type transport, unsigned int length)
{
    assert(pdu);

    memset(pdu, 0, sizeof(coap_pdu_t) + size);
    pdu->max_size = size;
    pdu->hdr = (coap_hdr_t *) ((unsigned char *) pdu + sizeof(coap_pdu_t));

    if (coap_udp == transport)
    {
        pdu->hdr->coap_hdr_udp_t.version = COAP_DEFAULT_VERSION;
        /* data is NULL unless explicitly set by coap_add_data() */
        pdu->length = sizeof(pdu->hdr->coap_hdr_udp_t);
    }
#ifdef WITH_TCP
    else
    {
        /* data is NULL unless explicitly set by coap_add_data() */
        pdu->length = length;
    }
#endif
}

#ifdef WITH_LWIP
coap_pdu_t *
coap_pdu_from_pbuf(struct pbuf *pbuf)
{
    LWIP_ASSERT("Can only deal with contiguous PBUFs", pbuf->tot_len == pbuf->len);
    LWIP_ASSERT("coap_read needs to receive an exclusive copy of the incoming pbuf", pbuf->ref == 1);

    void *data = pbuf->payload;
    coap_pdu_t *result;

    u8_t header_error = pbuf_header(pbuf, sizeof(coap_pdu_t));
    LWIP_ASSERT("CoAP PDU header does not fit in existing header space", header_error == 0);

    result = (coap_pdu_t *)pbuf->payload;

    memset(result, 0, sizeof(coap_pdu_t));

    result->max_size = pbuf->tot_len - sizeof(coap_pdu_t);
    result->length = pbuf->tot_len - sizeof(coap_pdu_t);
    result->hdr = data;
    result->pbuf = pbuf;

    return result;
}
#endif

coap_pdu_t *
coap_pdu_init(unsigned char type, unsigned char code, unsigned short id,
              size_t size, coap_transport_type transport)
{
    coap_pdu_t *pdu;
#ifdef WITH_LWIP
    struct pbuf *p;
#endif

    unsigned int length = 0;
    switch(transport)
    {
        case coap_udp:
            length = sizeof(pdu->hdr->coap_hdr_udp_t);
            break;
#ifdef WITH_TCP
        case coap_tcp:
            length = COAP_TCP_HEADER_NO_FIELD;
            break;
        case coap_tcp_8bit:
            length = COAP_TCP_HEADER_8_BIT;
            break;
        case coap_tcp_16bit:
            length = COAP_TCP_HEADER_16_BIT;
            break;
        case coap_tcp_32bit:
            length = COAP_TCP_HEADER_32_BIT;
            break;
#endif
        default:
            debug("it has wrong type\n");
    }

#ifndef WITH_TCP
    assert(size <= COAP_MAX_PDU_SIZE);
    /* Size must be large enough to fit the header. */
    if (size < length || size > COAP_MAX_PDU_SIZE)
        return NULL;
#endif

    /* size must be large enough for hdr */
#if defined(WITH_POSIX) || defined(WITH_ARDUINO) || defined(WIN32)
    pdu = (coap_pdu_t *) coap_malloc(sizeof(coap_pdu_t) + size);
#endif
#ifdef WITH_CONTIKI
    pdu = (coap_pdu_t *)memb_alloc(&pdu_storage);
#endif
#ifdef WITH_LWIP
    p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RAM);
    if (p != NULL)
    {
        u8_t header_error = pbuf_header(p, sizeof(coap_pdu_t));
        /* we could catch that case and allocate larger memory in advance, but then
         * again, we'd run into greater trouble with incoming packages anyway */
        LWIP_ASSERT("CoAP PDU header does not fit in transport header", header_error == 0);
        pdu = p->payload;
    }
    else
    {
        pdu = NULL;
    }
#endif
    if (pdu)
    {
        coap_pdu_clear(pdu, size, transport, length);

        switch(transport)
        {
            case coap_udp:
                pdu->hdr->coap_hdr_udp_t.id = id;
                pdu->hdr->coap_hdr_udp_t.type = type;
                pdu->hdr->coap_hdr_udp_t.code = code;
                break;
#ifdef WITH_TCP
            case coap_tcp:
                pdu->hdr->coap_hdr_tcp_t.header_data[0] = 0;
                pdu->hdr->coap_hdr_tcp_t.header_data[1] = code;
                break;
            case coap_tcp_8bit:
                pdu->hdr->coap_hdr_tcp_8bit_t.header_data[0] = COAP_TCP_LENGTH_FIELD_NUM_8_BIT << 4;
                pdu->hdr->coap_hdr_tcp_8bit_t.header_data[2] = code;
                break;
            case coap_tcp_16bit:
                pdu->hdr->coap_hdr_tcp_16bit_t.header_data[0] = COAP_TCP_LENGTH_FIELD_NUM_16_BIT << 4;
                pdu->hdr->coap_hdr_tcp_16bit_t.header_data[3] = code;
                break;
            case coap_tcp_32bit:
                pdu->hdr->coap_hdr_tcp_32bit_t.header_data[0] = COAP_TCP_LENGTH_FIELD_NUM_32_BIT << 4;
                pdu->hdr->coap_hdr_tcp_32bit_t.header_data[5] = code;
                break;
#endif
            default:
                debug("it has wrong type\n");
        }

#ifdef WITH_LWIP
        pdu->pbuf = p;
#endif
    }
    return pdu;
}

coap_pdu_t *
coap_new_pdu(coap_transport_type transport, unsigned int size)
{
    coap_pdu_t *pdu;

#ifndef WITH_CONTIKI
    pdu = coap_pdu_init(0, 0,
                        ntohs(COAP_INVALID_TID),
#ifndef WITH_TCP
                        COAP_MAX_PDU_SIZE,
#else
                        size,
#endif
                        transport);
#else /* WITH_CONTIKI */
    pdu = coap_pdu_init(0, 0, uip_ntohs(COAP_INVALID_TID),
#ifndef WITH_TCP
                        COAP_MAX_PDU_SIZE,
#else
                        size,
#endif
                        transport);
#endif /* WITH_CONTIKI */

#ifndef NDEBUG
    if (!pdu)
        coap_log(LOG_CRIT, "coap_new_pdu: cannot allocate memory for new PDU\n");
#endif
    return pdu;
}

void coap_delete_pdu(coap_pdu_t *pdu)
{
#if defined(WITH_POSIX) || defined(WITH_ARDUINO) || defined(WIN32)
    coap_free( pdu );
#endif
#ifdef WITH_LWIP
    if (pdu != NULL) /* accepting double free as the other implementation accept that too */
        pbuf_free(pdu->pbuf);
#endif
#ifdef WITH_CONTIKI
    memb_free(&pdu_storage, pdu);
#endif
}

#ifdef WITH_TCP
coap_transport_type coap_get_tcp_header_type_from_size(unsigned int size)
{
    if (COAP_TCP_LENGTH_LIMIT_8_BIT < size && COAP_TCP_LENGTH_LIMIT_16_BIT >= size)
    {
        return coap_tcp_8bit;
    }
    else if (COAP_TCP_LENGTH_LIMIT_16_BIT < size && COAP_TCP_LENGTH_LIMIT_32_BIT >= size)
    {
        return coap_tcp_16bit;
    }
    else if (COAP_TCP_LENGTH_LIMIT_32_BIT < size)
    {
        return coap_tcp_32bit;
    }
    else
    {
        return coap_tcp;
    }
}

coap_transport_type coap_get_tcp_header_type_from_initbyte(unsigned int length)
{
    coap_transport_type type;
    switch(length)
    {
        case COAP_TCP_LENGTH_FIELD_NUM_8_BIT:
            type = coap_tcp_8bit;
            break;
        case COAP_TCP_LENGTH_FIELD_NUM_16_BIT:
            type = coap_tcp_16bit;
            break;
        case COAP_TCP_LENGTH_FIELD_NUM_32_BIT:
            type = coap_tcp_32bit;
            break;
        default:
            type = coap_tcp;
    }
    return type;
}

void coap_add_length(const coap_pdu_t *pdu, coap_transport_type transport, unsigned int length)
{
    assert(pdu);

    switch(transport)
    {
        case coap_tcp:
            pdu->hdr->coap_hdr_tcp_t.header_data[0] = length << 4;
            break;
        case coap_tcp_8bit:
            if (length > COAP_TCP_LENGTH_FIELD_8_BIT)
            {
                pdu->hdr->coap_hdr_tcp_8bit_t.header_data[1] =
                        length - COAP_TCP_LENGTH_FIELD_8_BIT;
            }
            break;
        case coap_tcp_16bit:
            if (length > COAP_TCP_LENGTH_FIELD_16_BIT)
            {
                unsigned int total_length = length - COAP_TCP_LENGTH_FIELD_16_BIT;
                pdu->hdr->coap_hdr_tcp_16bit_t.header_data[1] = (total_length >> 8) & 0x0000ff;
                pdu->hdr->coap_hdr_tcp_16bit_t.header_data[2] = total_length & 0x000000ff;
            }
            break;
        case coap_tcp_32bit:
            if (length > COAP_TCP_LENGTH_FIELD_32_BIT)
            {
                unsigned int total_length = length - COAP_TCP_LENGTH_FIELD_32_BIT;
                pdu->hdr->coap_hdr_tcp_32bit_t.header_data[1] = total_length >> 24;
                pdu->hdr->coap_hdr_tcp_32bit_t.header_data[2] = (total_length >> 16) & 0x00ff;
                pdu->hdr->coap_hdr_tcp_32bit_t.header_data[3] = (total_length >> 8) & 0x0000ff;
                pdu->hdr->coap_hdr_tcp_32bit_t.header_data[4] = total_length & 0x000000ff;
            }
            break;
        default:
            debug("it has wrong type\n");
    }
}

unsigned int coap_get_length_from_header(const unsigned char *header, coap_transport_type transport)
{
    assert(header);

    unsigned int length = 0;
    unsigned int length_field_data = 0;
    switch(transport)
    {
        case coap_tcp_8bit:
            length = header[1] + COAP_TCP_LENGTH_FIELD_8_BIT;
            break;
        case coap_tcp_16bit:
            length_field_data = (header[1] << 8 | header[2]);
            length = length_field_data + COAP_TCP_LENGTH_FIELD_16_BIT;
            break;
        case coap_tcp_32bit:
            length_field_data = header[1] << 24 | header[2] << 16 | header[3] << 8 | header[4];
            length = length_field_data + COAP_TCP_LENGTH_FIELD_32_BIT;
            break;
        default:
            debug("it has wrong type\n");
    }

    return length;
}

unsigned int coap_get_length(const coap_pdu_t *pdu, coap_transport_type transport)
{
    assert(pdu);

    unsigned int length = 0;
    unsigned int length_field_data = 0;
    switch(transport)
    {
        case coap_tcp:
            length = pdu->hdr->coap_hdr_tcp_t.header_data[0] >> 4;
            break;
        case coap_tcp_8bit:
            length = pdu->hdr->coap_hdr_tcp_8bit_t.header_data[1] + COAP_TCP_LENGTH_FIELD_8_BIT;
            break;
        case coap_tcp_16bit:
            length_field_data =
                    pdu->hdr->coap_hdr_tcp_16bit_t.header_data[1] << 8 |
                    pdu->hdr->coap_hdr_tcp_16bit_t.header_data[2];
            length = length_field_data + COAP_TCP_LENGTH_FIELD_16_BIT;
            break;
        case coap_tcp_32bit:
            length_field_data =
                    pdu->hdr->coap_hdr_tcp_32bit_t.header_data[1] << 24 |
                    pdu->hdr->coap_hdr_tcp_32bit_t.header_data[2] << 16 |
                    pdu->hdr->coap_hdr_tcp_32bit_t.header_data[3] << 8 |
                    pdu->hdr->coap_hdr_tcp_32bit_t.header_data[4];
            length = length_field_data + COAP_TCP_LENGTH_FIELD_32_BIT;
            break;
        default:
            debug("it has wrong type\n");
    }

    return length;
}

unsigned int coap_get_tcp_header_length(unsigned char *data)
{
    assert(data);

    unsigned int tokenLength =  data[0] & 0x0f;
    coap_transport_type transport =
            coap_get_tcp_header_type_from_initbyte(data[0] >> 4);
    unsigned int length = 0;

    length = coap_get_tcp_header_length_for_transport(transport) + tokenLength;
    return length;
}

unsigned int coap_get_tcp_header_length_for_transport(coap_transport_type transport)
{
    unsigned int length = 0;
    switch(transport)
    {
        case coap_tcp:
            length = COAP_TCP_HEADER_NO_FIELD;
            break;
        case coap_tcp_8bit:
            length = COAP_TCP_HEADER_8_BIT;
            break;
        case coap_tcp_16bit:
            length = COAP_TCP_HEADER_16_BIT;
            break;
        case coap_tcp_32bit:
            length = COAP_TCP_HEADER_32_BIT;
            break;
        default:
            debug("it has wrong type\n");
    }

    return length;
}

size_t coap_get_opt_header_length(unsigned short key, size_t length)
{
    size_t headerLength = 0;

    unsigned short optDeltaLength = 0;
    if (COAP_OPTION_FIELD_8_BIT >= key)
    {
        optDeltaLength = 0;
    }
    else if (COAP_OPTION_FIELD_8_BIT < key && COAP_OPTION_FIELD_16_BIT >= key)
    {
        optDeltaLength = 1;
    }
    else if (COAP_OPTION_FIELD_16_BIT < key && COAP_OPTION_FIELD_32_BIT >= key)
    {
        optDeltaLength = 2;
    }
    else
    {
        printf("Error : Reserved for the Payload marker for Delta");
        return 0;
    }

    size_t optLength = 0;
    if (COAP_OPTION_FIELD_8_BIT >= length)
    {
        optLength = 0;
    }
    else if (COAP_OPTION_FIELD_8_BIT < length && COAP_OPTION_FIELD_16_BIT >= length)
    {
        optLength = 1;
    }
    else if (COAP_OPTION_FIELD_16_BIT < length && COAP_OPTION_FIELD_32_BIT >= length)
    {
        optLength = 2;
    }
    else
    {
        printf("Error : Reserved for the Payload marker for length");
        return 0;
    }

    headerLength = length + optDeltaLength + optLength + 1;

    return headerLength;
}

#endif

void coap_add_code(const coap_pdu_t *pdu, coap_transport_type transport, unsigned int code)
{
    assert(pdu);

    switch(transport)
    {
        case coap_udp:
            pdu->hdr->coap_hdr_udp_t.code = COAP_RESPONSE_CODE(code);
            break;
#ifdef WITH_TCP
        case coap_tcp:
            pdu->hdr->coap_hdr_tcp_t.header_data[1] = COAP_RESPONSE_CODE(code);
            break;
        case coap_tcp_8bit:
            pdu->hdr->coap_hdr_tcp_8bit_t.header_data[2] = COAP_RESPONSE_CODE(code);
            break;
        case coap_tcp_16bit:
            pdu->hdr->coap_hdr_tcp_16bit_t.header_data[3] = COAP_RESPONSE_CODE(code);
            break;
        case coap_tcp_32bit:
            pdu->hdr->coap_hdr_tcp_32bit_t.header_data[5] = COAP_RESPONSE_CODE(code);
            break;
#endif
        default:
            debug("it has wrong type\n");
    }
}

unsigned int coap_get_code(const coap_pdu_t *pdu, coap_transport_type transport)
{
    assert(pdu);

    unsigned int code = 0;
    switch(transport)
    {
        case coap_udp:
            code = pdu->hdr->coap_hdr_udp_t.code;
            break;
#ifdef WITH_TCP
        case coap_tcp:
            code = pdu->hdr->coap_hdr_tcp_t.header_data[1];
            break;
        case coap_tcp_8bit:
            code = pdu->hdr->coap_hdr_tcp_8bit_t.header_data[2];
            break;
        case coap_tcp_16bit:
            code = pdu->hdr->coap_hdr_tcp_16bit_t.header_data[3];
            break;
        case coap_tcp_32bit:
            code = pdu->hdr->coap_hdr_tcp_32bit_t.header_data[5];
            break;
#endif
        default:
            debug("it has wrong type\n");
    }
    return code;
}

int coap_add_token(coap_pdu_t *pdu, size_t len, const unsigned char *data,
                   coap_transport_type transport)
{
    const size_t HEADERLENGTH = len + 4;
    /* must allow for pdu == NULL as callers may rely on this */
    if (!pdu || len > 8 || pdu->max_size < HEADERLENGTH)
        return 0;

    unsigned char* token = NULL;
    switch(transport)
    {
        case coap_udp:
            pdu->hdr->coap_hdr_udp_t.token_length = len;
            token = pdu->hdr->coap_hdr_udp_t.token;
            pdu->length = HEADERLENGTH;
            break;
#ifdef WITH_TCP
        case coap_tcp:
            pdu->hdr->coap_hdr_tcp_t.header_data[0] =
                    pdu->hdr->coap_hdr_tcp_t.header_data[0] | len;
            token = pdu->hdr->coap_hdr_tcp_t.token;
            pdu->length = len + COAP_TCP_HEADER_NO_FIELD;
            break;
        case coap_tcp_8bit:
            pdu->hdr->coap_hdr_tcp_8bit_t.header_data[0] =
                    pdu->hdr->coap_hdr_tcp_8bit_t.header_data[0] | len;
            token = pdu->hdr->coap_hdr_tcp_8bit_t.token;
            pdu->length = len + COAP_TCP_HEADER_8_BIT;
            break;
        case coap_tcp_16bit:
            pdu->hdr->coap_hdr_tcp_16bit_t.header_data[0] =
                    pdu->hdr->coap_hdr_tcp_16bit_t.header_data[0] | len;
            token = pdu->hdr->coap_hdr_tcp_16bit_t.token;
            pdu->length = len + COAP_TCP_HEADER_16_BIT;
            break;
        case coap_tcp_32bit:
            pdu->hdr->coap_hdr_tcp_32bit_t.header_data[0] =
                    pdu->hdr->coap_hdr_tcp_32bit_t.header_data[0] | len;
            token = pdu->hdr->coap_hdr_tcp_32bit_t.token;
            pdu->length = len + COAP_TCP_HEADER_32_BIT;
            break;
#endif
        default:
            debug("it has wrong type\n");
    }

    if (len)
    {
        memcpy(token, data, len);
    }

    pdu->max_delta = 0;
    pdu->data = NULL;

    return 1;
}

void coap_get_token(const coap_hdr_t *pdu_hdr, coap_transport_type transport,
                    unsigned char **token, unsigned int *token_length)
{
    assert(pdu_hdr);
    assert(token);
    assert(token_length);

    switch(transport)
    {
        case coap_udp:
            *token_length = pdu_hdr->coap_hdr_udp_t.token_length;
            *token = (unsigned char *)pdu_hdr->coap_hdr_udp_t.token;
            break;
#ifdef WITH_TCP
        case coap_tcp:
            *token_length = (pdu_hdr->coap_hdr_tcp_t.header_data[0]) & 0x0f;
            *token = (unsigned char *)pdu_hdr->coap_hdr_tcp_t.token;
            break;
        case coap_tcp_8bit:
            *token_length = (pdu_hdr->coap_hdr_tcp_8bit_t.header_data[0]) & 0x0f;
            *token = (unsigned char *)pdu_hdr->coap_hdr_tcp_8bit_t.token;
            break;
        case coap_tcp_16bit:
            *token_length = (pdu_hdr->coap_hdr_tcp_16bit_t.header_data[0]) & 0x0f;
            *token = (unsigned char *)pdu_hdr->coap_hdr_tcp_16bit_t.token;
            break;
        case coap_tcp_32bit:
            *token_length = (pdu_hdr->coap_hdr_tcp_32bit_t.header_data[0]) & 0x0f;
            *token = (unsigned char *)pdu_hdr->coap_hdr_tcp_32bit_t.token;
            break;
#endif
        default:
            debug("it has wrong type\n");
    }
}

/** @FIXME de-duplicate code with coap_add_option_later */
size_t coap_add_option(coap_pdu_t *pdu, unsigned short type, unsigned int len,
        const unsigned char *data, coap_transport_type transport)
{
    size_t optsize;
    coap_opt_t *opt;

    assert(pdu);
    pdu->data = NULL;

    if (type < pdu->max_delta)
    {
        warn("coap_add_option: options are not in correct order\n");
        return 0;
    }

    switch(transport)
    {
#ifdef WITH_TCP
        case coap_tcp:
            opt = (unsigned char *) &(pdu->hdr->coap_hdr_tcp_t) + pdu->length;
            break;
        case coap_tcp_8bit:
            opt = (unsigned char *) &(pdu->hdr->coap_hdr_tcp_8bit_t) + pdu->length;
            break;
        case coap_tcp_16bit:
            opt = (unsigned char *) &(pdu->hdr->coap_hdr_tcp_16bit_t) + pdu->length;
            break;
        case coap_tcp_32bit:
            opt = (unsigned char *) &(pdu->hdr->coap_hdr_tcp_32bit_t) + pdu->length;
            break;
#endif
        default:
            opt = (unsigned char *) &(pdu->hdr->coap_hdr_udp_t) + pdu->length;
            break;
    }

    /* encode option and check length */
    optsize = coap_opt_encode(opt, pdu->max_size - pdu->length, type - pdu->max_delta, data, len);

    if (!optsize)
    {
        warn("coap_add_option: cannot add option\n");
        /* error */
        return 0;
    }
    else
    {
        pdu->max_delta = type;
        pdu->length += optsize;
    }

    return optsize;
}

/** @FIXME de-duplicate code with coap_add_option */
unsigned char*
coap_add_option_later(coap_pdu_t *pdu, unsigned short type, unsigned int len)
{
    size_t optsize;
    coap_opt_t *opt;

    assert(pdu);
    pdu->data = NULL;

    if (type < pdu->max_delta)
    {
        warn("coap_add_option: options are not in correct order\n");
        return NULL;
    }

    opt = (unsigned char *) pdu->hdr + pdu->length;

    /* encode option and check length */
    optsize = coap_opt_encode(opt, pdu->max_size - pdu->length, type - pdu->max_delta, NULL, len);

    if (!optsize)
    {
        warn("coap_add_option: cannot add option\n");
        /* error */
        return NULL;
    }
    else
    {
        pdu->max_delta = type;
        pdu->length += optsize;
    }

    return ((unsigned char*) opt) + optsize - len;
}

int coap_add_data(coap_pdu_t *pdu, unsigned int len, const unsigned char *data)
{
    assert(pdu);
    assert(pdu->data == NULL);

    if (len == 0)
        return 1;

    if (pdu->length + len + 1 > pdu->max_size)
    {
        warn("coap_add_data: cannot add: data too large for PDU\n");
        assert(pdu->data == NULL);
        return 0;
    }

    pdu->data = (unsigned char *) pdu->hdr + pdu->length;
    *pdu->data = COAP_PAYLOAD_START;
    pdu->data++;

    memcpy(pdu->data, data, len);
    pdu->length += len + 1;
    return 1;
}

int coap_get_data(const coap_pdu_t *pdu, size_t *len, unsigned char **data)
{
    assert(pdu);
    assert(len);
    assert(data);

    if (pdu->data)
    {
        *len = (unsigned char *) pdu->hdr + pdu->length - pdu->data;
        *data = pdu->data;
    }
    else
    { /* no data, clear everything */
        *len = 0;
        *data = NULL;
    }

    return *data != NULL;
}

#ifndef SHORT_ERROR_RESPONSE
typedef struct
{
    unsigned char code;
    char *phrase;
} error_desc_t;

/* if you change anything here, make sure, that the longest string does not
 * exceed COAP_ERROR_PHRASE_LENGTH. */
error_desc_t coap_error[] =
{
{ COAP_RESPONSE_CODE(65), "2.01 Created" },
{ COAP_RESPONSE_CODE(66), "2.02 Deleted" },
{ COAP_RESPONSE_CODE(67), "2.03 Valid" },
{ COAP_RESPONSE_CODE(68), "2.04 Changed" },
{ COAP_RESPONSE_CODE(69), "2.05 Content" },
{ COAP_RESPONSE_CODE(400), "Bad Request" },
{ COAP_RESPONSE_CODE(401), "Unauthorized" },
{ COAP_RESPONSE_CODE(402), "Bad Option" },
{ COAP_RESPONSE_CODE(403), "Forbidden" },
{ COAP_RESPONSE_CODE(404), "Not Found" },
{ COAP_RESPONSE_CODE(405), "Method Not Allowed" },
{ COAP_RESPONSE_CODE(408), "Request Entity Incomplete" },
{ COAP_RESPONSE_CODE(413), "Request Entity Too Large" },
{ COAP_RESPONSE_CODE(415), "Unsupported Media Type" },
{ COAP_RESPONSE_CODE(500), "Internal Server Error" },
{ COAP_RESPONSE_CODE(501), "Not Implemented" },
{ COAP_RESPONSE_CODE(502), "Bad Gateway" },
{ COAP_RESPONSE_CODE(503), "Service Unavailable" },
{ COAP_RESPONSE_CODE(504), "Gateway Timeout" },
{ COAP_RESPONSE_CODE(505), "Proxying Not Supported" },
{ 0, NULL } /* end marker */
};

char *
coap_response_phrase(unsigned char code)
{
    int i;
    for (i = 0; coap_error[i].code; ++i)
    {
        if (coap_error[i].code == code)
            return coap_error[i].phrase;
    }
    return NULL;
}
#endif

/**
 * Advances *optp to next option if still in PDU. This function
 * returns the number of bytes opt has been advanced or @c 0
 * on error.
 */
static size_t next_option_safe(coap_opt_t **optp, size_t *length, coap_option_t* option)
{
    //coap_option_t option;
    size_t optsize;

    assert(optp);
    assert(*optp);
    assert(length);
    optsize = coap_opt_parse(*optp, *length, option);
    if (optsize)
    {
        assert(optsize <= *length);

        *optp += optsize;
        *length -= optsize;
    }

    return optsize;
}

int coap_pdu_parse(unsigned char *data, size_t length, coap_pdu_t *pdu,
                   coap_transport_type transport)
{
    assert(data);
    assert(pdu);

    if (pdu->max_size < length)
    {
        debug("insufficient space to store parsed PDU\n");
        printf("[COAP] insufficient space to store parsed PDU\n");
        return -1;
    }

    unsigned int headerSize = 0;

    if (coap_udp == transport)
    {
        headerSize = sizeof(pdu->hdr->coap_hdr_udp_t);
    }
#ifdef WITH_TCP
    else
    {
        headerSize = coap_get_tcp_header_length_for_transport(transport);
    }
#endif

    if (length < headerSize)
    {
        debug("discarded invalid PDU\n");
    }

    coap_opt_t *opt = NULL;
    unsigned int tokenLength = 0;
#ifdef WITH_TCP
    switch(transport)
    {
        case coap_udp:
            break;
        case coap_tcp:
            for (size_t i = 0 ; i < headerSize ; i++)
            {
                pdu->hdr->coap_hdr_tcp_t.header_data[i] = data[i];
            }

            tokenLength = data[0] & 0x0f;
            opt = (unsigned char *) (&(pdu->hdr->coap_hdr_tcp_t) + 1) + tokenLength;
            break;
        case coap_tcp_8bit:
            for (size_t i = 0 ; i < headerSize ; i++)
            {
                pdu->hdr->coap_hdr_tcp_8bit_t.header_data[i] = data[i];
            }

            tokenLength = data[0] & 0x0f;
            opt = (unsigned char *) (&(pdu->hdr->coap_hdr_tcp_8bit_t))
                    + tokenLength + COAP_TCP_HEADER_8_BIT;
            break;
        case coap_tcp_16bit:
            for (size_t i = 0 ; i < headerSize ; i++)
            {
                pdu->hdr->coap_hdr_tcp_16bit_t.header_data[i] = data[i];
            }

            tokenLength = data[0] & 0x0f;
            opt = (unsigned char *) (&(pdu->hdr->coap_hdr_tcp_16bit_t) + 1) + tokenLength;
            break;
        case coap_tcp_32bit:
            for (size_t i = 0 ; i < headerSize ; i++)
            {
                pdu->hdr->coap_hdr_tcp_32bit_t.header_data[i] = data[i];
            }

            tokenLength = data[0] & 0x0f;
            opt = ((unsigned char *) &(pdu->hdr->coap_hdr_tcp_32bit_t)) +
                    headerSize + tokenLength;
            break;
        default:
            printf("it has wrong type\n");
    }
#endif
    pdu->length = length;

    if (coap_udp == transport)
    {
        pdu->hdr->coap_hdr_udp_t.version = data[0] >> 6;
        pdu->hdr->coap_hdr_udp_t.type = (data[0] >> 4) & 0x03;
        pdu->hdr->coap_hdr_udp_t.token_length = data[0] & 0x0f;
        pdu->hdr->coap_hdr_udp_t.code = data[1];
        pdu->data = NULL;

        tokenLength = pdu->hdr->coap_hdr_udp_t.token_length;

        /* sanity checks */
        if (pdu->hdr->coap_hdr_udp_t.code == 0)
        {
            if (length != headerSize || tokenLength)
            {
                debug("coap_pdu_parse: empty message is not empty\n");
                goto discard;
            }
        }

        if (length < headerSize + tokenLength || tokenLength > 8)
        {
            debug("coap_pdu_parse: invalid Token\n");
            goto discard;
        }

        memcpy(&pdu->hdr->coap_hdr_udp_t.id, data + 2, 2);

        /* Finally calculate beginning of data block and thereby check integrity
         * of the PDU structure. */

        /* append data (including the Token) to pdu structure */
        memcpy(&(pdu->hdr->coap_hdr_udp_t) + 1, data + headerSize, length - headerSize);

        /* skip header + token */
        length -= (tokenLength + headerSize);
        opt = (unsigned char *) (&(pdu->hdr->coap_hdr_udp_t) + 1) + tokenLength;
    }
#ifdef WITH_TCP
    else // common for tcp header setting
    {
        pdu->data = NULL;

        if (length < headerSize + tokenLength || tokenLength > 8)
        {
            debug("coap_pdu_parse: invalid Token\n");
            goto discard;
        }
        /* Finally calculate beginning of data block and thereby check integrity
         * of the PDU structure. */

        /* append data (including the Token) to pdu structure */
        memcpy(((unsigned char *) pdu->hdr) + headerSize,
               data + headerSize, length - headerSize);

        /* skip header + token */
        length -= (tokenLength + headerSize);
    }
#endif

    while (length && *opt != COAP_PAYLOAD_START)
    {
        coap_option_t option;
        memset(&option, 0, sizeof(coap_option_t));
        if (!next_option_safe(&opt, (size_t *) &length, &option))
        {
            debug("coap_pdu_parse: drop\n");
            goto discard;
        }
    }

    /* end of packet or start marker */
    if (length)
    {
        assert(*opt == COAP_PAYLOAD_START);
        opt++;
        length--;

        if (!length)
        {
            debug("coap_pdu_parse: message ending in payload start marker\n");
            goto discard;
        }

        debug(
                "set data to %p (pdu ends at %p)\n", (unsigned char *)opt,
                (unsigned char *)pdu->hdr + pdu->length);
        pdu->data = (unsigned char *) opt;
        //printf("[COAP] pdu - data : %s\n", pdu->data);
    }

    return 1;

    discard: return 0;
}

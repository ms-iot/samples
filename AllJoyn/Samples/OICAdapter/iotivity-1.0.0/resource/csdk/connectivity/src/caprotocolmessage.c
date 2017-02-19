/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

// Defining _BSD_SOURCE or _DEFAULT_SOURCE causes header files to expose
// definitions that may otherwise be skipped. Skipping can cause implicit
// declaration warnings and/or bugs and subtle problems in code execution.
// For glibc information on feature test macros,
// Refer http://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
//
// This file requires #define use due to random() and srandom()
// For details on compatibility and glibc support,
// Refer http://www.gnu.org/software/libc/manual/html_node/BSD-Random.html
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include "caprotocolmessage.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"

// ARM GCC compiler doesnt define srandom function.
#if defined(ARDUINO) && !defined(ARDUINO_ARCH_SAM)
#define HAVE_SRANDOM 1
#endif

#define TAG "CA_PRTCL_MSG"

/**
 * @def VERIFY_NON_NULL_RET
 * @brief Macro to verify the validity of input argument
 */
#define VERIFY_NON_NULL_RET(arg, log_tag, log_message,ret) \
    if (NULL == arg ){ \
        OIC_LOG_V(ERROR, log_tag, "Invalid input:%s", log_message); \
        return ret; \
    }

#define CA_BUFSIZE (128)
#define CA_PDU_MIN_SIZE (4)
#define CA_PORT_BUFFER_SIZE (4)

static const char COAP_URI_HEADER[] = "coap://[::]/";

static unsigned int SEED = 0;

CAResult_t CAGetRequestInfoFromPDU(const coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                                   CARequestInfo_t *outReqInfo)
{
    if (NULL == pdu || NULL == outReqInfo)
    {
        OIC_LOG(ERROR, TAG, "parameter is null");
        return CA_STATUS_INVALID_PARAM;
    }

    uint32_t code = CA_NOT_FOUND;
    CAResult_t ret = CAGetInfoFromPDU(pdu, endpoint, &code, &(outReqInfo->info));
    outReqInfo->method = code;

    return ret;
}

CAResult_t CAGetResponseInfoFromPDU(const coap_pdu_t *pdu, CAResponseInfo_t *outResInfo,
                                    const CAEndpoint_t *endpoint)
{
    if (NULL == pdu || NULL == outResInfo)
    {
        OIC_LOG(ERROR, TAG, "parameter is null");
        return CA_STATUS_INVALID_PARAM;
    }

    uint32_t code = CA_NOT_FOUND;
    CAResult_t ret = CAGetInfoFromPDU(pdu, endpoint, &code, &(outResInfo->info));
    outResInfo->result = code;

    return ret;
}

CAResult_t CAGetErrorInfoFromPDU(const coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                                 CAErrorInfo_t *errorInfo)
{
    if (!pdu)
    {
        OIC_LOG(ERROR, TAG, "parameter is null");
        return CA_STATUS_INVALID_PARAM;
    }

    uint32_t code = 0;
    CAResult_t ret = CAGetInfoFromPDU(pdu, endpoint, &code, &errorInfo->info);

    return ret;
}

coap_pdu_t *CAGeneratePDU(uint32_t code, const CAInfo_t *info, const CAEndpoint_t *endpoint,
                          coap_list_t **optlist, coap_transport_type *transport)
{
    VERIFY_NON_NULL_RET(info, TAG, "info", NULL);
    VERIFY_NON_NULL_RET(endpoint, TAG, "endpoint", NULL);
    VERIFY_NON_NULL_RET(optlist, TAG, "optlist", NULL);

    coap_pdu_t *pdu = NULL;

    // RESET have to use only 4byte (empty message)
    // and ACKNOWLEDGE can use empty message when code is empty.
    if (CA_MSG_RESET == info->type || (CA_EMPTY == code && CA_MSG_ACKNOWLEDGE == info->type))
    {
        OIC_LOG(DEBUG, TAG, "code is empty");
        if (!(pdu = CAGeneratePDUImpl((code_t) code, info, endpoint, NULL, transport)))
        {
            OIC_LOG(ERROR, TAG, "pdu NULL");
            return NULL;
        }
    }
    else
    {
        if (CA_MSG_ACKNOWLEDGE != info->type && info->resourceUri)
        {
            uint32_t length = strlen(info->resourceUri);
            if (CA_MAX_URI_LENGTH < length)
            {
                OIC_LOG(ERROR, TAG, "URI len err");
                return NULL;
            }

            uint32_t uriLength = length + sizeof(COAP_URI_HEADER);
            char *coapUri = (char *) OICCalloc(1, uriLength);
            if (NULL == coapUri)
            {
                OIC_LOG(ERROR, TAG, "out of memory");
                return NULL;
            }
            OICStrcat(coapUri, uriLength, COAP_URI_HEADER);
            OICStrcat(coapUri, uriLength, info->resourceUri);

            // parsing options in URI
            CAResult_t res = CAParseURI(coapUri, optlist);
            if (CA_STATUS_OK != res)
            {
                OICFree(coapUri);
                return NULL;
            }

            OICFree(coapUri);
        }
        // parsing options in HeadOption
        CAResult_t ret = CAParseHeadOption(code, info, optlist);
        if (CA_STATUS_OK != ret)
        {
            return NULL;
        }

        pdu = CAGeneratePDUImpl((code_t) code, info, endpoint, *optlist, transport);
        if (NULL == pdu)
        {
            OIC_LOG(ERROR, TAG, "pdu NULL");
            return NULL;
        }
    }

    // pdu print method : coap_show_pdu(pdu);
    return pdu;
}

coap_pdu_t *CAParsePDU(const char *data, uint32_t length, uint32_t *outCode,
                       const CAEndpoint_t *endpoint)
{
    if (NULL == data)
    {
        OIC_LOG(ERROR, TAG, "data is null");
        return NULL;
    }

    coap_transport_type transport;
#ifdef TCP_ADAPTER
    if (CA_ADAPTER_TCP == endpoint->adapter)
    {
        transport = coap_get_tcp_header_type_from_initbyte(((unsigned char *)data)[0] >> 4);
    }
    else
#endif
    {
        transport = coap_udp;
    }

    coap_pdu_t *outpdu = coap_new_pdu(transport, length);
    if (NULL == outpdu)
    {
        OIC_LOG(ERROR, TAG, "outpdu is null");
        return NULL;
    }

    OIC_LOG_V(DEBUG, TAG, "pdu parse-transport type : %d", transport);

    int ret = coap_pdu_parse((unsigned char *) data, length, outpdu, transport);
    OIC_LOG_V(DEBUG, TAG, "pdu parse ret: %d", ret);
    if (0 >= ret)
    {
        OIC_LOG(ERROR, TAG, "pdu parse failed");
        coap_delete_pdu(outpdu);
        return NULL;
    }

#ifdef TCP_ADAPTER
    if (CA_ADAPTER_TCP == endpoint->adapter)
    {
        OIC_LOG(INFO, TAG, "there is no version info in coap header");
    }
    else
#endif
    {
        if (outpdu->hdr->coap_hdr_udp_t.version != COAP_DEFAULT_VERSION)
        {
            OIC_LOG_V(ERROR, TAG, "coap version is not available : %d",
                      outpdu->hdr->coap_hdr_udp_t.version);
            coap_delete_pdu(outpdu);
            return NULL;
        }
        if (outpdu->hdr->coap_hdr_udp_t.token_length > CA_MAX_TOKEN_LEN)
        {
            OIC_LOG_V(ERROR, TAG, "token length has been exceed : %d",
                      outpdu->hdr->coap_hdr_udp_t.token_length);
            coap_delete_pdu(outpdu);
            return NULL;
        }
    }

    if (outCode)
    {
        (*outCode) = (uint32_t) CA_RESPONSE_CODE(coap_get_code(outpdu, transport));
    }

    return outpdu;
}

coap_pdu_t *CAGeneratePDUImpl(code_t code, const CAInfo_t *info,
                              const CAEndpoint_t *endpoint, coap_list_t *options,
                              coap_transport_type *transport)
{
    VERIFY_NON_NULL_RET(info, TAG, "info", NULL);
    VERIFY_NON_NULL_RET(endpoint, TAG, "endpoint", NULL);
    VERIFY_NON_NULL_RET(transport, TAG, "transport", NULL);

    unsigned int length = COAP_MAX_PDU_SIZE;
#ifdef TCP_ADAPTER
    unsigned int msgLength = 0;
    if (CA_ADAPTER_TCP == endpoint->adapter)
    {
        if (options)
        {
            unsigned short prevOptNumber = 0;
            for (coap_list_t *opt = options; opt; opt = opt->next)
            {
                unsigned short curOptNumber = COAP_OPTION_KEY(*(coap_option *) opt->data);
                if (prevOptNumber > curOptNumber)
                {
                    OIC_LOG(ERROR, TAG, "option list is wrong");
                    return NULL;
                }

                size_t optValueLen = COAP_OPTION_LENGTH(*(coap_option *) opt->data);
                size_t optLength = coap_get_opt_header_length(curOptNumber - prevOptNumber, optValueLen);
                if (0 == optLength)
                {
                    OIC_LOG(ERROR, TAG, "Reserved for the Payload marker for the option");
                    return NULL;
                }
                msgLength += optLength;
                prevOptNumber = curOptNumber;
                OIC_LOG_V(DEBUG, TAG, "curOptNumber[%d], prevOptNumber[%d], optValueLen[%d], "
                        "optLength[%d], msgLength[%d]",
                          curOptNumber, prevOptNumber, optValueLen, optLength, msgLength);
            }
        }

        if (info->payloadSize > 0)
        {
            msgLength = msgLength + info->payloadSize + PAYLOAD_MARKER;
        }
        *transport = coap_get_tcp_header_type_from_size(msgLength);
        length = msgLength + coap_get_tcp_header_length_for_transport(*transport)
                + info->tokenLength;
    }
    else
#endif
    {
        *transport = coap_udp;
    }

    coap_pdu_t *pdu = coap_new_pdu(*transport, length);

    if (NULL == pdu)
    {
        OIC_LOG(ERROR, TAG, "malloc failed");
        return NULL;
    }

    OIC_LOG_V(DEBUG, TAG, "transport type: %d, payload size: %d",
              *transport, info->payloadSize);

#ifdef TCP_ADAPTER
    if (CA_ADAPTER_TCP == endpoint->adapter)
    {
        coap_add_length(pdu, *transport, msgLength);
    }
    else
#endif
    {
        OIC_LOG_V(DEBUG, TAG, "msgID is %d", info->messageId);
        uint16_t message_id;
        if (0 == info->messageId)
        {
            /* initialize message id */
            prng((uint8_t * ) &message_id, sizeof(message_id));

            OIC_LOG_V(DEBUG, TAG, "gen msg id=%d", message_id);
        }
        else
        {
            /* use saved message id */
            message_id = info->messageId;
        }
        pdu->hdr->coap_hdr_udp_t.id = message_id;
        OIC_LOG_V(DEBUG, TAG, "messageId in pdu is %d, %d", message_id, pdu->hdr->coap_hdr_udp_t.id);

        pdu->hdr->coap_hdr_udp_t.type = info->type;
    }

    coap_add_code(pdu, *transport, code);

    if (info->token && CA_EMPTY != code)
    {
        uint32_t tokenLength = info->tokenLength;
        OIC_LOG_V(DEBUG, TAG, "token info token length: %d, token :", tokenLength);
        OIC_LOG_BUFFER(DEBUG, TAG, (const uint8_t *)info->token, tokenLength);

        int32_t ret = coap_add_token(pdu, tokenLength, (unsigned char *)info->token, *transport);
        if (0 == ret)
        {
            OIC_LOG(ERROR, TAG, "can't add token");
        }
    }

#ifdef WITH_BWT
    if (CA_ADAPTER_GATT_BTLE != endpoint->adapter
#ifdef TCP_ADAPTER
            && CA_ADAPTER_TCP != endpoint->adapter
#endif
            )
    {
        // option list will be added in blockwise-transfer
        return pdu;
    }
#endif

    if (options)
    {
        for (coap_list_t *opt = options; opt; opt = opt->next)
        {
            OIC_LOG_V(DEBUG, TAG, "[%s] opt will be added.",
                      COAP_OPTION_DATA(*(coap_option *) opt->data));

            OIC_LOG_V(DEBUG, TAG, "[%d] pdu length", pdu->length);
            coap_add_option(pdu, COAP_OPTION_KEY(*(coap_option *) opt->data),
                            COAP_OPTION_LENGTH(*(coap_option *) opt->data),
                            COAP_OPTION_DATA(*(coap_option *) opt->data), *transport);
        }
    }

    OIC_LOG_V(DEBUG, TAG, "[%d] pdu length after option", pdu->length);

    if (NULL != info->payload && 0 < info->payloadSize)
    {
        OIC_LOG(DEBUG, TAG, "payload is added");
        coap_add_data(pdu, info->payloadSize, (const unsigned char *) info->payload);
    }

    return pdu;
}

CAResult_t CAParseURI(const char *uriInfo, coap_list_t **optlist)
{
    if (NULL == uriInfo)
    {
        OIC_LOG(ERROR, TAG, "uriInfo is null");
        return CA_STATUS_INVALID_PARAM;
    }

    OIC_LOG_V(DEBUG, TAG, "url : %s", uriInfo);

    if (NULL == optlist)
    {
        OIC_LOG(ERROR, TAG, "optlist is null");
        return CA_STATUS_INVALID_PARAM;
    }

    /* split arg into Uri-* options */
    coap_uri_t uri;
    coap_split_uri((unsigned char *) uriInfo, strlen(uriInfo), &uri);

    if (uri.port != COAP_DEFAULT_PORT)
    {
        unsigned char portbuf[CA_PORT_BUFFER_SIZE] = { 0 };
        int ret = coap_insert(optlist,
                              CACreateNewOptionNode(COAP_OPTION_URI_PORT,
                                                    coap_encode_var_bytes(portbuf, uri.port),
                                                    (char *)portbuf),
                              CAOrderOpts);
        if (ret <= 0)
        {
            return CA_STATUS_INVALID_PARAM;
        }
    }

    if (uri.path.s && uri.path.length)
    {
        CAResult_t ret = CAParseUriPartial(uri.path.s, uri.path.length,
                                           COAP_OPTION_URI_PATH, optlist);
        if (CA_STATUS_OK != ret)
        {
            OIC_LOG(ERROR, TAG, "CAParseUriPartial failed(uri path)");
            return ret;
        }
    }

    if (uri.query.s && uri.query.length)
    {
        CAResult_t ret = CAParseUriPartial(uri.query.s, uri.query.length, COAP_OPTION_URI_QUERY,
                                           optlist);
        if (CA_STATUS_OK != ret)
        {
            OIC_LOG(ERROR, TAG, "CAParseUriPartial failed(uri query)");
            return ret;
        }
    }

    return CA_STATUS_OK;
}

CAResult_t CAParseUriPartial(const unsigned char *str, size_t length, int target,
                             coap_list_t **optlist)
{
    if (!optlist)
    {
        OIC_LOG(ERROR, TAG, "optlist is null");
        return CA_STATUS_INVALID_PARAM;
    }

    if ((target != COAP_OPTION_URI_PATH) && (target != COAP_OPTION_URI_QUERY))
    {
        // should never occur. Log just in case.
        OIC_LOG(DEBUG, TAG, "Unexpected URI component.");
        return CA_NOT_SUPPORTED;
    }
    else if (str && length)
    {
        unsigned char uriBuffer[CA_BUFSIZE] = { 0 };
        unsigned char *pBuf = uriBuffer;
        size_t buflen = sizeof(uriBuffer);
        int res = (target == COAP_OPTION_URI_PATH) ? coap_split_path(str, length, pBuf, &buflen) :
                                                     coap_split_query(str, length, pBuf, &buflen);

        if (res > 0)
        {
            size_t prevIdx = 0;
            while (res--)
            {
                int ret = coap_insert(optlist,
                                      CACreateNewOptionNode(target, COAP_OPT_LENGTH(pBuf),
                                                            (char *)COAP_OPT_VALUE(pBuf)),
                                      CAOrderOpts);
                if (ret <= 0)
                {
                    return CA_STATUS_INVALID_PARAM;
                }

                size_t optSize = COAP_OPT_SIZE(pBuf);
                if ((prevIdx + optSize) < buflen)
                {
                    pBuf += optSize;
                    prevIdx += optSize;
                }
            }
        }
        else
        {
            OIC_LOG_V(ERROR, TAG, "Problem parsing URI : %d for %d", res, target);
            return CA_STATUS_FAILED;
        }
    }
    else
    {
        OIC_LOG(ERROR, TAG, "str or length is not available");
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

CAResult_t CAParseHeadOption(uint32_t code, const CAInfo_t *info, coap_list_t **optlist)
{
    (void)code;
    VERIFY_NON_NULL_RET(info, TAG, "info is NULL", CA_STATUS_INVALID_PARAM);

    OIC_LOG_V(DEBUG, TAG, "parse Head Opt: %d", info->numOptions);

    if (!optlist)
    {
        OIC_LOG(ERROR, TAG, "optlist is null");
        return CA_STATUS_INVALID_PARAM;
    }

    for (uint32_t i = 0; i < info->numOptions; i++)
    {
        if(!(info->options + i))
        {
            OIC_LOG(ERROR, TAG, "options is not available");
            return CA_STATUS_FAILED;
        }

        uint32_t id = (info->options + i)->optionID;
        if (COAP_OPTION_URI_PATH == id || COAP_OPTION_URI_QUERY == id)
        {
            OIC_LOG_V(DEBUG, TAG, "not Header Opt: %d", id);
        }
        else
        {
            OIC_LOG_V(DEBUG, TAG, "Head opt ID: %d", id);
            OIC_LOG_V(DEBUG, TAG, "Head opt data: %s", (info->options + i)->optionData);
            OIC_LOG_V(DEBUG, TAG, "Head opt length: %d", (info->options + i)->optionLength);
            int ret = coap_insert(optlist,
                                  CACreateNewOptionNode(id, (info->options + i)->optionLength,
                                                        (info->options + i)->optionData),
                                  CAOrderOpts);
            if (ret <= 0)
            {
                return CA_STATUS_INVALID_PARAM;
            }
        }
    }

    // insert one extra header with the payload format if applicable.
    if (CA_FORMAT_UNDEFINED != info->payloadFormat)
    {
        coap_list_t* node = NULL;
        uint8_t buf[3] = {0};
        switch (info->payloadFormat) {
            case CA_FORMAT_APPLICATION_CBOR:
                node = CACreateNewOptionNode(
                        COAP_OPTION_CONTENT_FORMAT,
                        coap_encode_var_bytes(buf, (uint16_t)COAP_MEDIATYPE_APPLICATION_CBOR),
                        (char *)buf);
                break;
            default:
                OIC_LOG_V(ERROR, TAG, "format option:[%d] not supported", info->payloadFormat);
        }
        if (!node)
        {
            OIC_LOG(ERROR, TAG, "format option not created");
            return CA_STATUS_INVALID_PARAM;
        }
        int ret = coap_insert(optlist, node, CAOrderOpts);
        if (ret <= 0)
        {
            coap_delete(node);
            OIC_LOG(ERROR, TAG, "format option not inserted in header");
            return CA_STATUS_INVALID_PARAM;
        }
    }
    if (CA_FORMAT_UNDEFINED != info->acceptFormat)
    {
        coap_list_t* node = NULL;
        uint8_t buf[3] = {0};
        switch (info->acceptFormat) {
            case CA_FORMAT_APPLICATION_CBOR:
                node = CACreateNewOptionNode(
                        COAP_OPTION_ACCEPT,
                        coap_encode_var_bytes(buf, (uint16_t)COAP_MEDIATYPE_APPLICATION_CBOR),
                        (char *)buf);
                break;
            default:
                OIC_LOG_V(ERROR, TAG, "format option:[%d] not supported", info->acceptFormat);
        }
        if (!node)
        {
            OIC_LOG(ERROR, TAG, "format option not created");
            return CA_STATUS_INVALID_PARAM;
        }
        int ret = coap_insert(optlist, node, CAOrderOpts);
        if (ret <= 0)
        {
            coap_delete(node);
            OIC_LOG(ERROR, TAG, "format option not inserted in header");
            return CA_STATUS_INVALID_PARAM;
        }
    }

    return CA_STATUS_OK;
}

coap_list_t *CACreateNewOptionNode(uint16_t key, uint32_t length, const char *data)
{
    if (!data)
    {
        OIC_LOG(ERROR, TAG, "invalid pointer parameter");
        return NULL;
    }

    coap_option *option = coap_malloc(sizeof(coap_option) + length + 1);
    if (!option)
    {
        OIC_LOG(ERROR, TAG, "Out of memory");
        return NULL;
    }
    memset(option, 0, sizeof(coap_option) + length + 1);

    COAP_OPTION_KEY(*option) = key;

    coap_option_def_t* def = coap_opt_def(key);
    if (NULL != def && coap_is_var_bytes(def))
    {
       if (length > def->max)
        {
            // make sure we shrink the value so it fits the coap option definition
            // by truncating the value, disregard the leading bytes.
            OIC_LOG_V(DEBUG, TAG, "Option [%d] data size [%d] shrunk to [%d]",
                    def->key, length, def->max);
            data = &(data[length-def->max]);
            length = def->max;
        }
        // Shrink the encoding length to a minimum size for coap
        // options that support variable length encoding.
         COAP_OPTION_LENGTH(*option) = coap_encode_var_bytes(
                COAP_OPTION_DATA(*option),
                coap_decode_var_bytes((unsigned char *)data, length));
    }
    else
    {
        COAP_OPTION_LENGTH(*option) = length;
        memcpy(COAP_OPTION_DATA(*option), data, length);
    }

    /* we can pass NULL here as delete function since option is released automatically  */
    coap_list_t *node = coap_new_listnode(option, NULL);

    if (!node)
    {
        OIC_LOG(ERROR, TAG, "node is NULL");
        coap_free(option);
        return NULL;
    }

    return node;
}

int CAOrderOpts(void *a, void *b)
{
    if (!a || !b)
    {
        return a < b ? -1 : 1;
    }

    if (COAP_OPTION_KEY(*(coap_option *) a) < COAP_OPTION_KEY(*(coap_option * ) b))
    {
        return -1;
    }

    return COAP_OPTION_KEY(*(coap_option *) a) == COAP_OPTION_KEY(*(coap_option * ) b);
}

uint32_t CAGetOptionCount(coap_opt_iterator_t opt_iter)
{
    uint32_t count = 0;
    coap_opt_t *option;

    while ((option = coap_option_next(&opt_iter)))
    {
        if (COAP_OPTION_URI_PATH != opt_iter.type && COAP_OPTION_URI_QUERY != opt_iter.type
            && COAP_OPTION_BLOCK1 != opt_iter.type && COAP_OPTION_BLOCK2 != opt_iter.type
            && COAP_OPTION_SIZE1 != opt_iter.type && COAP_OPTION_SIZE2 != opt_iter.type
            && COAP_OPTION_CONTENT_FORMAT != opt_iter.type
            && COAP_OPTION_ACCEPT != opt_iter.type)
        {
            count++;
        }
    }

    return count;
}

CAResult_t CAGetInfoFromPDU(const coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                            uint32_t *outCode, CAInfo_t *outInfo)
{
    if (!pdu || !outCode || !outInfo)
    {
        OIC_LOG(ERROR, TAG, "NULL pointer param");
        return CA_STATUS_INVALID_PARAM;
    }

    coap_transport_type transport;
#ifdef TCP_ADAPTER
    if (CA_ADAPTER_TCP == endpoint->adapter)
    {
        transport = coap_get_tcp_header_type_from_initbyte(((unsigned char *)pdu->hdr)[0] >> 4);
    }
    else
#endif
    {
        transport = coap_udp;
    }

    coap_opt_iterator_t opt_iter;
    coap_option_iterator_init((coap_pdu_t *) pdu, &opt_iter, COAP_OPT_ALL, transport);

    if (outCode)
    {
        (*outCode) = (uint32_t) CA_RESPONSE_CODE(coap_get_code(pdu, transport));
    }

    // init HeaderOption list
    uint32_t count = CAGetOptionCount(opt_iter);
    memset(outInfo, 0, sizeof(*outInfo));

    outInfo->numOptions = count;

#ifdef TCP_ADAPTER
    if (CA_ADAPTER_TCP == endpoint->adapter)
    {
        // set type
        outInfo->type = CA_MSG_NONCONFIRM;
        outInfo->payloadFormat = CA_FORMAT_UNDEFINED;
    }
    else
#endif
    {
        // set type
        outInfo->type = pdu->hdr->coap_hdr_udp_t.type;

        // set message id
        outInfo->messageId = pdu->hdr->coap_hdr_udp_t.id;
        outInfo->payloadFormat = CA_FORMAT_UNDEFINED;
        outInfo->acceptFormat = CA_FORMAT_UNDEFINED;
    }

    if (count > 0)
    {
        outInfo->options = (CAHeaderOption_t *) OICCalloc(count, sizeof(CAHeaderOption_t));
        if (NULL == outInfo->options)
        {
            OIC_LOG(ERROR, TAG, "Out of memory");
            return CA_MEMORY_ALLOC_FAILED;
        }
    }

    coap_opt_t *option;
    char optionResult[CA_MAX_URI_LENGTH] = {0};
    uint32_t idx = 0;
    uint32_t optionLength = 0;
    bool isfirstsetflag = false;
    bool isQueryBeingProcessed = false;

    while ((option = coap_option_next(&opt_iter)))
    {
        char buf[COAP_MAX_PDU_SIZE] = {0};
        uint32_t bufLength =
            CAGetOptionData(opt_iter.type, (uint8_t *)(COAP_OPT_VALUE(option)),
                    COAP_OPT_LENGTH(option), (uint8_t *)buf, sizeof(buf));
        if (bufLength)
        {
            OIC_LOG_V(DEBUG, TAG, "COAP URI element : %s", buf);
            if (COAP_OPTION_URI_PATH == opt_iter.type || COAP_OPTION_URI_QUERY == opt_iter.type)
            {
                if (false == isfirstsetflag)
                {
                    isfirstsetflag = true;
                    optionResult[optionLength] = '/';
                    optionLength++;
                    // Make sure there is enough room in the optionResult buffer
                    if ((optionLength + bufLength) < sizeof(optionResult))
                    {
                        memcpy(&optionResult[optionLength], buf, bufLength);
                        optionLength += bufLength;
                    }
                    else
                    {
                        goto exit;
                    }
                }
                else
                {
                    if (COAP_OPTION_URI_PATH == opt_iter.type)
                    {
                        // Make sure there is enough room in the optionResult buffer
                        if (optionLength < sizeof(optionResult))
                        {
                            optionResult[optionLength] = '/';
                            optionLength++;
                        }
                        else
                        {
                            goto exit;
                        }
                    }
                    else if (COAP_OPTION_URI_QUERY == opt_iter.type)
                    {
                        if (false == isQueryBeingProcessed)
                        {
                            // Make sure there is enough room in the optionResult buffer
                            if (optionLength < sizeof(optionResult))
                            {
                                optionResult[optionLength] = '?';
                                optionLength++;
                                isQueryBeingProcessed = true;
                            }
                            else
                            {
                                goto exit;
                            }
                        }
                        else
                        {
                            // Make sure there is enough room in the optionResult buffer
                            if (optionLength < sizeof(optionResult))
                            {
                                optionResult[optionLength] = ';';
                                optionLength++;
                            }
                            else
                            {
                                goto exit;
                            }
                        }
                    }
                    // Make sure there is enough room in the optionResult buffer
                    if ((optionLength + bufLength) < sizeof(optionResult))
                    {
                        memcpy(&optionResult[optionLength], buf, bufLength);
                        optionLength += bufLength;
                    }
                    else
                    {
                        goto exit;
                    }
                }
            }
            else if (COAP_OPTION_BLOCK1 == opt_iter.type || COAP_OPTION_BLOCK2 == opt_iter.type
                    || COAP_OPTION_SIZE1 == opt_iter.type || COAP_OPTION_SIZE2 == opt_iter.type)
            {
                OIC_LOG_V(DEBUG, TAG, "option[%d] will be filtering", opt_iter.type);
            }
            else if (COAP_OPTION_CONTENT_FORMAT == opt_iter.type)
            {
                if (1 == COAP_OPT_LENGTH(option))
                {
                    outInfo->payloadFormat = CAConvertFormat((uint8_t)buf[0]);
                }
                else
                {
                    outInfo->payloadFormat = CA_FORMAT_UNSUPPORTED;
                    OIC_LOG_V(DEBUG, TAG, "option[%d] has an unsupported format [%d]",
                            opt_iter.type, (uint8_t)buf[0]);
                }
            }
            else if (COAP_OPTION_ACCEPT == opt_iter.type)
            {
                if (1 == COAP_OPT_LENGTH(option))
                {
                    outInfo->acceptFormat = CAConvertFormat((uint8_t)buf[0]);
                }
                else
                {
                    outInfo->acceptFormat = CA_FORMAT_UNSUPPORTED;
                }
                OIC_LOG_V(DEBUG, TAG, "option[%d] has an unsupported format [%d]",
                        opt_iter.type, (uint8_t)buf[0]);
            }
            else
            {
                if (idx < count)
                {
                    if (bufLength <= sizeof(outInfo->options[0].optionData))
                    {
                        outInfo->options[idx].optionID = opt_iter.type;
                        outInfo->options[idx].optionLength = bufLength;
                        outInfo->options[idx].protocolID = CA_COAP_ID;
                        memcpy(outInfo->options[idx].optionData, buf, bufLength);
                        idx++;
                    }
                }
            }
        }
    }

    unsigned char* token = NULL;
    unsigned int token_length = 0;
    coap_get_token(pdu->hdr, transport, &token, &token_length);

    // set token data
    if (token_length > 0)
    {
        OIC_LOG_V(DEBUG, TAG, "inside token length : %d", token_length);
        outInfo->token = (char *) OICMalloc(token_length);
        if (NULL == outInfo->token)
        {
            OIC_LOG(ERROR, TAG, "Out of memory");
            OICFree(outInfo->options);
            return CA_MEMORY_ALLOC_FAILED;
        }
        memcpy(outInfo->token, token, token_length);
    }

    outInfo->tokenLength = token_length;

    // set payload data
    size_t dataSize;
    uint8_t *data;
    if (coap_get_data(pdu, &dataSize, &data))
    {
        OIC_LOG(DEBUG, TAG, "inside pdu->data");
        outInfo->payload = (uint8_t *) OICMalloc(dataSize);
        if (NULL == outInfo->payload)
        {
            OIC_LOG(ERROR, TAG, "Out of memory");
            OICFree(outInfo->options);
            OICFree(outInfo->token);
            return CA_MEMORY_ALLOC_FAILED;
        }
        memcpy(outInfo->payload, pdu->data, dataSize);
        outInfo->payloadSize = dataSize;
    }

    if (optionResult[0] != '\0')
    {
        OIC_LOG_V(DEBUG, TAG, "URL length:%d", strlen(optionResult));
        outInfo->resourceUri = OICStrdup(optionResult);
        if (!outInfo->resourceUri)
        {
            OIC_LOG(ERROR, TAG, "Out of memory");
            OICFree(outInfo->options);
            OICFree(outInfo->token);
            return CA_MEMORY_ALLOC_FAILED;
        }
    }

    return CA_STATUS_OK;

exit:
    OIC_LOG(ERROR, TAG, "buffer too small");
    OICFree(outInfo->options);
    return CA_STATUS_FAILED;
}

CAResult_t CAGetTokenFromPDU(const coap_hdr_t *pdu_hdr, CAInfo_t *outInfo,
                             const CAEndpoint_t *endpoint)
{
    if (NULL == pdu_hdr)
    {
        OIC_LOG(ERROR, TAG, "pdu_hdr is null");
        return CA_STATUS_INVALID_PARAM;
    }

    if (NULL == outInfo)
    {
        OIC_LOG(ERROR, TAG, "outInfo is null");
        return CA_STATUS_INVALID_PARAM;
    }

    coap_transport_type transport;
#ifdef TCP_ADAPTER
    if (CA_ADAPTER_TCP == endpoint->adapter)
    {
        transport = coap_get_tcp_header_type_from_initbyte(((unsigned char *)pdu_hdr)[0] >> 4);
    }
    else
#endif
    {
        transport = coap_udp;
    }

    unsigned char* token = NULL;
    unsigned int token_length = 0;
    coap_get_token(pdu_hdr, transport, &token, &token_length);

    // set token data
    if (token_length > 0)
    {
        OIC_LOG_V(DEBUG, TAG, "token len:%d", token_length);
        outInfo->token = (char *) OICMalloc(token_length);
        if (NULL == outInfo->token)
        {
            OIC_LOG(ERROR, TAG, "Out of memory");
            return CA_MEMORY_ALLOC_FAILED;
        }
        memcpy(outInfo->token, token, token_length);
    }

    outInfo->tokenLength = token_length;

    return CA_STATUS_OK;
}

CAResult_t CAGenerateTokenInternal(CAToken_t *token, uint8_t tokenLength)
{
    if (!token)
    {
        OIC_LOG(ERROR, TAG, "invalid token pointer");
        return CA_STATUS_INVALID_PARAM;
    }

    if ((tokenLength > CA_MAX_TOKEN_LEN) || (0 == tokenLength))
    {
        OIC_LOG(ERROR, TAG, "invalid token length");
        return CA_STATUS_INVALID_PARAM;
    }

    if (SEED == 0)
    {
#ifdef ARDUINO
        SEED = now();
#else
        SEED = time(NULL);
#endif
        if (SEED == (unsigned int)((time_t)-1))
        {
            OIC_LOG(ERROR, TAG, "seed is not made");
            SEED = 0;
            return CA_STATUS_FAILED;
        }
#if HAVE_SRANDOM
        srandom(SEED);
#else
        srand(SEED);
#endif
    }

    // memory allocation
    char *temp = (char *) OICCalloc(tokenLength, sizeof(char));
    if (NULL == temp)
    {
        OIC_LOG(ERROR, TAG, "Out of memory");
        return CA_MEMORY_ALLOC_FAILED;
    }

    // set random byte
    for (uint8_t index = 0; index < tokenLength; index++)
    {
        // use valid characters
#if defined(ARDUINO) || defined(WIN32)
        temp[index] = rand() & 0x00FF;
#else
        temp[index] = random() & 0x00FF;
#endif
    }

    // save token
    *token = temp;

    OIC_LOG_V(DEBUG, TAG, "token len:%d, token:", tokenLength);
    OIC_LOG_BUFFER(DEBUG, TAG, (const uint8_t *)(*token), tokenLength);

    return CA_STATUS_OK;
}

void CADestroyTokenInternal(CAToken_t token)
{
    OICFree(token);
}

void CADestroyInfo(CAInfo_t *info)
{
    if (NULL != info)
    {
        OIC_LOG(DEBUG, TAG, "free options");
        OICFree(info->options);

        OIC_LOG(DEBUG, TAG, "free token");
        OICFree(info->token);

        OIC_LOG(DEBUG, TAG, "free payload");
        OICFree(info->payload);
    }
}

uint32_t CAGetOptionData(uint16_t key, const uint8_t *data, uint32_t len,
        uint8_t *option, uint32_t buflen)
{
    if (0 == buflen)
    {
        OIC_LOG(ERROR, TAG, "buflen 0");
        return 0;
    }

    if (buflen <= len)
    {
        OIC_LOG(ERROR, TAG, "option buffer too small");
        return 0;
    }

    coap_option_def_t* def = coap_opt_def(key);
    if(NULL != def && coap_is_var_bytes(def) && 0 == len) {
        // A 0 length option is permitted in CoAP but the
        // rest or the stack is unaware of variable byte encoding
        // should remain that way so a 0 byte of length 1 is inserted.
        len = 1;
        option[0]=0;
    } else {
        memcpy(option, data, len);
        option[len] = '\0';
    }

    return len;
}

CAMessageType_t CAGetMessageTypeFromPduBinaryData(const void *pdu, uint32_t size)
{
    if (NULL == pdu)
    {
        OIC_LOG(ERROR, TAG, "pdu is null");
        return CA_MSG_NONCONFIRM;
    }

    // pdu minimum size is 4 byte.
    if (size < CA_PDU_MIN_SIZE)
    {
        OIC_LOG(ERROR, TAG, "min size");
        return CA_MSG_NONCONFIRM;
    }

    coap_hdr_t *hdr = (coap_hdr_t *) pdu;

    return (CAMessageType_t) hdr->coap_hdr_udp_t.type;
}

uint16_t CAGetMessageIdFromPduBinaryData(const void *pdu, uint32_t size)
{
    if (NULL == pdu)
    {
        OIC_LOG(ERROR, TAG, "pdu is null");
        return 0;
    }

    // pdu minimum size is 4 byte.
    if (size < CA_PDU_MIN_SIZE)
    {
        OIC_LOG(ERROR, TAG, "min size");
        return 0;
    }

    coap_hdr_t *hdr = (coap_hdr_t *) pdu;

    return hdr->coap_hdr_udp_t.id;
}

CAResponseResult_t CAGetCodeFromPduBinaryData(const void *pdu, uint32_t size)
{
    if (NULL == pdu)
    {
        OIC_LOG(ERROR, TAG, "pdu is null");
        return CA_NOT_FOUND;
    }

    // pdu minimum size is 4 byte.
    if (size < CA_PDU_MIN_SIZE)
    {
        OIC_LOG(ERROR, TAG, "min size");
        return CA_NOT_FOUND;
    }

    coap_hdr_t *hdr = (coap_hdr_t *) pdu;

    return (CAResponseResult_t) CA_RESPONSE_CODE(hdr->coap_hdr_udp_t.code);
}

CAPayloadFormat_t CAConvertFormat(uint8_t format)
{
    switch (format)
    {
        case COAP_MEDIATYPE_TEXT_PLAIN:
            return CA_FORMAT_TEXT_PLAIN;
        case COAP_MEDIATYPE_APPLICATION_LINK_FORMAT:
            return CA_FORMAT_APPLICATION_LINK_FORMAT;
        case COAP_MEDIATYPE_APPLICATION_XML:
            return CA_FORMAT_APPLICATION_XML;
        case COAP_MEDIATYPE_APPLICATION_OCTET_STREAM:
            return CA_FORMAT_APPLICATION_OCTET_STREAM;
        case COAP_MEDIATYPE_APPLICATION_RDF_XML:
            return CA_FORMAT_APPLICATION_RDF_XML;
        case COAP_MEDIATYPE_APPLICATION_EXI:
            return CA_FORMAT_APPLICATION_EXI;
        case COAP_MEDIATYPE_APPLICATION_JSON:
            return CA_FORMAT_APPLICATION_JSON;
        case COAP_MEDIATYPE_APPLICATION_CBOR:
            return CA_FORMAT_APPLICATION_CBOR;
        default:
            return CA_FORMAT_UNSUPPORTED;
    }
}

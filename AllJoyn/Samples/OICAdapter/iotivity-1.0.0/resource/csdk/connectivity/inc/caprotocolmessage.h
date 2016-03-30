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
/**
 * @file
 * This file contains common function for handling protocol messages.
 */

#ifndef CA_PROTOCOL_MESSAGE_H_
#define CA_PROTOCOL_MESSAGE_H_

#include "cacommon.h"
#include "config.h"
#include "coap.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint32_t code_t;

#define CA_RESPONSE_CLASS(C) (((C) >> 5)*100)
#define CA_RESPONSE_CODE(C) (CA_RESPONSE_CLASS(C) + (C - COAP_RESPONSE_CODE(CA_RESPONSE_CLASS(C))))


// Include files from the arduino platform do not provide these conversions:
#ifdef ARDUINO
#define htons(x) ( ((x)<< 8 & 0xFF00) | ((x)>> 8 & 0x00FF) )
#define ntohs(x) htons(x)
#else
#define HAVE_TIME_H 1
#endif

static const uint8_t PAYLOAD_MARKER = 1;

/**
 * generates pdu structure from the given information.
 * @param[in]   code                 code of the pdu packet.
 * @param[in]   info                 pdu information.
 * @param[in]   endpoint             endpoint information.
 * @return  generated pdu.
 */
coap_pdu_t *CAGeneratePDU(uint32_t code, const CAInfo_t *info, const CAEndpoint_t *endpoint,
                          coap_list_t **optlist, coap_transport_type *transport);

/**
 * extracts request information from received pdu.
 * @param[in]   pdu                   received pdu.
 * @param[in]   endpoint              endpoint information.
 * @param[out]  outReqInfo            request info structure made from received pdu.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetRequestInfoFromPDU(const coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                                   CARequestInfo_t *outReqInfo);

/**
 * extracts response information from received pdu.
 * @param[in]   pdu                   received pdu.
 * @param[out]  outResInfo            response info structure made from received pdu.
 * @param[in]   endpoint              endpoint information.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetResponseInfoFromPDU(const coap_pdu_t *pdu, CAResponseInfo_t *outResInfo,
                                    const CAEndpoint_t *endpoint);

/**
 * extracts error information from received pdu.
 * @param[in]   pdu                   received pdu.
 * @param[in]   endpoint              endpoint information.
 * @param[out]  errorInfo             error info structure made from received pdu.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetErrorInfoFromPDU(const coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                                 CAErrorInfo_t *errorInfo);

/**
 * creates pdu from the request information.
 * @param[in]   code                 request or response code.
 * @param[in]   info                 information to create pdu.
 * @param[in]   endpoint             endpoint information.
 * @param[out]  options              options for the request and response.
 * @return  generated pdu.
 */
coap_pdu_t *CAGeneratePDUImpl(code_t code, const CAInfo_t *info,
                              const CAEndpoint_t *endpoint, coap_list_t *options,
                              coap_transport_type *transport);

/**
 * parse the URI and creates the options.
 * @param[in]    uriInfo             uri information.
 * @param[out]   options             options information.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAParseURI(const char *uriInfo, coap_list_t **options);

/**
 * Helper that uses libcoap to parse either the path or the parameters of a URI
 * and populate the supplied options list.
 *
 * @param[in]   str                  the input partial URI string (either path or query).
 * @param[in]   length               the length of the supplied partial URI.
 * @param[in]   target               the part of the URI to parse (either COAP_OPTION_URI_PATH.
 *                                   or COAP_OPTION_URI_QUERY).
 * @param[out]  optlist              options information.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAParseUriPartial(const unsigned char *str, size_t length, int target,
                             coap_list_t **optlist);

/**
 * create option list from header information in the info.
 * @param[in]   code                 uri information.
 * @param[in]   info                 information of the request/response.
 * @param[out]  optlist              options information.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAParseHeadOption(uint32_t code, const CAInfo_t *info, coap_list_t **optlist);

/**
 * creates option node from key length and data.
 * @param[in]   key                  key for the that needs to be sent.
 * @param[in]   length               length of the data that needs to be sent.
 * @param[in]   data                 data that needs to be sent.
 * @return  created list.
 */
coap_list_t *CACreateNewOptionNode(uint16_t key, uint32_t length, const char *data);

/**
 * order the inserted options.
 * need to replace queue head if new node has to be added before the existing queue head.
 * @param[in]   a                    option 1 for insertion.
 * @param[in]   b                    option 2 for insertion.
 * @return  0 or 1.
 */
int CAOrderOpts(void *a, void *b);

/**
 * number of options count.
 * @param[in]   opt_iter            option iteration for count.
 * @return number of options.
 */
uint32_t CAGetOptionCount(coap_opt_iterator_t opt_iter);

/**
 * gets option data.
 * @param[in]   key                  ID of the option
 * @param[in]   data                 data that is received.
 * @param[in]   length               length of the data.
 * @param[out]  option               result of the operation.
 * @param[in]   buflen               buffer length of the result.
 * @return  option count.
 */
uint32_t CAGetOptionData(uint16_t key, const uint8_t *data, uint32_t len,
                         uint8_t *option, uint32_t buflen);

/**
 * extracts request information from received pdu.
 * @param[in]    pdu                  received pdu.
 * @param[in]    endpoint             endpoint information.
 * @param[out]   outCode              code of the received pdu.
 * @param[out]   outInfo              request info structure made from received pdu.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetInfoFromPDU(const coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                            uint32_t *outCode, CAInfo_t *outInfo);

/**
 * create pdu from received data.
 * @param[in]   data                received data.
 * @param[in]   length              length of the data received.
 * @param[out]  outCode             code received.
 * @param[in]   endpoint            endpoint information.
 * @return  coap_pdu_t value.
 */
coap_pdu_t *CAParsePDU(const char *data, uint32_t length, uint32_t *outCode,
                       const CAEndpoint_t *endpoint);

/**
 * get Token from received data(pdu).
 * @param[in]    pdu_hdr             header of received pdu.
 * @param[out]   outInfo             information with token received.
 * @param[in]    endpoint            endpoint information.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetTokenFromPDU(const coap_hdr_t *pdu_hdr, CAInfo_t *outInfo,
                             const CAEndpoint_t *endpoint);

/**
 * generates the token.
 * @param[out]   token           generated token.
 * @param[in]    tokenLength     length of the token.
 * @return  CA_STATUS_OK or ERROR CODES (CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGenerateTokenInternal(CAToken_t *token, uint8_t tokenLength);

/**
 * destroys the token.
 * @param[in]   token           generated token.
 */
void CADestroyTokenInternal(CAToken_t token);

/**
 * destroy the ca info structure.
 * @param[in]   info            info structure  created from received  packet.
 */
void CADestroyInfo(CAInfo_t *info);

/**
 * gets message type from PDU binary data.
 * @param[in]   pdu                 pdu data.
 * @param[in]   size                size of pdu data.
 * @return  message type.
 */
CAMessageType_t CAGetMessageTypeFromPduBinaryData(const void *pdu, uint32_t size);

/**
 * gets message ID PDU binary data.
 * @param[in]   pdu                 pdu data.
 * @param[in]   size                size of pdu data.
 * @return  message ID.
 */
uint16_t CAGetMessageIdFromPduBinaryData(const void *pdu, uint32_t size);

/**
 * gets code PDU binary data.
 * @param[in]   pdu                 pdu data.
 * @param[in]   size                size of pdu data.
 * @return  code.
 */
CAResponseResult_t CAGetCodeFromPduBinaryData(const void *pdu, uint32_t size);

/**
 * convert format from CoAP media type encoding to CAPayloadFormat_t.
 * @param[in]   format              coap format code.
 * @return format.
 */
CAPayloadFormat_t CAConvertFormat(uint8_t format);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_PROTOCOL_MESSAGE_H_ */

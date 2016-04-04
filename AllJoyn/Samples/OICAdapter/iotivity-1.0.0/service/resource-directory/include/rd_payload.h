//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef _RESOURCE_DIRECTORY_PAYLOAD_H_
#define _RESOURCE_DIRECTORY_PAYLOAD_H_

#include <cbor.h>

#include "rd_types.h"
#include "logger.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Converts RD payload from structure to CBOR format. It creates the outPayload
 * which is then transmitted over the wire.
 *
 * @param rdPayload Contains structure holding values of OCRDPayload.
 * @param outPayload The payload in the CBOR format converting OCRDPayload
 * structure.
 * @param size Length of the payload.
 *
 * @return ::OC_STACK_OK returns if successful and OC_STACK_ERROR returns if
 * failed in creating CBOR.
 */
OCStackResult OCRDPayloadToCbor(const OCRDPayload *rdPayload,
                                uint8_t *outPayload, size_t *size);

/**
 * Converts CBOR to OCRDPayload.
 *
 * @param rdCBORPayload Payload received from other end in CBOR format.
 * @param outPayload Parsing the values from CBOR into OCRDPayload structure.
 *
 * @return ::OC_STACK_OK returns if successful and OC_STACK_ERROR returns if
 * failed in parsing CBOR.
 */
OCStackResult OCRDCborToPayload(const CborValue *rdCBORPayload, OCPayload **outPayload);

/**
 * Initializes RD payload structure.
 *
 * @param payloadType Defines whether payload is RD_PAYLOAD_TYPE_DISCOVERY or
 * RD_PAYLOAD_TYPE_PUBLISH.
 *
 * @return Allocated memory for the OCRDPayload and NULL in case if failed to
 * allocate memory
 */
OCRDPayload *OCRDPayloadCreate(OCRDPayloadType payloadType);

/**
 * Initializes RD Discovery payload structure and sets the bias factor.
 *
 * @param biasFactor Value specifies the selection factor. It is weigthage of
 * CPU, Memory, Network, Load and Power capability of the RD server.
 *
 * @return Allocated memory for the OCRDDiscoveryPayload and NULL in case if
 * failed to allocate memory.
 */
OCRDDiscoveryPayload *OCRDDiscoveryPayloadCreate(int biasFactor);

/**
 * Creates the links payload, which is then embedded inside OCRDPublishPayload.
 *
 * @param uri  The Web Link Address of the resource.
 * @param rt   The resource type of the published resource.
 * @param itf  The interface type of the published resource.
 * @param linkPayload The address of the allocated memory or NULL in case if failed
 *                    to allocate memory.
 */
void OCRDLinksPayloadCreate(const char *uri, const char *rt, const char *itf,
        OCRDLinksPayload **linkPayload);

/**
 * Creates the links payload, which is then embedded inside OCRDPublishPayload.
 *
 * @param ttl  Time to live of the published resource..
 * @param linkPayload  The link payload with uri, rt and itf.
 *
 * @return  Allocated memory of OCRDPublishPayload or NULL in case if failed
 *          to allocate memory.
 */
OCRDPublishPayload *OCRDPublishPayloadCreate(const int ttl,
        OCRDLinksPayload *linkPayload);

/**
 * Free memory allocation of the RDPayload and its internal structure.
 *
 * @param payload Pointer to already allocated memory for OCRDPayload.
 */
void OCRDPayloadDestroy(OCRDPayload *payload);

/**
 * Logs the content of the OCRDPayload.
 *
 * @param level Log level DEBUG or INFO or ERROR.
 * @param tag File specific tag to use.
 * @param payload Pointer to already allocated memory for OCRDPayload.
 */
void OCRDPayloadLog(LogLevel level, const char *tag, const OCRDPayload *payload);

/**
 * Logs the subset of the OCRDPayload, prints separately OCRDPublish.
 *
 * @param level Log level DEBUG or INFO or ERROR.
 * @param tag File specific tag to use.
 * @param payload Pointer to already allocated memory for OCRDPublish.
 */
void OCRDPublishPayloadLog(LogLevel level, const char *tag,
        const OCRDPublishPayload *payload);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

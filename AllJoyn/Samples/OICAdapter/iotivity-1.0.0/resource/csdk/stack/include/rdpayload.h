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

#ifndef _RDPAYLOAD_H_
#define _RDPAYLOAD_H_

#include <cbor.h>
#include "octypes.h"
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
int64_t OCRDPayloadToCbor(const OCRDPayload *rdPayload, uint8_t *outPayload, size_t *size);

/**
 * Converts tags structure to the tags cbor payload.
 *
 * @param tags Allocated Tag structure
 * @param setMap The cbor map where result will be stored.
 *
 * @return ::OC_STACK_OK returns if successful and OC_STACK_ERROR returns if
 * failed in creating CBOR.
 */
OCStackResult OCTagsPayloadToCbor(OCTagsPayload *tags, CborEncoder *setMap);

/**
 * Converts links structure to cbor map structure
 *
 * @param links Allocated links structure.
 * @param setMap The cbor map where result will be stored.
 *
 * @return ::OC_STACK_OK returns if successful and OC_STACK_ERROR returns if
 * failed in creating CBOR.
 */
OCStackResult OCLinksPayloadToCbor(OCLinksPayload *rtPtr, CborEncoder *setMap);

/**
 * Converts CBOR to OCRDPayload.
 *
 * @param rdCBORPayload Payload received from other end in CBOR format.
 * @param outPayload Parsing the values from CBOR into OCRDPayload structure.
 *
 * @return ::OC_STACK_OK returns if successful and OC_STACK_ERROR returns if
 * failed in parsing CBOR.
 */
OCStackResult OCRDCborToPayload(const CborValue *cborPayload, OCPayload **outPayload);

/**
 * Converts cbor map payload to OCTags payload.
 *
 * @param tagstMap CborValue holding tags structure.
 * @param tagsPayload Allocated tags payload.
 *
 * @return ::OC_STACK_OK returns if successful and OC_STACK_ERROR returns if
 * failed in creating CBOR.
 */
OCStackResult OCTagsCborToPayload(CborValue *tagsMap, OCTagsPayload **tagsPayload);

/**
 * Converts cbor map payload to OCLinks payload.
 *
 * @param tagstMap CborValue holding links structure.
 * @param tagsPayload Allocated links payload.
 *
 * @return ::OC_STACK_OK returns if successful and OC_STACK_ERROR returns if
 * failed in creating CBOR.
 */
OCStackResult OCLinksCborToPayload(CborValue *linksArray, OCLinksPayload **linksPayload);

/**
 * Initializes RD payload structure.
 *
 * @param payloadType Defines whether payload is RD_PAYLOAD_TYPE_DISCOVERY or
 * RD_PAYLOAD_TYPE_PUBLISH.
 *
 * @return Allocated memory for the OCRDPayload and NULL in case if failed to
 * allocate memory
 */
OCRDPayload *OCRDPayloadCreate();

/**
 * Initializes RD Discovery payload structure and sets the bias factor.
 *
 * @param name Name of the discovery device payload.
 * @param identity Device identity of the discovery device.
 * @param biasFactor Value specifies the selection factor. It is weigthage of
 * CPU, Memory, Network, Load and Power capability of the RD server.
 *
 * @return Allocated memory for the OCRDDiscoveryPayload and NULL in case if
 * failed to allocate memory.
 */
OCRDDiscoveryPayload *OCRDDiscoveryPayloadCreate(const char *name, const char *identity, int biasFactor);

/**
 * Free memory allocation of the RDPayload and its internal structure.
 *
 * @param payload Pointer to already allocated memory for OCRDPayload.
 */
void OCRDPayloadDestroy(OCRDPayload *payload);

/**
 * Copies tag paramter to creates OCTagsPayload.
 *
 * @param deviceName The device name as set during enrollment.
 * @param id The device UUID
 * @param baseURI baseURI pointing to the resource directory location.
 * @param bitmap The bitmap value include observe, discovery and secure bit set.
 * @param port The secure port in case above bitmap is set to secure.
 * @param ins Unique value per collection.
 * @param rts Defines allowed resource types.
 * @param drel Defines defaultr relationship.
 * @param ttl Time to leave for the . Used only in resource directory.
 *
 * @retun Allocated memory for OCTagsPayload or else NULL in case of error.
 */
OCTagsPayload* OCCopyTagsResources(const char *deviceName, const unsigned char *id,
    const char *baseURI, uint8_t bitmap, uint16_t port, uint8_t ins, const char *rts, const char *drel, uint32_t ttl);

/**
 * Copies link resource to create LinksPayload.
 *
 * @param href URI of the resource
 * @param rt Array of String pointing to resource types.
 * @param itf Array of String pointing to interface
 * @param rel Relation
 * @param obs Whether to observe or not.
 * @param title Title
 * @param uri URI
 * @param ins Unique value per link.
 * @param mt Media Type

 * @retun Allocated memory for OCLinksPayload or else NULL in case of error.
 */
OCLinksPayload* OCCopyLinksResources(const char *href, OCStringLL *rt, OCStringLL *itf,
    const char *rel, bool obs, const char *title, const char *uri, uint8_t ins, OCStringLL *mt);

/**
 * Creates a resource collection object.
 *
 * @param tags Pointer pointing to tags payload.
 * @param links Pointer pointing to links payload.
 *
 * @return Memory allocation for OCResourceCollectionPayload, else NULL.
 */
OCResourceCollectionPayload* OCCopyCollectionResource(OCTagsPayload *tags, OCLinksPayload *links);

/**
 * Adds discocvery collection in discovery payload.
 *
 * @param payload Pointer to the discovery payload. It adds allocated collection resource.
 * @param tags Pointer to the tags payload.
 * @param links Pointer to the links payload.
 *
 * @return ::OC_STACK_OK returns if successful and OC_STACK_ERROR returns if
 * failed in creating CBOR.
 */

OCStackResult OCDiscoveryCollectionPayloadAddResource(OCDiscoveryPayload *payload,  OCTagsPayload *tags,
    OCLinksPayload *links);

/**
 * Destroys tags payload including internal structure allocated
 *
 * @param tags - Allocated memory of the tags payload.
 */
void OCFreeTagsResource(OCTagsPayload *tags);

/**
 * Destroys allocated links payload including internal structure allocated.
 *
 * @param links - Allocated memory to the links payload.
 */
void OCFreeLinksResource(OCLinksPayload *links);

/**
 * ResourceCollection payload destroy. Includes free up tags and links structure.
 *
 * @param payload Pointer pointing to allocated memroy of ResourceCollection.
 */
void OCFreeCollectionResource(OCResourceCollectionPayload *payload);

/**
 * Discovery collection payload destroy includes internal structure OCResourceCollectionPayload.
 *
 * @param payload Pointer pointing to allocated memory of OCDiscoveryPayload.
 */
void OCDiscoveryCollectionPayloadDestroy(OCDiscoveryPayload* payload);

/**
 * Prints tags payload.
 *
 * @param level LogLevel for the print.
 * @param tags Structure of the tags payload.
 */
void OCTagsLog(const LogLevel level, const OCTagsPayload *tags);

/**
 * Prints links payload.
 *
 * @param level LogLevel for the print.
 * @param tags Structure of the links payload.
 */
void OCLinksLog(const LogLevel level, const OCLinksPayload *links);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* OCTYPES_H_ */

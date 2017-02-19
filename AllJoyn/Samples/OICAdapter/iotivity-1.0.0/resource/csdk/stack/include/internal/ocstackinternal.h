//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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
//



/**
 * @file
 *
 * This file contains the Internal include file used by lower layers of the OC stack
 *
 */

#ifndef OCSTACKINTERNAL_H_
#define OCSTACKINTERNAL_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdbool.h>
#include "ocstack.h"
#include "ocstackconfig.h"
#include "occlientcb.h"
#include <ocrandom.h>

#include "cacommon.h"
#include "cainterface.h"
#include "securevirtualresourcetypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

/** Default device entity Handler.*/
extern OCDeviceEntityHandler defaultDeviceHandler;

/** Default Callback parameter.*/
extern void* defaultDeviceHandlerCallbackParameter;

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

/** The coap scheme */
#define OC_COAP_SCHEME "coap://"

/** the first outgoing sequence number will be 5*/
#define OC_OFFSET_SEQUENCE_NUMBER (4)

/**
 * This structure will be created in occoap and passed up the stack on the server side.
 */
typedef struct
{
    /** Observe option field.*/
    uint32_t observationOption;

    /** The REST method retrieved from received request PDU.*/
    OCMethod method;

    /** the requested payload format. */
    OCPayloadFormat acceptFormat;

    /** resourceUrl will be filled in occoap using the path options in received request PDU.*/
    char resourceUrl[MAX_URI_LENGTH];

    /** resource query send by client.*/
    char query[MAX_QUERY_LENGTH];

    /** reqJSON is retrieved from the payload of the received request PDU.*/
    uint8_t *payload;

    /** qos is indicating if the request is CON or NON.*/
    OCQualityOfService qos;

    /** Number of the received vendor specific header options.*/
    uint8_t numRcvdVendorSpecificHeaderOptions;

    /** Array of received vendor specific header option .*/
    OCHeaderOption rcvdVendorSpecificHeaderOptions[MAX_HEADER_OPTIONS];

    /** Remote end-point address **/
    OCDevAddr devAddr;

    /** Token for the observe request.*/
    CAToken_t requestToken;

    /** token length.*/
    uint8_t tokenLength;

    /** The ID of CoAP PDU.*/
    uint16_t coapID;

    /** For delayed Response.*/
    uint8_t delayedResNeeded;

    /** For More packet.*/
    uint8_t reqMorePacket;

    /** The number of requested packet.*/
    uint32_t reqPacketNum;

    /** The size of requested packet.*/
    uint16_t reqPacketSize;

    /** The number of responded packet.*/
    uint32_t resPacketNum;

    /** Responded packet size.*/
    uint16_t resPacketSize;

    /** The total size of requested packet.*/
    size_t reqTotalSize;
} OCServerProtocolRequest;

/**
 * This structure will be created in occoap and passed up the stack on the client side.
 */
typedef struct
{
    /** handle is retrieved by comparing the token-handle pair in the PDU.*/
    ClientCB * cbNode;

    /** This is how long this response is valid for (in seconds).*/
    uint32_t maxAge;

    /** This is the Uri of the resource. (ex. "coap://192.168.1.1/a/led").*/
    char * fullUri;

    /** This is the relative Uri of the resource. (ex. "/a/led").*/
    char * rcvdUri;

    /** This is the received payload.*/
    char * bufRes;

    /** This is the token received OTA.*/
    CAToken_t rcvdToken;

    /** this structure will be passed to client.*/
    OCClientResponse * clientResponse;
} OCResponse;

/**
 * This typedef is to represent our Server Instance identification.
 */
typedef uint8_t ServerID[16];

//-----------------------------------------------------------------------------
// Internal function prototypes
//-----------------------------------------------------------------------------


/**
 * Handler function for sending a response from multiple resources, such as a collection.
 * Aggregates responses from multiple resource until all responses are received then sends the
 * concatenated response
 *
 * TODO: Need to add a timeout in case a (remote?) resource does not respond
 *
 * @param token         Token to search for.
 * @param tokenLength   Length of token.
 * @param status        Feedback status.
 * @return
 *     ::OCStackResult
 */

OCStackResult OCStackFeedBack(CAToken_t token, uint8_t tokenLength, uint8_t status);


/**
 * Handler function to execute stack requests
 *
 * @param protocolRequest      Pointer to the protocol requests from server.
 *
 * @return
 *     ::OCStackResult
 */
OCStackResult HandleStackRequests(OCServerProtocolRequest * protocolRequest);

OCStackResult SendDirectStackResponse(const CAEndpoint_t* endPoint, const uint16_t coapID,
        const CAResponseResult_t responseResult, const CAMessageType_t type,
        const uint8_t numOptions, const CAHeaderOption_t *options,
        CAToken_t token, uint8_t tokenLength, const char *resourceUri);

#ifdef WITH_PRESENCE

/**
 * Notify Presence subscribers that a resource has been modified.
 *
 * @param resourceType    Handle to the resourceType linked list of resource that was modified.
 * @param trigger         The simplified reason this API was invoked.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult SendPresenceNotification(OCResourceType *resourceType,
        OCPresenceTrigger trigger);

/**
 * Send Stop Notification to Presence subscribers.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult SendStopNotification();
#endif // WITH_PRESENCE

/**
 * Function to parse the IPv4 address.
 *
 * @param ipAddrStr       Pointer to a string of IPv4 address.
 * @param ipAddr          pointer to IPv4 adress.
 * @param port            Port number.
 *
 * @return true on success, false upon failure.
 */
bool ParseIPv4Address(char * ipAddrStr, uint8_t * ipAddr, uint16_t * port);

/**
 * Bind a resource interface to a resource.
 *
 * @param resource Target resource.
 * @param resourceInterfaceName Resource interface.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult BindResourceInterfaceToResource(OCResource* resource,
                                            const char *resourceInterfaceName);
/**
 * Bind a resource type to a resource.
 *
 * @param resource Target resource.
 * @param resourceTypeName Name of resource type.
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult BindResourceTypeToResource(OCResource* resource,
                                            const char *resourceTypeName);


/**
 * Converts a CAResult_t type to a OCStackResult type.
 *
 * @param caResult CAResult_t value to convert.
 * @return OCStackResult that was converted from the input CAResult_t value.
 */
OCStackResult CAResultToOCResult(CAResult_t caResult);

/**
 * Get a byte representation of the server instance ID.
 * The memory is managed internal to this function, so freeing it externally will
 * result in a runtime error.
 *
 * Note: This will NOT seed the RNG, so it must be called after the RNG is seeded.
 * This is done automatically during the OCInit process,
 * so ensure that this call is done after that.
 *
 * @return A uint8_t representation the server instance ID.
 */
const OicUuid_t* OCGetServerInstanceID(void);

/**
 * Map OCQualityOfService to CAMessageType.
 *
 * @param qos Input qos.
 *
 * @return CA message type for a given qos.
 */
CAMessageType_t qualityOfServiceToMessageType(OCQualityOfService qos);

#ifdef WITH_PRESENCE
/**
 * Enable/disable a resource property.
 *
 * @param inputProperty             Pointer to resource property.
 * @param resourceProperties        Property to be enabled/disabled.
 * @param enable                    0:disable, 1:enable.
 *
 * @return OCStackResult that was converted from the input CAResult_t value.
 */
//TODO: should the following function be public?
OCStackResult OCChangeResourceProperty(OCResourceProperty * inputProperty,
        OCResourceProperty resourceProperties, uint8_t enable);
#endif

const char *convertTriggerEnumToString(OCPresenceTrigger trigger);

OCPresenceTrigger convertTriggerStringToEnum(const char * triggerStr);

void CopyEndpointToDevAddr(const CAEndpoint_t *in, OCDevAddr *out);

void CopyDevAddrToEndpoint(const OCDevAddr *in, CAEndpoint_t *out);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* OCSTACKINTERNAL_H_ */

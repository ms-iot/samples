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
 * This file contains APIs for OIC Stack to be implemented.
 */

#ifndef OCSTACK_H_
#define OCSTACK_H_

#include <stdio.h>
#include <stdint.h>
#include "octypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** Macro to use Random port.*/
#define USE_RANDOM_PORT (0)

/*
 * Function prototypes
 */

/**
 * This function Initializes the OC Stack.  Must be called prior to starting the stack.
 *
 * @param mode            OCMode Host device is client, server, or client-server.
 * @param serverFlags     OCTransportFlags Default server transport flags.
 * @param clientFlags     OCTransportFlags Default client transport flags.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCInit1(OCMode mode, OCTransportFlags serverFlags, OCTransportFlags clientFlags);

/**
 * This function Initializes the OC Stack.  Must be called prior to starting the stack.
 *
 * @param ipAddr      IP Address of host device. Deprecated parameter.
 * @param port        Port of host device. Deprecated parameter.
 * @param mode        OCMode Host device is client, server, or client-server.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCInit(const char *ipAddr, uint16_t port, OCMode mode);

#ifdef RA_ADAPTER
/**
 * @brief   Set Remote Access information for XMPP Client.
 * @param   raInfo            [IN] remote access info.
 *
 * @return  #CA_STATUS_OK
 */
OCStackResult OCSetRAInfo(const OCRAInfo_t *raInfo);
#endif

/**
 * This function Stops the OC stack.  Use for a controlled shutdown.
 *
 * @note: OCStop() performs operations similar to OCStopPresence(), as well as OCDeleteResource() on
 * all resources this server is hosting. OCDeleteResource() performs operations similar to
 * OCNotifyAllObservers() to notify all client observers that the respective resource is being
 * deleted.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCStop();

/**
 * This function starts receiving the multicast traffic. This can be only called
 * when stack is in OC_STACK_INITIALIZED state but device is not receiving multicast
 * traffic.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCStartMulticastServer();

/**
 * This function stops receiving the multicast traffic. The rest of the stack
 * keeps working and no resource are deleted. Device can still receive the unicast
 * traffic. Once this is set, no response to multicast /oic/res will be sent by the
 * device. This is to be used for devices that uses other entity to push resources.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCStopMulticastServer();

/**
 * This function is Called in main loop of OC client or server.
 * Allows low-level processing of stack services.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCProcess();

/**
 * This function discovers or Perform requests on a specified resource
 * (specified by that Resource's respective URI).
 *
 * @param handle            To refer to the request sent out on behalf of
 *                          calling this API. This handle can be used to cancel this operation
 *                          via the OCCancel API.
 *                          @note: This reference is handled internally, and should not be free'd by
 *                          the consumer.  A NULL handle is permitted in the event where the caller
 *                          has no use for the return value.
 * @param method            To perform on the resource.
 * @param requestUri       URI of the resource to interact with. (Address prefix is deprecated in
 *                          favor of destination.)
 * @param destination       Complete description of destination.
 * @param payload           Encoded request payload.
 * @param connectivityType  Modifier flags when destination is not given.
 * @param qos               Quality of service. Note that if this API is called on a uri with the
 *                          well-known multicast IP address, the qos will be forced to ::OC_LOW_QOS
 *                          since it is impractical to send other QOS levels on such addresses.
 * @param cbData            Asynchronous callback function that is invoked by the stack when
 *                          discovery or resource interaction is complete.
 * @param options           The address of an array containing the vendor specific header options
 *                          to be sent with the request.
 * @param numOptions        Number of header options to be included.
 *
 * @note: Presence subscription amendments (i.e. adding additional resource type filters by calling
 * this API again) require the use of the same base URI as the original request to successfully
 * amend the presence filters.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCDoResource(OCDoHandle *handle,
                            OCMethod method,
                            const char *requestUri,
                            const OCDevAddr *destination,
                            OCPayload* payload,
                            OCConnectivityType connectivityType,
                            OCQualityOfService qos,
                            OCCallbackData *cbData,
                            OCHeaderOption *options,
                            uint8_t numOptions);
/**
 * This function cancels a request associated with a specific @ref OCDoResource invocation.
 *
 * @param handle       Used to identify a specific OCDoResource invocation.
 * @param qos          Used to specify Quality of Service(read below).
 * @param options      Used to specify vendor specific header options when sending
 *                     explicit observe cancellation.
 * @param numOptions   Number of header options to be included.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCCancel(OCDoHandle handle, OCQualityOfService qos, OCHeaderOption * options,
        uint8_t numOptions);

/**
 * Register Persistent storage callback.
 * @param   persistentStorageHandler  Pointers to open, read, write, close & unlink handlers.
 *
 * @return
 *     OC_STACK_OK                    No errors; Success.
 *     OC_STACK_INVALID_PARAM         Invalid parameter.
 */
OCStackResult OCRegisterPersistentStorageHandler(OCPersistentStorage* persistentStorageHandler);

#ifdef WITH_PRESENCE
/**
 * When operating in  OCServer or  OCClientServer mode,
 * this API will start sending out presence notifications to clients via multicast.
 * Once this API has been called with a success, clients may query for this server's presence and
 * this server's stack will respond via multicast.
 *
 * Server can call this function when it comes online for the first time, or when it comes back
 * online from offline mode, or when it re enters network.
 *
 * @param ttl         Time To Live in seconds.
 *                    @note: If ttl is '0', then the default stack value will be used (60 Seconds).
 *                    If ttl is greater than ::OC_MAX_PRESENCE_TTL_SECONDS, then the ttl will be
 *                    set to ::OC_MAX_PRESENCE_TTL_SECONDS.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCStartPresence(const uint32_t ttl);

/**
 * When operating in OCServer or OCClientServer mode, this API will stop sending
 * out presence notifications to clients via multicast.
 * Once this API has been called with a success this server's stack will not respond to clients
 * querying for this server's presence.
 *
 * Server can call this function when it is terminating, going offline, or when going
 * away from network.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */

OCStackResult OCStopPresence();
#endif


/**
 * This function sets default device entity handler.
 *
 * @param entityHandler      Entity handler function that is called by ocstack to handle requests
 *                           for any undefined resources or default actions.If NULL is passed it
 *                           removes the device default entity handler.
 * @param callbackParameter  Parameter passed back when entityHandler is called.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCSetDefaultDeviceEntityHandler(OCDeviceEntityHandler entityHandler, void* callbackParameter);

/**
 * This function sets device information.
 *
 * @param deviceInfo   Structure passed by the server application containing the device information.
 *
 * @return
 *     ::OC_STACK_OK               no errors.
 *     ::OC_STACK_INVALID_PARAM    invalid parameter.
 *     ::OC_STACK_ERROR            stack process error.
 */
OCStackResult OCSetDeviceInfo(OCDeviceInfo deviceInfo);

/**
 * This function sets platform information.
 *
 * @param platformInfo   Structure passed by the server application containing
 *                       the platform information.
 *
 *
 * @return
 *     ::OC_STACK_OK               no errors.
 *     ::OC_STACK_INVALID_PARAM    invalid parameter.
 *     ::OC_STACK_ERROR            stack process error.
 */
OCStackResult OCSetPlatformInfo(OCPlatformInfo platformInfo);

/**
 * This function creates a resource.
 *
 * @param handle                Pointer to handle to newly created resource. Set by ocstack and
 *                              used to refer to resource.
 * @param resourceTypeName      Name of resource type.  Example: "core.led".
 * @param resourceInterfaceName Name of resource interface.  Example: "core.rw".
 * @param uri                   URI of the resource.  Example:  "/a/led".
 * @param entityHandler         Entity handler function that is called by ocstack to handle
 *                              requests, etc.
 *                              NULL for default entity handler.
 * @param callbackParam     parameter passed back when entityHandler is called.
 * @param resourceProperties    Properties supported by resource.
 *                              Example: ::OC_DISCOVERABLE|::OC_OBSERVABLE.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCCreateResource(OCResourceHandle *handle,
                               const char *resourceTypeName,
                               const char *resourceInterfaceName,
                               const char *uri,
                               OCEntityHandler entityHandler,
                               void* callbackParam,
                               uint8_t resourceProperties);


/**
 * This function adds a resource to a collection resource.
 *
 * @param collectionHandle    Handle to the collection resource.
 * @param resourceHandle      Handle to resource to be added to the collection resource.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCBindResource(OCResourceHandle collectionHandle, OCResourceHandle resourceHandle);

/**
 * This function removes a resource from a collection resource.
 *
 * @param collectionHandle   Handle to the collection resource.
 * @param resourceHandle     Handle to resource to be removed from the collection resource.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCUnBindResource(OCResourceHandle collectionHandle, OCResourceHandle resourceHandle);

/**
 * This function binds a resource type to a resource.
 *
 * @param handle            Handle to the resource.
 * @param resourceTypeName  Name of resource type.  Example: "core.led".
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCBindResourceTypeToResource(OCResourceHandle handle,
                                           const char *resourceTypeName);
/**
 * This function binds a resource interface to a resource.
 *
 * @param handle                  Handle to the resource.
 * @param resourceInterfaceName   Name of resource interface.  Example: "core.rw".
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCBindResourceInterfaceToResource(OCResourceHandle handle,
                                                const char *resourceInterfaceName);

/**
 * This function binds an entity handler to the resource.
 *
 * @param handle            Handle to the resource that the contained resource is to be bound.
 * @param entityHandler     Entity handler function that is called by ocstack to handle requests.
 * @param callbackParameter Context parameter that will be passed to entityHandler.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCBindResourceHandler(OCResourceHandle handle, OCEntityHandler entityHandler,
                                        void *callbackParameter);

/**
 * This function gets the number of resources that have been created in the stack.
 *
 * @param numResources    Pointer to count variable.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCGetNumberOfResources(uint8_t *numResources);

/**
 * This function gets a resource handle by index.
 *
 * @param index   Index of resource, 0 to Count - 1.
 *
 * @return Found  resource handle or NULL if not found.
 */
OCResourceHandle OCGetResourceHandle(uint8_t index);

/**
 * This function deletes resource specified by handle.  Deletes resource and all
 * resource type and resource interface linked lists.
 *
 * @note: OCDeleteResource() performs operations similar to OCNotifyAllObservers() to notify all
 * client observers that "this" resource is being deleted.
 *
 * @param handle          Handle of resource to be deleted.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCDeleteResource(OCResourceHandle handle);

/**
 * Get a string representation the server instance ID.
 * The memory is managed internal to this function, so freeing externally will result
 * in a runtime error.
 * Note: This will NOT seed the RNG, so it must be called after the RNG is seeded.
 * This is done automatically during the OCInit process,
 * so ensure that this call is done after that.
 *
 * @return A string representation  the server instance ID.
 */
const char* OCGetServerInstanceIDString(void);

/**
 * This function gets the URI of the resource specified by handle.
 *
 * @param handle     Handle of resource.
 *
 * @return URI string if resource found or NULL if not found.
 */
const char *OCGetResourceUri(OCResourceHandle handle);

/**
 * This function gets the properties of the resource specified by handle.
 *
 * @param handle                Handle of resource.
 *
 * @return OCResourceProperty   Bitmask or -1 if resource is not found.
 *
 * @note that after a resource is created, the OC_ACTIVE property is set for the resource by the
 * stack.
 */
OCResourceProperty OCGetResourceProperties(OCResourceHandle handle);

/**
 * This function gets the number of resource types of the resource.
 *
 * @param handle            Handle of resource.
 * @param numResourceTypes  Pointer to count variable.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCGetNumberOfResourceTypes(OCResourceHandle handle, uint8_t *numResourceTypes);

/**
 * This function gets name of resource type of the resource.
 *
 * @param handle       Handle of resource.
 * @param index        Index of resource, 0 to Count - 1.
 *
 * @return Resource type name if resource found or NULL if resource not found.
 */
const char *OCGetResourceTypeName(OCResourceHandle handle, uint8_t index);

/**
 * This function gets the number of resource interfaces of the resource.
 *
 * @param handle                 Handle of resource.
 * @param numResourceInterfaces  Pointer to count variable.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCGetNumberOfResourceInterfaces(OCResourceHandle handle,
        uint8_t *numResourceInterfaces);

/**
 * This function gets name of resource interface of the resource.
 *
 * @param handle      Handle of resource.
 * @param index       Index of resource, 0 to Count - 1.
 *
 * @return Resource interface name if resource found or NULL if resource not found.
 */
const char *OCGetResourceInterfaceName(OCResourceHandle handle, uint8_t index);

/**
 * This function gets methods of resource interface of the resource.
 *
 * @param handle      Handle of resource.
 * @param index       Index of resource, 0 to Count - 1.
 *
 * @return Allowed methods if resource found or NULL if resource not found.
 */
uint8_t OCGetResourceInterfaceAllowedMethods(OCResourceHandle handle, uint8_t index);

/**
 * This function gets resource handle from the collection resource by index.
 *
 * @param collectionHandle   Handle of collection resource.
 * @param index              Index of contained resource, 0 to Count - 1.
 *
 * @return Handle to contained resource if resource found or NULL if resource not found.
 */
OCResourceHandle OCGetResourceHandleFromCollection(OCResourceHandle collectionHandle,
        uint8_t index);

/**
 * This function gets the entity handler for a resource.
 *
 * @param handle            Handle of resource.
 *
 * @return Entity handler if resource found or NULL resource not found.
 */
OCEntityHandler OCGetResourceHandler(OCResourceHandle handle);

/**
 * This function notify all registered observers that the resource representation has
 * changed. If observation includes a query the client is notified only if the query is valid after
 * the resource representation has changed.
 *
 * @param handle   Handle of resource.
 * @param qos      Desired quality of service for the observation notifications.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCNotifyAllObservers(OCResourceHandle handle, OCQualityOfService qos);

/**
 * Notify specific observers with updated value of representation.
 * Before this API is invoked by entity handler it has finished processing
 * queries for the associated observers.
 *
 * @param handle                    Handle of resource.
 * @param obsIdList                 List of observation IDs that need to be notified.
 * @param numberOfIds               Number of observation IDs included in obsIdList.
 * @param payload                   Object representing the notification
 * @param qos                       Desired quality of service of the observation notifications.
 *
 * @note: The memory for obsIdList and payload is managed by the entity invoking the API.
 * The maximum size of the notification is 1015 bytes for non-Arduino platforms. For Arduino
 * the maximum size is 247 bytes.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult
OCNotifyListOfObservers (OCResourceHandle handle,
                            OCObservationId  *obsIdList,
                            uint8_t          numberOfIds,
                            const OCRepPayload *payload,
                            OCQualityOfService qos);


/**
 * This function sends a response to a request.
 * The response can be a normal, slow, or block (i.e. a response that
 * is too large to be sent in a single PDU and must span multiple transmissions).
 *
 * @param response   Pointer to structure that contains response parameters.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCDoResponse(OCEntityHandlerResponse *response);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* OCSTACK_H_ */

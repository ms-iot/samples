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

/**
 * @file
 *
 * Implementation of the OCPlatform functionality. It contains a singleton
 * interface that is used only by the OCPlatform namespace and is the
 * central entrance to the stack.
 */

#ifndef __OCPLATFORM_IMPL_H
#define __OCPLATFORM_IMPL_H

#include <map>

#include "OCApi.h"
#include "OCResource.h"
#include "WrapperFactory.h"
#include "OCResourceRequest.h"
#include "OCResourceResponse.h"
#include "OCRepresentation.h"

#include "oc_logger.hpp"

namespace OC
{
    class OCPlatform_impl
    {
    private:
        static PlatformConfig& globalConfig();
    public:
        static void Configure(const PlatformConfig& config);

        static OCPlatform_impl& Instance();

    public:
        // typedef for handle to cancel presence info with
        typedef OCDoHandle OCPresenceHandle;

        virtual ~OCPlatform_impl(void);

        OCStackResult notifyAllObservers(OCResourceHandle resourceHandle);

        OCStackResult notifyAllObservers(OCResourceHandle resourceHandle, QualityOfService QoS);

        OCStackResult notifyListOfObservers(
                    OCResourceHandle resourceHandle,
                    ObservationIds& observationIds,
                    const std::shared_ptr<OCResourceResponse> responsePtr);

        OCStackResult notifyListOfObservers(
                    OCResourceHandle resourceHandle,
                    ObservationIds& observationIds,
                    const std::shared_ptr<OCResourceResponse> responsePtr,
                    QualityOfService QoS);

        OCStackResult findResource(const std::string& host, const std::string& resourceURI,
                    OCConnectivityType connectivityType, FindCallback resourceHandler);

        OCStackResult findResource(const std::string& host, const std::string& resourceURI,
                    OCConnectivityType connectivityType, FindCallback resourceHandler,
                    QualityOfService QoS);

        OCStackResult getDeviceInfo(const std::string& host, const std::string& deviceURI,
                    OCConnectivityType connectivityType, FindDeviceCallback deviceInfoHandler);

        OCStackResult getDeviceInfo(const std::string& host, const std::string& deviceURI,
                    OCConnectivityType connectivityType, FindDeviceCallback deviceInfoHandler,
                    QualityOfService QoS);

        OCStackResult getPlatformInfo(const std::string& host, const std::string& platformURI,
                    OCConnectivityType connectivityType, FindPlatformCallback platformInfoHandler);

        OCStackResult getPlatformInfo(const std::string& host, const std::string& platformURI,
                    OCConnectivityType connectivityType, FindPlatformCallback platformInfoHandler,
                    QualityOfService QoS);

        /**
         * API for Device Discovery
         *
         * @param host Host IP Address. If null or empty, Multicast is performed.
         * @param deviceURI Uri containing address to the virtual device in C Stack
         *                       ("/oic/d")
         * @param deviceInfoHandler device discovery callback
         * @param QualityOfService the quality of communication
         * @return Returns ::OC_STACK_OK if success.
         * @note OCStackResult is defined in ocstack.h.
         */
        OCStackResult getDeviceInfo(const std::string& host, const std::string& deviceURI,
                    FindDeviceCallback deviceInfoHandler);
        OCStackResult getDeviceInfo(const std::string& host, const std::string& deviceURI,
                    FindDeviceCallback deviceInfoHandler, QualityOfService QoS);

        /**
         * API for Platform Discovery
         *
         * @param host Host IP Address. If null or empty, Multicast is performed.
         * @param platformURI Uri containing address to the virtual platform in C Stack
         *                       ("/oic/p")
         * @param platformInfoHandler platform discovery callback
         * @param QualityOfService the quality of communication
         * @return Returns ::OC_STACK_OK if success.
         * @note OCStackResult is defined in ocstack.h.
         */
        OCStackResult getPlatformInfo(const std::string& host, const std::string& platformURI,
                    FindPlatformCallback platformInfoHandler);
        OCStackResult getPlatformInfo(const std::string& host, const std::string& platformURI,
                    FindPlatformCallback platformInfoHandler, QualityOfService QoS);

        /**
        * This API registers a resource with the server
        * @note This API applies to server side only.
        *
        * @param resourceHandle Upon successful registration, resourceHandle will be filled
        * @param resourceURI The URI of the resource. Example: "a/light". See NOTE below
        * @param resourceTypeName The resource type. Example: "light"
        * @param resourceInterface The resource interface (whether it is collection etc).
        * @param entityHandler entity handler callback.
        * @param resourceProperty indicates the property of the resource. Defined in ocstack.h.
        * setting resourceProperty as OC_DISCOVERABLE will allow Discovery of this resource
        * setting resourceProperty as OC_OBSERVABLE will allow observation
        * settings resourceProperty as OC_DISCOVERABLE | OC_OBSERVABLE will allow both discovery
        * and observation
        *
        * @return Returns ::OC_STACK_OK if success.
        * @note "a/light" is a relative URI.
        * Above relative URI will be prepended (by core) with a host IP + namespace "oc"
        * Therefore, fully qualified URI format would be //HostIP-Address/namespace/relativeURI"
        * Example, a relative URI: 'a/light' will result in a fully qualified URI:
        *   //192.168.1.1/oic/a/light"
        * First parameter can take a relative URI and core will take care of preparing the fully
        * qualified URI OR
        * first parameter can take fully qualified URI and core will take that as is for further
        * operations
        * @note OCStackResult is defined in ocstack.h.
        */
        OCStackResult registerResource(OCResourceHandle& resourceHandle,
                        std::string& resourceURI,
                        const std::string& resourceTypeName,
                        const std::string& resourceInterface,
                        EntityHandler entityHandler,
                        uint8_t resourceProperty);

        OCStackResult registerResource(OCResourceHandle& resourceHandle,
                        const std::shared_ptr<OCResource> resource);

        /**
         * This API registers all the device specific information
         *
         * @param deviceInfo Structure containing all the device related information
         *
         * @return Returns ::OC_STACK_OK if success
         * @note OCDeviceInfo is defined in OCStack.h
         */
        OCStackResult registerDeviceInfo(const OCDeviceInfo deviceInfo);

        /**
         * This API registers all the platform specific information
         *
         * @param platformInfo Structure containing all the platform related information
         *
         * @return Returns ::OC_STACK_OK if success
         * @note OCPlatformInfo is defined in OCStack.h
         */
        OCStackResult registerPlatformInfo(const OCPlatformInfo platformInfo);

        OCStackResult setDefaultDeviceEntityHandler(EntityHandler entityHandler);

        OCStackResult unregisterResource(const OCResourceHandle& resourceHandle) const;

        OCStackResult bindResource(const OCResourceHandle collectionHandle,
                    const OCResourceHandle resourceHandle);

        OCStackResult bindResources(const OCResourceHandle collectionHandle,
                    const std::vector<OCResourceHandle>& addedResourceHandleList);

        OCStackResult unbindResource(const OCResourceHandle collectionHandle,
                    const OCResourceHandle resourceHandle);

        OCStackResult unbindResources(const OCResourceHandle collectionHandle,
                        const std::vector<OCResourceHandle>& resourceHandleList);

        OCStackResult bindTypeToResource(const OCResourceHandle& resourceHandle,
                        const std::string& resourceTypeName) const;

        OCStackResult bindInterfaceToResource(const OCResourceHandle& resourceHandle,
                        const std::string& resourceInterfaceName) const;

        OCStackResult startPresence(const unsigned int ttl);

        OCStackResult stopPresence();

        OCStackResult subscribePresence(OCPresenceHandle& presenceHandle, const std::string& host,
                        OCConnectivityType connectivityType, SubscribeCallback presenceHandler);

        OCStackResult subscribePresence(OCPresenceHandle& presenceHandle, const std::string& host,
                        const std::string& resourceType, OCConnectivityType connectivityType,
                        SubscribeCallback presenceHandler);
        OCStackResult unsubscribePresence(OCPresenceHandle presenceHandle);

        OCResource::Ptr constructResourceObject(const std::string& host, const std::string& uri,
                        OCConnectivityType connectivityType, bool isObservable,
                        const std::vector<std::string>& resourceTypes,
                        const std::vector<std::string>& interfaces);
        OCStackResult sendResponse(const std::shared_ptr<OCResourceResponse> pResponse);

        std::weak_ptr<std::recursive_mutex> csdkLock();

    private:
        PlatformConfig m_cfg;

    private:
        std::unique_ptr<WrapperFactory> m_WrapperInstance;
        IServerWrapper::Ptr m_server;
        IClientWrapper::Ptr m_client;
        std::shared_ptr<std::recursive_mutex> m_csdkLock;

    private:
        /**
        * Constructor for OCPlatform_impl. Constructs a new OCPlatform_impl from a given
        * PlatformConfig with appropriate fields
        * @param config PlatformConfig struct which has details such as modeType
        * (server/client/both), in-proc/out-of-proc etc.
        */
        OCPlatform_impl(const PlatformConfig& config);

        /**
        * Private function to initialize the platform
        */
        void init(const PlatformConfig& config);

        /**
        * Private constructor/operators to prevent copying
        * of this object
        */
        OCPlatform_impl(const OCPlatform_impl& other)= delete;
        OCPlatform_impl& operator=(const OCPlatform_impl&) = delete;
        OCPlatform_impl& operator=(const OCPlatform_impl&&) = delete;
    };
}

#endif //__OCPLATFORM_IMPL_H




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

/**
 * @file
 *
 * This file contains the declaration of classes and its members related to RCSRemoteResourceObject
 */

#ifndef RCSREMOTERESOURCEOBJECT_H
#define RCSREMOTERESOURCEOBJECT_H

#include <vector>

#include "RCSResourceAttributes.h"

namespace OIC
{
    namespace Service
    {
        /**
         * The states of caching.
         *
         * @see startCaching
         * @see getCacheState
         */
        enum class CacheState
        {
            NONE, /**< Caching is not started.*/
            UNREADY, /**< Caching is started, but the data is not ready yet.
                          This is the default state after startCaching. */
            READY, /**< The data is ready.*/
            LOST_SIGNAL, /**< Failed to reach the resource. */
        };

        /**
         * The states of monitoring.
         *
         * @see startMonitoring
         * @see getState
         */
        enum class ResourceState
        {
            NONE, /**< Monitoring is not started.*/
            REQUESTED, /**< Monitoring is started and checking state is in progress.
                            This is the default state after startMonitoring. */
            ALIVE, /**< The resource is alive. */
            LOST_SIGNAL, /**< Failed to reach the resource. */
            DESTROYED /**< The resource is deleted. */
        };

        class PrimitiveResource;

        /**
         *
         * This represents a remote resource and provides simple ways to interact with it.
         * Basically this is a client of a remote resource that runs on other device.
         *
         * The class supports features to help get information of a remote resource
         * such as monitoring and caching.
         *
         * @see RCSDiscoveryManager
         *
         */
        class RCSRemoteResourceObject
        {
        public:
            typedef std::shared_ptr< RCSRemoteResourceObject > Ptr;

            /**
             * Callback definition to be invoked when monitoring state is changed.
             *
             * @see startMonitioring
             * @see ResourceState
             */
            typedef std::function< void(ResourceState) > StateChangedCallback;

            /**
             * Callback definition to be invoked when cache is updated.
             *
             * @param attrs the updated attributes
             */
            typedef std::function< void(const RCSResourceAttributes& attrs) > CacheUpdatedCallback;

            /**
             * Callback definition to be invoked when the response of getRemoteAttributes is
             * received.
             *
             * @param attrs the result attributes
             * @param eCode the error code received from the resource
             *
             * @see getRemoteAttributes
             */
            typedef std::function< void(const RCSResourceAttributes& attrs, int eCode) >
                RemoteAttributesGetCallback;

            /**
             * Callback definition to be invoked when the response of setRemoteAttributes is
             * received.
             *
             * @param attrs the result attributes
             * @param eCode the error code received from the resource
             *
             * @see setRemoteAttributes
             */
            typedef std::function< void(const RCSResourceAttributes&, int) >
                RemoteAttributesSetCallback;

        private:
            typedef int CacheID;
            typedef unsigned int BrokerID;

        public:
            //! @cond
            RCSRemoteResourceObject(std::shared_ptr< PrimitiveResource >);
            //! @endcond

            ~RCSRemoteResourceObject();

            /**
             * Returns whether monitoring is enabled.
             *
             * @see startMonitoring()
             */
            bool isMonitoring() const;

            /**
             * Returns whether caching is enabled.
             *
             * @see startCaching()
             */

            bool isCaching() const;

            /**
             * Returns whether the resource is observable.
             *
             */
            bool isObservable() const;

            /**
             * Starts monitoring the resource.
             *
             * Monitoring provides a feature to check the presence of a resource,
             * even when the server is not announcing Presence using startPresnece.
             *
             * @param cb A Callback to get changed resource state.
             *
             * @throws InvalidParameterException If cb is an empty function or null.
             * @throws BadRequestException If monitoring is already started.
             *
             * @note The callback will be invoked in an internal thread.
             *
             * @see StateChangedCallback
             * @see ResourceState
             * @see isMonitoring()
             * @see stopMonitoring()
             *
             */
            void startMonitoring(StateChangedCallback cb);

            /**
             * Stops monitoring the resource.
             *
             * It does nothing if monitoring is not started.
             *
             * @see startMonitoring()
             *
             */
            void stopMonitoring();

            /**
             * Returns the current state of the resource.
             *
             * @see startMonitoring
             */
            ResourceState getState() const;

            /**
             * Starts caching attributes of the resource.
             *
             * This will start caching for the resource.
             * Once caching started it will look for the data updation on the resource
             * and updates the cache data accordingly.
             *
             * It is equivalent to calling startCaching(CacheUpdatedCallback) with an empty function.
             *
             * @see getCacheState()
             * @see getCachedAttributes()
             * @see getCachedAttribute(const std::string&) const
             *
             * @throws BadRequestException
             *
             */
            void startCaching();

            /**
             * Starts caching attributes for the resource.
             *
             * This will start data caching for the resource.
             * Once caching started it will look for the data updation on the resource and
             * updates the cached data accordingly.
             *
             * @param cb If non-empty function, it will be invoked whenever the cache updated.
             *
             * @throws BadRequestException If caching is already started.
             *
             * @note The callback will be invoked in an internal thread.
             *
             * @see CacheUpdatedCallback
             * @see getCacheState()
             * @see isCachedAvailable()
             * @see getCachedAttributes()
             * @see getCachedAttribute(const std::string&) const
             *
             */
            void startCaching(CacheUpdatedCallback cb);

            /**
             * Stops caching.
             *
             * It does nothing if caching is not started.
             *
             * @see startCaching()
             * @see startCaching(CacheUpdatedCallback)
             */
            void stopCaching();

            /**
             * Returns the current cache state.
             *
             */
            CacheState getCacheState() const;

            /**
             * Returns whether cached data is available.
             *
             * Cache will be available always once cache state had been CacheState::READY
             * even if current state is CacheState::LOST_SIGNAL.
             *
             * @see getCacheState()
             */
            bool isCachedAvailable() const;

            /**
             * Gets the cached RCSResourceAttributes data.
             *
             * @pre Cache should be available.
             *
             * @return The cached attributes.
             *
             * @throws BadRequestException If the precondition is not fulfilled.
             *
             * @see RCSResourceAttributes
             * @see isCachedAvailable()
             * @see startCaching()
             * @see startCaching(CacheUpdatedCallback)
             *
             */
            RCSResourceAttributes getCachedAttributes() const;

            /**
             * Gets a particular cached a ResourceAttribute Value.
             *
             * @pre Cache should be available.
             *
             * @return A requested attribute value.
             *
             * @throws BadRequestException If the precondition is not fulfilled.
             * @throws InvalidKeyException If @a key doesn't match the key of any value.
             *
             * @see RCSResourceAttributes::Value
             * @see isCachedAvailable()
             * @see startCaching()
             * @see startCaching(CacheUpdatedCallback)
             *
             */
            RCSResourceAttributes::Value getCachedAttribute(const std::string& key) const;

            /**
             * Gets resource attributes directly from the server.
             *
             * This API send a get request to the resource of interest and provides
             * the attributes to the caller in the RemoteAttributesReceivedCallback.
             *
             * @throws PlatformException If the operation failed
             * @throws InvalidParameterException If cb is an empty function or null.
             *
             * @see RCSResourceAttributes::Value
             *
             * @note The callback will be invoked in an internal thread.
             */
            void getRemoteAttributes(RemoteAttributesGetCallback cb);

            /**
             * Sends a set request with resource attributes to the server.
             *
             * The SetRequest behavior depends on the server, whether updating its attributes or not.
             *
             * @param attributes Attributes to set
             * @param cb A callback to receive the response.
             *
             * @throws PlatformException If the operation failed
             * @throws InvalidParameterException If cb is an empty function or null.
             *
             * @see RCSResourceObject
             * @see RCSResourceObject::SetRequestHandlerPolicy
             *
             * @note The callback will be invoked in an internal thread.
             */
            void setRemoteAttributes(const RCSResourceAttributes& attributes,
                    RemoteAttributesSetCallback cb);

            /**
             * Returns the uri of the resource.
             *
             */
            std::string getUri() const;

            /**
             * Returns the address of the resource .
             *
             */
            std::string getAddress() const;

            /**
             * Returns the resource types of the resource.
             *
             */
            std::vector< std::string > getTypes() const;

            /**
             * Returns the resource interfaces of the resource.
             *
             */
            std::vector< std::string > getInterfaces() const;

        private:
            std::shared_ptr< PrimitiveResource > m_primitiveResource;
            CacheID m_cacheId;
            BrokerID m_brokerId;
        };
    }
}
#endif // RCSREMOTERESOURCEOBJECT_H

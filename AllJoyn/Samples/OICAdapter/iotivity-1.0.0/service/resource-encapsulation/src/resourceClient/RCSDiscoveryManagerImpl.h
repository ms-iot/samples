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
 * This file contains the RCSActiveDiscoveryManagerImpl class which provide APIs to discover the Resource in the network
 * and discovery requests management.
 *
 */

#ifndef RCSDISCOVERYMANAGER_IMPL_H
#define RCSDISCOVERYMANAGER_IMPL_H

#include <memory>
#include <functional>
#include <list>
#include <mutex>
#include <unordered_map>

#include "RCSDiscoveryManager.h"
#include "ExpiryTimer.h"
#include "PrimitiveResource.h"
#include "RCSRemoteResourceObject.h"

namespace OIC
{
    namespace Service
    {
        class RCSDiscoveryManager;
        class PrimitiveResource;
        class RCSAddress;

        /**
         * The class contains discovery request information
         *
         * @see RCSDiscoveryManager
         */
        class DiscoveryRequestInfo
        {
            public:
                DiscoveryRequestInfo(const std::string &, const std::string &,
                        const std::string &, DiscoverCallback);

            private:
                std::string m_address;
                std::string m_relativeUri;
                std::string m_resourceType;
                std::list<std::string> m_receivedIds;
                DiscoverCallback m_discoverCB;
            public:
                void discoverRequest() const;
                bool isKnownResource(const std::shared_ptr<PrimitiveResource>&);
                bool isMatchingAddress(const std::string&) const;
        };


        /**
         * The class contains the resource discovery and management requests methods.
         */
        class RCSDiscoveryManagerImpl
        {
            static unsigned int s_uniqueId;

            public:

                /*
                 * Typedef for callback of requesting presence API
                 *
                 * @see requestMulticastPresence
                 */
                typedef std::function<void(OCStackResult, const unsigned int,
                        const std::string&)> PresenceCallback;

                /*
                 * Typedef for discovery request ID
                 *
                 * @note This is generated for each discovery request
                 */
                typedef unsigned int ID;

            public:

                /*
                 * @return Returns RCSDiscoveryManagerImpl instance.
                 */
                static RCSDiscoveryManagerImpl* getInstance();

                /**
                 * Starting discovery of resource
                 *
                 * @return DiscoverTask pointer
                 *
                 * @param address        A RCSAddress object
                 * @param relativeURI    The relative uri of resource to be searched
                 * @param resourceType   Resource Type
                 * @param cb             A callback to obtain discovered resource
                 *
                 * @throws InvalidParameterException If cb is empty
                 *
                 * @note If relativeURI is empty, will be discovered after be changed into "OC_RSRVD_WELL_KNOWN_URI"
                 * @note If resourceType is empty, will be discovered all resources in network
                 *
                 * @see RCSAddress
                 * @see RCSDiscoveryManager
                 */
                std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> startDiscovery(const RCSAddress& address,
                        const std::string& relativeURI,const std::string& resourceType,
                        RCSDiscoveryManager::ResourceDiscoveredCallback cb);

                void cancel(ID);
                bool isCanceled(ID);

            private:
                RCSDiscoveryManagerImpl();
                ~RCSDiscoveryManagerImpl() = default;

                /**
                 * Requesting presence by multicast
                 */
                void requestMulticastPresence();

                /**
                 * Checking duplicated callback and invoking callback when resource is discovered
                 *
                 * @param resource     A pointer of discovered resource
                 * @param discoverID   The ID of discovery request
                 * @param cb           Callback
                 *
                 * @see PrimitiveResource
                 */
                void onResourceFound(std::shared_ptr<PrimitiveResource> resource, ID discoveryId,
                        const RCSDiscoveryManager::ResourceDiscoveredCallback& cb);

                /**
                 * Discovering resource on all requests and posting timer when timer is expired
                 */
                void onPolling();

                /**
                 * Discovering resource on all requests when supporting presence function resource enter into network
                 *
                 * @param ret          Not used in this class
                 * @param seq          Not used in this class
                 * @param address      A address of supporting presence function resource
                 */
                void onPresence(OCStackResult ret, const unsigned int seq, const std::string& address);

                /**
                 * Creating unique id
                 *
                 * @return Returns the id
                 */
                ID createId();

            public:
                ExpiryTimer m_timer;

            private:
                std::unordered_map<ID,DiscoveryRequestInfo> m_discoveryMap;
                std::mutex m_mutex;
        };
    }
}
#endif // RCSDISCOVERYMANAGER_IMPL_H

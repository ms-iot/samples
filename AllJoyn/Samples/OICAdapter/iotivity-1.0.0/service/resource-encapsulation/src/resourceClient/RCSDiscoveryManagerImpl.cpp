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
#include <limits>

#include "RCSDiscoveryManagerImpl.h"

#include "OCPlatform.h"
#include "PresenceSubscriber.h"
#include "RCSAddressDetail.h"
#include "RCSAddress.h"

namespace
{
    constexpr unsigned int LIMITNUMBER = std::numeric_limits<unsigned int>::max();
    constexpr unsigned int INTERVALTIME = 60000;
}

namespace OIC
{
    namespace Service
    {
        RCSDiscoveryManagerImpl::RCSDiscoveryManagerImpl()
        {
            srand(time(NULL));
            requestMulticastPresence();
            m_timer.post(INTERVALTIME, std::bind(&RCSDiscoveryManagerImpl::onPolling, this));
        }

        RCSDiscoveryManagerImpl* RCSDiscoveryManagerImpl::getInstance()
        {
            static RCSDiscoveryManagerImpl instance;
            return &instance;
        }

        void RCSDiscoveryManagerImpl::onResourceFound(std::shared_ptr< PrimitiveResource > resource,
                    RCSDiscoveryManagerImpl::ID discoveryId,
                    const RCSDiscoveryManager::ResourceDiscoveredCallback& discoverCB)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto it = m_discoveryMap.find(discoveryId);

                if(it == m_discoveryMap.end()) return;
                if(it->second.isKnownResource(resource)) return;
            }
            discoverCB(std::make_shared<RCSRemoteResourceObject>(resource));
        }

        RCSDiscoveryManager::DiscoveryTask::Ptr RCSDiscoveryManagerImpl::startDiscovery
                (const RCSAddress& address, const std::string& relativeUri, const std::string& resourceType,
                        RCSDiscoveryManager::ResourceDiscoveredCallback cb)
        {
            if (!cb)
            {
                throw RCSInvalidParameterException { "Callback is empty" };
            }

            ID discoveryId = createId();
            auto discoverCb = std::bind(&RCSDiscoveryManagerImpl::onResourceFound, this,
                    std::placeholders::_1, discoveryId, std::move(cb));
            DiscoveryRequestInfo discoveryItem(RCSAddressDetail::getDetail(address)->getAddress(), relativeUri,
                    resourceType, std::move(discoverCb));
            discoveryItem.discoverRequest();

            std::lock_guard<std::mutex> lock(m_mutex);
            m_discoveryMap.insert(std::make_pair(discoveryId, std::move(discoveryItem)));

            return std::unique_ptr<RCSDiscoveryManager::DiscoveryTask>(
                    new RCSDiscoveryManager::DiscoveryTask(discoveryId));
        }

        void RCSDiscoveryManagerImpl::requestMulticastPresence()
        {
            static constexpr char MULTICAST_PRESENCE_ADDRESS[] = "coap://" OC_MULTICAST_PREFIX;
            OCDoHandle presenceHandle;
            subscribePresence(presenceHandle, MULTICAST_PRESENCE_ADDRESS, OCConnectivityType::CT_DEFAULT,
                    std::move(std::bind(&RCSDiscoveryManagerImpl::onPresence, this,
                            std::placeholders::_1, std::placeholders::_2,std::placeholders::_3)));
        }

        void RCSDiscoveryManagerImpl::onPolling()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for(const auto& it : m_discoveryMap)
            {
                it.second.discoverRequest();
            }
            m_timer.post(INTERVALTIME, std::bind(&RCSDiscoveryManagerImpl::onPolling, this));
        }

        void RCSDiscoveryManagerImpl::onPresence(OCStackResult ret,
                const unsigned int /*seq*/, const std::string& address)
        {
            if(ret != OC_STACK_OK && ret != OC_STACK_RESOURCE_CREATED) return;

            std::lock_guard<std::mutex> lock(m_mutex);
            for(const auto& it : m_discoveryMap)
            {
                if(it.second.isMatchingAddress(address))
                {
                    it.second.discoverRequest();
                }
            }
        }

        RCSDiscoveryManagerImpl::ID RCSDiscoveryManagerImpl::createId()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            static unsigned int s_uniqueId;

            if(m_discoveryMap.size() >= LIMITNUMBER)
            {
                throw RCSException { "Discovery request is full!" };
            }
            s_uniqueId++;
            while(m_discoveryMap.find(s_uniqueId) != m_discoveryMap.end())
            {
                s_uniqueId++;
            }
            return s_uniqueId;
        }

        void RCSDiscoveryManagerImpl::cancel(unsigned int id)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_discoveryMap.erase(id);
        }

        bool RCSDiscoveryManagerImpl::isCanceled(unsigned int id)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_discoveryMap.find(id);
            if(it == m_discoveryMap.end()) return true;

            return false;
        }

        DiscoveryRequestInfo::DiscoveryRequestInfo(const std::string &address, const std::string &relativeUri,
                const std::string &resourceType, DiscoverCallback cb) : m_address(address),
                        m_relativeUri(relativeUri), m_resourceType(resourceType), m_discoverCB(cb) {}

        void DiscoveryRequestInfo::discoverRequest() const
        {
            OIC::Service::discoverResource(m_address, m_relativeUri + "?rt=" + m_resourceType,
                    OCConnectivityType::CT_DEFAULT, m_discoverCB);
        }

        bool DiscoveryRequestInfo::isKnownResource(const std::shared_ptr<PrimitiveResource>& resource)
        {
            std::string resourceId = resource->getSid() + resource->getUri();

            auto it = std::find(m_receivedIds.begin(), m_receivedIds.end(), resourceId);

            if(it != m_receivedIds.end()) return true;
            m_receivedIds.push_back(resourceId);
            return false;
        }

        bool DiscoveryRequestInfo::isMatchingAddress(const std::string& address) const
        {
            return m_address == RCSAddressDetail::getDetail(RCSAddress::multicast())->getAddress()
                    || m_address == address;
        }
    }
}

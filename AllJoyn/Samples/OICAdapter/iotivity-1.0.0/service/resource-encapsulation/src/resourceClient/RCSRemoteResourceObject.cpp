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

#include "RCSRemoteResourceObject.h"

#include "ResourceBroker.h"
#include "ResourceCacheManager.h"

#include "ScopeLogger.h"

#define TAG PCF("RCSRemoteResourceObject")

namespace
{
    using namespace OIC::Service;

    ResourceState convertBrokerState(BROKER_STATE state)
    {
        SCOPE_LOG_F(DEBUG, TAG);

        switch (state)
        {
            case BROKER_STATE::ALIVE:
                return ResourceState::ALIVE;

            case BROKER_STATE::REQUESTED:
                return ResourceState::REQUESTED;

            case BROKER_STATE::LOST_SIGNAL:
                return ResourceState::LOST_SIGNAL;

            case BROKER_STATE::DESTROYED:
                return ResourceState::DESTROYED;

            case BROKER_STATE::NONE:
                return ResourceState::NONE;
        }

        return ResourceState::NONE;
    }

    CacheState convertCacheState(CACHE_STATE state)
    {
        SCOPE_LOG_F(DEBUG, TAG);

        switch (state)
        {
            case CACHE_STATE::READY:
                return CacheState::READY;

            case CACHE_STATE::READY_YET:
            case CACHE_STATE::UPDATING:
                return CacheState::UNREADY;

            case CACHE_STATE::LOST_SIGNAL:
                return CacheState::LOST_SIGNAL;

            case CACHE_STATE::DESTROYED:
            case CACHE_STATE::NONE:
                return CacheState::NONE;
        }

        return CacheState::NONE;
    }

    OCStackResult hostingCallback(BROKER_STATE state,
            RCSRemoteResourceObject::StateChangedCallback onResourceStateChanged)
    {
        SCOPE_LOG_F(DEBUG, TAG);

        onResourceStateChanged(convertBrokerState(state));
        return OC_STACK_OK;
    }

    OCStackResult cachingCallback(std::shared_ptr< PrimitiveResource >,
            const RCSResourceAttributes& data,
            RCSRemoteResourceObject::CacheUpdatedCallback onCacheUpdated)
    {
        SCOPE_LOG_F(DEBUG, TAG);

        onCacheUpdated(data);
        return OC_STACK_OK;
    }

    void setCallback(const HeaderOptions&, const ResponseStatement& response, int eCode,
            RCSRemoteResourceObject::RemoteAttributesSetCallback onRemoteAttributesSet)
    {
        SCOPE_LOG_F(DEBUG, TAG);

        onRemoteAttributesSet(response.getAttributes(), eCode);
    }

    void getCallback(const HeaderOptions&, const ResponseStatement& response, int eCode,
            RCSRemoteResourceObject::RemoteAttributesGetCallback onRemoteAttributesReceived)
    {
        SCOPE_LOG_F(DEBUG, TAG);

        onRemoteAttributesReceived(response.getAttributes(), eCode);
    }
}

namespace OIC
{
    namespace Service
    {
        RCSRemoteResourceObject::RCSRemoteResourceObject(
                std::shared_ptr< PrimitiveResource > pResource) :
                m_primitiveResource{ pResource },
                m_cacheId{ },
                m_brokerId{ }
        {
        }

        RCSRemoteResourceObject::~RCSRemoteResourceObject()
        {
            SCOPE_LOG_F(DEBUG, TAG);

            stopCaching();
            stopMonitoring();
        }

        bool RCSRemoteResourceObject::isMonitoring() const
        {
            return m_brokerId != 0;
        }

        bool RCSRemoteResourceObject::isCaching() const
        {
            return m_cacheId != 0;
        }

        bool RCSRemoteResourceObject::isObservable() const
        {
            return m_primitiveResource->isObservable();
        }

        void RCSRemoteResourceObject::startMonitoring(StateChangedCallback cb)
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (!cb)
            {
                throw RCSInvalidParameterException{ "startMonitoring : Callback is NULL" };
            }

            if (isMonitoring())
            {
                OC_LOG(DEBUG, TAG, "startMonitoring : already started");
                throw RCSBadRequestException{ "Monitoring already started." };
            }

            m_brokerId = ResourceBroker::getInstance()->hostResource(m_primitiveResource,
                    std::bind(hostingCallback, std::placeholders::_1, std::move(cb)));
        }

        void RCSRemoteResourceObject::stopMonitoring()
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (!isMonitoring())
            {
                OC_LOG(DEBUG, TAG, "stopMonitoring : Not started");
                return;
            }

            ResourceBroker::getInstance()->cancelHostResource(m_brokerId);
            m_brokerId = 0;
        }

        ResourceState RCSRemoteResourceObject::getState() const
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (!isMonitoring())
            {
                return ResourceState::NONE;
            }

            return convertBrokerState(
                    ResourceBroker::getInstance()->getResourceState(m_primitiveResource));
        }

        void RCSRemoteResourceObject::startCaching()
        {
            startCaching({ });
        }

        void RCSRemoteResourceObject::startCaching(CacheUpdatedCallback cb)
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (isCaching())
            {
                OC_LOG(DEBUG, TAG, "startCaching : already Started");
                throw RCSBadRequestException{ "Caching already started." };
            }

            if (cb)
            {
                m_cacheId = ResourceCacheManager::getInstance()->requestResourceCache(
                        m_primitiveResource,
                        std::bind(cachingCallback, std::placeholders::_1, std::placeholders::_2,
                                std::move(cb)), REPORT_FREQUENCY::UPTODATE, 0);
            }
            else
            {
                m_cacheId = ResourceCacheManager::getInstance()->requestResourceCache(
                        m_primitiveResource, { }, REPORT_FREQUENCY::NONE, 0);
            }

            OC_LOG_V(DEBUG, TAG, "startCaching CACHE ID %d", m_cacheId);
        }

        void RCSRemoteResourceObject::stopCaching()
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (!isCaching())
            {
                OC_LOG(DEBUG, TAG, "Caching already terminated");
                return;
            }

            ResourceCacheManager::getInstance()->cancelResourceCache(m_cacheId);
            m_cacheId = 0;
        }

        CacheState RCSRemoteResourceObject::getCacheState() const
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (!isCaching())
            {
                return CacheState::NONE;
            }

            return convertCacheState(
                    ResourceCacheManager::getInstance()->getResourceCacheState(m_primitiveResource));
        }

        bool RCSRemoteResourceObject::isCachedAvailable() const
        {
            if (!isCaching())
            {
                return false;
            }

            return ResourceCacheManager::getInstance()->isCachedData(m_cacheId);
        }

        RCSResourceAttributes RCSRemoteResourceObject::getCachedAttributes() const
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (!isCaching())
            {
                throw RCSBadRequestException{ "Caching not started." };
            }

            if (!isCachedAvailable())
            {
                throw RCSBadRequestException{ "Cache data is not available." };
            }

            return ResourceCacheManager::getInstance()->getCachedData(m_primitiveResource);
        }

        RCSResourceAttributes::Value RCSRemoteResourceObject::getCachedAttribute(
                const std::string& key) const
        {
            SCOPE_LOG_F(DEBUG, TAG);

            return getCachedAttributes().at(key);
        }

        std::string RCSRemoteResourceObject::getUri() const
        {
            return m_primitiveResource->getUri();
        }

        std::string RCSRemoteResourceObject::getAddress() const
        {
            return m_primitiveResource->getHost();
        }

        std::vector< std::string > RCSRemoteResourceObject::getTypes() const
        {
            return m_primitiveResource->getTypes();
        }

        std::vector< std::string > RCSRemoteResourceObject::getInterfaces() const
        {
            return m_primitiveResource->getInterfaces();
        }

        void RCSRemoteResourceObject::getRemoteAttributes(RemoteAttributesGetCallback cb)
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (!cb)
            {
                throw RCSInvalidParameterException{ "getRemoteAttributes : Callback is empty" };
            }

            m_primitiveResource->requestGet(
                    std::bind(getCallback, std::placeholders::_1, std::placeholders::_2,
                            std::placeholders::_3, std::move(cb)));
        }

        void RCSRemoteResourceObject::setRemoteAttributes(const RCSResourceAttributes& attribute,
                RemoteAttributesSetCallback cb)
        {
            SCOPE_LOG_F(DEBUG, TAG);

            if (!cb)
            {
                throw RCSInvalidParameterException{ "setRemoteAttributes : Callback is empty" };
            }

            m_primitiveResource->requestSet(attribute,
                    std::bind(setCallback, std::placeholders::_1, std::placeholders::_2,
                            std::placeholders::_3, cb));
        }
    }
}

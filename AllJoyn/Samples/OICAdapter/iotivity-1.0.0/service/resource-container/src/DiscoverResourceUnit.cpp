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
#include "DiscoverResourceUnit.h"
#include "RCSAddress.h"

using namespace OIC::Service;

DiscoverResourceUnit::DiscoverResourceUnit(const std::string &bundleId)
    : m_bundleId(bundleId)
{
    pUpdatedCB = nullptr;
    isStartedDiscovery = false;
    discoveryTask = nullptr;

    pUpdatedCBFromServer = std::bind(&DiscoverResourceUnit::onUpdate, this,
                                     std::placeholders::_1, std::placeholders::_2);
}

DiscoverResourceUnit::~DiscoverResourceUnit()
{
    pUpdatedCB = nullptr;
    discoveryTask = nullptr;
    pUpdatedCBFromServer = nullptr;

    m_vecRemoteResource.clear();
}

void DiscoverResourceUnit::startDiscover(DiscoverResourceInfo info, UpdatedCB updatedCB)
{
    if (isStartedDiscovery)
    {
        // Already start Discovery
        return;
    }

    m_Uri = info.resourceUri;
    m_ResourceType = info.resourceType;
    m_AttrubuteName = info.attributeName;
    pUpdatedCB = updatedCB;

    try
    {
        // TODO may be will changed active discovery
        if (m_Uri.empty())
        {
            pDiscoveredCB = std::bind(&DiscoverResourceUnit::discoverdCB, this, std::placeholders::_1,
                                      std::string(""));
        }
        else
        {
            pDiscoveredCB = std::bind(&DiscoverResourceUnit::discoverdCB, this, std::placeholders::_1, m_Uri);
        }

        discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
                            RCSAddress::multicast(), m_ResourceType, pDiscoveredCB);
    }
    catch (RCSInvalidParameterException &e)
    {
        // TODO Handle Exception
        return;
    }

    isStartedDiscovery = true;
}

void DiscoverResourceUnit::discoverdCB(RCSRemoteResourceObject::Ptr remoteObject, std::string uri)
{
    if (remoteObject && !isAlreadyDiscoveredResource(remoteObject))
    {
        RemoteResourceUnit::Ptr newDiscoveredResource =
            RemoteResourceUnit::createRemoteResourceInfo(remoteObject, pUpdatedCBFromServer);

        if (uri.empty() || uri.compare(remoteObject->getUri()) == 0)
        {
            m_vecRemoteResource.push_back(newDiscoveredResource);
            newDiscoveredResource->startMonitoring();
            newDiscoveredResource->startCaching();
        }
    }
    else
    {
        // Already Discovered Resource
    }
}

void DiscoverResourceUnit::onUpdate(REMOTE_MSG msg, RCSRemoteResourceObject::Ptr updatedResource)
{
    if (msg == REMOTE_MSG::DATA_UPDATED)
    {
        if (updatedResource == nullptr)
        {
            return;
        }
        try
        {
            updatedResource->getCachedAttribute(m_AttrubuteName);
        }
        catch (RCSInvalidKeyException &e)
        {
            // TODO Handle Exception
            return;
        }
        catch (std::exception &e)
        {
            return;
        }

        std::vector<RCSResourceAttributes::Value> retVector
            = buildInputResourceData(updatedResource);
        if (!retVector.empty() && pUpdatedCB != nullptr)
        {
            pUpdatedCB(m_AttrubuteName, retVector);
        }
    }
    else
    {
        // TODO find & delete
    }
}

std::vector<RCSResourceAttributes::Value> DiscoverResourceUnit::buildInputResourceData(
    RCSRemoteResourceObject::Ptr updatedResource)
{
    (void)updatedResource;
    std::vector<RCSResourceAttributes::Value> retVector = {};
    for (auto iter : m_vecRemoteResource)
    {
        if (iter->getRemoteResourceObject()->getCacheState() != CacheState::READY)
        {
            continue;
        }

        try
        {
            RCSResourceAttributes::Value value =
                iter->getRemoteResourceObject()->getCachedAttribute(m_AttrubuteName);
            retVector.push_back(value);

        }
        catch (RCSInvalidKeyException &e)
        {
            // TODO Handle Exception
        }
    }

    return retVector;
}

bool DiscoverResourceUnit::isAlreadyDiscoveredResource(
    RCSRemoteResourceObject::Ptr discoveredResource)
{
    for (auto iter : m_vecRemoteResource)
    {
        if (discoveredResource->getUri().compare(iter->getRemoteResourceUri()) == 0 &&
            discoveredResource->getAddress().compare(iter->getRemoteResourceObject()->getAddress()) == 0 )
        {
            return true;
        }
    }
    return false;
}

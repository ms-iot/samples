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

#include "RemoteResourceUnit.h"

using namespace OIC::Service;

RemoteResourceUnit::RemoteResourceUnit()
{
    pStateChangedCB = std::bind(&RemoteResourceUnit::stateChangedCB, this,
                        std::placeholders::_1);
    pCacheUpdateCB = std::bind(&RemoteResourceUnit::cacheUpdateCB, this,
                    std::placeholders::_1);
}

RemoteResourceUnit::~RemoteResourceUnit()
{
    if (remoteObject)
    {
        if(remoteObject->isCaching())
        {
            remoteObject->stopCaching();
        }
        if(remoteObject->isMonitoring())
        {
            remoteObject->stopMonitoring();
        }
    }
}

RemoteResourceUnit::Ptr RemoteResourceUnit::createRemoteResourceInfo(
        RCSRemoteResourceObject::Ptr ptr, UpdatedCBFromServer updatedCB)
{
    RemoteResourceUnit::Ptr retRemoteResourceUnit = std::make_shared<RemoteResourceUnit>();
    retRemoteResourceUnit->remoteObject = ptr;
    retRemoteResourceUnit->pUpdatedCB = updatedCB;

    return retRemoteResourceUnit;
}

RemoteResourceUnit::Ptr RemoteResourceUnit::createRemoteResourceInfoWithStateCB(
    RCSRemoteResourceObject::Ptr ptr, UpdatedCBFromServer updatedCB,
    RCSRemoteResourceObject::StateChangedCallback stateCB)
{
    RemoteResourceUnit::Ptr retRemoteResourceUnit = std::make_shared<RemoteResourceUnit>();
    retRemoteResourceUnit->remoteObject = ptr;
    retRemoteResourceUnit->pUpdatedCB = updatedCB;

    retRemoteResourceUnit->pStateChangedCB = stateCB;

    return retRemoteResourceUnit;
}

RemoteResourceUnit::Ptr RemoteResourceUnit::createRemoteResourceInfoWithCacheCB(
    RCSRemoteResourceObject::Ptr ptr, UpdatedCBFromServer updatedCB,
    RCSRemoteResourceObject::CacheUpdatedCallback cacheCB)
{
    RemoteResourceUnit::Ptr retRemoteResourceUnit = std::make_shared<RemoteResourceUnit>();
    retRemoteResourceUnit->remoteObject = ptr;
    retRemoteResourceUnit->pUpdatedCB = updatedCB;

    retRemoteResourceUnit->pCacheUpdateCB = cacheCB;

    return retRemoteResourceUnit;
}

RCSRemoteResourceObject::Ptr RemoteResourceUnit::getRemoteResourceObject() const
{
    return remoteObject;
}

std::string RemoteResourceUnit::getRemoteResourceUri() const
{
    return remoteObject->getUri();
}

void RemoteResourceUnit::startCaching() const
{
    remoteObject->startCaching(pCacheUpdateCB);
}

void RemoteResourceUnit::startMonitoring() const
{
    remoteObject->startMonitoring(pStateChangedCB);
}

void RemoteResourceUnit::stateChangedCB(ResourceState changedState) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if(changedState == ResourceState::LOST_SIGNAL ||
       changedState == ResourceState::DESTROYED)
    {
        pUpdatedCB(UPDATE_MSG::RESOURCE_DELETED, remoteObject);
    }
}

void RemoteResourceUnit::cacheUpdateCB(const RCSResourceAttributes & updatedAtt) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    (void)updatedAtt;
    pUpdatedCB(UPDATE_MSG::DATA_UPDATED, remoteObject);
}

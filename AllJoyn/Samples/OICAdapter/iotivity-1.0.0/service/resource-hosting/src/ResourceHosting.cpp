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

#include "ResourceHosting.h"

#include "PresenceSubscriber.h"
#include "OCPlatform.h"
#include "RCSDiscoveryManager.h"

namespace OIC
{
namespace Service
{

namespace
{
    std::string HOSTING_TAG = "/hosting";
    size_t HOSTING_TAG_SIZE = (size_t)HOSTING_TAG.size();
    std::string MULTICAST_PRESENCE_ADDRESS = std::string("coap://") + OC_MULTICAST_PREFIX;
    std::string HOSTING_RESOURSE_TYPE = "oic.r.resourcehosting";
}

ResourceHosting * ResourceHosting::s_instance(nullptr);
std::mutex ResourceHosting::s_mutexForCreation;

ResourceHosting::ResourceHosting()
: hostingObjectList(),
  discoveryManager(nullptr),
  pDiscoveryCB(nullptr)
{
}

ResourceHosting * ResourceHosting::getInstance()
{
    if (!s_instance)
    {
        s_mutexForCreation.lock();
        if (!s_instance)
        {
            s_instance = new ResourceHosting();
            s_instance->initializeResourceHosting();
        }
        s_mutexForCreation.unlock();
    }
    return s_instance;
}

void ResourceHosting::startHosting()
{
    try
    {
        requestMulticastDiscovery();
    }catch(const RCSPlatformException &e)
    {
        OIC_HOSTING_LOG(DEBUG,
                "[ResourceHosting::startHosting]PlatformException:%s", e.what());
        throw;
    }catch(const RCSInvalidParameterException &e)
    {
        OIC_HOSTING_LOG(DEBUG,
                "[ResourceHosting::startHosting]InvalidParameterException:%s", e.what());
        throw;
    }catch(const std::exception &e)
    {
        OIC_HOSTING_LOG(DEBUG,
                "[ResourceHosting::startHosting]std::exception:%s", e.what());
        throw;
    }
}

void ResourceHosting::stopHosting()
{

    hostingObjectList.clear();
}

void ResourceHosting::initializeResourceHosting()
{
    pDiscoveryCB = std::bind(&ResourceHosting::discoverHandler, this,
            std::placeholders::_1);

    discoveryManager = RCSDiscoveryManager::getInstance();
}

void ResourceHosting::requestMulticastDiscovery()
{
    discoveryTask = discoveryManager->discoverResourceByType(
            RCSAddress::multicast(), OC_RSRVD_WELL_KNOWN_URI, HOSTING_RESOURSE_TYPE, pDiscoveryCB);
}

void ResourceHosting::discoverHandler(RemoteObjectPtr remoteResource)
{
    std::string discoverdUri = remoteResource->getUri();
    if(discoverdUri.compare(
            discoverdUri.size()-HOSTING_TAG_SIZE, HOSTING_TAG_SIZE, HOSTING_TAG) != 0)
    {
        return;
    }

    HostingObjectPtr foundHostingObject = findRemoteResource(remoteResource);
    if(foundHostingObject == nullptr)
    {
        try
        {
            foundHostingObject = std::make_shared<HostingObject>();
            foundHostingObject->initializeHostingObject(remoteResource,
                    std::bind(&ResourceHosting::destroyedHostingObject, this,
                            HostingObjectWeakPtr(foundHostingObject)));
            hostingObjectList.push_back(foundHostingObject);
        }catch(const RCSInvalidParameterException &e)
        {
            OIC_HOSTING_LOG(DEBUG,
                    "[ResourceHosting::discoverHandler]InvalidParameterException:%s", e.what());
        }
    }
}

ResourceHosting::HostingObjectPtr ResourceHosting::findRemoteResource(
        RemoteObjectPtr remoteResource)
{
    HostingObjectPtr retObject = nullptr;

    for(auto it : hostingObjectList)
    {
        RemoteObjectPtr inListPtr = it->getRemoteResource();
        if(inListPtr != nullptr && isSameRemoteResource(inListPtr, remoteResource))
        {
            retObject = it;
        }
    }

    return retObject;
}

bool ResourceHosting::isSameRemoteResource(
        RemoteObjectPtr remoteResource_1, RemoteObjectPtr remoteResource_2)
{
    bool ret = false;
    if(remoteResource_1->getAddress() == remoteResource_2->getAddress() &&
       remoteResource_1->getUri() == remoteResource_2->getUri())
    {
        ret = true;
    }
    return ret;
}

void ResourceHosting::destroyedHostingObject(HostingObjectWeakPtr destroyedWeakPtr)
{
    auto destroyedPtr = destroyedWeakPtr.lock();
    if (destroyedPtr) return;

    std::unique_lock<std::mutex> lock(mutexForList);
    hostingObjectList.remove(destroyedPtr);
}

} /* namespace Service */
} /* namespace OIC */

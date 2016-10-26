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

#include "HostingObject.h"

namespace OIC
{
namespace Service
{

void OIC_HOSTING_LOG(LogLevel level, const char * format, ...)
{
    if (!format)
    {
        return;
    }
    char buffer[MAX_LOG_V_BUFFER_SIZE] = {};
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof buffer - 1, format, args);
    va_end(args);
    OCLog(level, PCF("Hosting"), buffer);
}

namespace
{
    std::mutex mutexForCB;
}

HostingObject::HostingObject()
: remoteObject(nullptr), mirroredServer(nullptr),
  remoteState(ResourceState::NONE),
  pStateChangedCB(nullptr), pDataUpdateCB(nullptr),
  pDestroyCB(nullptr), pSetRequestHandler(nullptr)
{
}

HostingObject::~HostingObject()
{
	// shared_ptr release
    pStateChangedCB = {};
    pDataUpdateCB = {};

    if (remoteObject)
    {
        remoteObject->stopMonitoring();
        remoteObject->stopCaching();
    }
}

HostingObject::RemoteObjectPtr HostingObject::getRemoteResource() const
{
    return remoteObject;
}

void HostingObject::initializeHostingObject(RemoteObjectPtr rResource, DestroyedCallback destroyCB)
{
    remoteObject = rResource;

    pStateChangedCB = std::bind(&HostingObject::stateChangedCB, this,
            std::placeholders::_1, remoteObject);
    pDataUpdateCB = std::bind(&HostingObject::dataChangedCB, this,
            std::placeholders::_1, remoteObject);
    pDestroyCB = destroyCB;

    pSetRequestHandler = std::bind(&HostingObject::setRequestHandler, this,
            std::placeholders::_1, std::placeholders::_2);

    try
    {
        remoteObject->startMonitoring(pStateChangedCB);
        remoteObject->startCaching(pDataUpdateCB);
    }catch(...)
    {
        throw;
    }
}

void HostingObject::destroyHostingObject()
{
    pDestroyCB();
}

void HostingObject::stateChangedCB(ResourceState state, RemoteObjectPtr rObject)
{
    remoteState = state;

    switch (state)
    {
    case ResourceState::ALIVE:
    {
        if(rObject->isCaching() == false)
        {
            try
            {
                rObject->startCaching(pDataUpdateCB);
            }catch(const RCSInvalidParameterException &e)
            {
                OIC_HOSTING_LOG(DEBUG,
                        "[HostingObject::stateChangedCB]startCaching InvalidParameterException:%s",
                        e.what());
            }
        }
        break;
    }
    case ResourceState::LOST_SIGNAL:
    case ResourceState::DESTROYED:
    {
        if(rObject->isCaching() == true)
        {
            try
            {
                rObject->stopCaching();
            }catch(const RCSInvalidParameterException &e)
            {
                OIC_HOSTING_LOG(DEBUG,
                        "[HostingObject::stateChangedCB]stopCaching InvalidParameterException:%s",
                        e.what());
            }
        }
        if(rObject->isMonitoring() == true)
        {
            try
            {
                rObject->stopMonitoring();
            }catch(const RCSInvalidParameterException &e)
            {
                OIC_HOSTING_LOG(DEBUG,
                        "[HostingObject::stateChangedCB]stopWatching InvalidParameterException:%s",
                        e.what());
            }
        }
        mirroredServer = nullptr;
        destroyHostingObject();
        break;
    }
    default:
        // not support of state
        break;
    }
}

void HostingObject::dataChangedCB(const RCSResourceAttributes & attributes, RemoteObjectPtr rObject)
{
    std::unique_lock<std::mutex> lock(mutexForCB);
    if(attributes.empty())
    {
        return;
    }

    if(mirroredServer == nullptr)
    {
        try
        {
            mirroredServer = createMirroredServer(rObject);
        }catch(const RCSPlatformException &e)
        {
            OIC_HOSTING_LOG(DEBUG,
                        "[HostingObject::dataChangedCB]createMirroredServer PlatformException:%s",
                        e.what());
            mirroredServer = nullptr;
            return;
        }
    }

    RCSResourceAttributes rData;
    {
        RCSResourceObject::LockGuard guard(mirroredServer);
        rData = mirroredServer->getAttributes();
    }
    if(rData.empty() || rData != attributes)
    {
        {
            RCSResourceObject::LockGuard guard(mirroredServer);
            for(auto it = rData.begin(); ; ++it)
            {
                if(it == rData.end())
                {
                    break;
                }
                mirroredServer->removeAttribute(it->key());
            }

            for(auto it = attributes.begin();; ++it)
            {
                if(it == attributes.end())
                {
                    break;
                }
                mirroredServer->setAttribute(it->key(), it->value());
            }
        }
    }
}

HostingObject::ResourceObjectPtr HostingObject::createMirroredServer(RemoteObjectPtr rObject)
{
    ResourceObjectPtr retResource = nullptr;
    if(rObject != nullptr)
    {
        std::string fulluri = rObject->getUri();
        std::string uri = fulluri.substr(0, fulluri.size()-8);
        std::vector<std::string> types = rObject->getTypes();
        std::vector<std::string> interfaces = rObject->getInterfaces();
        try
        {
            std::string type = types.begin()->c_str();
            std::string interface = interfaces.begin()->c_str();
            retResource = RCSResourceObject::Builder(uri, type, interface).
                    setDiscoverable(true).setObservable(true).build();

            // TODO need to bind types and interfaces
            retResource->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);
            retResource->setSetRequestHandler(pSetRequestHandler);
        }catch(...)
        {
            OIC_HOSTING_LOG(DEBUG, "[HostingObject::createMirroredServer] %s", "PlatformException");
            throw;
        }
    }
    else
    {
        throw RCSPlatformException(OC_STACK_ERROR);
    }

    return retResource;
}

RCSSetResponse HostingObject::setRequestHandler(const RCSRequest & primitiveRequest,
            RCSResourceAttributes & resourceAttibutes)
{
    (void)primitiveRequest;
    try
    {
        RequestObject newRequest = { };
        newRequest.invokeRequest(remoteObject, RequestObject::RequestMethod::Setter,
                resourceAttibutes);
    }catch(const RCSPlatformException &e)
    {
        OIC_HOSTING_LOG(DEBUG,
                "[HostingObject::setRequestHandler] PlatformException:%s",
                e.what());
        throw;
    }

    return RCSSetResponse::create(resourceAttibutes);
}

} /* namespace Service */
} /* namespace OIC */

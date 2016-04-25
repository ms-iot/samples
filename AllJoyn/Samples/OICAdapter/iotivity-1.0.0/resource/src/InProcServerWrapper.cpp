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

#include <random>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <map>
#include <sstream>
#include <string>

#include <InProcServerWrapper.h>
#include <InitializeException.h>
#include <OCResourceRequest.h>
#include <OCResourceResponse.h>
#include <ocstack.h>
#include <OCApi.h>
#include <oic_malloc.h>
#include <OCPlatform.h>
#include <OCUtilities.h>

using namespace std;
using namespace OC;

namespace OC
{
    namespace details
    {
        std::mutex serverWrapperLock;
        std::map <OCResourceHandle, OC::EntityHandler>  entityHandlerMap;
        std::map <OCResourceHandle, std::string> resourceUriMap;
        EntityHandler defaultDeviceEntityHandler{};
    }
}

void formResourceRequest(OCEntityHandlerFlag flag,
                         OCEntityHandlerRequest * entityHandlerRequest,
                         std::shared_ptr<OCResourceRequest> pRequest)
{
    if(pRequest && entityHandlerRequest)
    {
        pRequest->setRequestHandle(entityHandlerRequest->requestHandle);
        pRequest->setResourceHandle(entityHandlerRequest->resource);
    }

    if(flag & OC_REQUEST_FLAG)
    {
        pRequest->setRequestHandlerFlag(OC::RequestHandlerFlag::RequestFlag);

        if(entityHandlerRequest)
        {
            if(entityHandlerRequest->query)
            {
                OC::Utilities::QueryParamsKeyVal qp = OC::Utilities::getQueryParams(
                        entityHandlerRequest->query);

                if(qp.size() > 0)
                {
                    pRequest->setQueryParams(qp);
                }
            }
            if(entityHandlerRequest->numRcvdVendorSpecificHeaderOptions != 0)
            {
                //Set the header options here.
                uint16_t optionID;
                std::string optionData;
                HeaderOptions headerOptions;

                for(int i = 0;
                    i < entityHandlerRequest->numRcvdVendorSpecificHeaderOptions;
                    i++)
                {
                    optionID = entityHandlerRequest->rcvdVendorSpecificHeaderOptions[i].optionID;
                    optionData = reinterpret_cast<const char*>
                             (entityHandlerRequest->rcvdVendorSpecificHeaderOptions[i].optionData);
                    HeaderOption::OCHeaderOption headerOption(optionID, optionData);
                    headerOptions.push_back(headerOption);
                }
                pRequest->setHeaderOptions(headerOptions);
            }

            if(OC_REST_GET == entityHandlerRequest->method)
            {
                pRequest->setRequestType(OC::PlatformCommands::GET);
            }
            else if(OC_REST_PUT == entityHandlerRequest->method)
            {
                pRequest->setRequestType(OC::PlatformCommands::PUT);
                pRequest->setPayload(entityHandlerRequest->payload);
            }
            else if(OC_REST_POST == entityHandlerRequest->method)
            {
                pRequest->setRequestType(OC::PlatformCommands::POST);
                pRequest->setPayload(entityHandlerRequest->payload);
            }
            else if(OC_REST_DELETE == entityHandlerRequest->method)
            {
                pRequest->setRequestType(OC::PlatformCommands::DELETE);
            }
        }
    }

    if(flag & OC_OBSERVE_FLAG)
    {
        pRequest->setRequestHandlerFlag(
                   OC::RequestHandlerFlag::RequestFlag | OC::RequestHandlerFlag::ObserverFlag);

        if(entityHandlerRequest)
        {
            OC::ObservationInfo observationInfo;
            observationInfo.action = (OC::ObserveAction) entityHandlerRequest->obsInfo.action;
            observationInfo.obsId = entityHandlerRequest->obsInfo.obsId;

            observationInfo.connectivityType = static_cast<OCConnectivityType>(
                    (entityHandlerRequest->devAddr.adapter << CT_ADAPTER_SHIFT) |
                    (entityHandlerRequest->devAddr.flags & CT_MASK_FLAGS));
            observationInfo.address = entityHandlerRequest->devAddr.addr;
            observationInfo.port = entityHandlerRequest->devAddr.port;
            pRequest->setObservationInfo(observationInfo);
        }
    }
}

OCEntityHandlerResult DefaultEntityHandlerWrapper(OCEntityHandlerFlag flag,
                                                  OCEntityHandlerRequest * entityHandlerRequest,
                                                  char* uri,
                                                  void * /*callbackParam*/)
{
    OCEntityHandlerResult result = OC_EH_ERROR;

    OC::oclog() << "In Default device entity handler wrapper";

    if(NULL == entityHandlerRequest)
    {
        oclog() << "Entity handler request is NULL.";
        return OC_EH_ERROR;
    }

    auto pRequest = std::make_shared<OC::OCResourceRequest>();

    formResourceRequest(flag, entityHandlerRequest, pRequest);

    pRequest->setResourceUri(std::string(uri));

    EntityHandler defHandler;
    {
        std::lock_guard<std::mutex> lock(OC::details::serverWrapperLock);
        defHandler = OC::details::defaultDeviceEntityHandler;
    }

    if(defHandler)
    {
        result = defHandler(pRequest);
    }
    else
    {
        oclog() << "Default device entity handler was not set.";
        return OC_EH_ERROR;
    }

    return result;
}


OCEntityHandlerResult EntityHandlerWrapper(OCEntityHandlerFlag flag,
                                           OCEntityHandlerRequest * entityHandlerRequest,
                                           void* /*callbackParam*/)
{
    OCEntityHandlerResult result = OC_EH_ERROR;

    oclog() << "\nIn entity handler wrapper: " << endl;

    if(NULL == entityHandlerRequest)
    {
        oclog() << "Entity handler request is NULL."  << endl;
        return OC_EH_ERROR;
    }

    auto pRequest = std::make_shared<OC::OCResourceRequest>();

    formResourceRequest(flag, entityHandlerRequest, pRequest);

    std::map <OCResourceHandle, std::string>::iterator resourceUriEntry;
    std::map <OCResourceHandle, std::string>::iterator resourceUriEnd;
    {
        std::lock_guard<std::mutex> lock(OC::details::serverWrapperLock);
        resourceUriEntry = OC::details::resourceUriMap.find(entityHandlerRequest->resource);
        resourceUriEnd = OC::details::resourceUriMap.end();
    }
    // Finding the corresponding URI for a resource handle and set the URI in the request
    if(resourceUriEntry != resourceUriEnd)
    {
        pRequest->setResourceUri(resourceUriEntry->second);
    }
    else
    {
        oclog() << "Resource handle not found; Resource URI not set in request";
        return OC_EH_ERROR;
    }

    std::map <OCResourceHandle, OC::EntityHandler>::iterator entityHandlerEntry;
    std::map <OCResourceHandle, OC::EntityHandler>::iterator entityHandlerEnd;
    {
        // Finding the corresponding CPP Application entityHandler for a given resource
        std::lock_guard<std::mutex> lock(OC::details::serverWrapperLock);
        entityHandlerEntry = OC::details::entityHandlerMap.find(entityHandlerRequest->resource);
        entityHandlerEnd = OC::details::entityHandlerMap.end();
    }

    if(entityHandlerEntry != entityHandlerEnd)
    {
        // Call CPP Application Entity Handler
        if(entityHandlerEntry->second)
        {
            result = entityHandlerEntry->second(pRequest);
        }
        else
        {
            oclog() << "C stack should not call again for parent resource\n";
            return OC_EH_ERROR;
        }
    }
    else
    {
        oclog() << "No entity handler found."  << endl;
        return OC_EH_ERROR;
    }

    return result;
}

namespace OC
{
    InProcServerWrapper::InProcServerWrapper(
        std::weak_ptr<std::recursive_mutex> csdkLock, PlatformConfig cfg)
     : m_csdkLock(csdkLock)
    {
        OCMode initType;

        if(cfg.mode == ModeType::Server)
        {
            initType = OC_SERVER;
        }
        else if (cfg.mode == ModeType::Both)
        {
            initType = OC_CLIENT_SERVER;
        }
        else if (cfg.mode == ModeType::Gateway)
        {
            initType = OC_GATEWAY;
        }
        else
        {
            throw InitializeException(OC::InitException::NOT_CONFIGURED_AS_SERVER,
                                         OC_STACK_INVALID_PARAM);
        }

        OCTransportFlags serverFlags =
                            static_cast<OCTransportFlags>(cfg.serverConnectivity & CT_MASK_FLAGS);
        OCTransportFlags clientFlags =
                            static_cast<OCTransportFlags>(cfg.clientConnectivity & CT_MASK_FLAGS);
        OCStackResult result = OCInit1(initType, serverFlags, clientFlags);

        if(OC_STACK_OK != result)
        {
            throw InitializeException(OC::InitException::STACK_INIT_ERROR, result);
        }

        m_threadRun = true;
        m_processThread = std::thread(&InProcServerWrapper::processFunc, this);
    }

    void InProcServerWrapper::processFunc()
    {
        auto cLock = m_csdkLock.lock();
        while(cLock && m_threadRun)
        {
            OCStackResult result;

            {
                std::lock_guard<std::recursive_mutex> lock(*cLock);
                result = OCProcess();
            }

            if(OC_STACK_ERROR == result)
            {
                oclog() << "OCProcess failed with result " << result <<std::flush;
                // ...the value of variable result is simply ignored for now.
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    OCStackResult InProcServerWrapper::registerDeviceInfo(const OCDeviceInfo deviceInfo)
    {
        auto cLock = m_csdkLock.lock();
        OCStackResult result = OC_STACK_ERROR;
        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCSetDeviceInfo(deviceInfo);
        }
        return result;
    }

     OCStackResult InProcServerWrapper::registerPlatformInfo(const OCPlatformInfo platformInfo)
    {
        auto cLock = m_csdkLock.lock();
        OCStackResult result = OC_STACK_ERROR;
        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCSetPlatformInfo(platformInfo);
        }
        return result;
    }

    OCStackResult InProcServerWrapper::registerResource(
                    OCResourceHandle& resourceHandle,
                    std::string& resourceURI,
                    const std::string& resourceTypeName,
                    const std::string& resourceInterface,
                    EntityHandler& eHandler,
                    uint8_t resourceProperties)

    {
        OCStackResult result = OC_STACK_ERROR;

        auto cLock = m_csdkLock.lock();

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);

            if(NULL != eHandler)
            {
                result = OCCreateResource(&resourceHandle, // OCResourceHandle *handle
                            resourceTypeName.c_str(), // const char * resourceTypeName
                            resourceInterface.c_str(), //const char * resourceInterfaceName //TODO fix this
                            resourceURI.c_str(), // const char * uri
                            EntityHandlerWrapper, // OCEntityHandler entityHandler
                            NULL,
                            resourceProperties // uint8_t resourceProperties
                            );
            }
            else
            {
                result = OCCreateResource(&resourceHandle, // OCResourceHandle *handle
                            resourceTypeName.c_str(), // const char * resourceTypeName
                            resourceInterface.c_str(), //const char * resourceInterfaceName //TODO fix this
                            resourceURI.c_str(), // const char * uri
                            NULL, // OCEntityHandler entityHandler
                            NULL,
                            resourceProperties // uint8_t resourceProperties
                            );
            }

            if(result != OC_STACK_OK)
            {
                resourceHandle = (OCResourceHandle) 0;
            }
            else
            {
                std::lock_guard<std::mutex> lock(OC::details::serverWrapperLock);
                OC::details::entityHandlerMap[resourceHandle] = eHandler;
                OC::details::resourceUriMap[resourceHandle] = resourceURI;
            }
        }
        else
        {
            result = OC_STACK_ERROR;
        }

        return result;
    }



    OCStackResult InProcServerWrapper::setDefaultDeviceEntityHandler
                                        (EntityHandler entityHandler)
    {
        OCStackResult result = OC_STACK_ERROR;

        {
            std::lock_guard<std::mutex> lock(OC::details::serverWrapperLock);
            OC::details::defaultDeviceEntityHandler = entityHandler;
        }

        if(entityHandler)
        {
            result = OCSetDefaultDeviceEntityHandler(DefaultEntityHandlerWrapper, NULL);
        }
        else
        {
            // If Null passed we unset
            result = OCSetDefaultDeviceEntityHandler(NULL, NULL);
        }

        return result;
    }

    OCStackResult InProcServerWrapper::unregisterResource(const OCResourceHandle& resourceHandle)
    {
        auto cLock = m_csdkLock.lock();
        OCStackResult result = OC_STACK_ERROR;

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCDeleteResource(resourceHandle);

            if(result == OC_STACK_OK)
            {
                std::lock_guard<std::mutex> lock(OC::details::serverWrapperLock);
                OC::details::resourceUriMap.erase(resourceHandle);
            }
            else
            {
                throw OCException(OC::Exception::RESOURCE_UNREG_FAILED, result);
            }
        }
        else
        {
            result = OC_STACK_ERROR;
        }

        return result;
    }

    OCStackResult InProcServerWrapper::bindTypeToResource(const OCResourceHandle& resourceHandle,
                     const std::string& resourceTypeName)
    {
        auto cLock = m_csdkLock.lock();
        OCStackResult result;
        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCBindResourceTypeToResource(resourceHandle, resourceTypeName.c_str());
        }
        else
        {
            result = OC_STACK_ERROR;
        }

        if (result != OC_STACK_OK)
        {
            throw OCException(OC::Exception::BIND_TYPE_FAILED, result);
        }
        return result;
    }

    OCStackResult InProcServerWrapper::bindInterfaceToResource(
                     const OCResourceHandle& resourceHandle,
                     const std::string& resourceInterfaceName)
    {
        auto cLock = m_csdkLock.lock();
        OCStackResult result;
        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCBindResourceInterfaceToResource(resourceHandle,
                        resourceInterfaceName.c_str());
        }
        else
        {
            result = OC_STACK_ERROR;
        }

        if (result != OC_STACK_OK)
        {
            throw OCException(OC::Exception::BIND_INTERFACE_FAILED, result);
        }
        return result;
    }

    OCStackResult InProcServerWrapper::startPresence(const unsigned int seconds)
    {
        auto cLock = m_csdkLock.lock();
        OCStackResult result = OC_STACK_ERROR;
        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCStartPresence(seconds);
        }

        if(result != OC_STACK_OK)
        {
            throw OCException(OC::Exception::START_PRESENCE_FAILED, result);
        }
        return result;
    }

    OCStackResult InProcServerWrapper::stopPresence()
    {
        auto cLock = m_csdkLock.lock();
        OCStackResult result = OC_STACK_ERROR;
        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCStopPresence();
        }

        if(result != OC_STACK_OK)
        {
            throw OCException(OC::Exception::END_PRESENCE_FAILED, result);
        }
        return result;
    }

    OCStackResult InProcServerWrapper::sendResponse(
        const std::shared_ptr<OCResourceResponse> pResponse)
    {
        auto cLock = m_csdkLock.lock();
        OCStackResult result = OC_STACK_ERROR;

        if(!pResponse)
        {
            result = OC_STACK_MALFORMED_RESPONSE;
            throw OCException(OC::Exception::STR_NULL_RESPONSE, OC_STACK_MALFORMED_RESPONSE);
        }
        else
        {
            OCEntityHandlerResponse response;
//            OCRepPayload* payLoad = pResponse->getPayload();
            HeaderOptions serverHeaderOptions = pResponse->getHeaderOptions();

            response.requestHandle = pResponse->getRequestHandle();
            response.resourceHandle = pResponse->getResourceHandle();
            response.ehResult = pResponse->getResponseResult();

            response.payload = reinterpret_cast<OCPayload*>(pResponse->getPayload());

            response.persistentBufferFlag = 0;

            response.numSendVendorSpecificHeaderOptions = serverHeaderOptions.size();
            int i = 0;
            for (auto it=serverHeaderOptions.begin(); it != serverHeaderOptions.end(); ++it)
            {
                response.sendVendorSpecificHeaderOptions[i].protocolID = OC_COAP_ID;
                response.sendVendorSpecificHeaderOptions[i].optionID =
                    static_cast<uint16_t>(it->getOptionID());
                response.sendVendorSpecificHeaderOptions[i].optionLength =
                    (it->getOptionData()).length() + 1;
                std::string optionData = it->getOptionData();
                std::copy(optionData.begin(),
                         optionData.end(),
                         response.sendVendorSpecificHeaderOptions[i].optionData);
                response.sendVendorSpecificHeaderOptions[i].optionData[it->getOptionData().length()]
                    = '\0';
                i++;
            }

            if(OC_EH_RESOURCE_CREATED == response.ehResult)
            {
                pResponse->getNewResourceUri().copy(response.resourceUri,
                        sizeof (response.resourceUri) - 1);
                response.resourceUri[pResponse->getNewResourceUri().length()] = '\0';
            }

            if(cLock)
            {
                std::lock_guard<std::recursive_mutex> lock(*cLock);
                result = OCDoResponse(&response);
            }
            else
            {
                OICFree(response.payload);
                result = OC_STACK_ERROR;
            }

            if(result != OC_STACK_OK)
            {
                oclog() << "Error sending response\n";
            }
            return result;
        }
    }

    InProcServerWrapper::~InProcServerWrapper()
    {
        if(m_processThread.joinable())
        {
            m_threadRun = false;
            m_processThread.join();
        }

        OCStop();
    }
}


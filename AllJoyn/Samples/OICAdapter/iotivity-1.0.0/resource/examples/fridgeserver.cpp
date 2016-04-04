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

/// The purpose of this server is to simulate a refrigerator that contains a device resource for
/// its description, a light resource for the internal light, and 2 door resources for the purpose
/// of representing the doors attached to this fridge.  This is used by the fridgeclient to
/// demonstrate using std::bind to attach to instances of a class as well as using
/// constructResourceObject

#include <chrono>
#include <iostream>
#include <thread>
#include <stdexcept>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
namespace PH = std::placeholders;

// Option ID for API version and client token
const uint16_t API_VERSION = 2048;
const uint16_t TOKEN = 3000;

// Setting API version and token (shared out of band with client)
// This assumes the fact that this server responds
// only with a server with below version and token
const std::string FRIDGE_CLIENT_API_VERSION = "v.1.0";
const std::string FRIDGE_CLIENT_TOKEN = "21ae43gf";

class Resource
{
    protected:
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_rep;
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)=0;
};

class DeviceResource : public Resource
{
    public:

    DeviceResource():m_modelName{}
    {
        std::string resourceURI = "/device";
        std::string resourceTypeName = "intel.fridge";
        std::string resourceInterface = DEFAULT_INTERFACE;
        EntityHandler cb = std::bind(&DeviceResource::entityHandler, this,PH::_1);

        EntityHandler defaultEH = std::bind(&DeviceResource::defaultEntityHandler
                                            ,this, PH::_1);

        std::cout << "Setting device default entity handler\n";
        OCPlatform::setDefaultDeviceEntityHandler(defaultEH);

        uint8_t resourceProperty = OC_DISCOVERABLE;
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
            resourceURI,
            resourceTypeName,
            resourceInterface,
            cb,
            resourceProperty);

        if(OC_STACK_OK != result)
        {
            throw std::runtime_error(
                std::string("Device Resource failed to start")+std::to_string(result));
        }
    }
    private:
    OCRepresentation get()
    {
        m_rep.setValue("device_name", std::string("Intel Powered 2 door, 1 light refrigerator"));
        return m_rep;
    }

    OCStackResult deleteDeviceResource()
    {
        OCStackResult result = OCPlatform::unregisterResource(m_resourceHandle);
        if(OC_STACK_OK != result)
        {
            throw std::runtime_error(
               std::string("Device Resource failed to unregister/delete") + std::to_string(result));
        }
        return result;
    }

    std::string m_modelName;
    protected:
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        OCEntityHandlerResult ehResult = OC_EH_ERROR;
        if(request)
        {
            // Get the header options from the request
            HeaderOptions headerOptions = request->getHeaderOptions();
            std::string clientAPIVersion;
            std::string clientToken;

            // Search the header options map and look for API version and Client token
            for (auto it = headerOptions.begin(); it != headerOptions.end(); ++it)
            {
                uint16_t optionID = it->getOptionID();
                if(optionID == API_VERSION)
                {
                    clientAPIVersion = it->getOptionData();
                    std::cout << " Client API version: " << clientAPIVersion << std::endl;
                }
                else if(optionID == TOKEN)
                {
                    clientToken = it->getOptionData();
                    std::cout << " Client token: " << clientToken << std::endl;
                }
                else
                {
                    std::cout << " Invalid header option " << std::endl;
                }
            }

            // In this case Server entity handler verifies API version
            // and client token. If they are valid, client requests are handled.
            if(clientAPIVersion == FRIDGE_CLIENT_API_VERSION && clientToken == FRIDGE_CLIENT_TOKEN)
            {
                HeaderOptions serverHeaderOptions;
                try
                {
                    // Set API version from server side
                    HeaderOption::OCHeaderOption apiVersion(API_VERSION, FRIDGE_CLIENT_API_VERSION);
                    serverHeaderOptions.push_back(apiVersion);
                }
                catch(OCException& e)
                {
                    std::cout << "Error creating HeaderOption in server: " << e.what() << std::endl;
                }

                if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
                {
                    auto pResponse = std::make_shared<OC::OCResourceResponse>();
                    pResponse->setRequestHandle(request->getRequestHandle());
                    pResponse->setResourceHandle(request->getResourceHandle());
                    pResponse->setHeaderOptions(serverHeaderOptions);

                    if(request->getRequestType() == "GET")
                    {
                        std::cout<<"DeviceResource Get Request"<<std::endl;
                        pResponse->setErrorCode(200);
                        pResponse->setResponseResult(OC_EH_OK);
                        pResponse->setResourceRepresentation(get(), "");
                        if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                        {
                            ehResult = OC_EH_OK;
                        }
                    }
                    else if(request->getRequestType() == "DELETE")
                    {
                        std::cout<<"DeviceResource Delete Request"<<std::endl;
                        if(deleteDeviceResource() == OC_STACK_OK)
                        {
                            pResponse->setErrorCode(200);
                            pResponse->setResponseResult(OC_EH_RESOURCE_DELETED);
                            ehResult = OC_EH_OK;
                        }
                        else
                        {
                            pResponse->setResponseResult(OC_EH_ERROR);
                            ehResult = OC_EH_ERROR;
                        }
                        OCPlatform::sendResponse(pResponse);
                    }
                    else
                    {
                        std::cout <<"DeviceResource unsupported request type "
                        << request->getRequestType() << std::endl;
                        pResponse->setResponseResult(OC_EH_ERROR);
                        OCPlatform::sendResponse(pResponse);
                        ehResult = OC_EH_ERROR;
                    }
                }
                else
                {
                    std::cout << "DeviceResource unsupported request flag" <<std::endl;
                }
            }
            else
            {
                std::cout << "Unsupported/invalid header options/values" << std::endl;
            }
        }

        return ehResult;
    }

    virtual OCEntityHandlerResult defaultEntityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        OCEntityHandlerResult ehResult = OC_EH_ERROR;
        if(request)
        {
            std::cout << "In Default Entity Handler, uri received: "
                        << request->getResourceUri() << std::endl;

            if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
            {
                auto pResponse = std::make_shared<OC::OCResourceResponse>();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                if(request->getRequestType() == "GET")
                {
                    std::cout<<"Default Entity Handler: Get Request"<<std::endl;
                    pResponse->setErrorCode(200);
                    pResponse->setResourceRepresentation(get(), "");
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else
                {
                    std::cout <<"Default Entity Handler: unsupported request type "
                    << request->getRequestType() << std::endl;
                    pResponse->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(pResponse);
                    ehResult = OC_EH_ERROR;
                }
            }
            else
            {
                std::cout << "Default Entity Handler: unsupported request flag" <<std::endl;
            }
        }

        return ehResult;
   }

};

class LightResource : public Resource
{
    public:
    LightResource() : m_isOn(false)
    {
        std::string resourceURI = "/light";
        std::string resourceTypeName = "intel.fridge.light";
        std::string resourceInterface = DEFAULT_INTERFACE;
        EntityHandler cb = std::bind(&LightResource::entityHandler, this,PH::_1);
        uint8_t resourceProperty = 0;
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
            resourceURI,
            resourceTypeName,
            resourceInterface,
            cb,
            resourceProperty);

        if(OC_STACK_OK != result)
        {
            throw std::runtime_error(
                std::string("Light Resource failed to start")+std::to_string(result));
        }
    }
    private:
    OCRepresentation get()
    {
        m_rep.setValue("on", m_isOn);
        return m_rep;
    }

    void put(const OCRepresentation& rep)
    {
        rep.getValue("on", m_isOn);
    }

    bool m_isOn;
    protected:
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        OCEntityHandlerResult ehResult = OC_EH_ERROR;
        if(request)
        {
            std::cout << "In entity handler for Light, URI is : "
                      << request->getResourceUri() << std::endl;

            if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
            {
                auto pResponse = std::make_shared<OC::OCResourceResponse>();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                if(request->getRequestType() == "GET")
                {
                    std::cout<<"Light Get Request"<<std::endl;
                    pResponse->setErrorCode(200);
                    pResponse->setResourceRepresentation(get(), "");
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if(request->getRequestType() == "PUT")
                {
                    std::cout <<"Light Put Request"<<std::endl;
                    put(request->getResourceRepresentation());
                    pResponse->setErrorCode(200);
                    pResponse->setResourceRepresentation(get(), "");
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else
                {
                    std::cout << "Light unsupported request type"
                    << request->getRequestType() << std::endl;
                    pResponse->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(pResponse);
                    ehResult = OC_EH_ERROR;
                }
            }
            else
            {
                std::cout << "Light unsupported request flag" <<std::endl;
            }
        }

        return ehResult;
    }
};

class DoorResource : public Resource
{
    public:
    DoorResource(const std::string& side):m_isOpen{false}, m_side(side)
    {

        std::string resourceURI = "/door/"+ side;
        std::string resourceTypeName = "intel.fridge.door";
        std::string resourceInterface = DEFAULT_INTERFACE;
        EntityHandler cb = std::bind(&DoorResource::entityHandler, this,PH::_1);

        uint8_t resourceProperty = 0;
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
            resourceURI,
            resourceTypeName,
            resourceInterface,
            cb,
            resourceProperty);

        if(OC_STACK_OK != result)
        {
            throw std::runtime_error(
                std::string("Door Resource failed to start")+std::to_string(result));
        }
    }

    private:
    OCRepresentation get()
    {
        m_rep.setValue("open", m_isOpen);
        m_rep.setValue("side", m_side);
        return m_rep;
    }

    void put(const OCRepresentation& rep)
    {
        rep.getValue("open", m_isOpen);
        // Note, we won't let the user change the door side!
    }
    bool m_isOpen;
    std::string m_side;
    protected:
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        std::cout << "EH of door invoked " << std::endl;
        OCEntityHandlerResult ehResult = OC_EH_ERROR;

        if(request)
        {
            std::cout << "In entity handler for Door, URI is : "
                      << request->getResourceUri() << std::endl;

            if(request->getRequestHandlerFlag() == RequestHandlerFlag::RequestFlag)
            {
                auto pResponse = std::make_shared<OC::OCResourceResponse>();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                if(request->getRequestType() == "GET")
                {
                    // note that we know the side because std::bind gives us the
                    // appropriate object
                    std::cout<< m_side << " Door Get Request"<<std::endl;
                    pResponse->setErrorCode(200);
                    pResponse->setResourceRepresentation(get(), "");
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if(request->getRequestType() == "PUT")
                {
                    std::cout << m_side <<" Door Put Request"<<std::endl;
                    put(request->getResourceRepresentation());
                    pResponse->setErrorCode(200);
                    pResponse->setResourceRepresentation(get(),"");
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else
                {
                    std::cout <<m_side<<" Door unsupported request type "
                    << request->getRequestType() << std::endl;
                    pResponse->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(pResponse);
                    ehResult = OC_EH_ERROR;
                }
            }
            else
            {
                std::cout << "Door unsupported request flag" <<std::endl;
            }
        }

        return ehResult;
    }

};

class Refrigerator
{
    public:
    Refrigerator()
        :
        m_light(),
        m_device(),
        m_leftdoor("left"),
        m_rightdoor("right")
    {
    }
    private:
    LightResource m_light;
    DeviceResource m_device;
    DoorResource m_leftdoor;
    DoorResource m_rightdoor;
};

int main ()
{
    PlatformConfig cfg
    {
        ServiceType::InProc,
        ModeType::Server,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);
    Refrigerator rf;

    // we will keep the server alive for at most 30 minutes
    std::this_thread::sleep_for(std::chrono::minutes(30));
    return 0;
}


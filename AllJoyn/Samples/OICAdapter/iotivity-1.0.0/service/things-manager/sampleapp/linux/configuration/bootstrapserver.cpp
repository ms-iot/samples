//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

///
/// This sample shows how one could create a resource (collection) with children.
///

#include <functional>

#include <pthread.h>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;

// Forward declaring the entityHandler (bootstrap)
bool prepareResponse(std::shared_ptr< OCResourceRequest > request);
OCStackResult sendResponse(std::shared_ptr< OCResourceRequest > pRequest);
OCEntityHandlerResult entityHandlerBootstrap(std::shared_ptr< OCResourceRequest > request);

#define DefaultDeviceName "Legacy Device"
#define DefaultLocation "37.256616, 127.052806"
#define DefaultLocationName "Living Room"
#define DefaultCurrency "Won"
#define DefaultRegion "Seoul, Korea"

class BootstrapResource
{
public:
    // Room members
    std::string m_bootstrapUri;
    std::vector< std::string > m_bootstrapTypes;
    std::vector< std::string > m_bootstrapInterfaces;
    OCResourceHandle m_bootstrapHandle;
    OCRepresentation m_bootstrapRep;

public:
    /// Constructor
    BootstrapResource()
    {
        m_bootstrapUri = "/bootstrap"; // URI of the resource
        m_bootstrapTypes.push_back("bootstrap"); // resource type name. In this case, it is light
        m_bootstrapInterfaces.push_back(DEFAULT_INTERFACE); // resource interface.
        m_bootstrapRep.setUri(m_bootstrapUri);
        m_bootstrapRep.setResourceTypes(m_bootstrapTypes);
        m_bootstrapRep.setResourceInterfaces(m_bootstrapInterfaces);
        m_bootstrapHandle = NULL;
    }

    /// This function internally calls registerResource API.
    void createResources()
    {
        using namespace OC::OCPlatform;
        // This will internally create and register the resource.
        OCStackResult result = registerResource(m_bootstrapHandle, m_bootstrapUri,
                m_bootstrapTypes[0], m_bootstrapInterfaces[0], entityHandlerBootstrap,
                OC_DISCOVERABLE | OC_OBSERVABLE);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation (room) was unsuccessful\n";
        }
    }

    void setBootstrapRepresentation(OCRepresentation& /*rep*/)
    {
        // Not allowed
    }

    OCRepresentation getBootstrapRepresentation()
    {
        m_bootstrapRep.setValue< std::string >("n", DefaultDeviceName);
        m_bootstrapRep.setValue< std::string >("loc", DefaultLocation);
        m_bootstrapRep.setValue< std::string >("locn", DefaultLocationName);
        m_bootstrapRep.setValue< std::string >("c", DefaultCurrency);
        m_bootstrapRep.setValue< std::string >("r", DefaultRegion);

        return m_bootstrapRep;
    }
};

// Create the instance of the resource class (in this case instance of class 'RoomResource').
BootstrapResource myBootstrapResource;

// This function prepares a response for any incoming request to Light resource.
bool prepareResponse(std::shared_ptr< OCResourceRequest > request)
{
    cout << "\tIn Server CPP prepareResponse:\n";
    bool result = false;
    if (request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if (requestFlag == RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";

            // If the request type is GET
            if (requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";
                // GET operations are directly handled while sending the response
                // in the sendLightResponse function
                result = true;
            }
            else if (requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";

                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to PUT request
                myBootstrapResource.setBootstrapRepresentation(rep);
                result = true;
            }
            else if (requestType == "POST")
            {
                // POST request operations
            }
            else if (requestType == "DELETE")
            {
                // DELETE request operations
            }
        }
        else if (requestFlag == RequestHandlerFlag::ObserverFlag)
        {
            cout << "\t\trequestFlag : Observer\n";
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return result;
}

OCStackResult sendResponse(std::shared_ptr< OCResourceRequest > pRequest)
{
    auto pResponse = std::make_shared< OC::OCResourceResponse >();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(myBootstrapResource.getBootstrapRepresentation());
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}

OCEntityHandlerResult entityHandlerBootstrap(std::shared_ptr< OCResourceRequest > request)
{
    cout << "\tIn Server CPP (entityHandlerBootstrap) entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    if (prepareResponse(request))
    {
        if (OC_STACK_OK == sendResponse(request))
        {
            ehResult = OC_EH_OK;
        }
        else
        {
            std::cout << "sendResponse failed." << std::endl;
        }
    }
    else
    {
        std::cout << "PrepareResponse failed." << std::endl;
    }
    return ehResult;
}

int main()
{
    // Create PlatformConfig object
    PlatformConfig cfg
    { OC::ServiceType::InProc, OC::ModeType::Server, "0.0.0.0",
    // By setting to "0.0.0.0", it binds to all available interfaces
            0,// Uses randomly available port
            OC::QualityOfService::LowQos };

    OCPlatform::Configure(cfg);
    try
    {

        myBootstrapResource.createResources();

        // Perform app tasks
        while (true)
        {
            // some tasks
        }
    }
    catch (OCException e)
    {
        std::cout << "Exception in main: " << e.what();
    }

    // No explicit call to stop the platform.
    // When OCPlatform destructor is invoked, internally we do platform cleanup
}


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

///
/// This sample shows how one could create a resource (collection) with children.
///

#include <functional>

#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;


// Forward declaring the entityHandler (room)
OCEntityHandlerResult entityHandlerRoom(std::shared_ptr<OCResourceRequest> request);
OCEntityHandlerResult entityHandlerLight(std::shared_ptr<OCResourceRequest> request);
OCEntityHandlerResult entityHandlerFan(std::shared_ptr<OCResourceRequest> request);

/// Specifies whether default collection entity handler is used or not
bool useDefaultCollectionEH = false;

class RoomResource
{
public:

    // Room members
    std::string m_roomUri;
    std::string m_roomName;
    std::vector<std::string> m_roomTypes;
    std::vector<std::string> m_roomInterfaces;
    OCResourceHandle m_roomHandle;
    OCRepresentation m_roomRep;

    // light members
    bool m_lightState;
    int m_lightColor;
    std::string m_lightUri;
    std::vector<std::string> m_lightTypes;
    std::vector<std::string> m_lightInterfaces;
    OCResourceHandle m_lightHandle;
    OCRepresentation m_lightRep;

    // fan members
    bool m_fanState;
    int m_fanSpeed;
    std::string m_fanUri;
    std::vector<std::string> m_fanTypes;
    std::vector<std::string> m_fanInterfaces;
    OCResourceHandle m_fanHandle;
    OCRepresentation m_fanRep;

public:
    /// Constructor
    RoomResource(): m_roomName("John's Room"), m_roomHandle(nullptr), m_lightState(false),
                    m_lightColor(0),m_lightHandle(nullptr),  m_fanState(false), m_fanSpeed(0),
                    m_fanHandle(nullptr)
    {
        m_lightUri = "/a/light"; // URI of the resource
        m_lightTypes.push_back("core.light"); // resource type name. In this case, it is light
        m_lightInterfaces.push_back(DEFAULT_INTERFACE); // resource interface.

        m_lightRep.setUri(m_lightUri);
        m_lightRep.setResourceTypes(m_lightTypes);
        m_lightRep.setResourceInterfaces(m_lightInterfaces);
        m_lightRep.setValue("state", m_lightState);
        m_lightRep.setValue("color", m_lightColor);

        m_fanUri = "/a/fan"; // URI of the resource
        m_fanTypes.push_back("core.fan"); // resource type name. In this case, it is light
        m_fanInterfaces.push_back(DEFAULT_INTERFACE); // resource interface.

        m_fanRep.setUri(m_fanUri);
        m_fanRep.setResourceTypes(m_fanTypes);
        m_fanRep.setResourceInterfaces(m_fanInterfaces);
        m_fanRep.setValue("state", m_fanState);
        m_fanRep.setValue("speed", m_fanSpeed);

        m_roomUri = "/a/room"; // URI of the resource
        m_roomTypes.push_back("core.room"); // resource type name. In this case, it is light
        m_roomInterfaces.push_back(DEFAULT_INTERFACE); // resource interface.
        m_roomInterfaces.push_back(BATCH_INTERFACE); // resource interface.
        m_roomInterfaces.push_back(LINK_INTERFACE); // resource interface.
        m_roomRep.setValue("name", m_roomName);
        m_roomRep.setUri(m_roomUri);
        m_roomRep.setResourceTypes(m_roomTypes);
        m_roomRep.setResourceInterfaces(m_roomInterfaces);
    }

    /// This function internally calls registerResource API.
    void createResources()
    {
        // This function internally creates and registers the resource.
        using namespace OC::OCPlatform;
        OCStackResult result = OC_STACK_ERROR;

        // Based on the case, we will use default collection EH (by passing NULL in entity handler
        // parameter) or use application entity handler.
        if(useDefaultCollectionEH)
        {
            result = registerResource(
                                    m_roomHandle, m_roomUri, m_roomTypes[0],
                                    m_roomInterfaces[0], NULL,
                                    OC_DISCOVERABLE | OC_OBSERVABLE);
        }
        else
        {
            result = registerResource(
                                    m_roomHandle, m_roomUri, m_roomTypes[0],
                                    m_roomInterfaces[0], entityHandlerRoom,
                                    OC_DISCOVERABLE | OC_OBSERVABLE);
        }

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation (room) was unsuccessful\n";
        }

        result = bindInterfaceToResource(m_roomHandle, m_roomInterfaces[1]);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }

        result = bindInterfaceToResource(m_roomHandle, m_roomInterfaces[2]);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }

        result = registerResource(m_lightHandle, m_lightUri, m_lightTypes[0],
                                  m_lightInterfaces[0], entityHandlerLight,
                                  OC_DISCOVERABLE | OC_OBSERVABLE);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation (light) was unsuccessful\n";
        }

        result = registerResource(m_fanHandle, m_fanUri, m_fanTypes[0],
                                  m_fanInterfaces[0], entityHandlerFan,
                                  OC_DISCOVERABLE | OC_OBSERVABLE);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation (fan) was unsuccessful\n";
        }

        result = bindResource(m_roomHandle, m_lightHandle);
        if (OC_STACK_OK != result)
        {
            cout << "Binding fan resource to room was unsuccessful\n";
        }

        result = bindResource(m_roomHandle, m_fanHandle);
        if (OC_STACK_OK != result)
        {
            cout << "Binding light resource to room was unsuccessful\n";
        }

    }

    void setLightRepresentation(OCRepresentation& rep)
    {
        bool tempState = false;
        int tempColor = 0;

        // If both entries exist
        if(rep.getValue("state", tempState) && rep.getValue("color", tempColor))
        {
            m_lightState = tempState;
            m_lightColor= tempColor;

            cout << "\t\t\t\t" << "state: " << m_lightState << endl;
            cout << "\t\t\t\t" << "color: " << m_lightColor << endl;
        }
    }

    void setFanRepresentation(OCRepresentation& rep)
    {
        bool tempState = false;
        int tempSpeed = 0;

        // If both entries exist
        if(rep.getValue("state", tempState) && rep.getValue("speed", tempSpeed))
        {
            m_fanState = tempState;
            m_fanSpeed = tempSpeed;

            cout << "\t\t\t\t" << "state: " << m_fanState << endl;
            cout << "\t\t\t\t" << "speed: " << m_fanSpeed << endl;
        }
    }


    OCRepresentation getLightRepresentation()
    {
        m_lightRep.setValue("state", m_lightState);
        m_lightRep.setValue("color", m_lightColor);

        return m_lightRep;
    }

    OCRepresentation getFanRepresentation()
    {
        m_fanRep.setValue("state", m_fanState);
        m_fanRep.setValue("speed", m_fanSpeed);
        return m_fanRep;
    }

    OCRepresentation getRoomRepresentation(void)
    {
        m_roomRep.clearChildren();

        m_roomRep.addChild(getLightRepresentation());
        m_roomRep.addChild(getFanRepresentation());
        return m_roomRep;
    }

};

// Create the instance of the resource class (in this case instance of class 'RoomResource').
RoomResource myRoomResource;

OCStackResult sendRoomResponse(std::shared_ptr<OCResourceRequest> pRequest)
{
    auto pResponse = std::make_shared<OC::OCResourceResponse>();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());

    // Check for query params (if any)
    QueryParamsMap queryParamsMap = pRequest->getQueryParameters();

    cout << "\t\t\tquery params: \n";
    for(auto it = queryParamsMap.begin(); it != queryParamsMap.end(); it++)
    {
        cout << "\t\t\t\t" << it->first << ":" << it->second << endl;
    }

    OCRepresentation rep;
    rep = myRoomResource.getRoomRepresentation();

    auto findRes = queryParamsMap.find("if");

    if(findRes != queryParamsMap.end())
    {
        pResponse->setResourceRepresentation(rep, findRes->second);
    }
    else
    {
        pResponse->setResourceRepresentation(rep, DEFAULT_INTERFACE);
    }

    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}

// This function prepares a response for any incoming request to Light resource.
bool prepareLightResponse(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server CPP (Light) prepareLightResponse:\n";
    bool result = false;
    if(request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag == RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";

            // If the request type is GET
            if(requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";
                // GET operations are directly handled while sending the response
                // in the sendLightResponse function
                result = true;
            }
            else if(requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";
                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to PUT request
                myRoomResource.setLightRepresentation(rep);
                result= true;
            }
            else if(requestType == "POST")
            {
                // POST request operations
            }
            else if(requestType == "DELETE")
            {
                // DELETE request operations
            }
        }
        else if(requestFlag == RequestHandlerFlag::ObserverFlag)
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

// This function prepares a response for any incoming request to Fan resource.
bool prepareFanResponse(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server CPP (Fan) prepareFanResponse:\n";
    bool result = false;

    if(request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag == RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";

            // If the request type is GET
            if(requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";
                // GET operations are directly handled while sending the response
                // in the sendLightResponse function
                result = true;
            }
            else if(requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";

                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to PUT request
                myRoomResource.setFanRepresentation(rep);
                result = true;
            }
            else if(requestType == "POST")
            {
                // POST request operations
            }
            else if(requestType == "DELETE")
            {
                // DELETE request operations
            }
        }
        else if(requestFlag == RequestHandlerFlag::ObserverFlag)
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

OCEntityHandlerResult entityHandlerRoom(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server CPP entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    if(request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag == RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";

            // If the request type is GET
            if(requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";
                if(OC_STACK_OK == sendRoomResponse(request))
                {
                    ehResult = OC_EH_OK;
                }
            }
            else if(requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";
                // Call these functions to prepare the response for child resources and
                // then send the final response using sendRoomResponse function
                prepareLightResponse(request);
                prepareFanResponse(request);
                if(OC_STACK_OK == sendRoomResponse(request))
                {
                    ehResult = OC_EH_OK;
                }
            }
            else if(requestType == "POST")
            {
                // POST request operations
            }
            else if(requestType == "DELETE")
            {
                // DELETE request operations
            }
        }
        else if(requestFlag == RequestHandlerFlag::ObserverFlag)
        {
            cout << "\t\trequestFlag : Observer\n";
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return ehResult;
}

OCStackResult sendLightResponse(std::shared_ptr<OCResourceRequest> pRequest)
{
    auto pResponse = std::make_shared<OC::OCResourceResponse>();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(myRoomResource.getLightRepresentation());
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}



OCEntityHandlerResult entityHandlerLight(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server CPP (Light) entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    if(prepareLightResponse(request))
    {
        if(OC_STACK_OK == sendLightResponse(request))
        {
            ehResult = OC_EH_OK;
        }
        else
        {
            std::cout << "sendLightResponse failed." << std::endl;
        }
    }
    else
    {
        std::cout << "PrepareLightResponse failed." << std::endl;
    }
    return ehResult;
}

OCStackResult sendFanResponse(std::shared_ptr<OCResourceRequest> pRequest)
{
    auto pResponse = std::make_shared<OC::OCResourceResponse>();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(myRoomResource.getFanRepresentation());
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}


OCEntityHandlerResult entityHandlerFan(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server CPP (Fan) entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    if(prepareFanResponse(request))
    {
        if(OC_STACK_OK == sendFanResponse(request))
        {
            ehResult = OC_EH_OK;
        }
        else
        {
            std::cout << "sendFanResponse failed." << std::endl;
        }
    }
    else
    {
        std::cout << "PrepareFanResponse failed." << std::endl;
    }

    return ehResult;
}

void printUsage()
{
    std::cout << std::endl;
    std::cout << "Usage : roomserver <value>\n";
    std::cout << "1 : Create room resource with default collection entity handler.\n";
    std::cout << "2 : Create room resource with application collection entity handler.\n";
}

int main(int argc, char* argv[])
{
    printUsage();

    if(argc == 2)
    {
        int value = atoi(argv[1]);
        switch (value)
        {
            case 1:
                useDefaultCollectionEH = true;
                break;
            case 2:
            default:
                break;
       }
    }
    else
    {
        return -1;
    }

    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);
    try
    {

        myRoomResource.createResources();

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);

    }
    catch(OCException &e)
    {
        std::cout << "Exception in main: " << e.what();
    }

    // No explicit call to stop the platform.
    // When OCPlatform destructor is invoked, internally we do platform cleanup

    return 0;
}


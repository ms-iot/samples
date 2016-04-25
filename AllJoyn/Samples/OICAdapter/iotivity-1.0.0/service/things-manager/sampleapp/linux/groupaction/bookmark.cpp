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

#include <functional>
#include <pthread.h>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;

namespace PH = std::placeholders;

unsigned int startedThread;
unsigned int gObservation;
pthread_t threadId;

void* ObserveHandler(void *param);

class BookmarkResource
{

private:
    OCEntityHandlerResult entityHandler(std::shared_ptr< OCResourceRequest > request)
    {
        OCEntityHandlerResult ehResult = OC_EH_ERROR;

        if (request)
        {
            // Get the request type and request flag
            std::string requestType = request->getRequestType();
            int requestFlag = request->getRequestHandlerFlag();

            if (requestFlag & RequestHandlerFlag::RequestFlag)
            {
                auto pResponse = std::make_shared< OC::OCResourceResponse >();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                // If the request type is GET
                if (requestType == "GET")
                {
                }
                else if (requestType == "PUT")
                {
                    cout << "\t\t\trequestType : PUT\n";
                }
                else if (requestType == "POST")
                {
                    // POST request operations
                }
                else if (requestType == "DELETE")
                {
                    // DELETE request operations
                }

                pResponse->setErrorCode(200);
                pResponse->setResponseResult(OC_EH_OK);
                pResponse->setResourceRepresentation(getRepresentation());
                if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            }

            if (requestFlag & RequestHandlerFlag::ObserverFlag)
            {
                cout << "\t\trequestFlag : Observer\n";

                if (!startedThread)
                {
                    pthread_create(&threadId, NULL, ObserveHandler, (void *) NULL);
                    startedThread = 1;
                    gObservation = 1;
                }

                ehResult = OC_EH_OK;
            }
        }
        else
        {
            std::cout << "Request invalid" << std::endl;
        }

        return ehResult;
    }

public:
    /// Constructor
    BookmarkResource()
    {
        m_pressure = 0;

        m_BookmarkUri = "/core/bookmark"; // URI of the resource
        m_BookmarkType = "core.bookmark"; // resource type name. In this case, it is light

        m_BookmarkInterface = DEFAULT_INTERFACE; // resource interface.
        m_BookmarkHandle = 0;
    }

    /// This function internally calls registerResource API.
    void createResources()
    {
        EntityHandler cb = std::bind(&BookmarkResource::entityHandler, this, PH::_1);

        // This will internally create and register the resource.
        OCStackResult result = OC::OCPlatform::registerResource(m_BookmarkHandle, m_BookmarkUri,
                m_BookmarkType, m_BookmarkInterface, cb, OC_DISCOVERABLE | OC_OBSERVABLE);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation (bookmark) was unsuccessful\n";
        }
        else
        {
            cout << "Resource URI : " << m_BookmarkUri << endl;
            cout << "\tResource Type Name : " << m_BookmarkType << endl;
            cout << "\tResource Interface : " << DEFAULT_INTERFACE << endl;
            cout << "\tResource creation is successful with resource handle : " << m_BookmarkHandle
                    << endl;
        }
    }

    void setRepresentation(OCRepresentation& /*rep*/)
    {
        // AttributeMap attributeMap = rep.getAttributeMap();
        // if(rep.getValue("level", m_pressure) == true)
        {
            std::cout << m_pressure << endl;
        }
    }

    OCRepresentation getRepresentation()
    {
        OCRepresentation rep;

        rep.setValue("level", (int) m_pressure);

        return rep;
    }

public:
    // Members of Bookmark
    std::string m_BookmarkUri;
    std::string m_BookmarkType;
    std::string m_BookmarkInterface;
    unsigned int m_pressure;
    OCResourceHandle m_BookmarkHandle;
};

// Create the instance of the resource class (in this case instance of class 'BookmarkResource').
BookmarkResource myBookmarkResource;

void* ObserveHandler(void* /*param*/)
{
    while (startedThread)
    {
        sleep(1);

        cout << "input a integer(0:opened, 5:close) : ";
        cin >> myBookmarkResource.m_pressure;

        if (myBookmarkResource.m_pressure == 0 || // When your book opened.
                myBookmarkResource.m_pressure == 5) // When your book closed.
        {
            cout << "notifyObservers call!" << endl;

            OCStackResult result = OCPlatform::notifyAllObservers(
                    myBookmarkResource.m_BookmarkHandle);

            if (OC_STACK_NO_OBSERVERS == result)
            {
                cout << "No More observers, stopping notifications" << endl;
                gObservation = 0;
                startedThread = 0;
            }
        }
    }

    return NULL;
}

int main()
{
    // Create PlatformConfig object

    OC::PlatformConfig cfg
    { OC::ServiceType::InProc, OC::ModeType::Server, "0.0.0.0",
    // By setting to "0.0.0.0", it binds to all available interfaces
            0,// Uses randomly available port
            OC::QualityOfService::LowQos };

    // Create a OCPlatform instance.
    // Note: Platform creation is synchronous call.
    try
    {

        // Invoke createResource function of class bookmark.
        myBookmarkResource.createResources();

        // Perform app tasks
        while (true)
        {
            // some tasks
        }
    }
    catch (OCException e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

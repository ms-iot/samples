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
/// This sample provides steps to define an interface for a resource
/// (properties and methods) and host this resource on the server.
///

#include <functional>

#include <pthread.h>
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;
namespace PH = std::placeholders;

int gObservation = 0;
void * ChangeLightRepresentation (void *param);

// Specifies where to notify all observers or list of observers
// 0 - notifies all observers
// 1 - notifies list of observers
int isListOfObservers = 0;

/// This class represents a single resource named 'lightResource'. This resource has
/// two simple properties named 'state' and 'power'

class LightResource
{

public:
    /// Access this property from a TB client
    std::string m_name;
    bool m_state;
    int m_power;
    std::string m_lightUri;
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_lightRep;
    ObservationIds m_interestedObservers;

public:
    /// Constructor
    LightResource(PlatformConfig& /*cfg*/)
        :m_name("John's light"), m_state(false), m_power(0), m_lightUri("/a/light") {
        // Initialize representation
        m_lightRep.setUri(m_lightUri);

        m_lightRep.setValue("state", m_state);
        m_lightRep.setValue("power", m_power);
        m_lightRep.setValue("name", m_name);
    }

    /* Note that this does not need to be a member function: for classes you do not have
    access to, you can accomplish this with a free function: */

    /// This function internally calls registerResource API.
    void createResource()
    {
        std::string resourceURI = m_lightUri; // URI of the resource
        // resource type name. In this case, it is light
        std::string resourceTypeName = "core.light";
        std::string resourceInterface = DEFAULT_INTERFACE; // resource interface.

        // OCResourceProperty is defined ocstack.h
        uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

        EntityHandler cb = std::bind(&LightResource::entityHandler, this,PH::_1);

        // This will internally create and register the resource.
        OCStackResult result = OCPlatform::registerResource(
                                    m_resourceHandle, resourceURI, resourceTypeName,
                                    resourceInterface, cb, resourceProperty);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation was unsuccessful\n";
        }
    }

    OCStackResult createResource1()
    {
        std::string resourceURI = "/a/light1"; // URI of the resource
        std::string resourceTypeName = "core.light"; // resource type name. In this case, it is light
        std::string resourceInterface = DEFAULT_INTERFACE; // resource interface.

        // OCResourceProperty is defined ocstack.h
        uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

        EntityHandler cb = std::bind(&LightResource::entityHandler, this,PH::_1);

        OCResourceHandle resHandle;

        // This will internally create and register the resource.
        OCStackResult result = OCPlatform::registerResource(
                                    resHandle, resourceURI, resourceTypeName,
                                    resourceInterface, cb, resourceProperty);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation was unsuccessful\n";
        }

        return result;
    }

    OCResourceHandle getHandle()
    {
        return m_resourceHandle;
    }

    // Puts representation.
    // Gets values from the representation and
    // updates the internal state
    void put(OCRepresentation& rep)
    {
        try {
            if (rep.getValue("state", m_state))
            {
                cout << "\t\t\t\t" << "state: " << m_state << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "state not found in the representation" << endl;
            }

            if (rep.getValue("power", m_power))
            {
                cout << "\t\t\t\t" << "power: " << m_power << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "power not found in the representation" << endl;
            }
        }
        catch (exception& e)
        {
            cout << e.what() << endl;
        }

    }

    // Post representation.
    // Post can create new resource or simply act like put.
    // Gets values from the representation and
    // updates the internal state
    OCRepresentation post(OCRepresentation& rep)
    {
        static int first = 1;

        std::cout << "In POST\n";

        // for the first time it tries to create a resource
        if(first)
        {
            std::cout << "In POST/First\n";

            first = 0;

            if(OC_STACK_OK == createResource1())
            {
                std::cout << "Created a new resource\n";
                OCRepresentation rep1;
                rep1.setValue("createduri", std::string("/a/light1"));

                return rep1;
            }
        }

        // from second time onwards it just puts
        put(rep);
        return get();
    }


    // gets the updated representation.
    // Updates the representation with latest internal state before
    // sending out.
    OCRepresentation get()
    {
        m_lightRep.setValue("state", m_state);
        m_lightRep.setValue("power", m_power);

        return m_lightRep;
    }

    void addType(const std::string& type) const
    {
        OCStackResult result = OCPlatform::bindTypeToResource(m_resourceHandle, type);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

    void addInterface(const std::string& interface) const
    {
        OCStackResult result = OCPlatform::bindInterfaceToResource(m_resourceHandle, interface);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

private:

OCStackResult sendResponse(std::shared_ptr<OCResourceRequest> pRequest)
{
    auto pResponse = std::make_shared<OC::OCResourceResponse>();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(get());
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}

OCStackResult sendPostResponse(std::shared_ptr<OCResourceRequest> pRequest)
{
    auto pResponse = std::make_shared<OC::OCResourceResponse>();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());

    OCRepresentation rep = pRequest->getResourceRepresentation();
    OCRepresentation rep_post = post(rep);

    pResponse->setResourceRepresentation(rep_post);
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}

// This is just a sample implementation of entity handler.
// Entity handler can be implemented in several ways by the manufacturer
OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server CPP entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    if(request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag & RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";

            // If the request type is GET
            if(requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";
                if(OC_STACK_OK == sendResponse(request))
                {
                    ehResult = OC_EH_OK;
                }
            }
            else if(requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";

                OCRepresentation rep = request->getResourceRepresentation();
                // Do related operations related to PUT request
                // Update the lightResource
                put(rep);
                if(OC_STACK_OK == sendResponse(request))
                {
                    ehResult = OC_EH_OK;
                }
            }
            else if(requestType == "POST")
            {
                cout << "\t\t\trequestType : POST\n";
                if(OC_STACK_OK == sendPostResponse(request))
                {
                    ehResult = OC_EH_OK;
                }
            }
            else if(requestType == "DELETE")
            {
                // DELETE request operations
            }
        }

        if(requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            ObservationInfo observationInfo = request->getObservationInfo();
            if(ObserveAction::ObserveRegister == observationInfo.action)
            {
                m_interestedObservers.push_back(observationInfo.obsId);
            }
            else if(ObserveAction::ObserveUnregister == observationInfo.action)
            {
                m_interestedObservers.erase(std::remove(
                                                            m_interestedObservers.begin(),
                                                            m_interestedObservers.end(),
                                                            observationInfo.obsId),
                                                            m_interestedObservers.end());
            }

            pthread_t threadId;

            cout << "\t\trequestFlag : Observer\n";
            gObservation = 1;
            static int startedThread = 0;

            // Observation happens on a different thread in ChangeLightRepresentation function.
            // If we have not created the thread already, we will create one here.
            if(!startedThread)
            {
                pthread_create (&threadId, NULL, ChangeLightRepresentation, (void *)this);
                startedThread = 1;
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

};

// ChangeLightRepresentaion is an observation function,
// which notifies any changes to the resource to stack
// via notifyObservers
void * ChangeLightRepresentation (void *param)
{
    LightResource* lightPtr = (LightResource*) param;

    // This function continuously monitors for the changes
    while (1)
    {
        sleep (3);

        if (gObservation)
        {
            // If under observation if there are any changes to the light resource
            // we call notifyObservors
            //
            // For demostration we are changing the power value and notifying.
            lightPtr->m_power += 10;

            cout << "\nPower updated to : " << lightPtr->m_power << endl;
            cout << "Notifying observers with resource handle: " << lightPtr->getHandle() << endl;

            OCStackResult result = OC_STACK_OK;

            if(isListOfObservers)
            {
                std::shared_ptr<OCResourceResponse> resourceResponse =
                            std::make_shared<OCResourceResponse>();

                resourceResponse->setErrorCode(200);
                resourceResponse->setResourceRepresentation(lightPtr->get(), DEFAULT_INTERFACE);

                result = OCPlatform::notifyListOfObservers(
                                                            lightPtr->getHandle(),
                                                            lightPtr->m_interestedObservers,
                                                            resourceResponse,
                                                            OC::QualityOfService::HighQos);
            }
            else
            {
                result = OCPlatform::notifyAllObservers(lightPtr->getHandle(),
                                                            OC::QualityOfService::HighQos);
            }

            if(OC_STACK_NO_OBSERVERS == result)
            {
                cout << "No More observers, stopping notifications" << endl;
                gObservation = 0;
            }
        }
    }

    return NULL;
}

void PrintUsage()
{
    std::cout << std::endl;
    std::cout << "Usage : simpleserverHQ <ObserveType>\n";
    std::cout << "   ObserveType : 0 - Observe All\n";
    std::cout << "   ObserveType : 1 - Observe List of observers\n\n";
}


int main(int argc, char* argv[])
{
    PrintUsage();

    if (argc == 1)
    {
        isListOfObservers = 0;
    }
    else if (argc == 2)
    {
        int value = atoi(argv[1]);
        if (value == 1)
            isListOfObservers = 1;
        else
            isListOfObservers = 0;
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
        // Create the instance of the resource class
        // (in this case instance of class 'LightResource').
        LightResource myLight(cfg);

        // Invoke createResource function of class light.
        myLight.createResource();

        myLight.addType(std::string("core.brightlight"));
        myLight.addInterface(std::string(LINK_INTERFACE));

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock, []{return false;});
    }
    catch(OCException& e)
    {
        oclog() << "Exception in main: "<< e.what();
    }

    // No explicit call to stop the platform.
    // When OCPlatform destructor is invoked, internally we do platform cleanup

    return 0;
}


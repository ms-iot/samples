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

// OCClient.cpp : Defines the entry point for the console application.
//
#include <set>
#include <string>
#include <cstdlib>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

struct dereference_compare
{
    bool operator()(std::shared_ptr<OCResource> lhs, std::shared_ptr<OCResource> rhs )const
    {
        return *lhs < *rhs;
    }
};
typedef std::set<std::shared_ptr<OCResource>, dereference_compare> DiscoveredResourceSet;

DiscoveredResourceSet discoveredResources;
const int SUCCESS_RESPONSE = 0;
std::shared_ptr<OCResource> curResource;
std::mutex resourceLock;
static ObserveType OBSERVE_TYPE_TO_USE = ObserveType::Observe;

class Light
{
public:

    bool m_state;
    int m_power;
    std::string m_name;

    Light() : m_state(false), m_power(0), m_name("")
    {
    }
};

Light mylight;

int observe_count()
{
    static int oc = 0;
    return ++oc;
}

void onObserve(const HeaderOptions /*headerOptions*/, const OCRepresentation& rep,
                    const int& eCode, const int& sequenceNumber)
{
    if(eCode == SUCCESS_RESPONSE)
    {
        std::cout << "OBSERVE RESULT:"<<std::endl;
        if(sequenceNumber == (int) ObserveAction::ObserveRegister)
        {
            std::cout << "\tObserve Registration Confirmed: "<< std::endl;
        }
        else if (sequenceNumber == (int) ObserveAction::ObserveUnregister)
        {
            std::cout << "\tObserve Cancel Confirmed: "<< std::endl;
            sleep(10);
            std::cout << "DONE"<<std::endl;
            std::exit(0);
        }
        else
        {
            std::cout << "\tSequenceNumber: "<< sequenceNumber << std::endl;
        }

        rep.getValue("state", mylight.m_state);
        rep.getValue("power", mylight.m_power);
        rep.getValue("name", mylight.m_name);

        std::cout << "\tstate: " << mylight.m_state << std::endl;
        std::cout << "\tpower: " << mylight.m_power << std::endl;
        std::cout << "\tname: " << mylight.m_name << std::endl;

        if(observe_count() == 11)
        {
            std::cout<<"Cancelling Observe..."<<std::endl;
            OCStackResult result = curResource->cancelObserve(OC::QualityOfService::HighQos);

            std::cout << "Cancel result: "<< result << " waiting for confirmation ..." <<std::endl;
        }
    }
    else
    {
        std::cout << "onObserve Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

void onPost2(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    if(eCode == SUCCESS_RESPONSE)
    {
        std::cout << "POST request was successful" << std::endl;

        if(rep.hasAttribute("createduri"))
        {
            std::cout << "\tUri of the created resource: "
                      << rep.getValue<std::string>("createduri") << std::endl;
        }
        else
        {
            rep.getValue("state", mylight.m_state);
            rep.getValue("power", mylight.m_power);
            rep.getValue("name", mylight.m_name);

            std::cout << "\tstate: " << mylight.m_state << std::endl;
            std::cout << "\tpower: " << mylight.m_power << std::endl;
            std::cout << "\tname: " << mylight.m_name << std::endl;
        }

        if (OBSERVE_TYPE_TO_USE == ObserveType::Observe)
            std::cout << std::endl << "Observe is used." << std::endl << std::endl;
        else if (OBSERVE_TYPE_TO_USE == ObserveType::ObserveAll)
            std::cout << std::endl << "ObserveAll is used." << std::endl << std::endl;
        sleep(1);
        curResource->observe(OBSERVE_TYPE_TO_USE, QueryParamsMap(), &onObserve,
                OC::QualityOfService::HighQos);

    }
    else
    {
        std::cout << "onPost2 Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

void onPost(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& rep, const int eCode)
{
    if(eCode == SUCCESS_RESPONSE)
    {
        std::cout << "POST request was successful" << std::endl;

        if(rep.hasAttribute("createduri"))
        {
            std::cout << "\tUri of the created resource: "
                      << rep.getValue<std::string>("createduri") << std::endl;
        }
        else
        {
            rep.getValue("state", mylight.m_state);
            rep.getValue("power", mylight.m_power);
            rep.getValue("name", mylight.m_name);

            std::cout << "\tstate: " << mylight.m_state << std::endl;
            std::cout << "\tpower: " << mylight.m_power << std::endl;
            std::cout << "\tname: " << mylight.m_name << std::endl;
        }

        OCRepresentation rep2;

        std::cout << "Posting light representation..."<<std::endl;

        mylight.m_state = true;
        mylight.m_power = 55;

        rep2.setValue("state", mylight.m_state);
        rep2.setValue("power", mylight.m_power);
        sleep(1);
        curResource->post(rep2, QueryParamsMap(), &onPost2, OC::QualityOfService::HighQos);
    }
    else
    {
        std::cout << "onPost Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

// Local function to put a different state for this resource
void postLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Posting light representation..."<<std::endl;

        mylight.m_state = false;
        mylight.m_power = 105;

        rep.setValue("state", mylight.m_state);
        rep.setValue("power", mylight.m_power);

        // Invoke resource's post API with rep, query map and the callback parameter
        resource->post(rep, QueryParamsMap(), &onPost, OC::QualityOfService::HighQos);
    }
}

// callback handler on PUT request
void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    if(eCode == SUCCESS_RESPONSE)
    {
        std::cout << "PUT request was successful" << std::endl;

        rep.getValue("state", mylight.m_state);
        rep.getValue("power", mylight.m_power);
        rep.getValue("name", mylight.m_name);

        std::cout << "\tstate: " << mylight.m_state << std::endl;
        std::cout << "\tpower: " << mylight.m_power << std::endl;
        std::cout << "\tname: " << mylight.m_name << std::endl;
        sleep(1);
        postLightRepresentation(curResource);
    }
    else
    {
        std::cout << "onPut Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

// Local function to put a different state for this resource
void putLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Putting light representation..."<<std::endl;

        mylight.m_state = true;
        mylight.m_power = 15;

        rep.setValue("state", mylight.m_state);
        rep.setValue("power", mylight.m_power);

        // Invoke resource's put API with rep, query map and the callback parameter
        resource->put(rep, QueryParamsMap(), &onPut, OC::QualityOfService::HighQos);
    }
}

// Callback handler on GET request
void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    if(eCode == SUCCESS_RESPONSE)
    {
        std::cout << "GET request was successful" << std::endl;
        std::cout << "Resource URI: " << rep.getUri() << std::endl;

        rep.getValue("state", mylight.m_state);
        rep.getValue("power", mylight.m_power);
        rep.getValue("name", mylight.m_name);

        std::cout << "\tstate: " << mylight.m_state << std::endl;
        std::cout << "\tpower: " << mylight.m_power << std::endl;
        std::cout << "\tname: " << mylight.m_name << std::endl;
        sleep(1);
        putLightRepresentation(curResource);
    }
    else
    {
        std::cout << "onGET Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

// Local function to get representation of light resource
void getLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        std::cout << "Getting Light Representation..."<<std::endl;
        // Invoke resource's get API with the callback parameter

        QueryParamsMap test;
        resource->get(test, &onGet,OC::QualityOfService::HighQos);
    }
}

// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    std::string resourceURI;
    std::string hostAddress;
    try
    {
        // Do some operations with resource object.
        if(resource)
        {
            std::lock_guard<std::mutex> lk(resourceLock);

            if(discoveredResources.find(resource) == discoveredResources.end())
            {
                std::cout << "Found resource " << resource->uniqueIdentifier() <<
                    " for the first time on server with ID: "<< resource->sid()<<std::endl;
                discoveredResources.insert(resource);
            }
            else
            {
                std::cout<<"Found resource "<< resource->uniqueIdentifier() << " again!"<<std::endl;
            }

            if(curResource)
            {
                std::cout << "Found another resource, ignoring"<<std::endl;
                return;
            }

            std::cout<<"DISCOVERED Resource:"<<std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;
            for(auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for(auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }

            if(resourceURI == "/a/light")
            {
                curResource = resource;
                sleep(1);
                // Call a local function which will internally invoke get
                // API on the resource pointer
                getLightRepresentation(resource);
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch(std::exception& e)
    {
        std::cerr << "Exception in foundResource: "<< e.what() <<std::endl;
    }
}

void PrintUsage()
{
    std::cout << std::endl;
    std::cout << "Usage : simpleclientHQ <ObserveType> <ConnectivityType>" << std::endl;
    std::cout << "   ObserveType : 1 - Observe" << std::endl;
    std::cout << "   ObserveType : 2 - ObserveAll" << std::endl;
    std::cout << "   ConnectivityType: Default IP" << std::endl;
    std::cout << "   ConnectivityType : 0 - IP"<< std::endl;
}

int main(int argc, char* argv[]) {

    std::ostringstream requestURI;

    OCConnectivityType connectivityType = CT_ADAPTER_IP;
    try
    {
        if (argc == 1)
        {
            OBSERVE_TYPE_TO_USE = ObserveType::Observe;
        }
        else if (argc ==2 || argc==3)
        {
            int value = std::stoi(argv[1]);
            if (value == 1)
                OBSERVE_TYPE_TO_USE = ObserveType::Observe;
            else if (value == 2)
                OBSERVE_TYPE_TO_USE = ObserveType::ObserveAll;
            else
                OBSERVE_TYPE_TO_USE = ObserveType::Observe;

            if(argc == 3)
            {
                std::size_t inputValLen;
                int optionSelected = std::stoi(argv[2], &inputValLen);

                if(inputValLen == strlen(argv[2]))
                {
                    if(optionSelected == 0)
                    {
                        std::cout << "Using IP."<< std::endl;
                        connectivityType = CT_ADAPTER_IP;
                    }
                    else
                    {
                        std::cout << "Invalid connectivity type selected. Using default IP"
                            << std::endl;
                    }
                }
                else
                {
                    std::cout << "Invalid connectivity type selected. Using default IP"
                            << std::endl;
                }
            }
        }
        else
        {
            PrintUsage();
            return -1;
        }
    }
    catch(std::exception&)
    {
        std::cout << "Invalid input argument." << std::endl;
        PrintUsage();
        return -1;
    }


    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Client,
        "0.0.0.0",
        0,
        OC::QualityOfService::HighQos
    };

    OCPlatform::Configure(cfg);

    try
    {
        // Find all resources
        requestURI << OC_RSRVD_WELL_KNOWN_URI << "?rt=core.light";

        OCPlatform::findResource("", requestURI.str(),
                connectivityType, &foundResource, OC::QualityOfService::LowQos);
        std::cout<< "Finding Resource... " <<std::endl;

        // Find resource is done twice so that we discover the original resources a second time.
        // These resources will have the same uniqueidentifier (yet be different objects), so that
        // we can verify/show the duplicate-checking code in foundResource(above);
        OCPlatform::findResource("", requestURI.str(),
                connectivityType, &foundResource, OC::QualityOfService::LowQos);
        std::cout<< "Finding Resource for second time... " <<std::endl;

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);

    }
    catch(OCException& e)
    {
        oclog() << "Exception in main: "<<e.what();
    }

    return 0;
}



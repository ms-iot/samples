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
#include <string>
#include <cstdlib>
#include <pthread.h>
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

const int SUCCESS_RESPONSE = 0;
std::shared_ptr<OCResource> curResource;
std::mutex resourceLock;

int observe_count()
{
    static int oc = 0;
    return ++oc;
}

static void printUsage()
{
    std::cout << "Usage roomclient <0|1>" << std::endl;
    std::cout<<"connectivityType: Default" << std::endl;
    std::cout << "connectivityType 0: IP" << std::endl;
}
// Forward declaration
void putRoomRepresentation(std::shared_ptr<OCResource> resource);
void onPut(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode);

void printRoomRepresentation(const OCRepresentation& rep)
{
    std::cout << "\tResource URI: " << rep.getUri() << std::endl;

    if(rep.hasAttribute("name"))
    {
        std::cout << "\tRoom name: " << rep.getValue<std::string>("name") << std::endl;
    }

    std::vector<OCRepresentation> children = rep.getChildren();

    for(auto oit = children.begin(); oit != children.end(); ++oit)
    {
        std::cout << "\t\tChild Resource URI: " << oit->getUri() << std::endl;
        if(oit->getUri().find("light") != std::string::npos)
        {
            if(oit->hasAttribute("state") && oit->hasAttribute("color"))
            {
                std::cout << "\t\tstate:" << oit->getValue<bool>("state")  << std::endl;
                std::cout << "\t\tcolor:" << oit->getValue<int>("color")  << std::endl;
            }
        }
        else if(oit->getUri().find("fan") != std::string::npos)
        {
            if(oit->hasAttribute("state") && oit->hasAttribute("speed"))
            {
                std::cout << "\t\tstate:" << oit->getValue<bool>("state") << std::endl;
                std::cout << "\t\tspeed:" << oit->getValue<int>("speed") << std::endl;
            }
        }
    }
}

// callback handler on GET request
void onGet(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& rep, const int eCode)
{
    if(eCode == SUCCESS_RESPONSE)
    {
        std::cout << "GET request was successful" << std::endl;

        printRoomRepresentation(rep);

        putRoomRepresentation(curResource);
    }
    else
    {
        std::cout << "onGET Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

void onGet1(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& rep, const int eCode)
{
    if(eCode == SUCCESS_RESPONSE)
    {
        std::cout << "GET request was successful" << std::endl;

        printRoomRepresentation(rep);
    }
    else
    {
        std::cout << "onGET Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

// Local function to get representation of room resource
void getRoomRepresentation(std::shared_ptr<OCResource> resource,
                            std::string interface, GetCallback getCallback)
{
    if(resource)
    {
        std::cout << "Getting room representation on: "<< interface << std::endl;

        resource->get("core.room", interface, QueryParamsMap(), getCallback);
    }
}

// Local function to put a different state for this resource
void putRoomRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;
        std::cout << "Putting room representation on: " << BATCH_INTERFACE << std::endl;

        bool state = true;
        int speed = 10;
        rep.setValue("state", state);
        rep.setValue("speed", speed);

        // Invoke resource's pit API with attribute map, query map and the callback parameter
        resource->put("core.room", BATCH_INTERFACE, rep, QueryParamsMap(), &onPut);
    }
}

// callback handler on PUT request
void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    if(eCode == SUCCESS_RESPONSE)
    {
        std::cout << "PUT request was successful" << std::endl;

        printRoomRepresentation(rep);

        getRoomRepresentation(curResource, DEFAULT_INTERFACE, onGet1);
    }
    else
    {
        std::cout << "onPut Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    std::lock_guard<std::mutex> lock(resourceLock);
    if(curResource)
    {
        std::cout << "Found another resource, ignoring"<<std::endl;
        return;
    }

    std::string resourceURI;
    std::string hostAddress;
    try
    {
        // Do some operations with resource object.
        if(resource)
        {
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

            if(resourceURI == "/a/room")
            {
                curResource = resource;
                // Call a local function which will internally invoke get API on the resource pointer
                getRoomRepresentation(resource, BATCH_INTERFACE, onGet);
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
        std::cerr << "Exception caught in Found Resource: "<< e.what() <<std::endl;
    }
}

int main(int argc, char* argv[]) {

    std::ostringstream requestURI;

    OCConnectivityType connectivityType = CT_ADAPTER_IP;
    if(argc == 2)
    {
        try
        {
            std::size_t inputValLen;
            int optionSelected = std::stoi(argv[1], &inputValLen);

            if(inputValLen == strlen(argv[1]))
            {
                if(optionSelected == 0)
                {
                    std::cout << "Using IP."<< std::endl;
                    connectivityType = CT_ADAPTER_IP;
                }
                else
                {
                    std::cout << "Invalid connectivity type selected. Using default IP"<< std::endl;
                }
            }
            else
            {
                std::cout << "Invalid connectivity type selected. Using default IP" << std::endl;
            }
        }
        catch(std::exception&)
        {
            std::cout << "Invalid input argument. Using IP as connectivity type" << std::endl;
        }
    }
    else
    {
        std::cout << "Default input argument. Using IP as Default connectivity type" << std::endl;
        printUsage();
    }

    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Client,
        "0.0.0.0",
        0,
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);

    try
    {
        // Find all resources
        requestURI << OC_RSRVD_WELL_KNOWN_URI;

        OCPlatform::findResource("", requestURI.str(), connectivityType, &foundResource);
        std::cout<< "Finding Resource... " <<std::endl;

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);

    }catch(OCException& e)
    {
        oclog() << "Exception in main: "<< e.what();
    }

    return 0;
}



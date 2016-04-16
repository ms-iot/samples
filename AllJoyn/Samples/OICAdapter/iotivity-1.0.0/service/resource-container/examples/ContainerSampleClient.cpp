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
#include <map>
#include <cstdlib>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

typedef std::map<OCResourceIdentifier, std::shared_ptr<OCResource>> DiscoveredResourceMap;

DiscoveredResourceMap discoveredResources;
std::shared_ptr<OCResource> curResource;
std::shared_ptr<OCResource> DISensorResource;
static ObserveType OBSERVE_TYPE_TO_USE = ObserveType::Observe;
std::mutex curResourceLock;

class Light
{
    public:

        bool m_on_off;
        int m_color;
        int m_dim;
        std::string m_name;


        Light() : m_on_off(false), m_color(0), m_dim(0), m_name("")
        {
        }
};

Light mylight;

int observe_count()
{
    static int oc = 0;
    return ++oc;
}

void onObserve(const HeaderOptions headerOptions, const OCRepresentation &rep,
               const int &eCode, const int &sequenceNumber)
{
    (void)headerOptions;
    try
    {
        if (eCode == OC_STACK_OK)
        {
            std::cout << "OBSERVE RESULT:" << std::endl;
            std::cout << "\tSequenceNumber: " << sequenceNumber << std::endl;
            rep.getValue("on-off", mylight.m_on_off);
            rep.getValue("color", mylight.m_color);
            rep.getValue("dim", mylight.m_dim);
            rep.getValue("name", mylight.m_name);

            std::cout << "\ton-off: " << mylight.m_on_off << std::endl;
            std::cout << "\tcolor: " << mylight.m_color << std::endl;
            std::cout << "\tdim: " << mylight.m_dim << std::endl;

            if (observe_count() > 10)
            {
                std::cout << "Cancelling Observe..." << std::endl;
                OCStackResult result = curResource->cancelObserve();

                std::cout << "Cancel result: " << result << std::endl;
                sleep(10);
                std::cout << "DONE" << std::endl;
                std::exit(0);
            }
        }
        else
        {
            std::cout << "onObserve Response error: " << eCode << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onObserve" << std::endl;
    }

}

void onPost2(const HeaderOptions &headerOptions, const OCRepresentation &rep, const int eCode)
{
    (void)headerOptions;
    try
    {
        if (eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED)
        {
            std::cout << "POST request was successful" << std::endl;

            if (rep.hasAttribute("createduri"))
            {
                std::cout << "\tUri of the created resource: "
                          << rep.getValue<std::string>("createduri") << std::endl;
            }
            else
            {
                rep.getValue("on-off", mylight.m_on_off);
                rep.getValue("color", mylight.m_color);
                rep.getValue("dim", mylight.m_dim);

                std::cout << "\ton-off: " << mylight.m_on_off << std::endl;
                std::cout << "\tcolor: " << mylight.m_color << std::endl;
                std::cout << "\tdim: " << mylight.m_dim << std::endl;
            }

            if (OBSERVE_TYPE_TO_USE == ObserveType::Observe)
                std::cout << std::endl << "Observe is used." << std::endl << std::endl;
            else if (OBSERVE_TYPE_TO_USE == ObserveType::ObserveAll)
                std::cout << std::endl << "ObserveAll is used." << std::endl << std::endl;

            curResource->observe(OBSERVE_TYPE_TO_USE, QueryParamsMap(), &onObserve);

        }
        else
        {
            std::cout << "onPost2 Response error: " << eCode << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onPost2" << std::endl;
    }

}

void onPost(const HeaderOptions &headerOptions, const OCRepresentation &rep, const int eCode)
{
    (void)headerOptions;
    try
    {
        if (eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED)
        {
            std::cout << "POST request was successful" << std::endl;

            if (rep.hasAttribute("createduri"))
            {
                std::cout << "\tUri of the created resource: "
                          << rep.getValue<std::string>("createduri") << std::endl;
            }
            else
            {
                rep.getValue("on-off", mylight.m_on_off);
                rep.getValue("color", mylight.m_color);
                rep.getValue("dim", mylight.m_dim);

                std::cout << "\ton-off: " << mylight.m_on_off << std::endl;
                std::cout << "\tcolor: " << mylight.m_color << std::endl;
                std::cout << "\tdim: " << mylight.m_dim << std::endl;
            }

            OCRepresentation rep2;

            std::cout << "Posting light representation..." << std::endl;

            mylight.m_on_off = true;

            rep2.setValue("on-off", mylight.m_on_off);

            curResource->post(rep2, QueryParamsMap(), &onPost2);
        }
        else
        {
            std::cout << "onPost Response error: " << eCode << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onPost" << std::endl;
    }
}

// Local function to put a different state for this resource
void postLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if (resource)
    {
        OCRepresentation rep;

        std::cout << "Posting light representation..." << std::endl;

        mylight.m_on_off = "false";

        rep.setValue("on-off", mylight.m_on_off);

        // Invoke resource's post API with rep, query map and the callback parameter
        resource->post(rep, QueryParamsMap(), &onPost);
    }
}

// callback handler on PUT request
void onPut(const HeaderOptions &headerOptions, const OCRepresentation &rep, const int eCode)
{
    (void)headerOptions;
    (void)rep;
    try
    {
        if (eCode == OC_STACK_OK)
        {
            std::cout << "PUT request was successful" << std::endl;

            /*rep.getValue("on-off", mylight.m_on_off);
            rep.getValue("dim", mylight.m_dim);
            rep.getValue("color", mylight.m_color);

            std::cout << "\ton-off: " << mylight.m_on_off << std::endl;
            std::cout << "\tcolor: " << mylight.m_color << std::endl;
            std::cout << "\tdim: " << mylight.m_dim << std::endl;*/

            //postLightRepresentation(curResource);
        }
        else
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onPut" << std::endl;
    }
}

void onPutForDISensor(const HeaderOptions &headerOptions, const OCRepresentation &rep,
                      const int eCode)
{
    void onGetForDISensor(const HeaderOptions & headerOptions, const OCRepresentation & rep,
                          const int eCode);

    (void)headerOptions;
    (void)rep;
    try
    {
        if (eCode == OC_STACK_OK)
        {
            std::cout << "PUT request was successful" << std::endl;

            QueryParamsMap test;
            std::cout << "Sending request to: " << DISensorResource->uri() << std::endl;
            DISensorResource->get(test, &onGetForDISensor);
        }
        else
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onPut" << std::endl;
    }
}

// Local function to put a different state for this resource
void putLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if (resource)
    {
        OCRepresentation rep;

        std::cout << "Putting light representation..." << std::endl;

        mylight.m_on_off = true;

        std::cout << "Sending request to: " << resource->uri() << std::endl;
        rep.setValue("on-off", mylight.m_on_off);

        // Invoke resource's put API with rep, query map and the callback parameter

        resource->put(rep, QueryParamsMap(), &onPut);
    }
}

// Callback handler on GET request
void onGet(const HeaderOptions &headerOptions, const OCRepresentation &rep, const int eCode)
{
    (void)headerOptions;
    try
    {
        if (eCode == OC_STACK_OK)
        {
            std::cout << "GET request was successful" << std::endl;
            std::cout << "Resource URI: " << rep.getUri() << std::endl;

            std::cout << "Payload: " << rep.getPayload() << std::endl;

            rep.getValue("on-off", mylight.m_on_off);
            rep.getValue("dim", mylight.m_dim);
            rep.getValue("color", mylight.m_color);

            std::cout << "\ton-off: " << mylight.m_on_off << std::endl;
            std::cout << "\tcolor: " << mylight.m_color << std::endl;
            std::cout << "\tdim: " << mylight.m_dim << std::endl;

            putLightRepresentation(curResource);
        }
        else
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onGet" << std::endl;
    }
}

void onGetForDISensor(const HeaderOptions &headerOptions, const OCRepresentation &rep,
                      const int eCode)
{
    (void)headerOptions;
    try
    {
        if (eCode == OC_STACK_OK)
        {
            std::cout << "GET request was successful" << std::endl;
            std::cout << "Resource URI: " << DISensorResource->uri() << std::endl;

            std::cout << "Payload: " << rep.getPayload() << std::endl;

            std::cout << "\tdiscomfortIndex: " << rep.getValue<std::string>("discomfortIndex") << std::endl;
        }
        else
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onPut" << std::endl;
    }
}

// Local function to get representation of light resource
void getLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if (resource)
    {
        std::cout << "Getting Light Representation..." << std::endl;
        // Invoke resource's get API with the callback parameter

        QueryParamsMap test;
        std::cout << "Sending request to: " << resource->uri() << std::endl;
        resource->get(test, &onGet);
    }
}

// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    std::cout << "In foundResource\n";
    std::string resourceURI = resource->uri();
    std::string hostAddress;
    try
    {
        {
            std::lock_guard<std::mutex> lock(curResourceLock);
            if (discoveredResources.find(resource->uniqueIdentifier()) == discoveredResources.end())
            {
                std::cout << "Found resource " << resource->uniqueIdentifier() <<
                          " for the first time on server with ID: " << resource->sid() << std::endl;
                discoveredResources[resource->uniqueIdentifier()] = resource;

                if (resourceURI.find("/discomfortIndex") != std::string::npos)
                {
                    std::cout << "discomfortIndex found !!! " << std::endl;

                    DISensorResource = resource;

                    OCRepresentation rep;

                    rep.setValue("humidity", std::string("30"));
                    rep.setValue("temperature", std::string("27"));

                    resource->put(rep, QueryParamsMap(), &onPutForDISensor);
                }
            }
            else
            {
                std::cout << "Found resource " << resource->uniqueIdentifier() << " again!" << std::endl;
            }

            if (curResource)
            {
                std::cout << "Found another resource, ignoring" << std::endl;
                return;
            }
        }

        // Do some operations with resource object.
        if (resource)
        {
            std::cout << "DISCOVERED Resource:" << std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;
            for (auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;

                if (resourceTypes == "oic.r.light")
                {
                    curResource = resource;
                    // Call a local function which will internally invoke get API on the resource pointer
                    getLightRepresentation(resource);
                }
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for (auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch (std::exception &e)
    {
        std::cerr << "Exception in foundResource: " << e.what() << std::endl;
    }
}

void printUsage()
{
    std::cout << std::endl;
    std::cout << "---------------------------------------------------------------------\n";
    std::cout << "Usage : ContainerSampleClient <ObserveType>" << std::endl;
    std::cout << "   ObserveType : 1 - Observe" << std::endl;
    std::cout << "   ObserveType : 2 - ObserveAll" << std::endl;
    std::cout << "---------------------------------------------------------------------\n\n";
}

void checkObserverValue(int value)
{
    if (value == 1)
    {
        OBSERVE_TYPE_TO_USE = ObserveType::Observe;
        std::cout << "<===Setting ObserveType to Observe===>\n\n";
    }
    else if (value == 2)
    {
        OBSERVE_TYPE_TO_USE = ObserveType::ObserveAll;
        std::cout << "<===Setting ObserveType to ObserveAll===>\n\n";
    }
    else
    {
        std::cout << "<===Invalid ObserveType selected."
                  << " Setting ObserveType to Observe===>\n\n";
    }
}

static FILE *client_open(const char *path, const char *mode)
{
    (void)path;

    return fopen("./oic_svr_db_client.json", mode);
}

int main(int argc, char *argv[])
{

    std::ostringstream requestURI;
    OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };
    try
    {
        printUsage();
        if (argc == 1)
        {
            std::cout << "<===Setting ObserveType to Observe and ConnectivityType to IP===>\n\n";
        }
        else if (argc == 2)
        {
            checkObserverValue(std::stoi(argv[1]));
        }
        else
        {
            std::cout << "<===Invalid number of command line arguments===>\n\n";
            return -1;
        }
    }
    catch (std::exception &)
    {
        std::cout << "<===Invalid input arguments===>\n\n";
        return -1;
    }

    // Create PlatformConfig object
    PlatformConfig cfg
    {
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        "0.0.0.0",
        0,
        OC::QualityOfService::LowQos,
        &ps
    };

    OCPlatform::Configure(cfg);
    try
    {
        // makes it so that all boolean values are printed as 'true/false' in this stream
        std::cout.setf(std::ios::boolalpha);
        // Find all resources
        requestURI << OC_RSRVD_WELL_KNOWN_URI;// << "?rt=core.light";

        OCPlatform::findResource("", requestURI.str(),
                                 CT_DEFAULT, &foundResource);
        std::cout << "Finding Resource... " << std::endl;

        // Find resource is done twice so that we discover the original resources a second time.
        // These resources will have the same uniqueidentifier (yet be different objects), so that
        // we can verify/show the duplicate-checking code in foundResource(above);
        OCPlatform::findResource("", requestURI.str(),
                                 CT_DEFAULT, &foundResource);
        std::cout << "Finding Resource for second time..." << std::endl;

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);

    }
    catch (OCException &e)
    {
        oclog() << "Exception in main: " << e.what();
    }

    return 0;
}



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

/**
 * @file SampleConsumer.cpp
 * @brief Defines the entry point for the sample consumer application about Resource Hosting.
 */

#include <string>
#include <cstdlib>
#include <pthread.h>
#include "OCPlatform.h"
#include "OCApi.h"
#include <mutex>

using namespace OC;

const int SUCCESS_RESPONSE = OC_STACK_OK;

#define OC_WELL_KNOWN_COORDINATING_QUERY "/oic/res?rt=oic.r.resourcehosting"

#define OBSERVE 1
#define GET     2
#define PUT     3
#define DELETE  4

std::shared_ptr< OCResource > g_curResource;
std::shared_ptr< OCResource > g_curObserveResource;
std::mutex curResourceLock;

OCStackResult nmfindResource(const std::string &host , const std::string &resourceName);
void onObserve(const HeaderOptions &headerOption , const OCRepresentation &rep , const int &eCode,
               const int &sequenceNumber);

void onPut(const HeaderOptions &headerOption, const OCRepresentation &rep, const int eCode);
void onGet(const HeaderOptions &headerOption , const OCRepresentation &rep , const int eCode);
void onDelete(const HeaderOptions &headerOption , const int eCode);


void findResourceCandidate()
{
    try
    {
        nmfindResource("" , OC_WELL_KNOWN_COORDINATING_QUERY);
        std::cout << "Finding Resource... " << std::endl;

    }
    catch (OCException &e)
    {
        std::cout << "Exception for find resource : " << e.reason() << std::endl;
    }
}

void startObserve(std::shared_ptr< OCResource > resource)
{
    if (resource == NULL)
    {
        std::cout << "startObserve() error : resource == null" << std::endl;
        return;
    }

    if(g_curObserveResource == NULL)
    {
        g_curObserveResource = resource;
        std::cout << "request for new observation" << std::endl;
    }
    else if(g_curObserveResource == g_curResource)
    {
        std::cout << "already registered same observation" << std::endl;
        return;
    }
    else
    {
        std::cout << "change observed resource" << std::endl;
        g_curObserveResource->cancelObserve();
        g_curObserveResource = resource;
    }

    QueryParamsMap test;
    if (OC_STACK_OK != resource->observe(ObserveType::Observe , test , &onObserve))
        std::cout << "To Fail resource observe() process" << std::endl;
}

void startGet(std::shared_ptr< OCResource > resource)
{

    if (resource == NULL)
    {
        std::cout << "startObserve() error : resource == null" << std::endl;
        return;
    }

    QueryParamsMap test;
    std::cout << "URI :" << resource->uri() << std::endl;
    if (OC_STACK_OK != resource->get(test, &onGet))
        std::cout << "To Fail resource get() process" << std::endl;
}

void startPut(std::shared_ptr< OCResource > resource)
{
    if (resource == NULL)
    {
        std::cout << "startObserve() error : resource == null" << std::endl;
        return;
    }

    g_curResource = resource;
    OCRepresentation rep;
    rep.setValue("temperature", 25);
    rep.setValue("humidity", 10);

    QueryParamsMap test;
    if (OC_STACK_OK != resource->put(rep, test, &onPut))
        std::cout << "To Fail resource put() process" << std::endl;
}

void startDelete(std::shared_ptr< OCResource > resource)
{
    if (resource == NULL)
    {
        std::cout << "startObserve() error : resource == null" << std::endl;
        return;
    }

    g_curResource = resource;
    if (OC_STACK_OK != resource->deleteResource(&onDelete))
        std::cout << "To Fail resource delete() process" << std::endl;
}

int observe_count()
{
    static int oc = 0;
    return ++oc;
}

void onObserve(const HeaderOptions &/*headerOption*/, const OCRepresentation &rep , const int &eCode,
               const int &sequenceNumber)
{
    std::cout << "onObserve" << std::endl;

    if (eCode <= OC_STACK_OK)
    {
        std::cout << std::endl;
        std::cout << "========================================================" << std::endl;
        std::cout << "Receive OBSERVE RESULT:" << std::endl;
        std::cout << "\tUri: " << rep.getUri() << std::endl;
        std::cout << "\tSequenceNumber: " << sequenceNumber << std::endl;
        std::cout << "\tTemperature : " << rep.getValue<int>("temperature") << std::endl;
        std::cout << "\tHumidity : " << rep.getValue<int>("humidity") << std::endl;

        if (observe_count() > 30)
        {
            std::cout << "Cancelling Observe..." << std::endl;
            OCStackResult result = g_curResource->cancelObserve();

            std::cout << "Cancel result: " << result << std::endl;
            sleep(10);
            std::cout << "DONE" << std::endl;
            std::exit(0);
        }
    }
    else
    {
        std::cout << "onObserve Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

void foundResource(std::shared_ptr< OCResource > resource)
{
    std::string resourceURI;
    std::string hostAddress;

    std::cout << "foundResource" << std::endl;

    try
    {
        std::cout << "mutex lock passed" << std::endl;
        if (resource)
        {
            std::cout << resource->uri() << std::endl;
            if (resource->uri() == "/a/TempHumSensor")
            {
                std::cout << std::endl;
                std::cout << "========================================================" << std::endl;
                std::cout << "DISCOVERED Resource(Consumer):" << std::endl;

                resourceURI = resource->uri();
                std::cout << "\tURI of the resource: " << resourceURI << std::endl;

                hostAddress = resource->host();
                std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

                g_curResource = resource;
            }
        }
        else
        {
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in foundResource" << std::endl;
    }
}

OCStackResult nmfindResource(const std::string &host , const std::string &resourceName)
{
    return OCPlatform::findResource(host , resourceName , CT_DEFAULT, &foundResource);
}

void getRepresentation(std::shared_ptr< OCResource > resource)
{
    if (resource)
    {
        std::cout << "Getting Light Representation..." << std::endl;
    }
}

void onPut(const HeaderOptions &/*headerOption*/, const OCRepresentation &rep, const int eCode)
{
    try
    {
        if (eCode == OC_STACK_OK)
        {
            std::cout << "PUT request was successful" << std::endl;
            int humidity;
            int temperature;
            rep.getValue("temperature", temperature);
            rep.getValue("humidity", humidity);


            std::cout << "\t temperature: " << temperature << std::endl;
            std::cout << "\t humidity: " << humidity << std::endl;
        }
        else
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onPut" << std::endl;
    }
}

//callback hadnler on DELETE request
void onDelete(const HeaderOptions &/*headerOption*/, const int eCode)
{
    try
    {
        if (eCode == OC_STACK_RESOURCE_DELETED)
        {
            std::cout << "DELETE request was successful" << std::endl;
        }
        else
        {
            std::cout << "onDelete Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << " in onDelete" << std::endl;
    }
}

// callback handler on GET request
void onGet(const HeaderOptions &/*headerOption*/, const OCRepresentation &rep , const int eCode)
{
    std::cout << "GET request was successful1" << std::endl;
    if (eCode == SUCCESS_RESPONSE)
    {
        std::cout << "GET request was successful" << std::endl;
        std::cout << "Resource URI: " << rep.getUri().c_str() << std::endl;
        std::cout << "\tTemperature : " << rep.getValue<int>("temperature") << std::endl;
        std::cout << "\tHumidity : " << rep.getValue<int>("humidity") << std::endl;
    }
    else
    {
        std::cout << "onGET Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

void getLightRepresentation(std::shared_ptr< OCResource > resource)
{
    if (resource)
    {
        std::cout << "Getting Light Representation..." << std::endl;

        QueryParamsMap test;
        resource->get(test , &onGet);
    }
}

void PrintUsage()
{
    std::cout << std::endl;
    std::cout << "Usage : simpleclient <ObserveType>" << std::endl;
    std::cout << "   ObserveType : 1 - Observe" << std::endl;
    std::cout << "   ObserveType : 2 - ObserveAll" << std::endl;
}

void PRINT()
{
    std::cout << std::endl;
    std::cout << "********************************************" << std::endl;
    std::cout << "*  method Type : 1 - Observe               *" << std::endl;
    std::cout << "*  method Type : 2 - Get                   *" << std::endl;
    std::cout << "*  method Type : 3 - Put                   *" << std::endl;
    std::cout << "********************************************" << std::endl;
    std::cout << std::endl;
}

int main()
{

    int in;
    PlatformConfig cfg;

    OCPlatform::Configure(cfg);

    std::cout << "Created Platform..." << std::endl;

    g_curResource = NULL;
    g_curObserveResource = NULL;

    findResourceCandidate();

    while (1)
    {
        sleep(2);
        if(g_curResource == NULL)
        {
            continue;
        }
        PRINT();

        in = 0;
        std::cin >> in;

        if(std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input type, please try again" << std::endl;
            continue;
        }

        try {
            switch ((int)in)
            {
                case OBSERVE:
                    startObserve(g_curResource);
                    break;
                case GET:
                    startGet(g_curResource);
                    break;
                case PUT:
                    startPut(g_curResource);
                    break;
                default:
                    std::cout << "Invalid input, please try again" << std::endl;
                    break;
            }
        }catch(OCException & e) {
            std::cout<< "Caught OCException [Code: "<<e.code()<<" Reason: "<<e.reason()<<std::endl;
        }
    }

    return 0;
}

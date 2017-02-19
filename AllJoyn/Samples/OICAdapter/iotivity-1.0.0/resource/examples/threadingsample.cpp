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
/// This sample demonstrates : running one server in main thread, another
/// server in a separate thread, and running 2 clients in each thread.
///


#include <memory>
#include <iostream>
#include <stdexcept>
#include <condition_variable>
#include <map>
#include <vector>

#include "OCPlatform.h"
#include "OCApi.h"
using namespace OC;

static std::ostringstream requestURI;

struct FooResource
{
    bool m_isFoo;
    int m_barCount;
    std::string m_uri;
    std::string m_resourceType;
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_rep;

    FooResource(std::string uri): m_isFoo(true), m_barCount (0),
                                    m_uri(uri), m_resourceType("core.foo")
    {
        m_rep.setUri(m_uri);
        m_rep.setValue("isFoo", m_isFoo);
        m_rep.setValue("barCount", m_barCount);
    }

    bool createResource()
    {
        std::string resourceInterface = DEFAULT_INTERFACE;

        uint8_t resourceProperty = OC_DISCOVERABLE;

        EntityHandler eh(std::bind(&FooResource::entityHandler, this,
                                    std::placeholders::_1));
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle, m_uri,
                                    m_resourceType, resourceInterface, eh, resourceProperty);
        if(OC_STACK_OK != result)
        {
            std::cout<<"Resource creation unsuccessful"<<std::endl;
            return false;
        }

        return true;
    }

    OCRepresentation get()
    {
        m_rep.setValue("isFoo", m_isFoo);
        m_rep.setValue("barCount", m_barCount);

        return m_rep;
    }

    void put(OCRepresentation& rep)
    {
        rep.getValue("isFoo", m_isFoo);
        rep.getValue("barCount", m_barCount);
    }

    OCStackResult sendResponse(std::shared_ptr<OCResourceRequest> pRequest)
    {
        auto pResponse = std::make_shared<OC::OCResourceResponse>();
        pResponse->setRequestHandle(pRequest->getRequestHandle());
        pResponse->setResourceHandle(pRequest->getResourceHandle());
        pResponse->setResourceRepresentation(get(), "");
        pResponse->setErrorCode(200);
        pResponse->setResponseResult(OC_EH_OK);

        return OCPlatform::sendResponse(pResponse);
    }

    OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        std::cout<<"\tConsumer Entity Handler:"<<std::endl;
        OCEntityHandlerResult ehResult = OC_EH_ERROR;

        if(request)
        {
            // Note: Most of the handlers are not here, since this is for demoing client/server
            // co-process existence. See simpleserver for a more complete example.
            if(request->getRequestHandlerFlag()  == RequestHandlerFlag::RequestFlag)
            {
                std::cout << "\t\trequestFlag : Request"<<std::endl;

                if(request->getRequestType() == "GET")
                {
                    std::cout<<"\t\t\trequestType : GET"<<std::endl;
                    if(OC_STACK_OK == sendResponse(request))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if (request->getRequestType() == "PUT")
                {
                    std::cout<<"\t\t\trequestType : PUT"<<std::endl;

                    OCRepresentation rep = request->getResourceRepresentation();
                    put(rep);
                    if(OC_STACK_OK == sendResponse(request))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else
                {
                    std::cout<<"\t\t\trequestType : UNSUPPORTED: " <<
                                request->getRequestType()<<std::endl;
                }
            }
            else
            {
                std::cout <<"\t\trequestFlag : UNSUPPORTED: ";

                if(request->getRequestHandlerFlag()== RequestHandlerFlag::ObserverFlag)
                {
                    std::cout<<"ObserverFlag"<<std::endl;
                }
            }
        }
        else
        {
            std::cout << "Request Invalid!"<<std::endl;
        }

        return ehResult;
    }
};

void putResourceInfo(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation rep, const OCRepresentation /*rep2*/, const int eCode)
{
   bool m_isFoo = false;
   int m_barCount = 0;
   std::cout << "In PutResourceInfo" << std::endl;

   std::cout <<"Clientside Put response to get was: "<<std::endl;
   std::cout <<"ErrorCode: "<<eCode <<std::endl;

   if(eCode == 0)
   {
        std::cout<<"Successful Put.  Attributes sent were: "<<std::endl;

        rep.getValue("isFoo", m_isFoo);
        rep.getValue("barCount", m_barCount);

        std::cout << "\tisFoo: "<< m_isFoo << std::endl;
        std::cout << "\tbarCount: "<< m_barCount << std::endl;

        std::cout<<"Actual New values are: "<<std::endl;

        rep.getValue("isFoo", m_isFoo);
        rep.getValue("barCount", m_barCount);

        std::cout << "\tisFoo: "<< m_isFoo << std::endl;
        std::cout << "\tbarCount: "<< m_barCount << std::endl;
   }
}

void getResourceInfo(std::shared_ptr<OCResource> resource, const HeaderOptions& /*headerOptions*/,
            const OCRepresentation rep,
            const int eCode)
{
    bool m_isFoo = false;
    int m_barCount = 0;
    std::cout << "In getResourceInfo" << std::endl;

    std::cout<<"Clientside response to get was: "<<std::endl;
    std::cout<<"Error Code: "<<eCode<<std::endl;

    if(eCode == 0)
    {
        std::cout <<"Successful Get.  Attributes are: "<<std::endl;

        rep.getValue("isFoo", m_isFoo);
        rep.getValue("barCount", m_barCount);

        std::cout << "\tisFoo: "<< m_isFoo << std::endl;
        std::cout << "\tbarCount: "<< m_barCount << std::endl;

        std::cout << "Doing a put on q/foo" <<std::endl;
        OCRepresentation rep2(rep);
        m_isFoo = false;
        m_barCount = 211;

        rep2.setValue("isFoo", m_isFoo);
        rep2.setValue("barCount", m_barCount);

        resource->put(rep2, QueryParamsMap(),
            PutCallback(std::bind(putResourceInfo, std::placeholders::_1,
                 rep2, std::placeholders::_2, std::placeholders::_3)));
    }
}

void printResourceInfo(std::shared_ptr<OCResource> resource)
{
        std::cout << "Found Resource: "<<std::endl;
        std::cout << "\tHost: "<< resource->host()<<std::endl;
        std::cout << "\tURI:  "<< resource->uri()<<std::endl;

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
}

void foundResource2(std::shared_ptr<OCResource> resource)
{
    std::cout << "In foundResource2:" << std::endl;

    if(resource && resource->uri() == "/q/foo2")
    {
        printResourceInfo(resource);

        std::cout<<"Doing a get on q/foo."<<std::endl;

        resource->get(QueryParamsMap(),
            GetCallback(std::bind(getResourceInfo, resource,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    }
    else
    {
        std::cout << "foundResource2: Ignoring the resource which doesn't have uri /q/foo2\n";
    }
}

void foundResource1(std::shared_ptr<OCResource> resource)
{
    std::cout << "In foundResource1:" << std::endl;
    if(resource && resource->uri() == "/q/foo1")
    {
        printResourceInfo(resource);
    }
    else
    {
        std::cout << "foundResource1: Ignoring the resource which doesn't have uri /q/foo1\n";
    }
}

void client1()
{
    std::cout << "in client1\n";
    std::cout<<"result1:" << OCPlatform::findResource("", requestURI.str(),
            CT_DEFAULT, foundResource1)<< std::endl;

    // A condition variable will free the mutex it is given, then do a non-
    // intensive block until 'notify' is called on it.  In this case, since we
    // don't ever call cv.notify, this should be a non-processor intensive version
    // of while(true);
    std::mutex blocker;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(blocker);
    cv.wait(lock);
}

void client2()
{
    std::cout << "in client2\n";
    std::cout<<"result2:" << OCPlatform::findResource("",
                requestURI.str(),
                CT_DEFAULT, foundResource2)<< std::endl;

    // A condition variable will free the mutex it is given, then do a non-
    // intensive block until 'notify' is called on it.  In this case, since we
    // don't ever call cv.notify, this should be a non-processor intensive version
    // of while(true);
    std::mutex blocker;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(blocker);
    cv.wait(lock);
}

void server()
{
    FooResource fooRes("/q/foo2");

    if(!fooRes.createResource())
    {
        return;
    }

    // A condition variable will free the mutex it is given, then do a non-
    // intensive block until 'notify' is called on it.  In this case, since we
    // don't ever call cv.notify, this should be a non-processor intensive version
    // of while(true);
    std::mutex blocker;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(blocker);
    cv.wait(lock);
}

int main(int /*argc*/, char** /*argv[]*/)
{

    requestURI << OC_RSRVD_WELL_KNOWN_URI << "?rt=core.foo";

    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);

    try
    {
        // main thread running as server
        FooResource fooRes("/q/foo1");
        if(!fooRes.createResource())
        {
            return -1;
        }

        // Start a server in a seperate thread
        std::thread t(server);
        t.detach();

        sleep(10);

        // Start each client in a seperate thread
        std::thread t1(client1);
        t1.detach();

        // Start each client in a seperate thread
        std::thread t2(client2);
        t2.detach();

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
        std::cout<< "Exception in main: "<<e.what()<<std::endl;
    }

    return 0;
}


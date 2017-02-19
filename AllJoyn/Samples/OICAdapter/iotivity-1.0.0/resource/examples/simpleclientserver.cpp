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
/// Additionally, it'll have a client example to discover it as well.
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

static void printUsage()
{
    std::cout<< "    Usage simpleclientserver <0|1>" << std::endl;
    std::cout<< "    ConnectivityType: Default IP" << std::endl;
    std::cout << "   ConnectivityType : 0 - IP" << std::endl;
}

class ClientWorker
{
private:
    void putResourceInfo(const HeaderOptions& /*headerOptions*/,
            const OCRepresentation rep, const OCRepresentation /*rep2*/, const int eCode)
    {
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

            m_cv.notify_all();
       }
    }

    void getResourceInfo(const HeaderOptions& /*headerOptions*/, const OCRepresentation rep,
                const int eCode)
    {
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

            m_resource->put(rep2, QueryParamsMap(),
                PutCallback(std::bind(&ClientWorker::putResourceInfo, this, std::placeholders::_1,
                     rep2, std::placeholders::_2, std::placeholders::_3)));
        }
    }

    void foundResource(std::shared_ptr<OCResource> resource)
    {
        std::cout << "In foundResource" << std::endl;
        if(resource && resource->uri() == "/q/foo")
        {
            {
                std::lock_guard<std::mutex> lock(m_resourceLock);
                if(m_resource)
                {
                    return;
                }

                m_resource = resource;
            }

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

            std::cout<<"Doing a get on q/foo."<<std::endl;

            resource->get(QueryParamsMap(),
                GetCallback(std::bind(&ClientWorker::getResourceInfo, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
        }
    }

public:
    ClientWorker(OCConnectivityType ct):m_connectivityType{ct}
    {}

    void start()
    {
        std::ostringstream requestURI;
        requestURI << OC_RSRVD_WELL_KNOWN_URI << "?rt=core.foo";

        std::cout<<"Starting Client find:"<<std::endl;
        FindCallback f (std::bind(&ClientWorker::foundResource, this, std::placeholders::_1));
        std::cout<<"result:" <<
        OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, f)
        << std::endl;

        std::cout<<"Finding Resource..."<<std::endl;

        {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_cv.wait(lk);
        }
    }
private:
    std::mutex m_mutex;
    std::mutex m_resourceLock;
    std::condition_variable m_cv;
    std::shared_ptr<OCResource> m_resource;
    OCConnectivityType m_connectivityType;
    bool m_isFoo;
    int m_barCount;
};

struct FooResource
{
    bool m_isFoo;
    int m_barCount;
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_rep;

    FooResource(): m_isFoo(true), m_barCount (0)
    {
        m_rep.setUri("/q/foo");
        m_rep.setValue("isFoo", m_isFoo);
        m_rep.setValue("barCount", m_barCount);
    }

    bool createResource()
    {
        std::string resourceURI = "/q/foo";
        std::string resourceTypeName = "core.foo";
        std::string resourceInterface = DEFAULT_INTERFACE;

        uint8_t resourceProperty = OC_DISCOVERABLE;

        EntityHandler eh(std::bind(&FooResource::entityHandler,
                    this, std::placeholders::_1));
        OCStackResult result = OCPlatform::registerResource(m_resourceHandle,
                resourceURI, resourceTypeName,
                                    resourceInterface,
                                    eh, resourceProperty);
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
            // Note: Most of the handlers are not here, since this is for
            // demoing client/server co-process existence.
            // See simpleserver for a more complete example.
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
                    std::cout<<"\t\t\trequestType : UNSUPPORTED: "<<
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

int main(int argc, char* argv[])
{
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
                    std::cout << "Invalid connectivity type selected. Using default IP" << std::endl;
                }
            }
            else
            {
                std::cout << "Invalid connectivity type selected. Using default IP" << std::endl;
            }
        }
        catch(std::exception& )
        {
            std::cout << "Invalid input argument. Using IP as connectivity type" << std::endl;
        }
    }
    else
    {
        printUsage();
    }

    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);
    FooResource fooRes;

    try
    {

        if(!fooRes.createResource())
        {
            return -1;
        }

        ClientWorker cw(connectivityType);
        cw.start();
    }
    catch(OCException& e)
    {
        std::cout<< "Exception in main: "<<e.what()<<std::endl;
    }

    return 0;
}


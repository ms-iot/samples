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

/// This fridgeclient represents a client trying to discover the associated
/// fridgeserver.  The device resource is the only one available for discovery
/// on the server, so we have to take the fact that we know the device tag
/// to then generate a Resource object

#include <iostream>
#include <stdexcept>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
namespace PH = std::placeholders;

// Option ID for API version and client token
const uint16_t API_VERSION = 2048;
const uint16_t TOKEN = 3000;

static void printUsage()
{
    std::cout << "Usage: fridgeclient <0|1>" <<std::endl;
    std::cout << "connectivityType: Default IP" << std::endl;
    std::cout << "connectivityType 0: IP" << std::endl;
}
class ClientFridge
{
    public:
    ClientFridge(OCConnectivityType ct): m_callbackCount(0),
        m_callsMade(0),m_connectivityType(ct)
    {
        std::ostringstream requestURI;
        requestURI << OC_RSRVD_WELL_KNOWN_URI << "?rt=intel.fridge";
        std::cout << "Fridge Client has started " <<std::endl;
        FindCallback f (std::bind(&ClientFridge::foundDevice, this, PH::_1));
        OCStackResult result = OCPlatform::findResource(
                "", requestURI.str(), CT_DEFAULT, f);

        if(OC_STACK_OK != result)
        {
            throw new std::runtime_error("Fridge Find Resource Failed");
        }

        std::cout << "Waiting to discover fridge... "<<std::endl;
        {
            // we want to block this thread until the client has finished
            // its duties, so we block on the CV until we have completed
            // what we are looking to do
            std::unique_lock<std::mutex> lk(m_mutex);
            m_cv.wait(lk, [this]{ return m_callbackCount!=0 && m_callbackCount == m_callsMade;});
        }
    }

    private:
    void foundDevice(std::shared_ptr<OCResource> resource)
    {
        using namespace OC::OCPlatform;
        if(resource && resource->uri() == "/device")
        {
            std::cout << "Discovered a device object"<<std::endl;
            std::cout << "\tHost: "<<resource->host()<<std::endl;
            std::cout << "\tURI:  "<<resource->uri() <<std::endl;
        }

        // we have now found a resource, so lets create a few resource objects
        // for the other resources that we KNOW are associated with the intel.fridge
        // server, and query them.
        std::vector<std::string> lightTypes = {"intel.fridge.light"};
        std::vector<std::string> ifaces = {DEFAULT_INTERFACE};
        OCResource::Ptr light = constructResourceObject(resource->host(),
                                "/light", m_connectivityType, false, lightTypes, ifaces);

        if(!light)
        {
            std::cout << "Error: Light Resource Object construction returned null\n";
            return;
        }

        std::vector<std::string> doorTypes = {"intel.fridge.door"};
        OCResource::Ptr leftdoor = constructResourceObject(resource->host(),
                                "/door/left", m_connectivityType, false, doorTypes, ifaces);

        if(!leftdoor)
        {
            std::cout << "Error: Left Door Resource Object construction returned null\n";
            return;
        }

        OCResource::Ptr rightdoor = constructResourceObject(resource->host(),
                                "/door/right", m_connectivityType, false, doorTypes, ifaces);

        if(!rightdoor)
        {
            std::cout << "Error: Right Door Resource Object construction returned null\n";
            return;
        }

        OCResource::Ptr randomdoor = constructResourceObject(resource->host(),
                                "/door/random", m_connectivityType, false, doorTypes, ifaces);
        if(!randomdoor)
        {
            std::cout << "Error: Random Door Resource Object construction returned null\n";
            return;
        }

        // Set header options with API version and token
        HeaderOptions headerOptions;
        try
        {
            // Set API version and client token
            HeaderOption::OCHeaderOption apiVersion(API_VERSION, "v.1.0");
            HeaderOption::OCHeaderOption clientToken(TOKEN, "21ae43gf");
            headerOptions.push_back(apiVersion);
            headerOptions.push_back(clientToken);
        }
        catch(OCException& e)
        {
            std::cout << "Error creating HeaderOption: " << e.what() << std::endl;
        }


        // Setting header options will send above options in all requests
        // Header options are set per resource.
        // Below, header options are set only for device resource
        resource->setHeaderOptions(headerOptions);

        ++m_callsMade;
        resource->get(QueryParamsMap(), GetCallback(
                std::bind(&ClientFridge::getResponse, this, "Device", PH::_1,
                    PH::_2, PH::_3, resource, 0)
                ));
        ++m_callsMade;
        light->get(QueryParamsMap(), GetCallback(
                std::bind(&ClientFridge::getResponse, this, "Fridge Light", PH::_1,
                    PH::_2, PH::_3, light, 1)
                ));
        ++m_callsMade;
        leftdoor->get(QueryParamsMap(), GetCallback(
                std::bind(&ClientFridge::getResponse, this, "Left Door", PH::_1,
                    PH::_2, PH::_3, leftdoor, 2)
                ));
        ++m_callsMade;
        rightdoor->get(QueryParamsMap(), GetCallback(
                std::bind(&ClientFridge::getResponse, this, "Right Door", PH::_1,
                    PH::_2, PH::_3, rightdoor, 3)
                ));
        ++m_callsMade;
        randomdoor->get(QueryParamsMap(), GetCallback(
                std::bind(&ClientFridge::getResponse, this, "Random Door", PH::_1,
                    PH::_2, PH::_3, randomdoor, 4)
                ));
        ++m_callsMade;
        resource->deleteResource(DeleteCallback(
                std::bind(&ClientFridge::deleteResponse, this, "Device", PH::_1,
                    PH::_2, resource, 0)
                ));
    }

    // Note that resourceName, resource, and getId are all bound via the std::bind mechanism.
    // it is possible to attach ANY arbitrary data to do whatever you would like here.  It may,
    // however be a better fit to wrap each call in an object so a fuller context (and additional
    // requests) can be easily made inside of a simple context
    void getResponse(const std::string& resourceName, const HeaderOptions& headerOptions,
                const OCRepresentation& rep, const int eCode, OCResource::Ptr resource, int getId)
    {
        std::cout << "Got a response from get from the " << resourceName << std::endl;
        std::cout << "Get ID is "<<getId<<" and resource URI is " << resource->uri() << std::endl;
        std::cout << "Get eCode is "<< eCode << std::endl;

        printHeaderOptions(headerOptions);

        std::cout << "The Attribute Data is: "<<std::endl;

        switch(getId)
        {
            case 0:
                {
                    // Get on device
                    std::string name;
                    rep.getValue("device_name", name);
                    std::cout << "Name of device: "<< name << std::endl;
                    break;
                }
            case 1:
                {
                    bool isOn = false;
                    rep.getValue("on",isOn);
                    std::cout<<"The fridge light is "<< ((isOn)?"":"not ") <<"on"<<std::endl;
                }
                break;
            case 2:
            case 3:
                {
                    bool isOpen = false;
                    std::string side;
                    rep.getValue("open", isOpen);
                    rep.getValue("side", side);
                    std::cout << "Door is "<<isOpen<<" and is on the "<<side<<std::endl;
                }
                break;
            case 4:
                {
                    // Get on random resource called.
                    std::string name;
                    rep.getValue("device_name", name);
                    std::cout << "Name of fridge: "<< name << std::endl;
                    break;
                }
        }
        ++m_callbackCount;

        if(m_callbackCount == m_callsMade)
        {
            m_cv.notify_all();
        }
    }

    //Callback function to handle response for deleteResource call.
    void deleteResponse(const std::string& resourceName, const HeaderOptions& headerOptions,
                const int /*eCode*/, OCResource::Ptr resource, int deleteId)
    {
        std::cout << "Got a response from delete from the "<< resourceName << std::endl;
        std::cout << "Delete ID is "<<deleteId<<" and resource URI is "<<resource->uri()<<std::endl;
        printHeaderOptions(headerOptions);

        ++m_callbackCount;

        if(m_callbackCount == m_callsMade)
        {
            m_cv.notify_all();
        }
    }

    //Function to print the headerOptions received from the server
    void printHeaderOptions(const HeaderOptions& headerOptions)
    {
        for (auto it = headerOptions.begin(); it != headerOptions.end(); ++it)
        {
            if(it->getOptionID() == API_VERSION)
            {
                std::cout << "Server API version in GET response: " <<
                        it->getOptionData() << std::endl;
            }
        }
    }

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<int> m_callbackCount;
    std::atomic<int> m_callsMade;
    OCConnectivityType m_connectivityType;
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
                    printUsage();
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
        printUsage();
        std::cout << "Default input argument. Using IP as connectivity type" << std::endl;
    }

    PlatformConfig cfg
    {
        ServiceType::InProc,
        ModeType::Client,
        "0.0.0.0",
        0,
        QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);
    ClientFridge cf(connectivityType);
    return 0;
}


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
///This sample demonstrates the device discovery feature
///The client queries for the device related information
///stored by the server.
///

#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

static void printUsage()
{
    std::cout << "Usage devicediscoveryclient <0|1>" << std::endl;
    std::cout << "connectivityType: Default IP" << std::endl;
    std::cout << "connectivityType 0: IP" << std::endl;
}
//Callback after device information is received
void receivedPlatformInfo(const OCRepresentation& rep)
{
    std::cout << "\nPlatform Information received ---->\n";
    std::string value;
    std::string values[] =
    {
        "pi",   "Platform ID                    ",
        "mnmn", "Manufacturer name              ",
        "mnml", "Manufacturer url               ",
        "mnmo", "Manufacturer Model No          ",
        "mndt", "Manufactured Date              ",
        "mnpv", "Manufacturer Platform Version  ",
        "mnos", "Manufacturer OS version        ",
        "mnhw", "Manufacturer hardware version  ",
        "mnfv", "Manufacturer firmware version  ",
        "mnsl", "Manufacturer support url       ",
        "st",   "Manufacturer system time       "
    };

    for (unsigned int i = 0; i < sizeof(values) / sizeof(values[0]) ; i += 2)
    {
        if(rep.getValue(values[i], value))
        {
            std::cout << values[i + 1] << " : "<< value << std::endl;
        }
    }
}

void receivedDeviceInfo(const OCRepresentation& rep)
{
    std::cout << "\nDevice Information received ---->\n";
    std::string value;
    std::string values[] =
    {
        "di",   "Device ID        ",
        "n",    "Device name      ",
        "lcv",  "Spec version url ",
        "dmv",  "Data Model Model ",
    };

    for (unsigned int i = 0; i < sizeof(values) / sizeof(values[0]) ; i += 2)
    {
        if(rep.getValue(values[i], value))
        {
            std::cout << values[i + 1] << " : "<< value << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {

    std::ostringstream platformDiscoveryRequest;
    std::ostringstream deviceDiscoveryRequest;

    std::string platformDiscoveryURI = "/oic/p";
    std::string deviceDiscoveryURI   = "/oic/d";

    //Default Connectivity type
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
                    std::cout << "Invalid connectivity type selected." << std::endl;
                    printUsage();
                    return -1;
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
        platformDiscoveryRequest << OC_MULTICAST_PREFIX << platformDiscoveryURI;
        deviceDiscoveryRequest << OC_MULTICAST_PREFIX << deviceDiscoveryURI;

        OCStackResult ret;

        std::cout<< "Querying for platform information... ";

        ret = OCPlatform::getPlatformInfo("", platformDiscoveryRequest.str(), connectivityType,
                &receivedPlatformInfo);

        if (ret == OC_STACK_OK)
        {
            std::cout << "done." << std::endl;
        }
        else
        {
            std::cout << "failed." << std::endl;
        }

        std::cout<< "Querying for device information... ";

        ret = OCPlatform::getDeviceInfo("", deviceDiscoveryRequest.str(), connectivityType,
                &receivedDeviceInfo);

        if (ret == OC_STACK_OK)
        {
            std::cout << "done." << std::endl;
        }
        else
        {
            std::cout << "failed." << std::endl;
        }

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock, []{return false;});

    }catch(OCException& e)
    {
        std::cerr << "Failure in main thread: "<<e.reason()<<std::endl;
    }

    return 0;
}



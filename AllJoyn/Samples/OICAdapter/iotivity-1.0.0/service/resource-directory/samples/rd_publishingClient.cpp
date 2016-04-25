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
#include <iostream>

#include "OCPlatform.h"
#include "OCApi.h"
#include "oic_string.h"

#include "rd_client.h"

using namespace OC;

OCResourceHandle g_curResource_t = NULL;
OCResourceHandle g_curResource_l = NULL;
char rdAddress[MAX_ADDR_STR_SIZE];
uint16_t rdPort;

void registerLocalResources()
{
    std::string resourceURI_thermostat = "/a/thermostat";
    std::string resourceTypeName_thermostat = "core.thermostat";
    std::string resourceURI_light = "/a/light";
    std::string resourceTypeName_light = "core.light";
    std::string resourceInterface = DEFAULT_INTERFACE;
    uint8_t resourceProperty = OC_DISCOVERABLE;

    OCStackResult result = OCPlatform::registerResource(g_curResource_t,
                           resourceURI_thermostat,
                           resourceTypeName_thermostat,
                           resourceInterface,
                           NULL,
                           resourceProperty);

    if (OC_STACK_OK != result)
    {
        throw std::runtime_error(
            std::string("Device Resource failed to start") + std::to_string(result));
    }

    result = OCPlatform::registerResource(g_curResource_l,
                                          resourceURI_light,
                                          resourceTypeName_light,
                                          resourceInterface,
                                          NULL,
                                          resourceProperty);

    if (OC_STACK_OK != result)
    {
        throw std::runtime_error(
            std::string("Device Resource failed to start") + std::to_string(result));
    }
}

void printHelp()
{
    std::cout << std::endl;
    std::cout << "********************************************" << std::endl;
    std::cout << "*  method Type : 1 - Discover RD           *" << std::endl;
    std::cout << "*  method Type : 2 - Publish               *" << std::endl;
    std::cout << "*  method Type : 3 - Update                *" << std::endl;
    std::cout << "*  method Type : 4 - Delete                *" << std::endl;
    std::cout << "*  method Type : 5 - Status                *" << std::endl;
    std::cout << "********************************************" << std::endl;
    std::cout << std::endl;
}

int biasFactorCB(char addr[MAX_ADDR_STR_SIZE], uint16_t port)
{
    OICStrcpy(rdAddress, MAX_ADDR_STR_SIZE, addr);
    rdPort = port;
    std::cout << "RD Address is : " <<  addr << ":" << port << std::endl;
    return 0;
}

int main()
{
    int in;
    PlatformConfig cfg;

    OCPlatform::Configure(cfg);

    std::cout << "Created Platform..." << std::endl;

    registerLocalResources();

    while (1)
    {
        sleep(2);

        if (g_curResource_t == NULL || g_curResource_l == NULL)
        {
            continue;
        }
        printHelp();

        in = 0;
        std::cin >> in;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input type, please try again" << std::endl;
            continue;
        }

        try
        {
            switch ((int)in)
            {
                case 1:
                    OCRDDiscover(biasFactorCB);
                    break;
                case 2:
                    OCRDPublish(rdAddress, rdPort, 2, g_curResource_t, g_curResource_l);
                    break;
                case 3:
                    break;
                default:
                    std::cout << "Invalid input, please try again" << std::endl;
                    break;
            }
        }
        catch (OCException e)
        {
            std::cout << "Caught OCException [Code: " << e.code() << " Reason: " << e.reason() << std::endl;
        }
    }
    return 0;
}

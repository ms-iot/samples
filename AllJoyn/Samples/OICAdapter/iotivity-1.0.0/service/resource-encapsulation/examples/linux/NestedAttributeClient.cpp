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

#include<iostream>
#include "mutex"
#include "condition_variable"

#include "RCSDiscoveryManager.h"
#include "RCSRemoteResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSAddress.h"

#include "OCPlatform.h"

#define nestedAtrribute std::vector<std::vector<RCSResourceAttributes>>

using namespace OC;
using namespace OIC::Service;

constexpr int CORRECT_INPUT = 1;
constexpr int INCORRECT_INPUT = 2;
constexpr int QUIT_INPUT = 3;

std::shared_ptr<RCSRemoteResourceObject>  resource;

const std::string defaultKey = "deviceInfo";
const std::string resourceType = "core.ac";
const std::string relativetUri = OC_RSRVD_WELL_KNOWN_URI;

RCSResourceAttributes model;
RCSResourceAttributes speed;
RCSResourceAttributes airCirculation;
RCSResourceAttributes temperature;
RCSResourceAttributes humidity;
RCSResourceAttributes power;
RCSResourceAttributes capacity;
RCSResourceAttributes weight;
RCSResourceAttributes dimensions;
RCSResourceAttributes red;
RCSResourceAttributes green;

std::vector<RCSResourceAttributes> generalInfo;
std::vector<RCSResourceAttributes> fan;
std::vector<RCSResourceAttributes> tempSensor;
std::vector<RCSResourceAttributes> efficiency;
std::vector<RCSResourceAttributes> light;


std::mutex mtx;
std::condition_variable cond;

void getAttributeFromRemoteServer();
void setAttributeToRemoteServer();

enum Menu
{
    GET_ATTRIBUTE = 1,
    SET_ATTRIBUTE,
    QUIT,
    END_OF_MENU
};

typedef void(*ClientMenuHandler)();
typedef int ReturnValue;

struct ClientMenu
{
    Menu m_menu;
    ClientMenuHandler m_handler;
    ReturnValue m_result;
};

ClientMenu clientMenu[] =
{
    {Menu::GET_ATTRIBUTE, getAttributeFromRemoteServer, CORRECT_INPUT},
    {Menu::SET_ATTRIBUTE, setAttributeToRemoteServer, CORRECT_INPUT},
    {Menu::QUIT, [](){}, QUIT_INPUT},
    {Menu::END_OF_MENU, nullptr, INCORRECT_INPUT}
};

void displayMenu()
{
    std::cout << std::endl;
    std::cout << "1 :: Get Attribute" << std::endl;
    std::cout << "2 :: Set Attribute" << std::endl;
}

void onResourceDiscovered(std::shared_ptr<RCSRemoteResourceObject> foundResource)
{
    std::cout << "onResourceDiscovered callback" << std::endl;

    std::string resourceURI = foundResource->getUri();
    std::string hostAddress = foundResource->getAddress();

    std::cout << "\t\tResource URI : " << resourceURI << std::endl;
    std::cout << "\t\tResource Host : " << hostAddress << std::endl;

    resource = foundResource;

    cond.notify_all();
}

void onRemoteAttributesReceivedCallback(const RCSResourceAttributes &attributes, int /*eCode*/)
{
    std::cout << "onRemoteAttributesReceivedCallback callback\n" << std::endl;

    if (attributes.empty())
    {
        std::cout << "\tAttribute is Empty" << std::endl;
        return;
    }

    for (const auto & attr : attributes)
    {
        std::cout << "\tkey : " << attr.key() << "\n\tvalue : "
                  << attr.value().toString() << std::endl;
        std::cout << "=============================================\n" << std::endl;

        OIC::Service::RCSResourceAttributes::Value attrValue =  attr.value();
        std::vector< std::vector<RCSResourceAttributes >> attrVector =
                    attrValue.get<std::vector< std::vector<RCSResourceAttributes >>>();

        for (auto itr = attrVector.begin(); itr != attrVector.end(); ++itr)
        {
            std::vector<RCSResourceAttributes > attrKeyVector = *itr;
            for (auto itrKey = attrKeyVector.begin(); itrKey != attrKeyVector.end(); ++itrKey)
            {
                for (const auto & attribute : *itrKey)
                {
                    std::cout << "\t" << attribute.key() << "  :  "  << attribute.value().toString() << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }
    std::cout << "=============================================\n" << std::endl;
    displayMenu();
}

nestedAtrribute createNestedAttribute(int speedValue, int aircValue)
{
    nestedAtrribute *acServer = new nestedAtrribute();

    model["model"] = "SamsungAC";

    speed["speed"] = speedValue;
    airCirculation["air"] = aircValue;

    temperature["temp"] = 30;
    humidity["humidity"] = 30;

    power["power"] = 1600;
    capacity["capacity"] = 1;

    weight["weight"] = 3;
    dimensions["dimensions"] = "10x25x35";

    red["red"] = 50;
    green["green"] = 60;

    generalInfo.clear();
    generalInfo.push_back(model);
    generalInfo.push_back(weight);
    generalInfo.push_back(dimensions);

    fan.clear();
    fan.push_back(speed);
    fan.push_back(airCirculation);

    tempSensor.clear();
    tempSensor.push_back(temperature);
    tempSensor.push_back(humidity);

    efficiency.clear();
    efficiency.push_back(power);
    efficiency.push_back(capacity);

    light.clear();
    light.push_back(red);
    light.push_back(green);

    if (nullptr == acServer)
    {
         std::cout << "Null nestedAtrribute" << std::endl;
    }
    else
    {
        acServer->push_back(generalInfo);
        acServer->push_back(fan);
        acServer->push_back(tempSensor);
        acServer->push_back(efficiency);
        acServer->push_back(light);
    }

    return *acServer;
}

int processUserInput()
{
    int userInput;
    std::cin >> userInput;
    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    return userInput;
}

void getAttributeFromRemoteServer()
{
    if(nullptr == resource)
        return;

    resource->getRemoteAttributes(&onRemoteAttributesReceivedCallback);
}

void setAttributeToRemoteServer()
{
    if(nullptr == resource)
        return;

    int speed, airc;

    std::cout << "\tEnter the Fan Speed you want to set : ";
    std::cin >> speed;
    std::cout << "\tEnter the Air circulation value you want to set :";
    std::cin >> airc;

    nestedAtrribute nestedAttr = createNestedAttribute(speed, airc);

    RCSResourceAttributes setAttribute;
    setAttribute[defaultKey] = nestedAttr;

    resource->setRemoteAttributes(setAttribute,
                                  &onRemoteAttributesReceivedCallback);
}

int selectClientMenu(int selectedMenu)
{
    for (int i = 0; clientMenu[i].m_menu != Menu::END_OF_MENU; i++)
    {
        if (clientMenu[i].m_menu == selectedMenu)
        {
            clientMenu[i].m_handler();
            return clientMenu[i].m_result;
        }
    }

    std::cout << "Invalid input, please try again" << std::endl;

    return INCORRECT_INPUT;
}

void process()
{
    while (true)
    {
        displayMenu();

        if (selectClientMenu(processUserInput()) == QUIT_INPUT)
            break;
    }
}

void platFormConfigure()
{
    PlatformConfig config
    {
        OC::ServiceType::InProc, ModeType::Client, "0.0.0.0", 0, OC::QualityOfService::LowQos
    };
    OCPlatform::Configure(config);
}

bool discoverResource()
{
    std::cout << "Wait 2 seconds until discovered." << std::endl;

    try
    {
        RCSDiscoveryManager::getInstance()->discoverResourceByType(RCSAddress::multicast(),
                relativetUri, resourceType, &onResourceDiscovered);
    }
    catch(const RCSPlatformException& e)
    {
         std::cout << e.what() << std::endl;
    }
    std::unique_lock<std::mutex> lck(mtx);
    cond.wait_for(lck, std::chrono::seconds(2));

    return resource != nullptr;
}

int main()
{
    platFormConfigure();

    if (!discoverResource())
    {
        std::cout << "Can't discovered Server... Exiting the Client." << std::endl;
        return -1;
    }

    try
    {
        process();
    }
    catch (const std::exception &e)
    {
        std::cout << "main exception : " << e.what() << std::endl;
    }

    std::cout << "Stopping the Client" << std::endl;

    return 0;
}

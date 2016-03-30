/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "PrimitiveResource.h"
#include "RCSResourceObject.h"
#include "OCPlatform.h"
#include "OCApi.h"

#define nestedAtrribute std::vector<std::vector<RCSResourceAttributes>>

using namespace OC;
using namespace OC::OCPlatform;
using namespace OIC::Service;

constexpr int DEFAULT_SPEED = 30;
constexpr int UP_SPEED = 50;
constexpr int DOWN_SPEED = 10;

constexpr int DEFALUT_SERVER = 1;
constexpr int CUSTOM_SERVER = 2;
constexpr int STOP = 3;

constexpr int PRINT_ATTRIBUTES = 1;
constexpr int INCREASE_SPEEDATTRIBUTE = 2;
constexpr int DECREASE_SPEEDATTRIBUTE = 3;
constexpr int STOP_SENSOR = 4;

constexpr int CORRECT_INPUT = 1;
constexpr int INCORRECT_INPUT = 2;
constexpr int QUIT = 3;

std::string resourceUri = "/a/airConditioner";
std::string resourceType = "core.ac";
std::string resourceInterface = "oic.if.";
std::string attributeKey = "deviceInfo";

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

RCSResourceObject::Ptr server;

void displayMenu()
{
    std::cout << "====================================================================="
              << std::endl;
    std::cout << "   1 - Creation of Resource [Auto control for requests]" << std::endl;
    std::cout << "   2 - Creation of Resource [Developer control for Get and Set requests]"
              << std::endl;
    std::cout << "   3 - Quit" << std::endl;
    std::cout << "====================================================================="
              << std::endl;
}

void displayControlMenu()
{
    std::cout << "========================================================" << std::endl;
    std::cout << "1. Print Nested attributes" << std::endl;
    std::cout << "2. Increase Speed attributes" << std::endl;
    std::cout << "3. Decrease Speed attributes" << std::endl;
    std::cout << "4. Stop the Sensor" << std::endl;
    std::cout << "========================================================" << std::endl;
}

nestedAtrribute createNestedAttribute(int speedValue)
{
    nestedAtrribute *acServer = new nestedAtrribute();

    model["model"] = "SamsungAC";

    speed["speed"] = speedValue;
    airCirculation["air"] = 425;

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

void printAttribute(const RCSResourceAttributes &attrs)
{
    for (const auto & attr : attrs)
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
        std::cout << "=============================================\n" << std::endl;
    }
}

void printNestedAttribute()
{
    RCSResourceObject::LockGuard lock(*server);
    RCSResourceAttributes attributes = server->getAttributes();

    std::cout << "\nPrinting nested attributes" << std::endl;
    printAttribute(attributes);
    return;
}

void changeSpeedAttribute(int state)
{
    nestedAtrribute attr;

    if (INCREASE_SPEEDATTRIBUTE == state)
    {
        std::cout << "Increasing speed  attribute to : " << UP_SPEED  <<  std::endl;
        attr = createNestedAttribute(UP_SPEED);
    }
    else if (DECREASE_SPEEDATTRIBUTE == state)
    {
        std::cout << "Decreasing speed  attribute to : " << DOWN_SPEED << std::endl;
        attr = createNestedAttribute(DOWN_SPEED);
    }

    RCSResourceObject::LockGuard lock(*server);
    server->getAttributes()[attributeKey] = attr;
    printNestedAttribute();
}

//hander for get request (if developer choose second option for resource Creation)
RCSGetResponse requestHandlerForGet(const RCSRequest &request,
                                    RCSResourceAttributes &attrs)
{
    std::cout << "Recieved a Get request from Client" << std::endl;

    RCSResourceObject::LockGuard lock(*server);
    RCSResourceAttributes attributes = server->getAttributes();

    std::cout << "\nSending response to Client : " << std::endl;
    printAttribute(attributes);

    return RCSGetResponse::defaultAction();
}

//hander for set request (if developer choose second option for resource Creation)
RCSSetResponse requestHandlerForSet(const RCSRequest &request,
                                    RCSResourceAttributes &attrs)
{
    std::cout << "Recieved a Set request from Client" << std::endl;

    std::cout << "\n\nSending response to Client : " << std::endl;
    RCSResourceObject::LockGuard lock(*server);
    printAttribute(attrs);
    return RCSSetResponse::defaultAction();
}

void createResource()
{
    server = RCSResourceObject::Builder(resourceUri, resourceType,
                                        resourceInterface).setDiscoverable(true).setObservable(true).build();
}

void initServer()
{
    try
    {
        createResource();
    }
    catch (const RCSPlatformException &e)
    {
        std::cout << "Exception in initServer : " << e.what() << std::endl;
    }

    if (nullptr == server)
    {
         std::cout << "Null server resource" << std::endl;
         return;
    }

    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);
    server->setSetRequestHandlerPolicy(RCSResourceObject::SetRequestHandlerPolicy::NEVER);

    nestedAtrribute attr = createNestedAttribute(DEFAULT_SPEED);
    server->setAttribute(attributeKey, attr);
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

int selectControlMenu()
{
    switch (processUserInput())
    {
        case PRINT_ATTRIBUTES:
            printNestedAttribute();
            return CORRECT_INPUT;

        case INCREASE_SPEEDATTRIBUTE:
            changeSpeedAttribute(INCREASE_SPEEDATTRIBUTE);
            return CORRECT_INPUT;

        case DECREASE_SPEEDATTRIBUTE:
            changeSpeedAttribute(DECREASE_SPEEDATTRIBUTE);
            return CORRECT_INPUT;

        case STOP_SENSOR:
            return QUIT;

        default:
            std::cout << "Invalid input. Please try again." << std::endl;
            return INCORRECT_INPUT;
    }
}

int selectServerMenu()
{
    switch (processUserInput())
    {
        case DEFALUT_SERVER: // Creation of Resource & Auto control for all requests from Client.
            initServer();
            return CORRECT_INPUT;

        case CUSTOM_SERVER:
            // Creation of Resource & setting get and set handler for handling get and
            // set request from client in application.
            initServer();

            server->setGetRequestHandler(requestHandlerForGet);
            server->setSetRequestHandler(requestHandlerForSet);
            return CORRECT_INPUT;
        case STOP :
            return QUIT;

        default :
            std::cout << "Invalid input, please try again" << std::endl;
            return INCORRECT_INPUT;
    }
}

void process()
{
    while (true)
    {
        displayMenu();

        int ret = selectServerMenu();

        if (ret == QUIT) return;
        if (ret == CORRECT_INPUT) break;
    }

    while (true)
    {
        displayControlMenu();

        if (selectControlMenu() == QUIT) return;
    }
}

int main(void)
{
    startPresence(3);

    try
    {
        process();
        server = NULL;
    }
    catch (const std::exception &e)
    {
        std::cout << "main exception  : " << e.what() << std::endl;
    }

    std::cout << "Stopping the Server" << std::endl;
}

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

#include "RCSDiscoveryManager.h"
#include "RCSRemoteResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSAddress.h"

#include "OCPlatform.h"

using namespace OC;
using namespace OIC::Service;

#define DECLARE_MENU(FUNC, ...) { #FUNC, FUNC, __VA_ARGS__ }

void startMonitoring();
void startMonitoring();
void stopMonitoring();
void getAttributeFromRemoteServer();
void setAttributeToRemoteServer();
void startCachingWithoutCallback();
void startCachingWithCallback();
void getResourceCacheState();
void getCachedAttributes();
void getCachedAttribute();
void stopCaching();
void discoverResource();
void cancelDiscovery();
int processUserInput();
void selectResource();
void printAttributes(const RCSResourceAttributes& attributes);

class MenuList;

class MenuItem
{
private:
    typedef void(*MenuHandler)();

public:
    MenuItem(const char* funcName, MenuHandler handler = nullptr,
            MenuHandler optionHandler = nullptr, MenuList* state = nullptr)
        : m_name(funcName), m_handler(handler), m_optionHandler(optionHandler),
          m_nextState(state)
    {
    }

    MenuList* command() const
    {
        std::cout << m_name << " start.." << std::endl;
        if(m_handler) m_handler();

        if(m_optionHandler != nullptr) m_optionHandler();

        return m_nextState;
    }

    const char* getTitle() const
    {
        return m_name.c_str();
    }

private:
    const std::string m_name;
    const MenuHandler m_handler;
    const MenuHandler m_optionHandler;
    MenuList* const m_nextState ;
};

class MenuList
{
public:
    MenuList(std::initializer_list<MenuItem> il)
        :m_menuItemList(std::move(il))
    {
    }

    void display() const
    {
        std::cout << std::endl;
        int menuNum = 1;

        for(const auto& menuItem : m_menuItemList)
        {
            std::cout.width(2);
            std::cout << std::right << menuNum++ <<  ". ";
            std::cout << menuItem.getTitle() << std::endl;
        }
    }

    MenuList* select()
    {
        int input = processUserInput();

        if(input == static_cast<int>(m_menuItemList.size())) return nullptr;

        if(input > static_cast<int>(m_menuItemList.size()) || input <= 0)
        {
            std::cout << "Invalid input, please try again" << std::endl;
            return this;
        }
        auto nextMenuList = m_menuItemList[input-1].command();

        return nextMenuList == nullptr ? this : nextMenuList;
    }

private:
    const std::vector<MenuItem> m_menuItemList;
};

constexpr int REQUEST_TEMP = 1;
constexpr int REQUEST_LIGHT = 2;

const std::string TEMP = "oic.r.temperaturesensor";
const std::string LIGHT = "oic.r.light";

std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> discoveryTask;

std::vector<RCSRemoteResourceObject::Ptr> resourceList;
RCSRemoteResourceObject::Ptr selectedResource;

std::string defaultKey;

MenuList* currentMenuList;

MenuList resourceMenu = {
    DECLARE_MENU(startMonitoring),
    DECLARE_MENU(stopMonitoring),
    DECLARE_MENU(getAttributeFromRemoteServer),
    DECLARE_MENU(setAttributeToRemoteServer),
    DECLARE_MENU(startCachingWithoutCallback),
    DECLARE_MENU(startCachingWithCallback),
    DECLARE_MENU(getResourceCacheState),
    DECLARE_MENU(getCachedAttributes),
    DECLARE_MENU(getCachedAttribute),
    DECLARE_MENU(stopCaching),
    { "quit" }
};

MenuList cancelDiscoveryMenu = {
    DECLARE_MENU(cancelDiscovery, selectResource, &resourceMenu),
    { "quit" }
};

MenuList discoveryMenu = {
    DECLARE_MENU(discoverResource, nullptr, &cancelDiscoveryMenu),
    { "quit" }
};

void onResourceDiscovered(std::shared_ptr<RCSRemoteResourceObject> discoveredResource)
{
    std::cout << "onResourceDiscovered callback :: " << std::endl;

    std::cout << "resourceURI : " << discoveredResource->getUri() << std::endl;
    std::cout << "hostAddress : " << discoveredResource->getAddress() << std::endl;

    resourceList.push_back(discoveredResource);
}

void onResourceStateChanged(const ResourceState& resourceState)
{
    std::cout << "onResourceStateChanged callback" << std::endl;

    switch(resourceState)
    {
        case ResourceState::NONE:
            std::cout << "\tState changed to : NOT_MONITORING" << std::endl;
            break;

        case ResourceState::ALIVE:
            std::cout << "\tState changed to : ALIVE" << std::endl;
            break;

        case ResourceState::REQUESTED:
            std::cout << "\tState changed to : REQUESTED" << std::endl;
            break;

        case ResourceState::LOST_SIGNAL:
            std::cout << "\tState changed to : LOST_SIGNAL" << std::endl;
            break;

        case ResourceState::DESTROYED:
            std::cout << "\tState changed to : DESTROYED" << std::endl;
            break;
    }
}

void onCacheUpdated(const RCSResourceAttributes& attributes)
{
    std::cout << "onCacheUpdated callback" << std::endl;

    printAttributes(attributes);
}

void onRemoteAttributesReceived(const RCSResourceAttributes& attributes, int)
{
    std::cout << "onRemoteAttributesReceived callback" << std::endl;

    printAttributes(attributes);
}

void startMonitoring()
{
    if (!selectedResource->isMonitoring())
    {
        selectedResource->startMonitoring(&onResourceStateChanged);
        std::cout << "\tHosting Started..." << std::endl;
    }
    else
    {
        std::cout << "\tAlready Started..." << std::endl;
    }
}

void stopMonitoring()
{
    if (selectedResource->isMonitoring())
    {
        selectedResource->stopMonitoring();
        std::cout << "\tHosting stopped..." << std::endl;
    }
    else
    {
       std::cout << "\tHosting not started..." << std::endl;
    }
}

void getAttributeFromRemoteServer()
{
    selectedResource->getRemoteAttributes(&onRemoteAttributesReceived);
}

void setAttributeToRemoteServer()
{
    std::string key;
    RCSResourceAttributes setAttribute;

    std::cout << "\tEnter the Key you want to set : ";
    std::cin >> key;
    std::cout << "\tEnter the value(INT) you want to set :";

    setAttribute[key] = processUserInput();
    selectedResource->setRemoteAttributes(setAttribute,
                                  &onRemoteAttributesReceived);
}

void startCaching(RCSRemoteResourceObject::CacheUpdatedCallback cb)
{
    if (!selectedResource->isCaching())
    {
        selectedResource->startCaching(std::move(cb));
        std::cout << "\tCaching Started..." << std::endl;
    }
    else
    {
        std::cout << "\tAlready Started Caching..." << std::endl;
    }
}

void startCachingWithoutCallback()
{
    startCaching(nullptr);
}

void startCachingWithCallback()
{
    startCaching(onCacheUpdated);
}

void getResourceCacheState()
{
    switch(selectedResource->getCacheState())
    {
        case CacheState::READY:
            std::cout << "\tCurrent Cache State : CACHE_STATE::READY" << std::endl;
            break;

        case CacheState::UNREADY:
            std::cout << "\tCurrent Cache State : CACHE_STATE::UNREADY" << std::endl;
            break;

        case CacheState::LOST_SIGNAL:
            std::cout << "\tCurrent Cache State : CACHE_STATE::LOST_SIGNAL" << std::endl;
            break;

        case CacheState::NONE:
            std::cout << "\tCurrent Cache State : CACHE_STATE::NONE" << std::endl;
            break;
    }
}

void getCachedAttributes()
{
    try
    {
        printAttributes(selectedResource->getCachedAttributes());
    }
    catch (const RCSBadRequestException& e)
    {
        std::cout << "Exception in getCachedAttributes : " << e.what() << std::endl;
    }
}

void getCachedAttribute()
{
    try
    {
        std::cout << "\tkey : " << defaultKey << std::endl
                  << "\tvalue : " << selectedResource->getCachedAttribute(defaultKey).get< int >()
                  << std::endl;
    }
    catch (const RCSBadRequestException& e)
    {
        std::cout << "Exception in getCachedAttribute : " << e.what() << std::endl;
    }
    catch (const RCSBadGetException& e)
    {
        std::cout << "Exception in getCachedAttribute : " << e.what() << std::endl;
    }
}

void stopCaching()
{
    if(selectedResource->isCaching())
    {
        selectedResource->stopCaching();
        std::cout << "\tCaching stopped..." << std::endl;
    }
    else
    {
        std::cout << "\tCaching not started..." << std::endl;
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

int processUserInput()
{
    int userInput;

    while(true)
    {
        std::cin >> userInput;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid Input, please try again" << std::endl;
        }
        else break;
    }
    return userInput;
}

bool selectResourceType(std::string& resourceType)
{
    std::cout << "========================================================" << std::endl;
    std::cout << "1. Temperature Resource Discovery" << std::endl;
    std::cout << "2. Light Resource Discovery" << std::endl;
    std::cout << "========================================================" << std::endl;

    switch (processUserInput())
    {
    case REQUEST_TEMP:
        resourceType = TEMP;
        defaultKey = "Temperature";
        return true;
    case REQUEST_LIGHT:
        resourceType = LIGHT;
        defaultKey = "Light";
        return true;
    default :
        std::cout << "Invalid input, please try again" << std::endl;
        return false;
    }
}

RCSAddress inputAddress()
{
    std::cout << "========================================================" << std::endl;
    std::cout << "Please input address" << std::endl;
    std::cout << "(default is multicast. when you want to use unicast, input address" << std::endl;
    std::cout << "========================================================" << std::endl;

    std::string address;

    if(std::cin.peek() != '\n') std::cin >> address;

    return address.empty() ? RCSAddress::multicast() : RCSAddress::unicast(address);
}

void discoverResource()
{
    std::string resourceType = "";

    while(!selectResourceType(resourceType)) {}

    while(!discoveryTask)
    {
        try
        {
            discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
                    inputAddress(), resourceType, &onResourceDiscovered);
        }
        catch (const RCSPlatformException& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
}

void displayResourceList()
{
    int cnt = 1;
    for(const auto& resource : resourceList)
    {
        std::cout << cnt++ << ".\tresourceURI : " << resource->getUri() << std::endl <<
                              "\thostAddress : " << resource->getAddress() << std::endl;
    }
}

void selectResource()
{
    displayResourceList();

    std::cout << "select Resource..." << std::endl;

    selectedResource = resourceList[processUserInput()-1];

    resourceList.clear();
}

void cancelDiscovery()
{
    if(!discoveryTask)
    {
        std::cout << "There is no discovery request..." << std::endl;
        return;
    }
    discoveryTask->cancel();
}

void printAttribute(const std::string key, const RCSResourceAttributes::Value& value)
{
    std::cout << "\tkey : " << key << std::endl
              << "\tvalue : " << value.toString() << std::endl;
}

void printAttributes(const RCSResourceAttributes& attributes)
{
    if (attributes.empty())
    {
       std::cout << "\tattributes is empty" << std::endl;
    }
    for(const auto& attr : attributes)
    {
        printAttribute(attr.key(), attr.value());
    }
}

int main()
{
    platFormConfigure();

    currentMenuList = &discoveryMenu;

    try
    {
        while(currentMenuList)
        {
            currentMenuList->display();

            currentMenuList = currentMenuList->select();
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "main exception : " << e.what() << std::endl;
    }

    std::cout << "Stopping the Client" << std::endl;

    return 0;
}

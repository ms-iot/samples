//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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
#include <cstdlib>
#include <pthread.h>
#include <map>
#include <vector>
#include "OCPlatform.h"
#include "OCApi.h"
#include "ThingsConfiguration.h"
#include "ThingsMaintenance.h"
#include "GroupManager.h"

using namespace OC;
using namespace OIC;

int g_Steps = 0;
int isWaiting = 0; //0: none to wait, 1: wait for the response of "getConfigurationValue"
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

const int SUCCESS_RESPONSE = 0;

static GroupManager* g_groupmanager;
static ThingsConfiguration* g_thingsConf;
static ThingsMaintenance* g_thingsMnt;

OCResourceHandle configurationCollectionHandle;
std::shared_ptr< OCResource > g_configurationCollection; // for a group of multiple resources
std::shared_ptr< OCResource > g_configurationResource; // For a single resource

OCResourceHandle maintenanceCollectionHandle;
std::shared_ptr< OCResource > g_maintenanceCollection; // for a group of multiple resources
std::shared_ptr< OCResource > g_maintenanceResource; // For a single resource

OCResourceHandle setCollectionHandle;
std::shared_ptr< OCResource > g_setCollection; // for a group of multiple resources
std::shared_ptr< OCResource > g_setResource; // For a single resource

std::map< std::string, std::shared_ptr< OCResource > > resourceTable;
std::vector< OCResourceHandle > resourceHandleVector;

typedef std::string ConfigurationName;
typedef std::string ConfigurationValue;

void timeCheck(int timeSec)
{
    sleep(timeSec);
    pthread_mutex_lock(&mutex_lock);
    isWaiting = 0;
    pthread_mutex_unlock(&mutex_lock);
}

void onReboot(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    pthread_mutex_lock(&mutex_lock);
    isWaiting = 0;
    pthread_mutex_unlock(&mutex_lock);

    if (eCode != SUCCESS_RESPONSE)
    {
        return ;
    }

    std::cout << "\tResource URI: " << rep.getUri() << std::endl;
    std::cout << "\t\tReboot:" << rep.getValue< std::string >("value") << std::endl;
}

void onFactoryReset(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    pthread_mutex_lock(&mutex_lock);
    isWaiting = 0;
    pthread_mutex_unlock(&mutex_lock);

    if (eCode != SUCCESS_RESPONSE)
    {
       return ;
    }

    std::cout << "\tResource URI: " << rep.getUri() << std::endl;
    std::cout << "\t\tFactoryReset:" << rep.getValue< std::string >("value") << std::endl;
}

void onUpdate(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    pthread_mutex_lock(&mutex_lock);
    isWaiting = 0;
    pthread_mutex_unlock(&mutex_lock);

    if (eCode != SUCCESS_RESPONSE)
    {
        return ;
    }

    std::cout << "\tResource URI: " << rep.getUri() << std::endl;

    if (rep.hasAttribute("n"))
        std::cout << "\t\tDeviceName:" << rep.getValue< std::string >("n") << std::endl;
    if (rep.hasAttribute("loc"))
        std::cout << "\t\tLocation:" << rep.getValue< std::string >("loc") << std::endl;
    if (rep.hasAttribute("locn"))
        std::cout << "\t\tLocationName:" << rep.getValue< std::string >("locn") << std::endl;
    if (rep.hasAttribute("c"))
        std::cout << "\t\tCurrency:" << rep.getValue< std::string >("c") << std::endl;
    if (rep.hasAttribute("r"))
        std::cout << "\t\tRegion:" << rep.getValue< std::string >("r") << std::endl;
}

void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    pthread_mutex_lock(&mutex_lock);
    isWaiting = 0;
    pthread_mutex_unlock(&mutex_lock);

    if (eCode != SUCCESS_RESPONSE)
    {
        return ;
    }

    std::cout << "\tResource URI: " << rep.getUri() << std::endl;

    if (rep.hasAttribute("n"))
        std::cout << "\t\tDeviceName:" << rep.getValue< std::string >("n") << std::endl;
    if (rep.hasAttribute("loc"))
        std::cout << "\t\tLocation:" << rep.getValue< std::string >("loc") << std::endl;
    if (rep.hasAttribute("locn"))
        std::cout << "\t\tLocationName:" << rep.getValue< std::string >("locn") << std::endl;
    if (rep.hasAttribute("c"))
        std::cout << "\t\tCurrency:" << rep.getValue< std::string >("c") << std::endl;
    if (rep.hasAttribute("r"))
        std::cout << "\t\tRegion:" << rep.getValue< std::string >("r") << std::endl;
}

// Callback to found collection resource
void onFoundCollectionResource(std::vector< std::shared_ptr< OCResource > > resources)
{
    std::string resourceURI;
    std::string hostAddress;
    try
    {
        // Do some operations with resource object.
        for (unsigned int i = 0; i < resources.size(); ++i)
        {
            std::shared_ptr< OCResource > resource = resources.at(i);

            if (resource)
            {
                if (resource->uri() == "/core/a/configuration/resourceset")
                    g_configurationCollection = resource;
                else if (resource->uri() == "/core/a/maintenance/resourceset")
                    g_maintenanceCollection = resource;
                else if (resource->uri() == "/core/a/factoryset/resourceset")
                    g_setCollection = resource;
                else
                {
                    pthread_mutex_lock(&mutex_lock);
                    isWaiting = 0;
                    pthread_mutex_unlock(&mutex_lock);
                    return;
                }
            }
            else
            {
                // Resource is invalid
                std::cout << "Resource is invalid" << std::endl;
            }
        }

    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    pthread_mutex_lock(&mutex_lock);
    isWaiting = 0;
    pthread_mutex_unlock(&mutex_lock);
}

// Callback to found resources
void onFoundCandidateResource(std::vector< std::shared_ptr< OCResource > > resources)
{
    std::string resourceURI;
    std::string hostAddress;

    try
    {
        // Do some operations with resource object.
        for (unsigned int i = 0; i < resources.size(); ++i)
        {
            std::shared_ptr< OCResource > resource = resources.at(i);

            if (resource)
            {
                // Check if the resource is new one. If so, store it.

                std::map< std::string, std::shared_ptr< OCResource > >::iterator iter =
                        resourceTable.find(resource->host() + resource->uri());

                if (iter == resourceTable.end()) // new one
                {
                    resourceTable[resource->host() + resource->uri()] = resource;

                    OCResourceHandle foundResourceHandle;
                    OCStackResult result = OCPlatform::registerResource(foundResourceHandle,
                            resource);
                    std::cout << "\tResource ( " << resource->host() << " ) is registed!\t"
                            << std::endl;
                    if (result == OC_STACK_OK)
                    {
                        if (resource->uri() == "/oic/con")
                        {
                            OCPlatform::bindResource(configurationCollectionHandle,
                                    foundResourceHandle);
                            if (g_configurationResource == NULL)
                                g_configurationResource = resource;
                        }
                        else if (resource->uri() == "/oic/mnt")
                        {
                            OCPlatform::bindResource(maintenanceCollectionHandle,
                                    foundResourceHandle);
                            if (g_maintenanceResource == NULL)
                                g_maintenanceResource = resource;
                        }
                        else if (resource->uri() == "/factoryset")
                        {
                            OCPlatform::bindResource(setCollectionHandle, foundResourceHandle);
                            if (g_setResource == NULL)
                                g_setResource = resource;
                        }

                        resourceHandleVector.push_back(foundResourceHandle);
                    }
                    else
                    {
                        cout << "\tresource Error!" << endl;
                    }

                }

            }
            else
            {
                // Resource is invalid
                std::cout << "Resource is invalid" << std::endl;
            }
        }

    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    pthread_mutex_lock(&mutex_lock);
    isWaiting = 0;
    pthread_mutex_unlock(&mutex_lock);
}

int main()
{
    std::string str_steps;

    try
    {
        //**************************************************************
        // STEP 0
        PlatformConfig cfg
        { OC::ServiceType::InProc, OC::ModeType::Both, "0.0.0.0", 0, OC::QualityOfService::LowQos };

        OCPlatform::Configure(cfg);
        //g_thingsmanager = new ThingsManager();
        g_thingsConf = new ThingsConfiguration();
        g_thingsMnt = new ThingsMaintenance();
        g_groupmanager = new GroupManager();

        //**************************************************************

        while (true)
        {
            pthread_mutex_lock(&mutex_lock);
            if (isWaiting > 0)
            {
                pthread_mutex_unlock(&mutex_lock);
                continue;
            }

            isWaiting = 0;
            pthread_mutex_unlock(&mutex_lock);

            cout << endl << endl << "(0) Quit" << std::endl;
            cout << "(1) Find all resources(URI: /oic/con, /oic/mnt, /factoryset)" << std::endl;
            cout << "(2) Find all groups" << std::endl;
            cout << "(3) Get a Configuration resource" << std::endl;
            cout << "(4) Update a device name attribute value" << std::endl;
            cout << "(5) FactoryReset (for the group)" << std::endl;
            cout << "(6) Reboot (for the group)" << std::endl;
            cout << "(10) Show Configuration Units" << std::endl;

            try
            {
                std::getline (std::cin, str_steps);

                if(str_steps == "")
                {
                    continue;
                }
                else
                {
                    g_Steps = std::stoi(str_steps);
                }
            }
            catch(std::invalid_argument&)
            {
                std::cout << "Please put a digit, not string" << std::endl;
                continue;
            }

            if (g_Steps == 0)
            {
                break;
            }
            else if (g_Steps == 1)
            {
                std::vector< std::string > types;

                // For Registering a collection resource for configuration resources
                if (configurationCollectionHandle == NULL)
                {
                    string resourceURI = "/core/a/configuration/resourceset";
                    string resourceTypeName = "core.configuration.resourceset";
                    string resourceInterface = BATCH_INTERFACE;

                    OCPlatform::registerResource(configurationCollectionHandle, resourceURI,
                        resourceTypeName, resourceInterface, NULL,
                        //&entityHandler, // entityHandler
                        OC_DISCOVERABLE);

                    OCPlatform::bindInterfaceToResource(configurationCollectionHandle, GROUP_INTERFACE);
                    OCPlatform::bindInterfaceToResource(configurationCollectionHandle,
                        DEFAULT_INTERFACE);
                }

                // For Registering a collection resource for maintenance resources
                if (maintenanceCollectionHandle == NULL)
                {
                    string resourceURI = "/core/a/maintenance/resourceset";
                    string resourceTypeName = "core.maintenance.resourceset";
                    string resourceInterface = BATCH_INTERFACE;

                    OCPlatform::registerResource(maintenanceCollectionHandle, resourceURI,
                        resourceTypeName, resourceInterface, NULL,
                        //&entityHandler, // entityHandler
                        OC_DISCOVERABLE);

                    OCPlatform::bindInterfaceToResource(maintenanceCollectionHandle, GROUP_INTERFACE);
                    OCPlatform::bindInterfaceToResource(maintenanceCollectionHandle, DEFAULT_INTERFACE);
                }

                // For Registering a collection resource for set resources
                if (setCollectionHandle == NULL)
                {
                    string resourceURI = "/core/a/factoryset/resourceset";
                    string resourceTypeName = "core.factoryset.resourceset";
                    string resourceInterface = BATCH_INTERFACE;

                    OCPlatform::registerResource(setCollectionHandle, resourceURI, resourceTypeName,
                        resourceInterface, NULL,
                        //&entityHandler, // entityHandler
                        OC_DISCOVERABLE);

                    OCPlatform::bindInterfaceToResource(setCollectionHandle, GROUP_INTERFACE);
                    OCPlatform::bindInterfaceToResource(setCollectionHandle, DEFAULT_INTERFACE);
                }

                types.push_back("oic.wk.con");
                types.push_back("oic.wk.mnt");
                types.push_back("factoryset");

                std::cout << "Finding Configuration Resource... " << std::endl;
                std::cout << "Finding Maintenance Resource... " << std::endl;
                std::cout << "Finding Set Resource... " << std::endl;

                g_groupmanager->findCandidateResources(types, &onFoundCandidateResource, 5);

                pthread_mutex_lock(&mutex_lock);
                isWaiting = 1;
                pthread_mutex_unlock(&mutex_lock);

                thread t(&timeCheck, 5);
                t.join();       // After 5 seconds, isWaiting value will be 0.
            }
            else if (g_Steps == 2) // make a group with found things
            {
                std::vector< std::string > types;
                types.push_back("core.configuration.resourceset");
                types.push_back("core.maintenance.resourceset");
                types.push_back("core.factoryset.resourceset");

                g_groupmanager->findCandidateResources(types, &onFoundCollectionResource, 5);

                std::cout << "Finding Collection resource... " << std::endl;

                pthread_mutex_lock(&mutex_lock);
                isWaiting = 1;
                pthread_mutex_unlock(&mutex_lock);

                thread t(&timeCheck, 5);
                t.join();       // After 5 seconds, isWaiting value will be 0.
            }
            else if (g_Steps == 3)
            {
                // get a value

                ConfigurationName name = "all";

                std::cout << "For example, get configuration resources's value" << std::endl;

                std::vector< ConfigurationName > configurations;

                configurations.push_back(name);

                if (g_thingsConf->getConfigurations(g_configurationCollection, configurations, &onGet)
                        != OC_STACK_ERROR)
                {
                    pthread_mutex_lock(&mutex_lock);
                    isWaiting = 0;
                    pthread_mutex_unlock(&mutex_lock);
                }
            }
            else if (g_Steps == 4)
            {
                ConfigurationName name = "n";
                ConfigurationValue value = "OIC Device";

                if(g_configurationCollection == NULL)
                {
                    std::cout<<"Note that you first create a group to use this command." << std::endl;
                    continue;
                }

                std::cout << "For example, change a device name" << std::endl;
                std::cout << g_configurationCollection->uri() << std::endl;

                std::map< ConfigurationName, ConfigurationValue > configurations;

                configurations.insert(std::make_pair(name, value));

                if (g_thingsConf->updateConfigurations(g_configurationCollection, configurations,
                        &onUpdate) != OC_STACK_ERROR)
                {
                    pthread_mutex_lock(&mutex_lock);
                    isWaiting = 0;
                    pthread_mutex_unlock(&mutex_lock);
                }
            }
            else if (g_Steps == 5)
            {
                // factory reset
                if(g_maintenanceCollection == NULL)
                {
                    std::cout<<"Note that you first create a group to use this command." << std::endl;
                    continue;
                }

                if (g_thingsMnt->factoryReset(g_maintenanceCollection, &onFactoryReset)
                        != OC_STACK_ERROR)
                {
                    pthread_mutex_lock(&mutex_lock);
                    isWaiting = 0;
                    pthread_mutex_unlock(&mutex_lock);
                }
            }
            else if (g_Steps == 6)
            {
                // reboot
                if(g_maintenanceCollection == NULL)
                {
                    std::cout<<"Note that you first create a group to use this command." << std::endl;
                    continue;
                }

                if (g_thingsMnt->reboot(g_maintenanceCollection, &onReboot) != OC_STACK_ERROR)
                {
                    pthread_mutex_lock(&mutex_lock);
                    isWaiting = 0;
                    pthread_mutex_unlock(&mutex_lock);
                }
            }
            else if (g_Steps == 10)
            {
                std::cout << g_thingsConf->getListOfSupportedConfigurationUnits() << std::endl;

            }
        }
    }catch (OCException e)
    {
        std::cout << "Exception in main: " << e.what();
    }

    return 0;
}


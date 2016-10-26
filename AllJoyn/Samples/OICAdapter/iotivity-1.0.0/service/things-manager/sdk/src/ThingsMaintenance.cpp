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

/**
 * @file
 *
 */

#include <OCApi.h>
#include <OCPlatform.h>
#include <cstdlib>

#include "ThingsMaintenance.h"

using namespace OC;

namespace OIC
{
    std::map< std::string, MaintenanceRequestEntry > maintenanceRequestTable;
    ThingsMaintenance* ThingsMaintenance::thingsMaintenanceInstance = NULL;

    MaintenanceRequestEntry::MaintenanceRequestEntry(std::string ID, MaintenanceCallback callback,
                std::shared_ptr< OCResource > resource, std::string updateVal)
    {
        m_ID = ID;
        m_callback = callback;
        m_resource = resource;
        m_updateVal = updateVal;
    }

    MaintenanceUnitInfo::MaintenanceUnitInfo(std::string name,
                                            std::string attribute,
                                            std::string uri)
    {
        m_name = name;
        m_attribute = attribute;
        m_uri = uri;
    }

    std::string MaintenanceUnitInfo::getJSON()
    {
        std::string res;

        res = "{\"name\":\"" + m_name + "\",\"attribute\":\"" + m_attribute + "\"}";

        return res;
    }

    ThingsMaintenance::ThingsMaintenance()
    {
        MaintenanceUnitInfo unit[] =
                {
                { "rb", "Reboot", "/oic/mnt"},
                { "ssc", "StartStatCollection", "/oic/mnt"},
                { "fr", "Factory Reset", "/oic/mnt" } };

        for (int i = 0; i < NUMDIAGUNIT; i++)
            MaintenanceUnitTable.push_back(unit[i]);
    }

    ThingsMaintenance::~ThingsMaintenance()
    {
    }

    void ThingsMaintenance::setGroupManager(GroupManager *groupmanager)
    {
        g_groupmanager = groupmanager;
    }

    ThingsMaintenance* ThingsMaintenance::getInstance()
    {
        if (thingsMaintenanceInstance == NULL)
        {
            thingsMaintenanceInstance = new ThingsMaintenance();
        }
        return thingsMaintenanceInstance;
    }

    void ThingsMaintenance::deleteInstance()
    {
        if (thingsMaintenanceInstance)
        {
            delete thingsMaintenanceInstance;
            thingsMaintenanceInstance = NULL;
        }
    }

    std::string ThingsMaintenance::getAttributeByMaintenanceName(MaintenanceName name)
    {
        for (auto it = MaintenanceUnitTable.begin(); MaintenanceUnitTable.end() != it; it++)
        {
            if ((*it).m_name == name)
                return (*it).m_attribute;
        }

        return "";
    }

    std::string ThingsMaintenance::getUriByMaintenanceName(MaintenanceName name)
    {
        for (auto it = MaintenanceUnitTable.begin(); MaintenanceUnitTable.end() != it; it++)
        {
            if ((*it).m_name == name)
                return (*it).m_uri;
        }

        return "";
    }

    std::string ThingsMaintenance::getUpdateVal(std::string mnt)
    {
        std::map< std::string, MaintenanceRequestEntry >::iterator it =
                maintenanceRequestTable.find(mnt);

        if (it == maintenanceRequestTable.end())
            return NULL;
        else
            return it->second.m_updateVal;

    }
    std::shared_ptr< OCResource > ThingsMaintenance::getResource(std::string mnt)
    {
        std::map< std::string, MaintenanceRequestEntry >::iterator it =
                maintenanceRequestTable.find(mnt);

        if (it == maintenanceRequestTable.end())
            return NULL;
        else
            return it->second.m_resource;
    }

    MaintenanceCallback ThingsMaintenance::getCallback(std::string mnt)
    {
        std::map< std::string, MaintenanceRequestEntry >::iterator it =
                maintenanceRequestTable.find(mnt);

        if (it == maintenanceRequestTable.end())
            return NULL;
        else
            return it->second.m_callback;
    }

    std::string ThingsMaintenance::getHostFromURI(std::string oldUri)
    {
        size_t f;
        std::string newUri;

        if ((f = oldUri.find("/factoryset/oic/")) != string::npos)
            newUri = oldUri.replace(f, oldUri.size(), "");
        else if ((f = oldUri.find("/oic/")) != string::npos)
            newUri = oldUri.replace(f, oldUri.size(), "");

        return newUri;
    }

    std::string ThingsMaintenance::getListOfSupportedMaintenanceUnits()
    {
        std::string res;

        res = "{\"Maintenance Units\":[";

        auto it = MaintenanceUnitTable.begin();
        while (1)
        {
            res = res + (*it).getJSON();
            it++;

            if (it == MaintenanceUnitTable.end())
                break;
            else
                res += ",";
        }

        res += "]}";

        return res;
    }

    void ThingsMaintenance::onGetChildInfoForUpdate(const HeaderOptions& headerOptions,
            const OCRepresentation& rep, const int eCode, std::string mnt)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onGet Response error: " << eCode << std::endl;
            getCallback(mnt)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "GET request was successful" << std::endl;

        std::cout << "\tResource URI: " << rep.getUri() << std::endl;

        std::vector < OCRepresentation > children = rep.getChildren();
        for (auto oit = children.begin(); oit != children.end(); ++oit)
        {
            std::cout << "\t\tChild Resource URI: " << oit->getUri() << std::endl;
        }

        // Get information by using maintenance name(mnt)
        std::shared_ptr < OCResource > resource = getResource(mnt);
        std::string actionstring = mnt;
        std::string uri = getUriByMaintenanceName(mnt);
        std::string attrKey = mnt;

        if (uri == "")
            return;

        if (resource)
        {
            // In this nest, we create a new action set of which name is the dignostics name.
            // Required information consists of a host address, URI, attribute key, and
            // attribute value.
            ActionSet *newActionSet = new ActionSet();
            newActionSet->actionsetName = mnt;

            for (auto oit = children.begin(); oit != children.end(); ++oit)
            {
                Action *newAction = new Action();

                // oit->getUri() includes a host address as well as URI.
                // We should split these to each other and only use the host address to create
                // a child resource's URI. Note that the collection resource and its child
                // resource are located in same host.
                newAction->target = getHostFromURI(oit->getUri()) + uri;

                Capability *newCapability = new Capability();
                newCapability->capability = attrKey;
                newCapability->status = getUpdateVal(mnt);

                newAction->listOfCapability.push_back(newCapability);
                newActionSet->listOfAction.push_back(newAction);
            }

            // Request to create a new action set by using the above actionSet
            g_groupmanager->addActionSet(resource, newActionSet,
                    std::function<
                            void(const HeaderOptions& headerOptions,
                                    const OCRepresentation& rep, const int eCode) >(
                            std::bind(&ThingsMaintenance::onCreateActionSet, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, mnt)));

            delete(newActionSet);

        }
    }

    void ThingsMaintenance::onCreateActionSet(const HeaderOptions& headerOptions,
            const OCRepresentation& rep, const int eCode, std::string mnt)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            getCallback(mnt)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "PUT request was successful" << std::endl;

        std::shared_ptr < OCResource > resource = getResource(mnt);
        if (resource)
        {
            // Now, it is time to execute the action set.
            g_groupmanager->executeActionSet(resource, mnt,
                    std::function<
                            void(const HeaderOptions& headerOptions,
                                    const OCRepresentation& rep, const int eCode) >(
                            std::bind(&ThingsMaintenance::onExecuteForGroupAction, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, mnt)));
        }
    }

    void ThingsMaintenance::onExecuteForGroupAction(const HeaderOptions& headerOptions,
            const OCRepresentation& rep, const int eCode, std::string mnt)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            getCallback(mnt)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "PUT request was successful" << std::endl;
        getCallback(mnt)(headerOptions, rep, eCode);

        // Delete the created actionset
        std::shared_ptr < OCResource > resource = getResource(mnt);
        if (resource)
        {
            g_groupmanager->deleteActionSet(resource, mnt,
                    std::function<
                            void(const HeaderOptions& headerOptions,
                                    const OCRepresentation& rep, const int eCode) >(
                            std::bind(&ThingsMaintenance::onDeleteGroupAction, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, mnt)));
        }
    }

    void ThingsMaintenance::onDeleteGroupAction(const HeaderOptions& /*headerOptions*/,
            const OCRepresentation& /*rep*/, const int eCode, std::string mnt)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "Delete actionset returned with error: " << eCode << mnt << std::endl;
            return;
        }

        std::cout << "Deleted the actionset created!" << mnt<< std::endl;
    }

    void ThingsMaintenance::onPut(const HeaderOptions& headerOptions, const OCRepresentation& rep,
            const int eCode, std::string mnt)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            getCallback(mnt)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "PUT request was successful" << std::endl;

        getCallback(mnt)(headerOptions, rep, eCode);

    }

    bool ThingsMaintenance::isSimpleResource(std::shared_ptr< OCResource > resource)
    {
        for (unsigned int i = 0; i < resource->getResourceTypes().size(); ++i)
        {
            if (resource->getResourceTypes().at(0).find(".resourceset", 0) != std::string::npos)
                return false;
        }

        return true;
    }

    OCStackResult ThingsMaintenance::reboot(std::shared_ptr< OCResource > resource,
            MaintenanceCallback callback)
    {
        if (!resource)
        {
            std::cout << "resource is NULL\n";
            return OC_STACK_ERROR;
        }

        std::string mnt = "rb";

        // Check the request queue if a previous request is still left. If so, remove it.
        std::map< std::string, MaintenanceRequestEntry >::iterator iter =
                maintenanceRequestTable.find(mnt);
        if (iter != maintenanceRequestTable.end())
            maintenanceRequestTable.erase(iter);

        // Create new request entry stored in the queue
        MaintenanceRequestEntry newCallback(mnt, callback, resource, "true");
        maintenanceRequestTable.insert(std::make_pair(mnt, newCallback));

        QueryParamsMap query;
        OCRepresentation rep;

        if (isSimpleResource(resource))
        {
            // This resource is a simple resource. Just send a PUT message
            OCRepresentation rep;
            rep.setValue < std::string > (mnt, "true");

            return resource->put(resource->getResourceTypes().at(0), DEFAULT_INTERFACE, rep, query,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsMaintenance::onPut, this, std::placeholders::_1,
                                    std::placeholders::_2, std::placeholders::_3, mnt)));
        }
        else
        {
            // This resource is a collection resource. It just acquires child resource's URI and
            // send GET massages to the child resources in turn.
            // First, request the child resources's URI.
            // TODO: Add a deletion of actionset
            return resource->get(resource->getResourceTypes().at(0), DEFAULT_INTERFACE, query,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsMaintenance::onGetChildInfoForUpdate, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, mnt)));
        }
    }

    OCStackResult ThingsMaintenance::factoryReset(std::shared_ptr< OCResource > resource,
            MaintenanceCallback callback)
    {
        if (!resource)
        {
            std::cout << "resource is NULL\n";
            return OC_STACK_ERROR;
        }

        std::string mnt = "fr";

        // Check the request queue if a previous request is still left. If so, remove it.
        std::map< std::string, MaintenanceRequestEntry >::iterator iter =
                maintenanceRequestTable.find(mnt);
        if (iter != maintenanceRequestTable.end())
            maintenanceRequestTable.erase(iter);

        // Create new request entry stored in the queue
        MaintenanceRequestEntry newCallback(mnt, callback, resource, "true");
        maintenanceRequestTable.insert(std::make_pair(mnt, newCallback));

        QueryParamsMap query;
        OCRepresentation rep;

        if (isSimpleResource(resource))
        {
            // This resource is a simple resource. Just send a PUT message
            OCRepresentation rep;
            rep.setValue < std::string > ("value", "true");

            return resource->put(resource->getResourceTypes().at(0), DEFAULT_INTERFACE, rep, query,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsMaintenance::onPut, this, std::placeholders::_1,
                                    std::placeholders::_2, std::placeholders::_3, mnt)));
        }
        else
        {
            // This resource is a collection resource. It just acquires child resource's URI and
            // send GET massages to the child resources in turn.
            // First, request the child resources's URI.
            // TODO: Add a deletion of actionset
            return resource->get(resource->getResourceTypes().at(0), DEFAULT_INTERFACE, query,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsMaintenance::onGetChildInfoForUpdate, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, mnt)));
        }
    }
}


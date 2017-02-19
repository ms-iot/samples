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
#include <algorithm>
#include "ThingsConfiguration.h"

using namespace OC;

namespace OIC
{
    int cnt = 0;

    std::map< std::string, ConfigurationRequestEntry > configurationRequestTable;
    ThingsConfiguration* ThingsConfiguration::thingsConfigurationInstance = NULL;
    ConfigurationCallback g_bootstrapCallback;

    ConfigurationRequestEntry::ConfigurationRequestEntry(std::string ID,
                                                        ConfigurationCallback callback,
                                                        std::shared_ptr< OCResource > resource,
                                                        std::string updateVal)
    {
        m_ID = ID;
        m_callback = callback;
        m_resource = resource;
        m_updateVal = updateVal;
    }

    ConfigurationUnitInfo::ConfigurationUnitInfo(std::string name,
                                                std::string attribute,
                                                std::string uri)
    {
        m_name = name;
        m_attribute = attribute;
        m_uri = uri;
    }

    std::string ConfigurationUnitInfo::getJSON()
    {
        std::string res;

        res = "{\"name\":\"" + m_name + "\",\"property\":\"" + m_attribute + "\"}";

        return res;
    }

    ThingsConfiguration::ThingsConfiguration(void)
    {
        ConfigurationUnitInfo unit[] =
        {
        { "all", "All attributes", "/oic/con" },
        { "n", "Device Name", "/oic/con"},
        { "loc", "Location", "/oic/con"},
        { "locn", "Location Name", "/oic/con"},
        { "r", "Region", "/oic/con" },
        { "c","Currency", "/oic/con" } };

        for (int i = 0; i < NUMCONFUNIT; i++)
            ConfigurationUnitTable.push_back(unit[i]);
    }

    ThingsConfiguration::~ThingsConfiguration(void){}

    void ThingsConfiguration::setGroupManager(GroupManager *groupmanager)
    {
        g_groupmanager = groupmanager;
    }

    ThingsConfiguration* ThingsConfiguration::getInstance()
    {
        if (thingsConfigurationInstance == NULL)
        {
            thingsConfigurationInstance = new ThingsConfiguration();
        }
        return thingsConfigurationInstance;
    }

    void ThingsConfiguration::deleteInstance()
    {
        if (thingsConfigurationInstance)
        {
            delete thingsConfigurationInstance;
            thingsConfigurationInstance = NULL;
        }
    }

    std::string ThingsConfiguration::getAttributeByConfigurationName(ConfigurationName name)
    {
        for (auto it = ConfigurationUnitTable.begin(); ConfigurationUnitTable.end() != it; it++)
        {
            if ((*it).m_name == name)
                return (*it).m_attribute;
        }

        return "";
    }

    std::string ThingsConfiguration::getUriByConfigurationName(ConfigurationName name)
    {
        for (auto it = ConfigurationUnitTable.begin(); ConfigurationUnitTable.end() != it; it++)
        {
            if ((*it).m_name == name)
                return (*it).m_uri;
        }

        return "";
    }

    std::string ThingsConfiguration::getUpdateVal(std::string conf)
    {
        std::map< std::string, ConfigurationRequestEntry >::iterator it =
                configurationRequestTable.find(conf);

        if (it == configurationRequestTable.end())
            return NULL;
        else
            return it->second.m_updateVal;

    }
    std::shared_ptr< OCResource > ThingsConfiguration::getResource(std::string conf)
    {
        std::map< std::string, ConfigurationRequestEntry >::iterator it =
                configurationRequestTable.find(conf);

        if (it == configurationRequestTable.end())
            return NULL;
        else
            return it->second.m_resource;
    }

    ConfigurationCallback ThingsConfiguration::getCallback(std::string conf)
    {
        std::map< std::string, ConfigurationRequestEntry >::iterator it =
                configurationRequestTable.find(conf);

        if (it == configurationRequestTable.end())
            return NULL;
        else
            return it->second.m_callback;
    }

    std::string ThingsConfiguration::getListOfSupportedConfigurationUnits()
    {
        std::string res;

        res = "{\"Configuration Units\":[";

        auto it = ConfigurationUnitTable.begin();
        while (1)
        {
            res = res + (*it).getJSON();
            it++;

            if (it == ConfigurationUnitTable.end())
                break;
            else
                res += ",";
        }

        res += "]}";

        return res;
    }

    std::string ThingsConfiguration::getHostFromURI(std::string oldUri)
    {
        size_t f;
        std::string newUri;

        if ((f = oldUri.find("/factoryset/oic/")) != string::npos)
            newUri = oldUri.replace(f, oldUri.size(), "");
        else if ((f = oldUri.find("/oic/")) != string::npos)
            newUri = oldUri.replace(f, oldUri.size(), "");

        return newUri;
    }

    void ThingsConfiguration::onDeleteActionSet(const HeaderOptions& /*headerOptions*/,
            const OCRepresentation& /*rep*/, const int /*eCode*/, std::string conf)
    {
        std::shared_ptr < OCResource > resource = getResource(conf);

        if (resource)
        {
            QueryParamsMap query;

            // After deletion of the left action set, find target child resource's URIs by sending
            // GET message. Note that, this resource is surely a collection resource which has child
            // resources.
            resource->get(resource->getResourceTypes().at(0), DEFAULT_INTERFACE, query,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsConfiguration::onGetChildInfoForUpdate, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, conf)));
        }

    }

    void ThingsConfiguration::onGetChildInfoForUpdate(const HeaderOptions& headerOptions,
            const OCRepresentation& rep, const int eCode, std::string conf)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            getCallback(conf)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "GET request was successful" << std::endl;

        std::cout << "\tResource URI: " << rep.getUri() << std::endl;

        std::vector < OCRepresentation > children = rep.getChildren();
        for (auto oit = children.begin(); oit != children.end(); ++oit)
        {
            std::cout << "\t\tChild Resource URI: " << oit->getUri() << std::endl;
        }

        // Get information by using configuration name(conf)
        std::shared_ptr < OCResource > resource = getResource(conf);
        std::string actionstring = conf;
        std::string uri = getUriByConfigurationName(conf);
        std::string attrKey = conf;

        if (uri == "")
            return;

        if (resource)
        {
            // In this nest, we create a new action set of which name is the configuration name.
            // Required information consists of a host address, URI, attribute key, and
            // attribute value.
            ActionSet *newActionSet = new ActionSet();
            newActionSet->actionsetName = conf;

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
                newCapability->status = getUpdateVal(conf);

                newAction->listOfCapability.push_back(newCapability);
                newActionSet->listOfAction.push_back(newAction);
            }

            // Request to create a new action set by using the above actionSet
            g_groupmanager->addActionSet(resource, newActionSet,
                    std::function<
                            void(const HeaderOptions& headerOptions,
                                    const OCRepresentation& rep, const int eCode) >(
                            std::bind(&ThingsConfiguration::onCreateActionSet, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, conf)));

            delete(newActionSet);
        }
    }

    void ThingsConfiguration::onGetChildInfoForGet(const HeaderOptions& headerOptions,
            const OCRepresentation& rep, const int eCode, std::string conf)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onGet Response error: " << eCode << std::endl;
            getCallback(conf)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "GET request was successful" << std::endl;
        std::cout << "\tResource URI: " << rep.getUri() << std::endl;

        std::shared_ptr< OCResource > resource, tempResource;
        std::vector < std::shared_ptr< OCResource > > p_resources;
        std::vector < std::string > m_if;
        std::string uri = getUriByConfigurationName(conf);

        if (uri == "")
            return;

        if (uri == "/factoryset" || uri == "/factoryset/oic/con")
            m_if.push_back(BATCH_INTERFACE);
        else
            m_if.push_back(DEFAULT_INTERFACE);

        std::vector < OCRepresentation > children = rep.getChildren();
        for (auto oit = children.begin(); oit != children.end(); ++oit)
        {
            std::cout << "\t\tChild Resource URI: " << oit->getUri() << std::endl;

            // Using a host address and child URIs, we can dynamically create resource objects.
            // Note that the child resources have not found before, we have no resource objects.
            // For this reason, we create the resource objects.

            std::string host = getHostFromURI(oit->getUri());

            tempResource = OCPlatform::constructResourceObject(host, uri, CT_ADAPTER_IP, true,
                    oit->getResourceTypes(), m_if);

            p_resources.push_back(tempResource);
        }

        // Send GET messages to the child resources in turn.
        for (unsigned int i = 0; i < p_resources.size(); ++i)
        {
            resource = p_resources.at(i);
            if (resource)
            {
                try
                {
                    QueryParamsMap test;
                    resource->get(test, getCallback(conf));
                }
                catch (OCException& e)
                {
                    std::cout << e.reason() << std::endl;
                }

            }
        }
    }

    void ThingsConfiguration::onCreateActionSet(const HeaderOptions& headerOptions,
            const OCRepresentation& rep, const int eCode, std::string conf)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            getCallback(conf)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "PUT request was successful" << std::endl;

        std::shared_ptr < OCResource > resource = getResource(conf);
        if (resource)
        {
            // Now, it is time to execute the action set.
            g_groupmanager->executeActionSet(resource, conf,
                    std::function<
                            void(const HeaderOptions& headerOptions,
                                    const OCRepresentation& rep, const int eCode) >(
                            std::bind(&ThingsConfiguration::onExecuteForGroupAction, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, conf)));
        }
    }

    void ThingsConfiguration::onExecuteForGroupAction(const HeaderOptions& headerOptions,
            const OCRepresentation& rep, const int eCode, std::string conf)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            getCallback(conf)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "PUT request was successful" << std::endl;

        getCallback(conf)(headerOptions, rep, eCode);
    }

    bool ThingsConfiguration::isSimpleResource(std::shared_ptr< OCResource > resource)
    {

        for (unsigned int i = 0; i < resource->getResourceTypes().size(); ++i)
        {
            if (resource->getResourceTypes().at(i).find(".resourceset", 0) != std::string::npos)
                return false;
        }

        return true;
    }

    bool ThingsConfiguration::hasBatchInterface(std::shared_ptr< OCResource > resource)
    {
        for (unsigned int i = 0; i < resource->getResourceInterfaces().size(); ++i)
        {
            if (resource->getResourceInterfaces().at(i) == BATCH_INTERFACE)
                return true;
        }

        return false;
    }

    void ThingsConfiguration::onGet(const HeaderOptions& headerOptions, const OCRepresentation& rep,
            const int eCode, std::string conf)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onGet Response error: " << eCode << std::endl;
            getCallback(conf)(headerOptions, rep, eCode);
            return ;
        }

        std::cout << "Get request was successful" << std::endl;

        getCallback(conf)(headerOptions, rep, eCode);
    }

    void ThingsConfiguration::onPut(const HeaderOptions& headerOptions, const OCRepresentation& rep,
            const int eCode, std::string conf)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            getCallback(conf)(headerOptions, rep, eCode);
            return;
        }

        std::cout << "PUT request was successful" << std::endl;

        getCallback(conf)(headerOptions, rep, eCode);
    }

    OCStackResult ThingsConfiguration::updateConfigurations(std::shared_ptr< OCResource > resource,
            std::map< ConfigurationName, ConfigurationValue > configurations,
            ConfigurationCallback callback)
    {
        // For M2, # of configurations is 1
        // First, mapping a semantic name(ConfigurationUnit) into resource's name(uri ...)
        if (configurations.size() == 0)
        {
            std::cout << "# of request configuration is 0" << std::endl;
            return OC_STACK_ERROR;
        }

        if (!resource)
        {
            std::cout << "resource is NULL\n";
            return OC_STACK_ERROR;
        }

        std::map< ConfigurationName, ConfigurationValue >::iterator it = configurations.begin();
        std::string conf = it->first; // configuration name
        std::transform(conf.begin(), conf.end(), conf.begin(), ::tolower); // to lower case

        // Check the request queue if a previous request is still left. If so, remove it.
        std::map< std::string, ConfigurationRequestEntry >::iterator iter =
                configurationRequestTable.find(conf);
        if (iter != configurationRequestTable.end())
            configurationRequestTable.erase(iter);

        // Create new request entry stored in the queue
        ConfigurationRequestEntry newCallback(conf, callback, resource, it->second);
        configurationRequestTable.insert(std::make_pair(conf, newCallback));

        OCRepresentation rep;
        QueryParamsMap query;
        if (isSimpleResource(resource))
        {
            // This resource does not need to use a group manager. Just send a PUT message
            rep.setValue(conf, getUpdateVal(conf));
            return resource->put(resource->getResourceTypes().at(0), DEFAULT_INTERFACE, rep, query,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsConfiguration::onGet, this, std::placeholders::_1,
                                    std::placeholders::_2, std::placeholders::_3, conf)));
        }
        else
        {
            // This resource is a collection resource which uses group manager functionalities.
            // First, delete an existing action set of which name is same as a current action set
            // name. As of now, the name is determined by "Configuration Name" which a user just
            // specifies.
            return g_groupmanager->deleteActionSet(resource, conf,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsConfiguration::onDeleteActionSet, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, conf)));
        }
    }

    OCStackResult ThingsConfiguration::getConfigurations(std::shared_ptr< OCResource > resource,
            std::vector< ConfigurationName > configurations, ConfigurationCallback callback)
    {
        // For M2, # of configurations is 1
        // First, mapping a semantic name(ConfigurationUnit) into resource's name(uri ...)
        if (configurations.size() == 0)
        {
            std::cout << "# of request configuration is 0" << std::endl;
            return OC_STACK_ERROR;
        }
        if (!resource)
        {
            std::cout << "resource is NULL\n";
            return OC_STACK_ERROR;
        }

        std::vector< ConfigurationName >::iterator it = configurations.begin();
        std::string conf = (*it); // configuration name
        std::transform(conf.begin(), conf.end(), conf.begin(), ::tolower); // to lower case

        // Check the request queue if a previous request is still left. If so, remove it.
        std::map< std::string, ConfigurationRequestEntry >::iterator iter =
                configurationRequestTable.find(conf);
        if (iter != configurationRequestTable.end())
            configurationRequestTable.erase(iter);

        // Create new request entry stored in the queue
        ConfigurationRequestEntry newCallback(conf, callback, resource, conf);
        configurationRequestTable.insert(std::make_pair(conf, newCallback));

        QueryParamsMap query;
        OCRepresentation rep;

        if (isSimpleResource(resource))
        {
            // This resource is a simple resource. Just send a PUT message
            std::string m_if = DEFAULT_INTERFACE;

            if (hasBatchInterface(resource))
                m_if = BATCH_INTERFACE;

            return resource->get(resource->getResourceTypes().at(0), m_if, query,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsConfiguration::onGet, this, std::placeholders::_1,
                                    std::placeholders::_2, std::placeholders::_3, conf)));
        }
        else
        {
            // This resource is a collection resource. On the contrary of a update, it does not use
            // group manager functionality. It just acquires child resource's URI and send GET
            // massages to the child resources in turn.
            // First, request the child resources's URI.
            return resource->get(resource->getResourceTypes().at(0), DEFAULT_INTERFACE, query,
                    std::function<
                            void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                                    const int eCode) >(
                            std::bind(&ThingsConfiguration::onGetChildInfoForGet, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, conf)));
        }

    }

// callback handler on GET request
    void ThingsConfiguration::onGetBootstrapInformation(const HeaderOptions& headerOptions,
            const OCRepresentation& rep, const int eCode)
    {
        if (eCode != OC_STACK_OK)
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
            g_bootstrapCallback(headerOptions, rep, eCode);
            return;
        }

        g_bootstrapCallback(headerOptions, rep, eCode);
    }

    void ThingsConfiguration::onFoundBootstrapServer(
            std::vector< std::shared_ptr< OCResource > > resources)
    {
        std::string resourceURI;
        std::string hostAddress;

        try
        {
            // Do some operations with resource object.
            for (unsigned int i = 0; i < resources.size(); ++i)
            {
                std::shared_ptr < OCResource > resource = resources.at(i);

                if (resource)
                { // Request configuration resources

                    std::cout << "Getting bootstrap server representation on: " << DEFAULT_INTERFACE
                            << std::endl;

                    resource->get("bootstrap", DEFAULT_INTERFACE, QueryParamsMap(),
                            &onGetBootstrapInformation);

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
            //log(e.what());
        }
    }

    OCStackResult ThingsConfiguration::doBootstrap(ConfigurationCallback callback)
    {
        if(callback == NULL)
            return OC_STACK_ERROR;
        else
          g_bootstrapCallback = callback;

        // Find bootstrap server.
        std::vector < std::string > type;
        type.push_back("bootstrap");

        std::cout << "Finding Bootstrap Server resource... " << std::endl;
        return g_groupmanager->findCandidateResources(type, &onFoundBootstrapServer);
    }
}


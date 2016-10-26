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
 * This file contains the declaration of classes and its members related to
 * ThingsConfiguration.
 */

#ifndef __OC_THINGSCONFIGURATION__
#define __OC_THINGSCONFIGURATION__

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include "GroupManager.h"
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

namespace OIC
{
/// Declearation of Configuation Callback funtion type
    typedef std::function<
            void(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode)
            > ConfigurationCallback;

    typedef std::string ConfigurationName;
    typedef std::string ConfigurationValue;

    /**
     * @brief
     * The following class is used as a item stacking in request queue. The class stores a request
     * and referential information (e.g., a configuration name, a target resource object, a callback
     * function passed from the applications, and a update value). When the function for updating/
     * getting configuration value is called from applications, this class instance is created and
     * stored in the request queue. The queue is maintained in a std::map structure so if desiring
     * to find a specific request, you can find it by querying a configuration name.
     */
    class ConfigurationRequestEntry
    {
    public:
        ConfigurationRequestEntry(std::string ID, ConfigurationCallback callback,
                std::shared_ptr< OCResource > resource, std::string updateVal);

        // Configuration Name (used in key value in std::map structure)
        // e.g., time, network, security, and so on
        std::string m_ID;
        // Reference callback pointer
        ConfigurationCallback m_callback;
        // Reference resource object
        std::shared_ptr< OCResource > m_resource;
        // Update value only used for configuration update
        std::string m_updateVal;
    };

    /**
     * @brief
     * The following class is used to store providing configuration name and its relevant
     * information. The relevant information includes a brief description, uri, and attribute key.
     * Note that a developer only specifies a configuration name, not URI nor attribute key, to
     * update/get a value to a remote. Thus, using configuration name, we convert it to more
     * specific information (i.e. uri and attribute key) to send a request. This class is reponsible
     * to storing these information.
     */
    class ConfigurationUnitInfo
    {
    public:

        std::string m_name;
        std::string m_attribute;
        std::string m_uri;

        ConfigurationUnitInfo(std::string name, std::string attribute, std::string uri);

        // If a developer wants to know a list of configuration names, gives it in JSON format.
        std::string getJSON();
    };

#define NUMCONFUNIT 6
    typedef std::string ConfigurationName;
    typedef std::string ConfigurationValue;

    /**
     * @class ThingsConfiguration
     * @brief
     * There are two main usages of this class: (1) On a server side, bootstrapping requisite
     * information (i.e. system configuration parameters) from a bootstrap server to access other
     * IoT services, (2) On a client side, getting/updating the system configuration parameters
     * from/to multiple remote things.
     */
    class ThingsConfiguration
    {
    public:
        /**
         * Constructor for ThingsConfiguration. Constructs a new ThingsConfiguration
         */
        ThingsConfiguration(void);

        /**
         * Virtual destructor
         */
        ~ThingsConfiguration(void);

        static ThingsConfiguration *thingsConfigurationInstance;
        static ThingsConfiguration* getInstance();

        // TODO: deprecated
        void deleteInstance();
        void setGroupManager(GroupManager *groupmanager);

        /**
         * API for updating configuration value of multiple things of a target group or a single
         * thing.
         * Callback is called when a response arrives.
         * Before using the below function, a developer should acquire a resource pointer of
         * (collection) resource that he want to send a request by calling findResource() function
         * provided in OCPlatform. And he should also notice a "Configuration Name" term which
         * represents a nickname of a target attribute of a resource that he wants to update.
         * The base motivation to introduce the term is to avoid a usage of URI to access a resource
         * from a developer. Thus, a developer should know which configuration names are supported
         * by Things Configuration class and what the configuration name means.
         * To get a list of supported configuration names, use getListOfSupportedConfigurationUnits(
         * ) function, which provides the list in JSON format.
         * NOTICE: A series of callback functions is called from updateConfigurations() function:
         * (1) For a collection resource
         * updateConfiguration()->onDeleteActionSet()->onGetChildInfoForUpdate()->onCreateActionSet(
         * )->...(CoAP msg. is transmitted)->OnExecuteForGroupAction()->callback function in APP.
         * (2) For a simple resource
         * updateConfiguration()->...(CoAP msg. is transmitted)->OnPut()->callback function in APP.
         *
         * @param resource - resource pointer representing the target group or the single thing.
         * @param configurations - ConfigurationUnit: an attribute key of target resource
         *                         (e.g., loc, st, c, r)
         *                         Value : a value to be updated
         * @param callback - callback.
         *
         * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
         *
         * NOTE: OCStackResult is defined in ocstack.h.
         */
        OCStackResult updateConfigurations(std::shared_ptr< OCResource > resource,
                std::map< ConfigurationName, ConfigurationValue > configurations,
                ConfigurationCallback callback);

        /**
         * API for getting configuration value of multiple things of a target group or a single
         * thing.
         * Callback is called when a response arrives.
         * NOTICE: A series of callback functions is called from getConfigurations() function:
         * (1) For a collection resource
         * getConfigurations()->onGetChildInfoForGet()->...(CoAP msg. is transmitted)
         * ->callback function in APP.
         * (2) For a simple resource
         * getConfigurations()->...(CoAP msg. is transmitted)->onGet()->callback function in APP.
         *
         * @param resource - resource pointer representing the target group or the single thing.
         * @param configurations - ConfigurationUnit: an attribute key of target resource.
         * @param callback - callback.
         *
         * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
         *
         * NOTE: OCStackResult is defined in ocstack.h.
         */
        OCStackResult getConfigurations(std::shared_ptr< OCResource > resource,
                std::vector< ConfigurationName > configurations, ConfigurationCallback callback);

        /**
         * API to show a list of supported configuration units (configurable parameters)
         * Callback call when a response arrives.
         *
         * @return the list in JSON format
         */
        std::string getListOfSupportedConfigurationUnits();

        /**
         * API for bootstrapping functionality. Find a bootstrap server and get configuration
         * information from the bootstrap server. With the information, make a configuration
         * resource.
         *
         * @param callback - callback.
         *
         * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
         *
         * NOTE: OCStackResult is defined in ocstack.h.
         */
        OCStackResult doBootstrap(ConfigurationCallback callback);

    private:

        GroupManager *g_groupmanager;

        std::vector< ConfigurationUnitInfo > ConfigurationUnitTable;

        void onExecuteForGroupAction(const HeaderOptions& headerOptions,
                const OCRepresentation& rep, const int eCode, std::string conf);
        void onGetChildInfoForUpdate(const HeaderOptions& headerOptions,
                const OCRepresentation& rep, const int eCode, std::string conf);
        void onGetChildInfoForGet(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                const int eCode, std::string conf);
        void onCreateActionSet(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                const int eCode, std::string conf);
        void onGetActionSet(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                const int eCode, std::string conf);
        void onDeleteActionSet(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                const int eCode, std::string conf);
        void onGet(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode,
                std::string conf);
        void onPut(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode,
                std::string conf);
        static void onFoundBootstrapServer(std::vector< std::shared_ptr< OCResource > > resources);
        static void onGetBootstrapInformation(const HeaderOptions& headerOptions,
                const OCRepresentation& rep, const int eCode);

        std::shared_ptr< OCResource > getResource(std::string conf);
        ConfigurationCallback getCallback(std::string conf);
        std::string getUpdateVal(std::string conf);
        std::string getAttributeByConfigurationName(ConfigurationName name);
        std::string getUriByConfigurationName(ConfigurationName name);

        std::string getHostFromURI(std::string oldUri);

        bool isSimpleResource(std::shared_ptr< OCResource > resource);
        bool hasBatchInterface(std::shared_ptr< OCResource > resource);

    };
}
#endif  /* __OC_THINGSCONFIGURATION__*/


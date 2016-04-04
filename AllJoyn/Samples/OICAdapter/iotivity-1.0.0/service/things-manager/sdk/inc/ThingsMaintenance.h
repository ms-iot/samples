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
 * ThingsMaintenance.
 */

#ifndef __OC_THINGSMAINTENANCE__
#define __OC_THINGSMAINTENANCE__

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include "OCPlatform.h"
#include "OCApi.h"
#include "GroupManager.h"

using namespace OC;
namespace OIC
{
    /// Declearation of Maintenance Callback funtion type
    typedef std::function<
            void(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode)
            > MaintenanceCallback;

    /**
     *  @brief
     *  The following class is used as a item stacking in request queue. The class stores a request
     *  and referential information (e.g., a maintenance name, a target resource object, a callback
     *  function passed from the applications, and a update value). When the function for updating/
     *  getting maintenance value is called from applications, this class instance is created and
     *  stored in the request queue. The queue is maintained in a std::map structure so if desiring
     *  to find a specific request, you can find it by querying a maintenance name.
     */
    class MaintenanceRequestEntry
    {
    public:
        MaintenanceRequestEntry(std::string ID, MaintenanceCallback callback,
                std::shared_ptr< OCResource > resource, std::string updateVal);

        // Maintenance Name (used in key value in std::map structure)
        // e.g., reboot and factoryset
        std::string m_ID;

        // Reference callback pointer
        MaintenanceCallback m_callback;

        // Reference resource object
        std::shared_ptr< OCResource > m_resource;

        // Update value only used for maintenance update (always "true")
        std::string m_updateVal;
    };

    /**
     *  @brief
     *  The following class is used to store providing maintenance name and its relevant information
     *  The relevant information includes a brief description, uri, and attribute key.
     *  Note that a developer only specifies a maintenance name, not URI nor attribute key, to
     *  update a value to a remote. Thus, using maintenance name, we convert it to more specific
     *  information (i.e. uri and attribute key) to send a request. This class is reponsible to
     *  storing these information.
     */
    class MaintenanceUnitInfo
    {
    public:
        std::string m_name;
        std::string m_attribute;
        std::string m_uri;

        MaintenanceUnitInfo(std::string name, std::string attribute, std::string uri);

        // If a developer wants to know a list of configuration names, gives it in JSON format.
        std::string getJSON();
    };

#define NUMDIAGUNIT 3
    typedef std::string MaintenanceName;
    typedef std::string MaintenanceValue;

    /**
     * @class ThingsMaintenance
     * @brief
     * There are two functionalities in Things Maintenance; (1) FactoryReset to restore all
     * configuration parameters to default one, and (2) Reboot to request a system rebooting.
     */
    class ThingsMaintenance
    {
    public:
        /**
         * Constructor for ThingsMaintenance. Constructs a new ThingsMaintenance
         */
        ThingsMaintenance(void);

        /**
         * Virtual destructor
         */
        ~ThingsMaintenance(void);

        static ThingsMaintenance *thingsMaintenanceInstance;
        static ThingsMaintenance* getInstance();

        // TODO: deprecated
        void deleteInstance();
        void setGroupManager(GroupManager *groupmanager);

        /**
         * API to make things reboot
         * Callback call when a response arrives.
         *
         * @param resource - resource pointer representing the target group
         * @param callback - callback.
         *
         * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
         *
         * NOTE: OCStackResult is defined in ocstack.h.
         */
        OCStackResult reboot(std::shared_ptr< OCResource > resource, MaintenanceCallback callback);

        /**
         * API for factory reset on device
         * Callback call when a response arrives.
         *
         * @param resource - resource pointer representing the target group
         * @param callback - callback.
         *
         * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
         *
         * NOTE: OCStackResult is defined in ocstack.h.
         */

        OCStackResult factoryReset(std::shared_ptr< OCResource > resource,
                MaintenanceCallback callback);

        /**
         * API to show a list of supported maintenance units
         * Callback call when a response arrives.
         *
         * @return the list in JSON format
         */
        std::string getListOfSupportedMaintenanceUnits();

    private:

        GroupManager *g_groupmanager;

        std::vector< MaintenanceUnitInfo > MaintenanceUnitTable;

        void onExecuteForGroupAction(const HeaderOptions& headerOptions,
                const OCRepresentation& rep, const int eCode, std::string conf);
        void onDeleteGroupAction(const HeaderOptions& headerOptions,
                const OCRepresentation& rep, const int eCode, std::string conf);
        void onGetChildInfoForUpdate(const HeaderOptions& headerOptions,
                const OCRepresentation& rep, const int eCode, std::string conf);
        void onCreateActionSet(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                const int eCode, std::string conf);
        void onGet(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode,
                std::string conf);
        void onPut(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode,
                std::string conf);

        std::shared_ptr< OCResource > getResource(std::string conf);
        MaintenanceCallback getCallback(std::string conf);
        std::string getUpdateVal(std::string conf);
        std::string getAttributeByMaintenanceName(MaintenanceName name);
        std::string getUriByMaintenanceName(MaintenanceName name);

        std::string getHostFromURI(std::string oldUri);

        bool isSimpleResource(std::shared_ptr< OCResource > resource);

    };
}
#endif  /* __OC_THINGSCONFIGURATION__*/

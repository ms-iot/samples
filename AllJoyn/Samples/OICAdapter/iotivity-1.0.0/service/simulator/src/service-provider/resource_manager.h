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

/**
 * @file   resource_manager.h
 *
 * @brief   This file provides APIs for simulated resource management.
 */

#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "simulator_resource_server_impl.h"
#include "simulator_resource_creator.h"
#include "simulator_error_codes.h"

/**
 * @class   ResourceManager
 * @brief   This class provides a set of APIs for managing the simulated resource(s).
 */
class ResourceManager
{
    public:
        /**
             *  This method is to create/obtain the singleton instance of ResourceManager.
             */
        static ResourceManager *getInstance(void);

        /**
             * This method is for simulating/creating a resource based on the input data provided from
             * RAML file.
             *
             * @param configPath - RAML configuration file path.
             * @param callback - Callback method for receiving notifications when resource model changes.
             *
             * @return SimulatorResourceServer shared object representing simulated/created resource.
             */
        SimulatorResourceServerSP createResource(const std::string &configPath,
                SimulatorResourceServer::ResourceModelChangedCB callback);

        /**
             * This method is for creating multiple resources of same type based on the input data
             * provided from RAML file.
             *
             * @param configPath - RAML configuration file path.
             * @param count - Number of resource to be created.
             * @param callback - Callback method for receiving notifications when resource model changes.
             *
             * @return vector of SimulatorResourceServer shared objects representing simulated/created
             * resources.
             */
        std::vector<SimulatorResourceServerSP> createResource(const std::string &configPath,
                unsigned short count, SimulatorResourceServer::ResourceModelChangedCB callback);

        /**
             * This method is for obtaining a list of created resources.
             *
             * @param resourceType - Resource type. Empty value will fetch all resources.
             *                                          Default value is empty string.
             *
             * @return vector of SimulatorResourceServer shared objects representing simulated/created
             */
        std::vector<SimulatorResourceServerSP> getResources(const std::string &resourceType = "");

        /**
             * This method is for deleting/unregistering resource.
             *
             * @param resource - SimulatorResourceServer shared object.
             *
             */
        void deleteResource(const SimulatorResourceServerSP &resource);

        /**
             * This method is for deleting multiple resources based on resource type.
             *
             * @param resourceType - Resource type. Empty value will delete all the resources.
             *                                          Default value is empty string.
             *
             */
        void deleteResources(const std::string &resourceType = "");

    private:
        ResourceManager(): m_id(0) {}
        ~ResourceManager() = default;
        ResourceManager(const ResourceManager &) = delete;
        ResourceManager &operator=(const ResourceManager &) = delete;
        ResourceManager(const ResourceManager &&) = delete;
        ResourceManager &operator=(const ResourceManager && ) = delete;

        SimulatorResourceServerSP buildResource(const std::string &configPath,
                                                SimulatorResourceServer::ResourceModelChangedCB callback);
        std::string constructURI(const std::string &uri);

        /*Member variables*/
        int m_id;
        SimulatorResourceCreator m_resourceCreator;
        std::recursive_mutex m_lock;
        std::map<std::string, std::map<std::string, SimulatorResourceServerSP>> m_resources;
};

#endif


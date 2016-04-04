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
 * @file simulator_client.h
 *
 * @brief This file provides a class for realizing simulator client functionality.
 *
 */

#ifndef SIMULATOR_CLIENT_H_
#define SIMULATOR_CLIENT_H_

#include "simulator_client_types.h"
#include "simulator_remote_resource.h"
#include "simulator_exceptions.h"

/**
 * @class   SimulatorClient
 * @brief   This class provides a set of functions for discovering the resources over the network.
 */
class SimulatorClient
{
    public:

        /**
         * API for getting singleton instance of SimulatorClient class.
         *
         * @return Singleton instance of SimulatorClient class.
         *
         */
        static SimulatorClient *getInstance(void);

        /**
         * API for discovering all type of resources.
         * Discovered resources will be notified through the callback set using @callback parameter.
         *
         * @param callback - Method of type @ResourceFindCallback through which discoverd resources
         *                                   will be notified.
         *
         * NOTE: API throws @InvalidArgsException and @SimulatorException.
         */
        void findResources(ResourceFindCallback callback);

        /**
         * API for discovering resources of a particular resource type.
         * Discovered resources will be notified through the callback set using @callback parameter.
         *
         * @param resourceType - Type of resource to be searched for
         * @param callback - Method of type @ResourceFindCallback through which discoverd resources
         *                                   will be notified.
         *
         * NOTE: API throws @InvalidArgsException and @SimulatorException.
         */
        void findResources(const std::string &resourceType, ResourceFindCallback callback);

    private:
        SimulatorClient() = default;
        ~SimulatorClient() = default;
        SimulatorClient(const SimulatorClient &) = delete;
        SimulatorClient &operator=(const SimulatorClient &) = delete;
        SimulatorClient(const SimulatorClient &&) = delete;
        SimulatorClient &operator=(const SimulatorClient && ) = delete;

        void onResourceFound(std::shared_ptr<OC::OCResource> resource,
                             ResourceFindCallback callback);
};

#endif


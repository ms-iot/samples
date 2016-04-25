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
  * @file   simulator_manager.h
  *
  * @brief   This file contains the declaration of SimulatorManager class which has the methods
  *              for configuring the platform and creation/deletion of resources.
  */

#ifndef SIMULATOR_MANAGER_H_
#define SIMULATOR_MANAGER_H_

#include "simulator_server_types.h"
#include "simulator_client_types.h"
#include "simulator_device_info.h"
#include "simulator_platform_info.h"
#include "simulator_resource_server.h"
#include "simulator_remote_resource.h"
#include "simulator_exceptions.h"
#include "simulator_logger.h"

typedef std::function<void(DeviceInfo &deviceInfo)> DeviceInfoCallback;
typedef std::function<void(PlatformInfo &platformInfo)> PlatformInfoCallback;

/**
 * @class   SimulatorManager
 *
 * @brief   This class provides a set of methods for platform configuration,
 *              and creation/deletion of resources.
 *
 */
class SimulatorManager
{
    public:
        static SimulatorManager *getInstance();

        /**
         * This method is for simulating/creating a resource based on the input data provided from
         * RAML file.
         *
         * @param configPath - RAML configuration file path.
         * @param callback - Callback method for receiving notifications when resource model changes.
         *
         * @return SimulatorResourceServer shared object representing simulated/created resource.
         *
         *  NOTE: API would throw @InvalidArgsException when invalid arguments passed, and
          * @SimulatorException if any other error occured.
         */
        std::shared_ptr<SimulatorResourceServer> createResource(const std::string &configPath,
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
         *
         * NOTE: API would throw @InvalidArgsException when invalid arguments passed, and
         * @SimulatorException if any other error occured.
         */
        std::vector<std::shared_ptr<SimulatorResourceServer>> createResource(
                    const std::string &configPath, unsigned short count,
                    SimulatorResourceServer::ResourceModelChangedCB callback);

        /**
         * This method is for obtaining a list of created resources.
         *
         * @param resourceType - Resource type. Empty value will fetch all resources.
         *                                          Default value is empty string.
         *
         * @return vector of SimulatorResourceServer shared objects representing simulated/created
         */
        std::vector<std::shared_ptr<SimulatorResourceServer>> getResources(
                    const std::string &resourceType = "");

        /**
          * This method is for deleting/unregistering resource.
          *
          * @param resource - SimulatorResourceServer shared object.
          *
          * NOTE: API would throw @InvalidArgsException when invalid arguments passed
          */
        void deleteResource(const std::shared_ptr<SimulatorResourceServer> &resource);

        /**
          * This method is for deleting multiple resources based on resource type.
          *
          * @param resourceType - Resource type. Empty value will delete all the resources.
          *                                          Default value is empty string.
          *
          * NOTE: API would throw @InvalidArgsException when invalid arguments passed
          */
        void deleteResource(const std::string &resourceType = "");

        /**
         * API for discovering all type of resources.
         * Discovered resources will be notified through the callback set using @callback parameter.
         *
         * @param callback - Method of type @ResourceFindCallback through which discoverd resources
         *                                   will be notified.
         *
         * NOTE: API would throw @InvalidArgsException when invalid arguments passed, and
         * @SimulatorException if any other error occured.
         */
        void findResource(ResourceFindCallback callback);

        /**
         * API for discovering resources of a particular resource type.
         * Discovered resources will be notified through the callback set using @callback parameter.
         *
         * @param resourceType - Type of resource to be searched for
         * @param callback - Method of type @ResourceFindCallback through which discoverd resources
         *                                   will be notified.
         *
         * NOTE: API would throw @InvalidArgsException when invalid arguments passed, and
         * @SimulatorException if any other error occured.
         */
        void findResource(const std::string &resourceType, ResourceFindCallback callback);

        /**
         * API for getting device information from remote device.
         * Received device information will be notified through the callback set using
         * @callback parameter.
         *
         * @param callback - Method of type @DeviceInfoCallback through which device information
         *                                   will be notified.
         *
         * NOTE: API throws @InvalidArgsException and @SimulatorException on error.
         */
        void getDeviceInfo(DeviceInfoCallback callback);

        /**
         * API for registering device information with stack.
         *
         * @param deviceName - Device name to be registered.
         *
         * NOTE: API throws @InvalidArgsException and @SimulatorException on error.
         */
        void setDeviceInfo(const std::string &deviceName);

        /**
         * API for getting platform information from remote device.
         * Received platform information will be notified through the callback set using
         * @callback parameter.
         *
         * @param callback - Method of type @PlatformInfoCallback through which platform
         *                                   information will be notified.
         *
         * NOTE: API throws @InvalidArgsException and @SimulatorException on error.
         */
        void getPlatformInfo(PlatformInfoCallback callback);

        /**
         * API for registering platform information with stack.
         *
         * @param platformInfo - PlatformInfo contains all platform related information.
         *
         * NOTE: API throws @SimulatorException on error.
         */
        void setPlatformInfo(PlatformInfo &platformInfo);

        /**
         * API for setting logger target for receiving the log messages.
         *
         * @param logger - ILogger interface for handling the log messages.
         *
         */
        void setLogger(const std::shared_ptr<ILogger> &logger);

        /**
         * API for setting console as logger target.
         *
         * @return true if console set as logger target,
         *         otherwise false.
         *
         */
        bool setConsoleLogger();

        /**
         * API for setting file as logger target.
         *
         * @param path - File to which log messages to be saved.
         *
         * @return true if console set as logger target,
         *         otherwise false.
         *
         */
        bool setFileLogger(const std::string &path);

    private:
        SimulatorManager();
        ~SimulatorManager() = default;
        SimulatorManager(const SimulatorManager &) = delete;
        SimulatorManager &operator=(const SimulatorManager &) = delete;
        SimulatorManager(const SimulatorManager &&) = delete;
        SimulatorManager &operator=(const SimulatorManager && ) = delete;
};

#endif

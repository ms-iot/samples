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
 * @file simulator_remote_resource.h
 *
 * @brief This file provides a class for handling discovered resources.
 *
 */

#ifndef SIMULATOR_REMOTE_RESOURCE_H_
#define SIMULATOR_REMOTE_RESOURCE_H_

#include "simulator_client_types.h"
#include "simulator_resource_model.h"

/**
 * @class   SimulatorRemoteResource
 * @brief   This class provides a set of functions for the client to hande the resources currently running on the servers.
 */
class SimulatorRemoteResource
{
    public:

        /**
         * Callback method for receiving response for GET, PUT and POST requests.
         *
         */
        typedef std::function<void (std::string, SimulatorResult, SimulatorResourceModelSP)>
        ResponseCallback;

        /**
         * Callback method for receiving model change notifications from remote resource.
         *
         */
        typedef std::function<void (std::string, SimulatorResult, SimulatorResourceModelSP, int)>
        ObserveNotificationCallback;

        /**
         * Callback method for receiving auto request generation and verifiction progress state.
         *
         */
        typedef std::function<void(std::string, int, OperationState)>
        StateCallback;

        /**
         * API for getting URI of resource.
         *
         * @return URI of resource.
         *
         */
        virtual std::string getURI() const = 0;

        /**
         * API for getting host address of resource.
         *
         * @return Host address of resource.
         *
         */
        virtual std::string getHost() const = 0;

        /**
         * API for getting unique id of resource.
         *
         * @return ID of resource.
         *
         */
        virtual std::string getID() const = 0;


        /**
         * API for getting connectivity type of resource.
         *
         * @return enum SimulatorConnectivityType value
         *
         */
        virtual SimulatorConnectivityType getConnectivityType() const = 0;

        /**
         * API for getting resource types bound with the resource.
         *
         * @return vector of strings representing resource types.
         *
         */
        virtual std::vector < std::string > getResourceTypes() const = 0;

        /**
         * API for getting interface types bound with the resource.
         *
         * @return vector of strings representing interface types.
         *
         */
        virtual std::vector < std::string > getResourceInterfaces() const = 0;

        /**
         * API to check whether resource can be observed or not.
         *
         * @return true if resource is observable, otherwise false.
         *
         */
        virtual bool isObservable() const = 0;

        virtual void observe(ObserveType type, ObserveNotificationCallback callback) = 0;

        virtual void cancelObserve() = 0;

        virtual void get(const std::map<std::string, std::string> &queryParams,
                         ResponseCallback callback) = 0;

        virtual void get(const std::string &interfaceType,
                         const std::map<std::string, std::string> &queryParams,
                         ResponseCallback callback) = 0;

        virtual void put(const std::map<std::string, std::string> &queryParams,
                         SimulatorResourceModelSP representation,
                         ResponseCallback callback) = 0;

        virtual void put(const std::string &interfaceType,
                         const std::map<std::string, std::string> &queryParams,
                         SimulatorResourceModelSP representation,
                         ResponseCallback callback) = 0;

        virtual void post(const std::map<std::string, std::string> &queryParams,
                          SimulatorResourceModelSP representation,
                          ResponseCallback callback) = 0;

        virtual void post(const std::string &interfaceType,
                          const std::map<std::string, std::string> &queryParams,
                          SimulatorResourceModelSP representation,
                          ResponseCallback callback) = 0;

        virtual int startVerification(RequestType type, StateCallback callback) = 0;

        virtual void stopVerification(int id) = 0;

        virtual void configure(const std::string &path) = 0;
};

typedef std::shared_ptr<SimulatorRemoteResource> SimulatorRemoteResourceSP;

#endif


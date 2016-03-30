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
 * @file simulator_remote_resource_impl.h
 *
 * @brief This file provides internal implementation of simulator remote resource functionalities.
 *
 */

#ifndef SIMULATOR_REMOTE_RESOURCE_IMPL_H_
#define SIMULATOR_REMOTE_RESOURCE_IMPL_H_

#include "simulator_remote_resource.h"
#include "auto_request_gen_mngr.h"
#include "RamlParser.h"
#include "request_model.h"

#include <mutex>

class SimulatorRemoteResourceImpl : public SimulatorRemoteResource
{
    public:
        SimulatorRemoteResourceImpl(std::shared_ptr<OC::OCResource> &ocResource);
        std::string getURI() const;
        std::string getHost() const;
        std::string getID() const;
        SimulatorConnectivityType getConnectivityType() const;
        std::vector < std::string > getResourceTypes() const;
        std::vector < std::string > getResourceInterfaces() const;
        bool isObservable() const;

        void observe(ObserveType type, ObserveNotificationCallback callback);

        void cancelObserve();

        void get(const std::map<std::string, std::string> &queryParams,
                 ResponseCallback callback);

        void get(const std::string &interfaceType,
                 const std::map<std::string, std::string> &queryParams,
                 ResponseCallback callback);

        void put(const std::map<std::string, std::string> &queryParams,
                 SimulatorResourceModelSP representation,
                 ResponseCallback callback);

        void put(const std::string &interfaceType,
                 const std::map<std::string, std::string> &queryParams,
                 SimulatorResourceModelSP representation,
                 ResponseCallback callback);

        void post(const std::map<std::string, std::string> &queryParams,
                  SimulatorResourceModelSP representation,
                  ResponseCallback callback);

        void post(const std::string &interfaceType,
                  const std::map<std::string, std::string> &queryParams,
                  SimulatorResourceModelSP representation,
                  ResponseCallback callback);

        int startVerification(RequestType type, StateCallback callback);
        void stopVerification(int id);
        void configure(const std::string &path);

    private:
        void configure(std::shared_ptr<RAML::Raml> &raml);
        void onResponseReceived(SimulatorResult result, SimulatorResourceModelSP repModel,
                                ResponseCallback clientCallback);
        SimulatorConnectivityType convertConnectivityType(OCConnectivityType type) const;

        std::string m_id;
        std::mutex m_observeMutex;
        bool m_observeState;
        GETRequestSenderSP m_getRequestSender;
        PUTRequestSenderSP m_putRequestSender;
        POSTRequestSenderSP m_postRequestSender;
        AutoRequestGenMngrSP m_autoRequestGenMngr;
        std::map<RequestType, RequestModelSP> m_requestModelList;
        std::shared_ptr<OC::OCResource> m_ocResource;
};

#endif

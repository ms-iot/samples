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

#ifndef SIMULATOR_RESOURCE_SERVER_IMPL_H_
#define SIMULATOR_RESOURCE_SERVER_IMPL_H_

#include "simulator_resource_server.h"
#include "resource_update_automation_mngr.h"

class SimulatorResourceServerImpl : public SimulatorResourceServer
{
    public:
        SimulatorResourceServerImpl();

        void setURI(const std::string &uri);

        void setResourceType(const std::string &resourceType);

        void setInterfaceType(const std::string &interfaceType);

        void setName(const std::string &name);

        void setObservable(bool state);

        bool isObservable() const;

        int startUpdateAutomation(AutomationType type,
                                  updateCompleteCallback callback);

        int startUpdateAutomation(const std::string &attrName, AutomationType type,
                                  updateCompleteCallback callback);

        std::vector<int> getResourceAutomationIds();

        std::vector<int> getAttributeAutomationIds();

        void stopUpdateAutomation(const int id);

        void setModelChangeCallback(ResourceModelChangedCB callback);

        void setObserverCallback(ObserverCB callback);

        std::vector<ObserverInfo> getObserversList();

        void notify(uint8_t id);

        void notifyAll();

        void start();

        void stop();

        void notifyApp();

    private:
        OC::OCRepresentation getOCRepresentation();
        bool modifyResourceModel(OC::OCRepresentation &ocRep);
        OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request);
        void resourceModified();

        ResourceModelChangedCB m_callback;
        ObserverCB m_observeCallback;
        UpdateAutomationMngr m_updateAutomationMgr;
        std::vector<ObserverInfo> m_observersList;

        OCResourceProperty m_property;
        OCResourceHandle m_resourceHandle;
};

typedef std::shared_ptr<SimulatorResourceServerImpl> SimulatorResourceServerImplSP;

#endif

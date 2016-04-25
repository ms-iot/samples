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

#ifndef RESOURCE_UPDATE_AUTOMATION_MNGR_H_
#define RESOURCE_UPDATE_AUTOMATION_MNGR_H_

#include "simulator_resource_server.h"
#include "resource_update_automation.h"

class UpdateAutomationMngr
{
    public:
        UpdateAutomationMngr();

        int startResourceAutomation(SimulatorResourceServer *resource,
                                    AutomationType type, int interval, updateCompleteCallback callback);

        int startAttributeAutomation(SimulatorResourceServer *resource,
                                     const std::string &attrName, AutomationType type, int interval,
                                     updateCompleteCallback callback);

        std::vector<int> getResourceAutomationIds();

        std::vector<int> getAttributeAutomationIds();

        void stop(int id);

        void stopAll();

    private:
        void automationCompleted(int id);

        int m_id;
        std::mutex m_lock;
        std::map<int, ResourceUpdateAutomationSP> m_resourceUpdationList;
        std::map<int, AttributeUpdateAutomationSP> m_attrUpdationList;
};

#endif

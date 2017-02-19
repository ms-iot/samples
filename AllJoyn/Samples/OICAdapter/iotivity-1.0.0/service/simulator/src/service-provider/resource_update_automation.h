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

#ifndef RESOURCE_UPDATE_AUTOMATION_H_
#define RESOURCE_UPDATE_AUTOMATION_H_

#include "simulator_resource_server.h"
#include <thread>

class AttributeUpdateAutomation
{
    public:
        AttributeUpdateAutomation(int id, SimulatorResourceServer *resource,
                                  const std::string &attrName, AutomationType type, int interval,
                                  updateCompleteCallback callback,
                                  std::function<void (const int)> finishedCallback);

        void start();

        void stop();

    private:
        void updateAttribute();
        void setAttributeValue();

        SimulatorResourceServer *m_resource;
        std::string m_attrName;
        AutomationType m_type;
        int m_id;
        bool m_stopRequested;
        int m_updateInterval;
        SimulatorResourceModel::Attribute m_attribute;
        updateCompleteCallback m_callback;
        std::function<void (const int)> m_finishedCallback;
        std::thread *m_thread;
};

typedef std::shared_ptr<AttributeUpdateAutomation> AttributeUpdateAutomationSP;

class ResourceUpdateAutomation
{
    public:
        ResourceUpdateAutomation(int id, SimulatorResourceServer *resource,
                                 AutomationType type, int interval,
                                 updateCompleteCallback callback,
                                 std::function<void (const int)> finishedCallback);

        void start();

        void stop();

        void finished(int id);

    private:
        SimulatorResourceServer *m_resource;
        AutomationType m_type;
        int m_id;
        int m_updateInterval;
        SimulatorResourceModel m_resModel;
        std::map<int, AttributeUpdateAutomationSP> m_attrUpdationList;
        updateCompleteCallback m_callback;
        std::function<void (const int)> m_finishedCallback;
};

typedef std::shared_ptr<ResourceUpdateAutomation> ResourceUpdateAutomationSP;

#endif

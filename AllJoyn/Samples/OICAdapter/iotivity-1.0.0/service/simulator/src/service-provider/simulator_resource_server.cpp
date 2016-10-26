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

#include "simulator_resource_server.h"

SimulatorResourceServer::SimulatorResourceServer()
{
}

std::string SimulatorResourceServer::getURI() const
{
    return m_uri;
}

std::string SimulatorResourceServer::getResourceType() const
{
    return m_resourceType;
}

std::string SimulatorResourceServer::getInterfaceType() const
{
    return m_interfaceType;
}

std::string SimulatorResourceServer::getName() const
{
    return m_name;
}

void SimulatorResourceServer::addAttribute(SimulatorResourceModel::Attribute &attribute)
{
    m_resModel.addAttribute(attribute);
}

void SimulatorResourceServer::setRange(const std::string &attrName, const int min, const int max)
{
    m_resModel.setRange(attrName, min, max);
}

SimulatorResourceModel SimulatorResourceServer::getModel() const
{
    return m_resModel;
}

void SimulatorResourceServer::updateFromAllowedValues(const std::string &attrName,
        unsigned int index)
{
    m_resModel.updateAttributeFromAllowedValues(attrName, index);

    // Notify all the subscribers
    notifyAll();
}

void SimulatorResourceServer::removeAttribute(const std::string &attrName)
{
    m_resModel.removeAttribute(attrName);
}


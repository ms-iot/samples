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

#include "resource_update_automation.h"
#include "simulator_resource_server_impl.h"
#include "simulator_exceptions.h"
#include "simulator_logger.h"
#include "logger.h"

#define ATAG "ATTRIBUTE_AUTOMATION"
#define RTAG "RESOURCE_AUTOMATION"

#define SLEEP_FOR(X) if (X > 0) std::this_thread::sleep_for(std::chrono::milliseconds(X));

AttributeUpdateAutomation::AttributeUpdateAutomation(int id, SimulatorResourceServer *resource,
        const std::string &attrName, AutomationType type, int interval,
        updateCompleteCallback callback, std::function<void (const int)> finishedCallback)
    :   m_resource(resource),
        m_attrName(attrName),
        m_type(type),
        m_id(id),
        m_stopRequested(false),
        m_updateInterval(interval),
        m_callback(callback),
        m_finishedCallback(finishedCallback),
        m_thread(nullptr) {}

void AttributeUpdateAutomation::start()
{
    // Check the validity of attribute
    SimulatorResourceModel resModel = m_resource->getModel();
    if (false == resModel.getAttribute(m_attrName, m_attribute))
    {
        OC_LOG_V(ERROR, ATAG, "Attribute:%s not present in resource!", m_attrName.c_str());
        throw SimulatorException(SIMULATOR_ERROR, "Attribute is not present in resource!");
    }

    if (m_updateInterval < 0)
    {
        m_updateInterval = m_attribute.getUpdateFrequencyTime();
        if (0 > m_updateInterval)
            m_updateInterval = 0;
    }

    m_thread = new std::thread(&AttributeUpdateAutomation::updateAttribute, this);
}

void AttributeUpdateAutomation::stop()
{
    m_stopRequested = true;
    if (m_thread)
        m_thread->join();
}

void AttributeUpdateAutomation::updateAttribute()
{
    do
    {
        try
        {
            setAttributeValue();
        }
        catch (SimulatorException &e)
        {
            break;
        }
        if (m_stopRequested)
            break;
    }
    while (AutomationType::RECURRENT == m_type);

    if (!m_stopRequested)
    {
        OC_LOG_V(DEBUG, ATAG, "Attribute:%s automation is completed!", m_attrName.c_str());
        SIM_LOG(ILogger::INFO, "Automation of " << m_attrName << " attribute is completed.");
    }

    // Notify application through callback
    if (m_callback)
        m_callback(m_resource->getURI(), m_id);

    if (m_finishedCallback && !m_stopRequested)
        m_finishedCallback(m_id);
}

void AttributeUpdateAutomation::setAttributeValue()
{
    SimulatorResourceServerImpl *resourceImpl =
        dynamic_cast<SimulatorResourceServerImpl *>(m_resource);
    if (!resourceImpl)
        return;

    if (SimulatorResourceModel::Attribute::ValueType::INTEGER ==
            m_attribute.getValueType()) // For integer type values
    {
        int min;
        int max;

        m_attribute.getRange(min, max);
        for (int value = min; value <= max; value++)
        {
            if (m_stopRequested)
                break;
            resourceImpl->updateAttributeValue(m_attribute.getName(), value);
            resourceImpl->notifyApp();
            SLEEP_FOR(m_updateInterval);
        }
    }
    else
    {
        for (int index = 0; index < m_attribute.getAllowedValuesSize(); index++)
        {
            if (m_stopRequested)
                break;
            resourceImpl->updateFromAllowedValues(m_attribute.getName(), index);
            resourceImpl->notifyApp();
            SLEEP_FOR(m_updateInterval);
        }
    }
}

ResourceUpdateAutomation::ResourceUpdateAutomation(int id, SimulatorResourceServer *resource,
        AutomationType type, int interval, updateCompleteCallback callback,
        std::function<void (const int)> finishedCallback)
    :   m_resource(resource),
        m_type(type),
        m_id(id),
        m_updateInterval(interval),
        m_callback(callback),
        m_finishedCallback(finishedCallback) {}

void ResourceUpdateAutomation::start()
{
    m_resModel = m_resource->getModel();
    std::map<std::string, SimulatorResourceModel::Attribute> attributes = m_resModel.getAttributes();
    if (0 == attributes.size())
    {
        OC_LOG(ERROR, RTAG, "Resource has zero attributes!");
        throw SimulatorException(SIMULATOR_ERROR, "Resource has zero attributes!");
    }

    int id = 0;
    for (auto & attribute : attributes)
    {
        AttributeUpdateAutomationSP attributeAutomation(new AttributeUpdateAutomation(
                    id, m_resource, attribute.first, m_type, m_updateInterval, nullptr,
                    std::bind(&ResourceUpdateAutomation::finished, this, std::placeholders::_1)));

        m_attrUpdationList[id++] = attributeAutomation;
        try
        {
            attributeAutomation->start();
        }
        catch (SimulatorException &e)
        {
            stop();
            throw;
        }
    }
}

void ResourceUpdateAutomation::finished(int id)
{
    if (m_attrUpdationList.end() != m_attrUpdationList.find(id))
    {
        m_attrUpdationList.erase(m_attrUpdationList.find(id));
    }

    if (!m_attrUpdationList.size())
    {
        // Notify application through callback
        if (m_callback)
            m_callback(m_resource->getURI(), m_id);

        if (m_finishedCallback)
            m_finishedCallback(m_id);
    }
}

void ResourceUpdateAutomation::stop()
{
    // Stop all the attributes updation
    for (auto & attrAutomation : m_attrUpdationList)
    {
        (attrAutomation.second)->stop();
    }

    m_attrUpdationList.clear();
}

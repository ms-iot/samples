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

#include "attribute_generator.h"

AttributeGenerator::AttributeGenerator(SimulatorResourceModel::Attribute &attribute)
    :   m_name(attribute.getName()),
        m_min(INT_MIN),
        m_max(INT_MAX),
        m_rangeIndex(-1),
        m_nextAllowedValueIndex(0),
        m_prevAllowedValueIndex(0),
        m_hasRange(false),
        m_hasAllowedValue(false)
{
    if (attribute.getValueType() ==
        SimulatorResourceModel::Attribute::ValueType::INTEGER)
    {
        attribute.getRange(m_min, m_max);
        if (INT_MIN != m_min && INT_MAX != m_max)
        {
            m_hasRange = true;
            m_rangeIndex = m_min;
        }
    }
    else
    {
        m_allowedValues = attribute.getAllowedValues();
        if (0 != m_allowedValues.size())
        {
            m_hasAllowedValue = true;
        }
        m_prevAllowedValueIndex = m_allowedValues.size();
    }
}

bool AttributeGenerator::hasNext()
{
    if (m_hasRange && m_rangeIndex <= m_max)
    {
        return true;
    }

    if (m_hasAllowedValue && m_nextAllowedValueIndex < m_allowedValues.size())
    {
        return true;
    }

    return false;
}

bool AttributeGenerator::next(SimulatorResourceModel::Attribute &attribute)
{
    attribute.setName(m_name);

    if (m_hasRange)
    {
        attribute.setValue(m_rangeIndex++);
        return true;
    }
    else if (m_hasAllowedValue)
    {
        attribute.setValue(m_allowedValues[m_nextAllowedValueIndex++]);
        return true;
    }

    return false;
}

bool AttributeGenerator::previous(SimulatorResourceModel::Attribute &attribute)
{
    attribute.setName(m_name);

    if (m_hasRange)
    {
        attribute.setValue(m_rangeIndex - 1);
        return true;
    }
    else if (m_hasAllowedValue)
    {
        attribute.setValue(m_allowedValues[m_prevAllowedValueIndex - 1]);
        return true;
    }

    return false;
}


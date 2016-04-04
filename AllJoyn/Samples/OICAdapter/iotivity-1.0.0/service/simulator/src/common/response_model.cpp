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

#include "response_model.h"

ResponseModel::ResponseModel(int code) : m_code(code) {}

void ResponseModel::setRepSchema(SimulatorResourceModelSP &repSchema)
{
    m_repSchema = repSchema;
}

SimulatorResult ResponseModel::verifyResponse(const OC::OCRepresentation &rep)
{
    for (auto & ocAttribute : rep)
    {
        SimulatorResourceModel::Attribute attribute;
        if (false == m_repSchema->getAttribute(ocAttribute.attrname(), attribute))
        {
            return SIMULATOR_UKNOWN_PROPERTY;
        }

        switch (attribute.getValueType())
        {
            case SimulatorResourceModel::Attribute::ValueType::INTEGER : // Integer
                {
                    SimulatorResult result = validateAttributeInteger(attribute, ocAttribute);
                    if (SIMULATOR_OK != result)
                    {
                        return result;
                    }
                }
                break;

            case SimulatorResourceModel::Attribute::ValueType::DOUBLE : // Double
                {
                    SimulatorResult result = validateAttributeDouble(attribute, ocAttribute);
                    if (SIMULATOR_OK != result)
                    {
                        return result;
                    }
                }
                break;

            case SimulatorResourceModel::Attribute::ValueType::STRING : // String
                {
                    SimulatorResult result = validateAttributeString(attribute, ocAttribute);
                    if (SIMULATOR_OK != result)
                    {
                        return result;
                    }
                }
                break;
        }
    }

    return SIMULATOR_OK;
}

SimulatorResult ResponseModel::validateAttributeInteger(
    SimulatorResourceModel::Attribute &attrSchema,
    const OC::OCRepresentation::AttributeItem &ocAttribute)
{
    // Check the value type
    if (OC::AttributeType::Integer != ocAttribute.type())
    {
        return SIMULATOR_TYPE_MISMATCH;
    }

    // Check value if it is in range
    int min, max, value;
    attrSchema.getRange(min, max);
    value = ocAttribute.getValue<int>();
    if (value < min || value > max)
    {
        return SIMULATOR_BAD_VALUE;
    }

    return SIMULATOR_OK;
}

SimulatorResult ResponseModel::validateAttributeDouble(
    SimulatorResourceModel::Attribute &attrSchema,
    const OC::OCRepresentation::AttributeItem &ocAttribute)
{
    // Check the value type
    if (OC::AttributeType::Double != ocAttribute.type())
    {
        return SIMULATOR_TYPE_MISMATCH;
    }

    return SIMULATOR_OK;
}

SimulatorResult ResponseModel::validateAttributeString(
    SimulatorResourceModel::Attribute &attrSchema,
    const OC::OCRepresentation::AttributeItem &ocAttribute)
{
    // Check the value type
    if (OC::AttributeType::String != ocAttribute.type())
    {
        return SIMULATOR_TYPE_MISMATCH;
    }

    // TODO: Check the allowed values
    return SIMULATOR_OK;
}
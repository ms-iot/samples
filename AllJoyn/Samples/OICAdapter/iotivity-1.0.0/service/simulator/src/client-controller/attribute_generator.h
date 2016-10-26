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

#ifndef ATTRIBUTE_GENERATOR_H_
#define ATTRIBUTE_GENERATOR_H_

#include <map>
#include <vector>
#include "simulator_resource_model.h"

class AttributeGenerator
{
    public:
        AttributeGenerator(SimulatorResourceModel::Attribute &attribute);
        bool hasNext();
        bool next(SimulatorResourceModel::Attribute &attribute);
        bool previous(SimulatorResourceModel::Attribute &attribute);
        void reset();

    private:
        std::string m_name;
        SimulatorResourceModel::Attribute::ValueType m_type;
        int m_min;
        int m_max;
        int m_rangeIndex;
        int m_nextAllowedValueIndex;
        int m_prevAllowedValueIndex;
        bool m_hasRange;
        bool m_hasAllowedValue;
        std::vector<SimulatorResourceModel::Attribute::ValueVariant> m_allowedValues;
};

#endif


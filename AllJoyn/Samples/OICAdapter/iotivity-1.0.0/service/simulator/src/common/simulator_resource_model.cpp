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

#include "simulator_resource_model.h"
#include "OCPlatform.h"
#include <sstream>
#include <boost/lexical_cast.hpp>

template <typename T>
struct TypeConverter
{
    constexpr static SimulatorResourceModel::Attribute::ValueType type =
            SimulatorResourceModel::Attribute::ValueType::UNKNOWN;
};

template <>
struct TypeConverter<int>
{
    constexpr static SimulatorResourceModel::Attribute::ValueType type =
            SimulatorResourceModel::Attribute::ValueType::INTEGER;
};

template <>
struct TypeConverter<double>
{
    constexpr static SimulatorResourceModel::Attribute::ValueType type =
            SimulatorResourceModel::Attribute::ValueType::DOUBLE;
};

template <>
struct TypeConverter<bool>
{
    constexpr static SimulatorResourceModel::Attribute::ValueType type =
            SimulatorResourceModel::Attribute::ValueType::BOOLEAN;
};

template <>
struct TypeConverter<std::string>
{
    constexpr static SimulatorResourceModel::Attribute::ValueType type =
            SimulatorResourceModel::Attribute::ValueType::STRING;
};

class attribute_type_visitor : public boost::static_visitor<
        SimulatorResourceModel::Attribute::ValueType>
{
    public:
        template <typename T>
        result_type operator ()(const T &)
        {
            return TypeConverter<T>::type;
        }
};

class to_string_visitor : public boost::static_visitor<std::string>
{
    public:
        template <typename T>
        result_type operator ()(const T &value)
        {
            try
            {
                return boost::lexical_cast<std::string>(value);
            }
            catch (const boost::bad_lexical_cast &e)
            {
                return "";
            }
        }
};

class add_to_representation : public boost::static_visitor<>
{
    public:
        add_to_representation(OC::OCRepresentation &rep, const std::string &key)
            : m_rep(rep), m_key(key) {}

        template <typename T>
        void operator ()(const T &value)
        {
            m_rep.setValue(m_key, value);
        }

        OC::OCRepresentation &&getRep()
        {
            return std::move(m_rep);
        }

    private:
        OC::OCRepresentation m_rep;
        std::string m_key;
};

class range_validation : public boost::static_visitor<bool>
{
    public:
        range_validation (SimulatorResourceModel::Attribute &attrItem)
            : m_attrItem(attrItem) {}

        bool operator ()(int &value)
        {
            int min, max;
            m_attrItem.getRange(min, max);
            if (value >= min && value <= max)
                return true;
            return false;
        }

        bool operator ()(double &value)
        {
            std::vector<SimulatorResourceModel::Attribute::ValueVariant> values
                = m_attrItem.getAllowedValues();
            for (SimulatorResourceModel::Attribute::ValueVariant & val : values)
            {
                SimulatorResourceModel::Attribute::ValueVariant vVal = value;
                if (val == vVal)
                    return true;
            }
            return false;
        }

        bool operator ()(bool &value)
        {
            return true;
        }

        bool operator ()(std::string &value)
        {
            std::vector<SimulatorResourceModel::Attribute::ValueVariant> values
                = m_attrItem.getAllowedValues();
            for (SimulatorResourceModel::Attribute::ValueVariant & vVal : values)
            {
                std::string val = boost::get<std::string>(vVal);
                if (val == value)
                    return true;
            }

            return false;
        }

    private:
        SimulatorResourceModel::Attribute &m_attrItem;
};

SimulatorResourceModel::Attribute::ValueVariant
&SimulatorResourceModel::Attribute::AllowedValues::at(unsigned int index)
{
    return m_values.at(index);
}

int SimulatorResourceModel::Attribute::AllowedValues::size() const
{
    return m_values.size();
}

std::vector<std::string> SimulatorResourceModel::Attribute::AllowedValues::toString() const
{
    std::vector<std::string> values;

    for (auto & value : m_values)
    {
        to_string_visitor visitor;
        values.push_back(boost::apply_visitor(visitor, value));
    }
    return values;
}

std::vector<SimulatorResourceModel::Attribute::ValueVariant>
SimulatorResourceModel::Attribute::AllowedValues::getValues()
{
    return m_values;
}

std::string SimulatorResourceModel::Attribute::getName(void) const
{
    return m_name;
}

void SimulatorResourceModel::Attribute::setName(const std::string &name)
{
    m_name = name;
}

void SimulatorResourceModel::Attribute::getRange(int &min, int &max) const
{
    min = m_min;
    max = m_max;
}

void SimulatorResourceModel::Attribute::setRange(const int &min, const int &max)
{
    m_min = min;
    m_max = max;
}

int SimulatorResourceModel::Attribute::getAllowedValuesSize() const
{
    return m_allowedValues.size();
}

void SimulatorResourceModel::Attribute::setFromAllowedValue(unsigned int index)
{
    m_value = m_allowedValues.at(index);
}

SimulatorResourceModel::Attribute::ValueType SimulatorResourceModel::Attribute::getValueType() const
{
    attribute_type_visitor typeVisitor;
    return boost::apply_visitor(typeVisitor, m_value);
}

std::string SimulatorResourceModel::Attribute::valueToString() const
{
    to_string_visitor visitor;
    return boost::apply_visitor(visitor, m_value);
}

std::vector<std::string> SimulatorResourceModel::Attribute::allowedValuesToString() const
{
    return m_allowedValues.toString();
}

void SimulatorResourceModel::Attribute::addValuetoRepresentation(OC::OCRepresentation &rep,
        const std::string &key) const
{
    add_to_representation visitor(rep, key);
    boost::apply_visitor(visitor, m_value);
    rep = visitor.getRep();
}

bool SimulatorResourceModel::Attribute::compare(SimulatorResourceModel::Attribute &attribute)
{
    // Check the value types
    if (m_value.which() != attribute.getValue().which())
    {
        return false;
    }

    // Check the value in allowed range
    range_validation visitor(*this);
    return boost::apply_visitor(visitor, attribute.getValue());
}

std::vector<SimulatorResourceModel::Attribute::ValueVariant>
SimulatorResourceModel::Attribute::getAllowedValues()
{
    return m_allowedValues.getValues();
}

bool SimulatorResourceModel::getAttribute(const std::string &attrName, Attribute &value)
{
    if (m_attributes.end() != m_attributes.find(attrName))
    {
        value = m_attributes[attrName];
        return true;
    }

    return false;
}

std::map<std::string, SimulatorResourceModel::Attribute> SimulatorResourceModel::getAttributes()
const
{
    return m_attributes;
}

void SimulatorResourceModel::addAttribute(const SimulatorResourceModel::Attribute &attribute)
{
    if (!attribute.getName().empty() &&
        m_attributes.end() == m_attributes.find(attribute.getName()))
    {
        m_attributes[attribute.getName()] = attribute;
    }
}

void SimulatorResourceModel::setRange(const std::string &attrName, const int min, const int max)
{
    if (m_attributes.end() != m_attributes.find(attrName))
        m_attributes[attrName].setRange(min, max);
}

void SimulatorResourceModel::setUpdateInterval(const std::string &attrName, int interval)
{
    if (m_attributes.end() != m_attributes.find(attrName))
        m_attributes[attrName].setUpdateFrequencyTime(interval);
}

void SimulatorResourceModel::updateAttributeFromAllowedValues(const std::string &attrName,
        unsigned int index)
{
    if (m_attributes.end() != m_attributes.find(attrName))
        m_attributes[attrName].setFromAllowedValue(index);
}

void SimulatorResourceModel::removeAttribute(const std::string &attrName)
{
   if (m_attributes.end() == m_attributes.find(attrName))
   {
       return;
   }

    m_attributes.erase(attrName);
    return;
}

OC::OCRepresentation SimulatorResourceModel::getOCRepresentation() const
{
    OC::OCRepresentation rep;
    for (auto & attribute : m_attributes)
    {
        (attribute.second).addValuetoRepresentation(rep, attribute.first);
    }

    return rep;
}

bool SimulatorResourceModel::update(OC::OCRepresentation &ocRep)
{
    if (0 == ocRep.size())
        return true;

    // Convert OCRepresentation to SimulatorResourceModel
    SimulatorResourceModelSP resModel = create(ocRep);

    return update(resModel);
}

bool SimulatorResourceModel::update(SimulatorResourceModelSP &repModel)
{
    std::map<std::string, SimulatorResourceModel::Attribute> attributes = repModel->getAttributes();
    for (auto & attributeItem : attributes)
    {
        // Check the attribute presence
        SimulatorResourceModel::Attribute attribute;
        if (false == getAttribute((attributeItem.second).getName(), attribute))
        {
            return false;
        }

        // Check the validity of the value to be set
        if (false == attribute.compare(attributeItem.second))
        {
            return false;
        }
        m_attributes[(attributeItem.second).getName()].setValue((attributeItem.second).getValue());
    }

    return true;
}

SimulatorResourceModelSP SimulatorResourceModel::create(const OC::OCRepresentation &ocRep)
{
    SimulatorResourceModelSP resModel(new SimulatorResourceModel);
    for (auto & attributeItem : ocRep)
    {
        SimulatorResourceModel::Attribute attribute;
        if (attributeItem.type() == OC::AttributeType::Integer)
            attribute.setValue(attributeItem.getValue<int>());
        if (attributeItem.type() == OC::AttributeType::Double)
            attribute.setValue(attributeItem.getValue<double>());
        if (attributeItem.type() == OC::AttributeType::String)
            attribute.setValue(attributeItem.getValue<std::string>());

        attribute.setName(attributeItem.attrname());
        resModel->m_attributes[attributeItem.attrname()] = attribute;
    }
    return resModel;
}


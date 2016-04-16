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

#include "AbstractParam.h"


namespace RAML
{
    std::string AbstractParam::getDefaultValue() const
    {
        return m_defaultValue;
    }
    void AbstractParam::setDefaultValue(const std::string &defaultValue)
    {
        m_defaultValue = defaultValue;
    }

    std::string AbstractParam::getDescription() const
    {
        return m_description;
    }

    void AbstractParam::setDescription(const std::string &description)
    {
        m_description = description;
    }

    std::string AbstractParam::getDisplayName() const
    {
        return m_displayName;
    }

    void AbstractParam::setDisplayName(const std::string &displayName)
    {
        m_displayName = displayName;
    }

    std::list<std::string> AbstractParam::getEnumeration() const
    {
        return m_enumeration;
    }

    void AbstractParam::setEnumeration(const std::string &enumeration)
    {
        m_enumeration.push_back(enumeration);
    }

    std::string AbstractParam::getExample() const
    {
        return m_example;
    }

    void AbstractParam::setExample(const std::string &example)
    {
        m_example = example;
    }

    int AbstractParam::getMaxLength() const
    {
        return m_maxLength;
    }

    void AbstractParam::setMaxLength(int maxLength)
    {
        m_maxLength = maxLength;
    }

    int AbstractParam::getMaximum() const
    {
        return m_maximum;
    }

    void AbstractParam::setMaximum(int maximum)
    {
        m_maximum = maximum;
    }

    int AbstractParam::getMinLength() const
    {
        return m_minLength;
    }

    void AbstractParam::setMinLength(int minLength)
    {
        m_minLength = minLength;
    }

    int AbstractParam::getMinimum() const
    {
        return m_minimum;
    }

    void AbstractParam::setMinimum(int minimum)
    {
        m_minimum = minimum;
    }

    std::string AbstractParam::getPattern() const
    {
        return m_pattern;
    }

    void AbstractParam::setPattern(const std::string &pattern)
    {
        m_pattern = pattern;
    }

    std::string AbstractParam::getType() const
    {
        return m_type;
    }

    void AbstractParam::setType(const std::string &type)
    {
        m_type = type;
    }

    bool AbstractParam::isRepeat() const
    {
        return m_repeat;
    }

    void AbstractParam::setRepeat(bool repeat)
    {
        m_repeat = repeat;
    }

    bool AbstractParam::isRequired() const
    {
        return m_required;
    }

    void AbstractParam::setRequired(bool required)
    {
        m_required = required;
    }

    void AbstractParam::readParameters(const YAML::Node &yamlNode)
    {
        for ( YAML::const_iterator it = yamlNode.begin(); it != yamlNode.end(); ++it )
        {
            std::string key = READ_NODE_AS_STRING(it->first);

            if (key == Keys::Description)
                setDescription(READ_NODE_AS_STRING(it->second));
            else if (key == Keys::Default)
                setDefaultValue(READ_NODE_AS_STRING(it->second));
            else if (key == Keys::DisplayName)
                setDisplayName(READ_NODE_AS_STRING(it->second));

            else if (key == Keys::Example)
                setExample(READ_NODE_AS_STRING(it->second));
            else if (key == Keys::Maximum)
                setMaximum(READ_NODE_AS_LONG(it->second));
            else if (key == Keys::Minimum)
                setMinimum(READ_NODE_AS_LONG(it->second));
            else if (key == Keys::MaxLength)
                setMaxLength(READ_NODE_AS_INT(it->second));
            else if (key == Keys::MinLength)
                setMinLength(READ_NODE_AS_INT(it->second));

            else if (key == Keys::Pattern)
                setPattern(READ_NODE_AS_STRING(it->second));
            else if (key == Keys::Repeat)
                setRepeat(READ_NODE_AS_BOOL(it->second));
            else if (key == Keys::Required)
                setRequired(READ_NODE_AS_BOOL(it->second));
            else if (key == Keys::Type)
                setType(READ_NODE_AS_STRING(it->second));
            else if (key == Keys::Enum)
            {
                YAML::Node enumNode = it->second;
                for ( YAML::const_iterator tt = enumNode.begin(); tt != enumNode.end(); ++tt )
                    setEnumeration(READ_NODE_AS_STRING(*tt));
            }
        }
    }
}

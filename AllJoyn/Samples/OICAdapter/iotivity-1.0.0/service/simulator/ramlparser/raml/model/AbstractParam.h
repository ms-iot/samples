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

/**
 * @file   AbstractParam.h
 *
 * @brief   This file provides data Model for RAML AbstractParam.
 */

#ifndef ABSTRACT_PARAM_H
#define ABSTRACT_PARAM_H


#include <map>
#include <list>
#include <string>
#include "Utils.h"


namespace RAML
{
    /**
     * @class   AbstractParam
     * @brief   This class provides data Model for RAML AbstractParam.
     */
    class AbstractParam
    {
        public:
            /**
                 * This method is for getting DefaultValue from AbstractParam.
                 *
                 * @return DefaultValue as string.
                 */
            virtual std::string getDefaultValue() const;

            /**
                 * This method is for setting DefaultValue to AbstractParam.
                 *
                 * @param defaultValue - DefaultValue as string
                 */
            virtual void setDefaultValue(const std::string &defaultValue);

            /**
                 * This method is for getting Description from AbstractParam.
                 *
                 * @return Description as string.
                 */
            virtual std::string getDescription() const;

            /**
                 * This method is for setting Description to AbstractParam.
                 *
                 * @param description - Description as string
                 */
            virtual void setDescription(const std::string &description);

            /**
                 * This method is for getting DisplayName from AbstractParam.
                 *
                 * @return DisplayName as string.
                 */
            virtual std::string getDisplayName() const;

            /**
                 * This method is for setting DisplayName to AbstractParam.
                 *
                 * @param displayName - DisplayName as string
                 */
            virtual void setDisplayName(const std::string &displayName);

            /**
                 * This method is for getting Enumeration from AbstractParam.
                 *
                 * @return list of enumeration as string.
                 */
            virtual std::list<std::string> getEnumeration() const;

            /**
                 * This method is for setting Enumeration to AbstractParam.
                 *
                 * @param enumeration - Enumeration as string
                 */
            virtual void setEnumeration(const std::string &enumeration);

            /**
                 * This method is for getting Example from AbstractParam.
                 *
                 * @return Example as string.
                 */
            virtual std::string getExample() const;

            /**
                 * This method is for setting Example to AbstractParam.
                 *
                 * @param example - Example as string
                 */
            virtual void setExample(const std::string &example);

            /**
                 * This method is for getting MaxLength from AbstractParam.
                 *
                 * @return MaxLength as int.
                 */
            virtual int getMaxLength() const;

            /**
                 * This method is for setting MaxLength to AbstractParam.
                 *
                 * @param maxLength - MaxLength as int
                 */
            virtual void setMaxLength(int maxLength);

            /**
                 * This method is for getting Maximum from AbstractParam.
                 *
                 * @return Maximum as int.
                 */
            virtual int getMaximum() const;

            /**
                 * This method is for setting Maximum to AbstractParam.
                 *
                 * @param maximum - Maximum as int
                 */
            virtual void setMaximum(int maximum);

            /**
                 * This method is for getting MinLength from AbstractParam.
                 *
                 * @return MinLength as int.
                 */
            virtual int getMinLength() const;

            /**
                 * This method is for setting MinLength to AbstractParam.
                 *
                 * @param minLength - MinLength as int
                 */
            virtual void setMinLength(int minLength);

            /**
                 * This method is for getting Minimum from AbstractParam.
                 *
                 * @return Minimum as int.
                 */
            virtual int getMinimum() const;

            /**
                 * This method is for setting Minimum to AbstractParam.
                 *
                 * @param minimum - Minimum as int
                 */
            virtual void setMinimum(int minimum);

            /**
                 * This method is for getting Pattern from AbstractParam.
                 *
                 * @return Pattern as string.
                 */
            virtual std::string getPattern() const;

            /**
                 * This method is for setting Pattern to AbstractParam.
                 *
                 * @param pattern - Pattern as string
                 */
            virtual void setPattern(const std::string &pattern) ;

            /**
                 * This method is for getting Type from AbstractParam.
                 *
                 * @return Type as string.
                 */
            virtual std::string getType() const;

            /**
                 * This method is for setting Type to AbstractParam.
                 *
                 * @param type - Type as string
                 */
            virtual void setType(const std::string &type);

            /**
                 * This method is for getting isRepeat from AbstractParam.
                 *
                 * @return isRepeat as bool.
                 */
            virtual bool isRepeat() const;

            /**
                 * This method is for setting Repeat to AbstractParam.
                 *
                 * @param repeat - Repeat as bool
                 */
            virtual void setRepeat(bool repeat);

            /**
                 * This method is for getting isRequired from AbstractParam.
                 *
                 * @return isRequired as bool.
                 */
            virtual bool isRequired() const;

            /**
                 * This method is for setting Required to AbstractParam.
                 *
                 * @param required - Required as bool
                 */
            virtual void setRequired(bool required);

            /**
                  * Constructor of AbstractParam.
                  */
            AbstractParam() : m_minimum(0), m_maximum(0), m_minLength(0), m_maxLength(0),
                m_repeat(false), m_required(false) {}

            /**
                   * Constructor of AbstractParam.
                   *
                   * @param yamlNode - Reference to YamlNode for reading the AbstractParam
                   *
                   */
            AbstractParam(const YAML::Node &yamlNode) : m_minimum(0), m_maximum(0),
                m_minLength(0), m_maxLength(0), m_repeat(false), m_required(false)
            {
                readParameters(yamlNode);
            }
        private:
            virtual void readParameters(const YAML::Node &yamlNode);

        private:
            std::string m_defaultValue;
            std::string m_description;
            std::string m_displayName;
            std::list<std::string> m_enumeration;
            std::string m_example;
            int m_minimum;
            int m_maximum;
            int m_minLength;
            int m_maxLength;
            std::string m_pattern;
            bool m_repeat;
            bool m_required;
            std::string m_type;


    };

}
#endif

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
 * @file   simulator_resource_model.h
 *
 * @brief   This file contains a class which represents the resource model for simulator
 *             resources and provides a set of functions for updating the model.
 */

#ifndef SIMULATOR_RESOURCE_MODEL_H_
#define SIMULATOR_RESOURCE_MODEL_H_

#include <string>
#include <vector>
#include "OCPlatform.h"
#include <climits>

/**
 * @class   SimulatorResourceModel
 * @brief   This class provides a set of functions for accessing and manipulating the resource model.
 */
class SimulatorResourceModel
{
    public:
        SimulatorResourceModel() = default;
        SimulatorResourceModel(const SimulatorResourceModel &) = default;
        SimulatorResourceModel &operator=(const SimulatorResourceModel &) = default;
        SimulatorResourceModel(SimulatorResourceModel &&) = default;
        SimulatorResourceModel &operator=(SimulatorResourceModel && ) = default;

        /**
          * @class   Attribute
          * @brief   This class represents a resource attribute whose values can be generic.
          */
        class Attribute
        {
            public:
                typedef boost::variant <
                int,
                double,
                bool,
                std::string
                > ValueVariant;

                enum class ValueType
                {
                    UNKNOWN,
                    INTEGER,
                    DOUBLE,
                    BOOLEAN,
                    STRING
                };

                Attribute()
                {
                    m_min = INT_MIN;
                    m_max = INT_MAX;
                    m_updateInterval = -1;
                }

                Attribute(const std::string &attrName)
                {
                    m_name = attrName;
                    m_min = INT_MIN;
                    m_max = INT_MAX;
                    m_updateInterval = -1;
                }

                /**
                 * API to get attribute's name.
                 *
                 * @return Attribute name.
                 */
                std::string getName(void) const;

                /**
                 * API to set the name of attribute.
                 *
                 * @param name - Attribute name.
                 */
                void setName(const std::string &name);

                /**
                 * API to get attribute's value.
                 *
                 * @return value of attribute.
                 */
                template <typename T>
                T getValue() const
                {
                    T val = T();
                    return boost::get<T>(m_value);
                }

                /**
                 * API to get attribute's value.
                 *
                 * @return value of attribute as ValueVariant.
                 */
                ValueVariant &getValue()
                {
                    return m_value;
                }

                /**
                 * API to get attribute's value type.
                 *
                 * @return ValueType enum.
                 */
                ValueType getValueType() const;

                /**
                 * API to set the attribute's value.
                 *
                 * @param value - value to be set.
                 */
                template <typename T>
                void setValue(const T &value)
                {
                    m_value = value;
                }

                /**
                 * API to set the attribute's value from allowed values container.
                 *
                 * @param allowedValueIndex - Index of value to be set from allowed vaules container.
                 */
                void setFromAllowedValue(unsigned int index);

                /**
                 * API to get range of attribute's value.
                 */
                void getRange(int &min, int &max) const;

                /**
                 * API to set range of attribute's value.
                 *
                 * @param min - minimum value could be set as attribute value.
                 * @param max - maximum value could be set as attribute value.
                 */
                void setRange(const int &min, const int &max);

                /**
                 * API to set the values to allowed values set.
                 *
                 * @param values - vector of values which will be set as allowed values.
                 */
                template <typename T>
                bool setAllowedValues(const std::vector<T> &values)
                {
                    ValueVariant temp = values.at(0);
                    if (temp.which() != m_value.which())
                    {
                        return false;
                    }

                    m_allowedValues.addValues(values);
                    return true;
                }

                /**
                 * API to get the number of values present in allowed values set.
                 *
                 * @return Size of the allowed values.
                 */
                int getAllowedValuesSize() const;

                /**
                 * API to get the string representation of the value.
                 *
                 * @return Attribute's value as a string.
                 */
                std::string valueToString() const;

                /**
                 * API to get the string representation of all the allowed values.
                 *
                 * @return All allowed values as a string.
                 */
                std::vector<std::string> allowedValuesToString() const;

                void addValuetoRepresentation(OC::OCRepresentation &rep,
                                              const std::string &key) const;

                bool compare(Attribute &attribute);

                std::vector<ValueVariant> getAllowedValues();

                int getUpdateFrequencyTime() {return m_updateInterval;}
                void setUpdateFrequencyTime(int interval) {m_updateInterval = interval;}

            private:
                class AllowedValues
                {
                    public:
                        template <typename T>
                        void addValue(const T &value)
                        {
                            ValueVariant temp = value;
                            m_values.push_back(temp);
                        }

                        template <typename T>
                        void addValues(const std::vector<T> &values)
                        {
                            for (auto value : values)
                            {
                                ValueVariant vValue = value;
                                m_values.push_back(vValue);
                            }
                        }

                        ValueVariant &at(unsigned int index);
                        int size() const;
                        std::vector<std::string> toString() const;
                        std::vector<ValueVariant> getValues();
                    private:
                        std::vector<ValueVariant> m_values;
                };

                std::string m_name;
                ValueVariant m_value;
                int m_max;
                int m_min;
                AllowedValues m_allowedValues;
                int m_updateInterval;
        };

        /**
         * API to get the number of attributes in the resource model.
         *
         * @return Number of attributes.
         */
        int size() const { return m_attributes.size(); }

        /**
         * API to get the value of an attribute.
         *
         * @param attrName - Attribute name
         * @param value - Attribute value
         *
         * @return true if attribute exists, otherwise false.
         */
        bool getAttribute(const std::string &attrName, Attribute &value);

        /**
         * API to get the entire list of attributes in the form of key-value pair.
         * Attribute name is the key and an instance of Attribute is the value.
         *
         * @return A map of all the attributes
         */
        std::map<std::string, Attribute> getAttributes() const;

        /**
         * API to add new attribute to resource model.
         *
         * @param attrName - Attribute name
         * @param attrValue - Attribute value
         */
        template <typename T>
        void addAttribute(const std::string &attrName, const T &attrValue)
        {
            if (m_attributes.end() == m_attributes.find(attrName))
            {
                m_attributes[attrName] = Attribute(attrName);
                m_attributes[attrName].setValue(attrValue);
            }
        }

        /**
          * API to add new attribute to resource model.
          *
          * @param attr  - Attribute pointer
          *
          */
        void addAttribute(const Attribute &attribute);

        /**
         * API to set range of attribute value.
         *
         * @param attrName - Attribute name.
         * @param min - Minimum value could be set as attribute value.
         * @param max - Maximum value could be set as attribute value.
         */
        void setRange(const std::string &attrName, const int min, const int max);

        OC::OCRepresentation getOCRepresentation() const;
        static std::shared_ptr<SimulatorResourceModel> create(const OC::OCRepresentation &ocRep);

        template <typename T>
        void setAllowedValues(const std::string &attrName, const std::vector<T> &values)
        {
            if (m_attributes.end() != m_attributes.find(attrName))
                m_attributes[attrName].setAllowedValues(values);
        }

        bool update(OC::OCRepresentation &ocRep);

        bool update(std::shared_ptr<SimulatorResourceModel> &repModel);

        template <typename T>
        void updateAttribute(const std::string &attrName, const T &value)
        {
            if (m_attributes.end() != m_attributes.find(attrName))
                m_attributes[attrName].setValue(value);
        }

        void updateAttributeFromAllowedValues(const std::string &attrName, unsigned int index);

        void removeAttribute(const std::string &attrName);

        void setUpdateInterval(const std::string &attrName, int interval);

    private:
        std::map<std::string, Attribute> m_attributes;
};

typedef std::shared_ptr<SimulatorResourceModel> SimulatorResourceModelSP;
typedef std::shared_ptr<SimulatorResourceModel::Attribute> AttributeSP;

#endif

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
 * @file   Properties.h
 *
 * @brief   This file provides data Model for Json Schema Properties.
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <string>
#include <vector>
#include <map>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <limits>
#include "Items.h"
#include "AllowedValues.h"
#include "cJSON.h"
#include <memory>

namespace RAML
{
    /**
     * @class   Properties
     * @brief   This class provides data Model for Json Schema Properties.
     */
    class Properties
    {
        public:
            /**
                  * Constructor of Properties.
                  */
            Properties(): m_min(INT_MAX), m_max(INT_MAX), m_doubleMin(INT_MAX), m_doubleMax(INT_MAX),
                m_multipleOf(INT_MAX), m_unique(false), m_additionalItems(false) {}

            /**
                  * Constructor of Properties.
                  *
                  * @param name - Properties name as string.
                  */
            Properties(const std::string &name) : m_name(name), m_min(INT_MAX), m_max(INT_MAX),
                m_doubleMin(INT_MAX), m_doubleMax(INT_MAX), m_multipleOf(INT_MAX),
                m_unique(false), m_additionalItems(false) {}

            /**
                 * This method is for getting Name from Properties.
                 *
                 * @return Properties name as string
                 */
            inline std::string getName(void) const
            {
                return m_name;
            }

            /**
                 * This method is for setting name to Properties
                 *
                 * @param name - Properties name as string.
                 */
            inline void setName(const std::string &name)
            {
                m_name = name;
            }

            /**
                 * This method is for getting Value from Properties.
                 *
                 * @return Properties Value
                 */
            template <typename T>
            T getValue() const
            {
                return boost::get<T>(m_value);
            }

            /**
                 * This method is for getting Value from Properties.
                 *
                 * @return Properties Value
                 */
            ValueVariant &getValue()
            {
                return m_value;
            }

            /**
                 * This method is for getting ValueVariant type from Properties.
                 *
                 * @return Properties Value type as Int
                 */
            int getValueType() const
            {
                return m_value.which();
            }

            /**
                 * This method is for getting ValueVariant type from Properties.
                 *
                 * @return Properties VariantType type
                 */
            VariantType getVariantType() const
            {
                if (m_value.which() == 3)
                    return VariantType::STRING;
                else if (m_value.which() == 2)
                    return VariantType::BOOL;
                else if (m_value.which() == 1)
                    return VariantType::DOUBLE;
                else
                    return VariantType::INT;
            }

            /**
                 * This method is for getting Value type as Integer from Properties.
                 *
                 * @return Properties Value type as Integer
                 */
            int getValueInt()
            {
                return boost::lexical_cast<int> (m_value);
            }

            /**
                 * This method is for getting Value type as String from Properties.
                 *
                 * @return Properties Value type as String
                 */
            std::string getValueString()
            {
                return boost::lexical_cast<std::string> (m_value);
            }

            /**
                 * This method is for getting Value type as double from Properties.
                 *
                 * @return Properties Value type as double
                 */
            double getValueDouble()
            {
                return boost::lexical_cast<double> (m_value);
            }

            /**
                 * This method is for getting Value type as bool from Properties.
                 *
                 * @return Properties Value type as bool
                 */
            bool getValueBool()
            {
                return boost::lexical_cast<bool> (m_value);
            }

            /**
                 * This method is for setting Value to Properties
                 *
                 * @param value - Properties Value.
                 */
            template <typename T>
            void setValue(const T &value)
            {
                m_value = value;
            }

            /**
                 * This method is for getting Range from Properties.
                 *
                 * @param min - reference to hold Minimum value of Properties.
                 * @param max -  reference to hold Maximum value of Properties.
                 * @param multipleOf -  reference to hold multipleOf value of Properties.
                 */
            inline void getRange(int &min, int &max, int &multipleOf) const
            {
                min = m_min;
                max = m_max;
                multipleOf = m_multipleOf;
            }

            /**
                 * This method is for getting Range from Properties.
                 *
                 * @param min - reference to hold Minimum value of Properties.
                 * @param max -  reference to hold Maximum value of Properties.
                 * @param multipleOf -  reference to hold multipleOf value of Properties.
                 */
            inline void getRangeDouble(double &min, double &max, int &multipleOf) const
            {
                min = m_doubleMin;
                max = m_doubleMax;
                multipleOf = m_multipleOf;
            }
            /**
                 * This method is for setting Minimum to Properties
                 *
                 * @param min - Minimum value of Properties.
                 */
            inline void setMin(const int &min)
            {
                m_min = min;
            }

            /**
                 * This method is for setting Maximum to Properties
                 *
                 * @param max - Maximum value of Properties.
                 */
            inline void setMax(const int &max)
            {
                m_max = max;
            }

            /**
                 * This method is for setting Minimum to Properties
                 *
                 * @param min - Minimum value of Properties.
                 */
            inline void setMinDouble(const double &min)
            {
                m_doubleMin = min;
            }

            /**
                 * This method is for setting Maximum to Properties
                 *
                 * @param max - Maximum value of Properties.
                 */
            inline void setMaxDouble(const double &max)
            {
                m_doubleMax = max;
            }
            /**
                 * This method is for setting multipleOf to Properties
                 *
                 * @param multipleOf - multipleOf value of Properties.
                 */
            inline void setMultipleOf(const int &multipleOf)
            {
                m_multipleOf = multipleOf;
            }

            /**
                 * This method is for setting AllowedValues to Properties
                 *
                 * @param values - list of AllowedValues of Properties.
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
                 * This method is for getting size of AllowedValues from Properties.
                 *
                 * @return  size of AllowedValues
                 */
            inline int getAllowedValuesSize() const
            {
                return m_allowedValues.size();
            }

            /**
                 * This method is for getting AllowedValues from Properties.
                 *
                 * @return list of AllowedValues of Properties.
                 */
            inline std::vector<ValueVariant> getAllowedValues()
            {
                return m_allowedValues.getValues();
            }

            /**
                 * This method is for getting AllowedValues as integer from Properties.
                 *
                 * @return list of AllowedValues as integer
                 */
            inline std::vector<int> getAllowedValuesInt()
            {
                return m_allowedValues.getValuesInt();
            }

            /**
                 * This method is for getting AllowedValues as String from Properties.
                 *
                 * @return list of AllowedValues as String
                 */
            inline std::vector<std::string> getAllowedValuesString()
            {
                return m_allowedValues.getValuesString();
            }

            /**
                 * This method is for getting AllowedValues as Double from Properties.
                 *
                 * @return list of AllowedValues as Double
                 */
            inline std::vector<double> getAllowedValuesDouble()
            {
                return m_allowedValues.getValuesDouble();
            }

            /**
                 * This method is for getting AllowedValues as Bool from Properties.
                 *
                 * @return list of AllowedValues as Bool
                 */
            inline std::vector<bool> getAllowedValuesBool()
            {
                return m_allowedValues.getValuesBool();
            }

            /**
                 * This method is for setting Description to Properties
                 *
                 * @param description - Description as string.
                 */
            inline void setDescription(const std::string &description)
            {
                m_description = description;
            }

            /**
                 * This method is for getting Description from Properties.
                 *
                 * @return Description as string
                 */
            inline std::string getDescription()
            {
                return m_description;
            }

            /**
                 * This method is for setting Type to Properties
                 *
                 * @param type - Type as string.
                 */
            void setType(const std::string &type)
            {
                m_type = type;
            }

            /**
                 * This method is for getting Type from Properties.
                 *
                 * @return Type as string
                 */
            std::string getType()
            {
                return m_type;
            }

            /**
                 * This method is for setting Pattern to Properties
                 *
                 * @param pattern - Pattern as string.
                 */
            void setPattern(const std::string &pattern)
            {
                m_pattern = pattern;
            }


            /**
                 * This method is for getting Pattern from Properties.
                 *
                 * @return Pattern as string
                 */
            std::string getPattern()
            {
                return m_pattern;
            }

            /**
                 * This method is for setting Format to Properties
                 *
                 * @param format - Format as string.
                 */
            void setFormat(const std::string &format)
            {
                m_format = format;
            }

            /**
                 * This method is for getting Format from Properties.
                 *
                 * @return Format as string
                 */
            std::string getFormat()
            {
                return m_format;
            }

            /**
                 * This method is for setting Items to Properties
                 *
                 * @param item - pointer to Items
                 */
            void setItem(const ItemsPtr &item)
            {
                m_items.push_back(item);
            }

            /**
                 * This method is for getting Items from Properties.
                 *
                 * @return list of pointer to Items
                 */
            std::vector<ItemsPtr> const &getItems() const
            {
                return m_items;
            }

            /**
                 * This method is for setting Unique to Properties
                 *
                 * @param value - Unique as bool
                 */
            void setUnique( int value)
            {
                if (value == cJSON_True) m_unique = true;
                else m_unique = false;
            }

            /**
                 * This method is for getting isUnique from Properties.
                 *
                 * @return isUnique as bool
                 */
            bool getUnique()
            {
                return m_unique;
            }

            /**
                 * This method is for setting AdditionalItems to Properties
                 *
                 * @param value - AdditionalItems as bool
                 */
            void setAdditionalItems(int value)
            {
                if (value == cJSON_True) m_additionalItems = true;
                else m_additionalItems = false;
            }

            /**
                 * This method is for getting AdditionalItems from Properties.
                 *
                 * @return AdditionalItems as bool
                 */
            bool getAdditionalItems()
            {
                return m_additionalItems;
            }
        private:
            std::string m_name;
            ValueVariant m_value;
            int m_min;
            int m_max;
            double m_doubleMin;
            double m_doubleMax;
            int m_multipleOf;
            AllowedValues m_allowedValues;
            std::string m_type;
            std::string m_pattern;
            std::string m_format;
            std::string m_description;
            bool m_unique;
            bool m_additionalItems;
            std::vector<ItemsPtr > m_items;
    };

    /** PropertiesPtr - shared Ptr to Properties.*/
    typedef std::shared_ptr<Properties> PropertiesPtr;

}
#endif

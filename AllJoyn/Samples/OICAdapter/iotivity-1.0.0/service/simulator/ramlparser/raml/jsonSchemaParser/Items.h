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
 * @file   Items.h
 *
 * @brief   This file provides data Model for Json Schema Array Items.
 */

#ifndef ITEMS_H_
#define ITEMS_H_

#include <string>
#include <vector>
#include <map>
#include "Properties.h"
#include "Helpers.h"
#include "AllowedValues.h"
#include <memory>

namespace RAML
{
    class Properties;
    class AllowedValues;
    /**
     * @class   Items
     * @brief   This class provides data Model for Json Schema Array Items.
     */
    class Items
    {
        public:
            /**
                  * Constructor of Items.
                  */
            Items() {}

            /**
                 * This method is for setting Properties to Items
                 *
                 * @param propName - Properties name as string.
                 * @param property - pointer to Properties.
                 */
            void addProperty(const std::string &propName, const std::shared_ptr<Properties> &property)
            {
                if (m_properties.end() == m_properties.find(propName))
                {
                    m_properties[propName] =  property;
                }
            }

            /**
                 * This method is for getting Properties from Items.
                 *
                 * @param propName - Properties name as string.
                 *
                 * @return  pointer to Properties to put the value got
                 */
            std::shared_ptr<Properties> getproperty(const std::string &propName)
            {
                if (m_properties.end() != m_properties.find(propName))
                {
                    return m_properties[propName];
                }
                return nullptr;
            }

            /**
                 * This method is for getting Properties from Items.
                 *
                 * @return map of Properties name as string and pointer to Properties
                 */
            std::map<std::string, std::shared_ptr<Properties> > const &getProperties()
            {
                return m_properties;
            }

            /**
                 * This method is for setting Type to Items
                 *
                 * @param type - Type as string.
                 */
            void setType(const std::string &type)
            {
                m_type = type;
            }

            /**
                 * This method is for getting Type from Items.
                 *
                 * @return Type as string
                 */
            std::string getType()
            {
                return m_type;
            }

            /**
                 * This method is for setting RequiredValue to Items
                 *
                 * @param reqValue - RequiredValue as string.
                 */
            void setRequiredValue(const std::string &reqValue)
            {
                auto it = m_required.begin();
                for (; it != m_required.end(); ++it)
                {
                    if (*it == reqValue)
                        break;
                }
                if (m_required.end() == it)
                {
                    m_required.push_back(reqValue);
                }
            }

            /**
                 * This method is for getting RequiredValue from Items.
                 *
                 * @return list of RequiredValue as string
                 */
            std::vector<std::string> const &getRequiredValues()
            {
                return m_required;
            }

            /**
                 * This method is for setting AllowedValues to Items
                 *
                 * @param values -list of AllowedValues.
                 */
            template <typename T>
            bool setAllowedValues(const std::vector<T> &values)
            {
                m_allowedValues.addValues(values);
                return true;
            }

            /**
                 * This method is for getting size of AllowedValues from Items.
                 *
                 * @return size of AllowedValues
                 */
            inline int getAllowedValuesSize() const
            {
                return m_allowedValues.size();
            }

            /**
                 * This method is for getting AllowedValues from Items.
                 *
                 * @return list of AllowedValues
                 */
            inline std::vector<ValueVariant> getAllowedValues()
            {
                return m_allowedValues.getValues();
            }

            /**
                 * This method is for getting AllowedValues as Integer from Items.
                 *
                 * @return list of AllowedValues as Integer
                 */
            inline std::vector<int> getAllowedValuesInt()
            {
                return m_allowedValues.getValuesInt();
            }

            /**
                 * This method is for getting AllowedValues as String from Items.
                 *
                 * @return list of AllowedValues as String
                 */
            inline std::vector<std::string> getAllowedValuesString()
            {
                return m_allowedValues.getValuesString();
            }
        private:
            std::map<std::string, std::shared_ptr<Properties> > m_properties;
            std::string m_type;
            std::vector<std::string>  m_required;
            AllowedValues m_allowedValues;
    };

    /** ItemsPtr - shared Ptr to Items.*/
    typedef std::shared_ptr<Items> ItemsPtr;
}
#endif


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
 * @file   JsonSchema.h
 *
 * @brief   This file provides data Model for Json Schema file.
 */

#ifndef JSON_SCHEMA_H_
#define JSON_SCHEMA_H_

#include <string>
#include <vector>
#include <map>
#include "Properties.h"
#include "Items.h"
#include "Definitions.h"
#include "cJSON.h"
#include "Helpers.h"
#include "AllowedValues.h"
#include <memory>

#include "IncludeResolver.h"

namespace RAML
{
    /**
     * @class   JsonSchema
     * @brief   This class provides data Model for Json Schema file.
     */
    class JsonSchema
    {
        public:
            /**
                  * Constructor of JsonSchema.
                  */
            JsonSchema() : m_cjson(NULL), m_includeResolver(NULL)  {}

            /**
                  * Constructor of JsonSchema.
                  *
                  * @param includeResolver - Reference to IncludeResolver for reading external files
                  */
            JsonSchema(const IncludeResolverPtr &includeResolver) : m_cjson(NULL),
                m_includeResolver(includeResolver) {}

            /**
                  * Constructor of JsonSchema.
                  *
                  * @param cjson - pointer to cjson
                  * @param includeResolver - Reference to IncludeResolver for reading external files
                  */
            JsonSchema(cJSON *cjson , const IncludeResolverPtr &includeResolver) : m_cjson(cjson),
                m_includeResolver(includeResolver)  { readJson(); }


            /**
                 * This method is for setting cJson pointer to JsonSchema.
                 *
                 * @param cjson -pointer to cJson
                 */
            void setcJson(cJSON *cjson) {m_cjson = cjson; readJson(); }

            /**
                 * This method is for getting size of Properties from JsonSchema.
                 *
                 * @return  size of Properties
                 */
            int size() const
            {
                return m_properties.size();
            }

            /**
                 * This method is for getting Properties from JsonSchema.
                 *
                 * @param name -name of property as string
                 *
                 * @return pointer to Properties
                 */
            inline PropertiesPtr getProperty(const std::string &name)
            {
                if (m_properties.end() != m_properties.find(name))
                {
                    return m_properties[name];
                }
                return nullptr;
            }

            /**
                 * This method is for getting Properties from JsonSchema.
                 *
                 * @return map of Properties name and pointer to Properties
                 */
            inline std::map<std::string, PropertiesPtr > const &getProperties()
            {
                return m_properties;
            }

            /**
                 * This method is for getting Definitions from JsonSchema.
                 *
                 * @return map of Definitions name and pointer to Definitions
                 */
            inline std::map<std::string, DefinitionsPtr > const &getDefinitions()
            {
                return m_definition;
            }

            /**
                 * This method is for setting Properties to JsonSchema.
                 *
                 * @param name -name of property as string
                 * @param property -pointer to Properties
                 */
            void addProperty(const std::string &name, const PropertiesPtr &property)
            {
                if (m_properties.end() == m_properties.find(name))
                {
                    m_properties[name] = property;
                }
            }

            /**
                 * This method is for setting RequiredValue to JsonSchema.
                 *
                 * @param reqValue -name of RequiredValue as string
                 */
            void setRequiredValue(const std::string &reqValue)
            {
                if (m_required.end() == std::find(m_required.begin(), m_required.end(), reqValue))
                {
                    m_required.push_back(reqValue);
                }
            }

            /**
                 * This method is for getting RequiredValues from JsonSchema.
                 *
                 * @return vector of RequiredValues as string
                 */
            std::vector<std::string> const &getRequiredValues()
            {
                return m_required;
            }

            /**
                 * This method is for setting Definitions to JsonSchema.
                 *
                 * @param defName -name of Definitions as string
                 * @param definition -pointer to Definitions
                 */
            void addDefinition(const std::string &defName, const DefinitionsPtr &definition)
            {
                if (m_definition.end() == m_definition.find(defName))
                {
                    m_definition[defName] = definition;
                }
            }

            /**
                 * This method is for getting Definitions from JsonSchema.
                 *
                 * @param defName -Definition name  as string
                 *
                 * @return pointer to Definitions
                 */
            DefinitionsPtr getDefinition(const std::string &defName)
            {
                if (m_definition.end() != m_definition.find(defName))
                {
                    return m_definition[defName];
                }
                return nullptr;
            }

            /**
                 * This method is for getting Type from JsonSchema.
                 *
                 * @return JsonSchema Type as string
                 */
            std::string getType()
            {
                return  m_type;
            }

            /**
                 * This method is for getting Id from JsonSchema.
                 *
                 * @return JsonSchema Id as string
                 */
            std::string getId()
            {
                return  m_id;
            }

            /**
                 * This method is for getting Schema from JsonSchema.
                 *
                 * @return  Schema as string
                 */
            std::string getSchema()
            {
                return  m_schema;
            }

            /**
                 * This method is for getting Description from JsonSchema.
                 *
                 * @return JsonSchema Description as string
                 */
            std::string getDescription()
            {
                return  m_description;
            }

            /**
                 * This method is for getting Title from JsonSchema.
                 *
                 * @return JsonSchema Title as string
                 */
            std::string getTitle()
            {
                return  m_title;
            }

            /**
                 * This method is for getting AdditionalProperties from JsonSchema.
                 *
                 * @return AdditionalProperties as bool
                 */
            bool getAdditionalProperties()
            {
                return  m_additionalProperties;
            }

            /**
                 * This method is for setting Items to JsonSchema.
                 *
                 * @param item -pointer to Items
                 */
            void setItem(const ItemsPtr &item)
            {
                m_items.push_back(item);
            }

            /**
                 * This method is for getting Items from JsonSchema.
                 *
                 * @return vector of Items
                 */
            std::vector<ItemsPtr> const &getItems()
            {
                return m_items;
            }

        private:
            void readJson();
            DefinitionsPtr readDef(cJSON *childDefinitions, const std::string &defName);
            PropertiesPtr readProp(cJSON *childProperties, const std::string &attName );
            void readValues( cJSON *childProperties,  PropertiesPtr property ,
                             const std::string &attType);
            void readString( cJSON *childProperties, PropertiesPtr property);
            void readArray( cJSON *childProperties,  PropertiesPtr property);
            void readInteger( cJSON *childProperties,  PropertiesPtr property);
            void readDouble( cJSON *childProperties,  PropertiesPtr property);
            DefinitionsPtr readRef(std::string m_ref);


            void readJsonRef(cJSON *jsonReference);
            void readDefRef(cJSON *defReference, DefinitionsPtr definition);
            void readAllOf(cJSON *allofValues);
            void readDefAllOf(cJSON *allofValues, DefinitionsPtr definition);
            ItemsPtr readItems(cJSON *item);
            void readItemRef(cJSON *itemReference, ItemsPtr item);
            void readItemAllOf(cJSON *allofValues,  ItemsPtr item);

        private:
            std::map<std::string, PropertiesPtr > m_properties;
            std::map<std::string, DefinitionsPtr > m_definition;
            std::string m_id;
            std::string m_schema;
            std::string m_title;
            std::string m_description;
            bool m_additionalProperties;
            std::string m_type;
            cJSON *m_cjson;
            std::vector<std::string>  m_required;
            std::vector<ItemsPtr > m_items;
            IncludeResolverPtr m_includeResolver;
    };

    /** JsonSchemaPtr - shared Ptr to JsonSchema.*/
    typedef std::shared_ptr<JsonSchema> JsonSchemaPtr;

}

#endif

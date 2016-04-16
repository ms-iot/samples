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
 * @file   Schema.h
 *
 * @brief   This file provides data Model for RAML Schema.
 */

#ifndef SCHEMAS_H
#define SCHEMAS_H

#include <string>
#include "cJSON.h"
#include "IncludeResolver.h"

#include "JsonSchema.h"

namespace RAML
{
    /**
     * @class   Schema
     * @brief   This class provides data Model for RAML Schema.
     */
    class Schema
    {
        public:
            /**
                 * This method is for getting CJson object of schema.
                 *
                 * @return pointer to cJSON.
                 */
            virtual cJSON *getJson() const;

            /**
                 * This method is for setting schema as CJson object.
                 *
                 * @param cjson - Cjson pointer.
                 */
            virtual void setJson(cJSON *cjson);

            /**
                 * This method is for getting schema as string.
                 *
                 * @return string.
                 */
            virtual std::string getSchema() const;

            /**
                  * This method is for setting schema as string.
                  *
                  * @param schema - schema string.
                  */
            virtual void setSchema(const std::string &schema);

            /**
                 * This method is for getting Properties from JsonSchema.
                 *
                 * @return pointer to JsonSchema.
                 */
            virtual JsonSchemaPtr const &getProperties() const;

            /**
                   * Constructor of Schema.
                   *
                   * @param schema - contents of schema to be parsed
                   * @param includeResolver - Reference to IncludeResolver for reading external files
                   *
                   */
            Schema(const std::string &schema, const IncludeResolverPtr &includeResolver):
                m_schema(schema) , m_cjson(cJSON_Parse(schema.c_str())),
                m_resProperties(std::make_shared<JsonSchema>(m_cjson, includeResolver) ) ,
                m_includeResolver(includeResolver) {}

            /**
                  * Constructor of Schema.
                  */
            Schema(): m_cjson(NULL), m_resProperties(std::make_shared<JsonSchema>()),
                m_includeResolver(NULL) {}

            /**
                  * Destructor of Schema.
                  */
            ~Schema() { cJSON_Delete(m_cjson); }

        private:
            std::string m_schema;
            cJSON *m_cjson;
            JsonSchemaPtr m_resProperties;
            IncludeResolverPtr m_includeResolver;
    };

    /** SchemaPtr - shared Ptr to Schema.*/
    typedef std::shared_ptr<Schema> SchemaPtr;

}
#endif

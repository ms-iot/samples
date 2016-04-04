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
 * @file   RequestResponseBody.h
 *
 * @brief   This file provides data Model for RAML RequestResponseBody.
 */

#ifndef REQUEST_RESPONSE_BODY_H
#define REQUEST_RESPONSE_BODY_H

#include <map>
#include <list>
#include "FormParameter.h"
#include "Utils.h"
#include "IncludeResolver.h"
#include "Schema.h"

namespace RAML
{
    /**
     * @class   RequestResponseBody
     * @brief   This class provides data Model for RAML RequestResponseBody.
     */
    class RequestResponseBody
    {
        public:
            /**
                 * This method is for getting Type from RequestResponseBody.
                 *
                 * @return type as string.
                 */
            virtual std::string getType() const;

            /**
                 * This method is for setting Type to RequestResponseBody.
                 *
                 * @param type - type of RequestResponseBody
                 */
            virtual void setType(const std::string &type);

            /**
                 * This method is for getting Schema from RequestResponseBody.
                 *
                 * @return pointer to Schema.
                 */
            virtual SchemaPtr const &getSchema() const;

            /**
                 * This method is for setting Schema to RequestResponseBody.
                 *
                 * @param schema - pointer to Schema
                 */
            virtual void setSchema(const SchemaPtr &schema);

            /**
                 * This method is for getting Example from RequestResponseBody.
                 *
                 * @return Example as string.
                 */
            virtual std::string getExample() const;

            /**
                 * This method is for setting Example to RequestResponseBody.
                 *
                 * @param example - Example as string
                 */
            virtual void setExample(const std::string &example);

            /**
                 * This method is for getting FormParameter from RequestResponseBody.
                 *
                 * @return map of pointer to param Name and pointer to FormParameter.
                 */
            virtual std::map<std::string, FormParameterPtr> const &getFormParameters() const;

            /**
                 * This method is for setting FormParameter to RequestResponseBody.
                 *
                 * @param paramName - Name of FormParameter
                 * @param formParameter - pointer to FormParameter
                 */
            virtual void setFormParameter(const std::string &paramName,
                                          const FormParameterPtr &formParameter);

            /**
                  * Constructor of RequestResponseBody.
                  */
            RequestResponseBody(): m_schema(NULL), m_includeResolver(NULL) {}

            /**
                   * Constructor of RequestResponseBody.
                   *
                   * @param type - type of request response body
                   *
                   */
            RequestResponseBody(const std::string type) : m_type(type), m_schema(NULL),
                m_includeResolver(NULL) {}

            /**
                   * Constructor of RequestResponseBody.
                   *
                   * @param type - type of request response body
                   * @param yamlNode - Reference to YamlNode for reading the RequestResponseBody
                   * @param includeResolver - Reference to IncludeResolver for reading external files
                   *
                   */
            RequestResponseBody(const std::string type, const YAML::Node &yamlNode,
                                const IncludeResolverPtr &includeResolver): m_schema(NULL),
                m_includeResolver(includeResolver)
            { readRequestResponseBody(type, yamlNode); }

        private:
            virtual void readRequestResponseBody(const std::string &type,
                                                 const YAML::Node &yamlNode) ;
        private:
            std::string m_type;
            SchemaPtr m_schema;
            std::string m_example;
            std::map<std::string, FormParameterPtr> m_formParameters;
            IncludeResolverPtr m_includeResolver;
    };

    /** RequestResponseBodyPtr - shared Ptr to RequestResponseBody.*/
    typedef std::shared_ptr<RequestResponseBody> RequestResponseBodyPtr;
}
#endif

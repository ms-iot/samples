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
 * @file   Response.h
 *
 * @brief   This file provides data Model for RAML Response.
 */

#ifndef RESPONSE_H
#define RESPONSE_H

#include <map>
#include <list>
#include <string>
#include "RequestResponseBody.h"
#include "Header.h"
#include "Utils.h"
#include "IncludeResolver.h"

namespace RAML
{
    /**
     * @class   Response
     * @brief   This class provides data Model for RAML Response.
     */
    class Response
    {
        public:
            /**
                 * This method is for getting Headers from Response.
                 *
                 * @return map of headerName and Pointer to Header.
                 */
            virtual std::map<std::string, HeaderPtr> const &getHeaders() const;

            /**
                 * This method is for setting Header to Response.
                 *
                 * @param headerName - header Name
                 * @param header - pointer to Header Object.
                 */
            virtual void setHeader(const std::string &headerName, const HeaderPtr &header);

            /**
                 * This method is for getting Description from Response.
                 *
                 * @return Description string.
                 */
            virtual std::string getDescription() const;

            /**
                 * This method is for setting Description to Response.
                 *
                 * @param description - description string
                 */
            virtual void setDescription(const std::string &description);

            /**
                 * This method is for setting ResponseBody to Response.
                 *
                 * @param typeName - response body typeName
                 */
            virtual void setResponseBody(const std::string &typeName);

            /**
                 * This method is for setting ResponseBody to Response.
                 *
                 * @param type - response body typeName
                 * @param body - Pointer to RequestResponseBody
                 */
            virtual void setResponseBody(const std::string &type, const RequestResponseBodyPtr &body) ;

            /**
                 * This method is for getting ResponseBody from Response.
                 *
                 * @return map of body type and Pointer to RequestResponseBody.
                 */
            virtual std::map<std::string, RequestResponseBodyPtr> const &getResponseBody() const;

            /**
                 * This method is for getting ResponseBody from Response.
                 *
                 * @param bodyType - response body type
                 *
                 * @return  Pointer to RequestResponseBody.
                 */
            virtual RequestResponseBodyPtr getResponseBody(const std::string &bodyType);

            /**
                  * Constructor of Response.
                  */
            Response() : m_includeResolver(NULL) {}

            /**
                   * Constructor of Response.
                   *
                   * @param yamlNode - Reference to YamlNode for reading the Response
                   * @param includeResolver - Reference to IncludeResolver for reading external files
                   *
                   */
            Response(const YAML::Node &yamlNode,
                     const IncludeResolverPtr &includeResolver): m_includeResolver(includeResolver)
            { readResponse(yamlNode);}
        private:
            void readResponse(const YAML::Node &yamlNode) ;
        private:
            std::string m_description;
            std::map<std::string, RequestResponseBodyPtr> m_responseBody;
            std::map<std::string, HeaderPtr> m_headers;
            IncludeResolverPtr m_includeResolver;
    };

    /** ResponsePtr - shared Ptr to Response.*/
    typedef std::shared_ptr<Response> ResponsePtr;
}
#endif

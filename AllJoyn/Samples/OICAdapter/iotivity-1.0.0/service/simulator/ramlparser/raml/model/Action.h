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
 * @file   Action.h
 *
 * @brief   This file provides data Model for RAML Action.
 */

#ifndef ACTION_H
#define ACTION_H

#include <map>
#include <list>
#include <string>
#include "ActionType.h"
#include "Header.h"
#include "QueryParameter.h"
#include "RequestResponseBody.h"
#include "UriParameter.h"
#include "Response.h"
#include "Utils.h"
#include "IncludeResolver.h"

namespace RAML
{
    /**
     * @class   Action
     * @brief   This class provides data Model for RAML Action.
     */
    class Action
    {
        public:
            /**
                 * This method is for getting Type from Action.
                 *
                 * @return Type as string.
                 */
            virtual ActionType getType() const;

            /**
                 * This method is for setting Type to Action.
                 *
                 * @param type - Type as string
                 */
            virtual void setType(const ActionType &type);

            /**
                 * This method is for getting Description from Action.
                 *
                 * @return Description as string.
                 */
            virtual std::string getDescription() const;

            /**
                 * This method is for setting Description to Action.
                 *
                 * @param description - Description as string
                 */
            virtual void setDescription(const std::string &description);

            /**
                 * This method is for getting Header from Action.
                 *
                 * @return map of Header name and Pointer to Header.
                 */
            virtual std::map<std::string, HeaderPtr > const &getHeaders() const;

            /**
                 * This method is for setting Header to Action.
                 *
                 * @param headerName - Header name as string
                 * @param header - Pointer to Header
                 */
            virtual void setHeader(const std::string &headerName, const HeaderPtr &header);

            /**
                 * This method is for getting QueryParameter from Action.
                 *
                 * @return map of QueryParameter name and Pointer to QueryParameter.
                 */
            virtual std::map<std::string, QueryParameterPtr > const &getQueryParameters()const;

            /**
                 * This method is for setting QueryParameter to Action.
                 *
                 * @param paramName - QueryParameter name as string
                 * @param queryParameter - Pointer to QueryParameter
                 */
            virtual void setQueryParameter(const std::string &paramName,
                                           const QueryParameterPtr &queryParameter);

            /**
                 * This method is for getting RequestResponseBody from Action.
                 *
                 * @param bodyType - bodyType name as string
                 *
                 * @return Pointer to RequestResponseBody
                 */
            virtual RequestResponseBodyPtr getRequestBody(const std::string &bodyType);

            /**
                 * This method is for getting RequestResponseBody from Action.
                 *
                 * @return map of RequestResponseBody name and Pointer to RequestResponseBody.
                 */
            virtual std::map<std::string, RequestResponseBodyPtr> const &getRequestBody() const;

            /**
                 * This method is for setting RequestResponseBody to Action.
                 *
                 * @param typeName - RequestResponseBody name as string
                 */
            virtual void setRequestBody(const std::string &typeName);

            /**
                 * This method is for setting Type to Action.
                 *
                 * @param typeName - RequestResponseBody name as string
                 * @param body - Pointer to RequestResponseBody
                 */
            virtual void setRequestBody(const std::string &typeName ,
                                        const RequestResponseBodyPtr &body);

            /**
                 * This method is for getting Response from Action.
                 *
                 * @param responseCode - Response code as string
                 *
                 * @return Pointer to Response
                 */
            virtual ResponsePtr getResponse(const std::string &responseCode);

            /**
                 * This method is for getting Response from Action.
                 *
                 * @return map of response code and Pointer to Response
                 */
            virtual std::map<std::string, ResponsePtr> const &getResponses() const;

            /**
                 * This method is for setting Response to Action.
                 *
                 * @param responseCode - responseCode as string
                 * @param response - Pointer to Response
                 */
            virtual void setResponse(const std::string &responseCode, const ResponsePtr &response);

            /**
                 * This method is for getting Protocols from Action.
                 *
                 * @return list of Protocols as string.
                 */
            virtual std::list<std::string> const &getProtocols() const;

            /**
                 * This method is for setting Protocols to Action.
                 *
                 * @param protocol - protocol as string
                 */
            virtual void setProtocol(const std::string &protocol);

            /**
                 * This method is for getting BaseUriParameter from Action.
                 *
                 * @return map of BaseUriParameter name and pointer to UriParameter.
                 */
            virtual std::map< std::string, UriParameterPtr > const &getBaseUriParameters() const;

            /**
                 * This method is for setting BaseUriParameter to Action.
                 *
                 * @param paramName - BaseUriParameter name as string
                 * @param baseUriParameter - pointer to UriParameter.
                 */
            virtual void setBaseUriParameter(const std::string &paramName ,
                                             const UriParameterPtr &baseUriParameter);

            /**
                 * This method is for getting Traits from Action.
                 *
                 * @return list of traits as string.
                 */
            virtual std::list<std::string> const &getTraits() const;

            /**
                 * This method is for setting Trait to Action.
                 *
                 * @param trait - trait as string
                 */
            virtual void setTrait(const std::string &trait);

            /**
                  * Constructor of Action.
                  */
            Action(): m_includeResolver(NULL) { }

            /**
                   * Constructor of Action.
                   *
                   * @param actionType - type of action in ActionType enum
                   * @param yamlNode - Reference to YamlNode for reading the Action
                   * @param includeResolver - Reference to IncludeResolver for reading external files
                   *
                   */
            Action(const ActionType actionType, const YAML::Node &yamlNode,
                   const IncludeResolverPtr &includeResolver)
                : m_includeResolver(includeResolver)
            {
                readAction(actionType, yamlNode);
            }

            /**
                  * copy Constructor of Action.
                  */
            Action(const Action &) = default;
        private:
            virtual void readAction(const ActionType actionType, const YAML::Node &yamlNode);


        private:
            ActionType m_type;
            std::string m_description;
            std::map<std::string, HeaderPtr> m_headers;
            std::map<std::string, QueryParameterPtr> m_queryParameters;
            std::map<std::string, RequestResponseBodyPtr> m_requestBody;
            std::map<std::string, ResponsePtr> m_responses;
            std::list<std::string> m_protocols;
            std::map< std::string, UriParameterPtr > m_baseUriParameters;
            std::list<std::string> m_trait;

        private:
            IncludeResolverPtr m_includeResolver;
    };

    /** ActionPtr - shared Ptr to Action.*/
    typedef std::shared_ptr<Action> ActionPtr;
}
#endif

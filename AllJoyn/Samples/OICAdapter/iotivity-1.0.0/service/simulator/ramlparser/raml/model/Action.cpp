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

#include "Action.h"

namespace RAML
{
    ActionType Action::getType() const
    {
        return m_type;
    }
    void Action::setType(const ActionType &type)
    {
        m_type = type;
    }
    std::string Action::getDescription() const
    {
        return m_description;
    }
    void Action::setDescription(const std::string &description)
    {
        m_description = description;
    }
    std::map<std::string, HeaderPtr > const &Action::getHeaders() const
    {
        return m_headers;
    }
    void Action::setHeader(const std::string &headerName, const HeaderPtr &header)
    {
        m_headers[headerName] = header;
    }
    std::map<std::string, QueryParameterPtr > const &Action::getQueryParameters()const
    {
        return m_queryParameters;
    }
    void Action::setQueryParameter(const std::string &paramName,
                                   const QueryParameterPtr &queryParameter)
    {
        m_queryParameters[paramName] = queryParameter;
    }
    RequestResponseBodyPtr Action::getRequestBody(const std::string &bodyType)
    {
        return m_requestBody[bodyType];
    }

    std::map<std::string, RequestResponseBodyPtr> const &Action::getRequestBody() const
    {
        return m_requestBody;
    }
    void Action::setRequestBody(const std::string &typeName)
    {
        m_requestBody[typeName] = std::make_shared<RequestResponseBody>(typeName);
    }

    void Action::setRequestBody(const std::string &typeName , const RequestResponseBodyPtr &body)
    {
        m_requestBody[typeName] = body;
    }
    ResponsePtr Action::getResponse(const std::string &responseCode)
    {
        return m_responses[responseCode];
    }

    std::map<std::string, ResponsePtr> const &Action::getResponses() const
    {
        return m_responses;
    }
    void Action::setResponse(const std::string &responseCode, const ResponsePtr &response)
    {
        m_responses[responseCode] = response;
    }

    std::list<std::string> const &Action::getProtocols() const
    {
        return m_protocols;
    }
    void Action::setProtocol(const std::string &protocol)
    {
        m_protocols.push_back(protocol);
    }
    std::map<std::string, UriParameterPtr > const &Action::getBaseUriParameters() const
    {
        return m_baseUriParameters;
    }
    void Action::setBaseUriParameter(const std::string &paramName ,
                                     const UriParameterPtr &baseUriParameter)
    {
        m_baseUriParameters[paramName] = baseUriParameter;
    }

    std::list<std::string> const &Action::getTraits() const
    {
        return m_trait;
    }
    void Action::setTrait(const std::string &trait)
    {
        m_trait.push_back(trait);
    }
    void Action::readAction(const ActionType actionType, const YAML::Node &yamlNode)
    {
        m_type = actionType;
        for ( YAML::const_iterator it = yamlNode.begin(); it != yamlNode.end(); ++it )
        {
            std::string key = READ_NODE_AS_STRING(it->first);

            if (key == Keys::Description)
                setDescription(READ_NODE_AS_STRING(it->second));
            else if (key == Keys::Responses)
            {
                YAML::Node responseNode = it->second;
                for ( YAML::const_iterator tt = responseNode.begin(); tt != responseNode.end(); ++tt )
                {
                    std::string responseCode = READ_NODE_AS_STRING(tt->first);
                    setResponse(responseCode, std::make_shared<Response>(tt->second,
                                m_includeResolver));
                }
            }
            else if (key == Keys::Headers)
            {
                YAML::Node paramNode = it->second;
                for ( YAML::const_iterator tt = paramNode.begin(); tt != paramNode.end(); ++tt )
                {
                    setHeader(READ_NODE_AS_STRING(tt->first), std::make_shared<Header>(tt->second));
                }
            }
            else if (key == Keys::QueryParameters)
            {
                YAML::Node paramNode = it->second;
                for ( YAML::const_iterator tt = paramNode.begin(); tt != paramNode.end(); ++tt )
                {
                    setQueryParameter(READ_NODE_AS_STRING(tt->first),
                                      std::make_shared<QueryParameter>(tt->second));
                }
            }
            else if (key == Keys::Protocols)
            {
                YAML::Node protocolNode = it->second;
                for ( YAML::const_iterator tt = protocolNode.begin(); tt != protocolNode.end(); ++tt )
                {
                    setProtocol(READ_NODE_AS_STRING(*tt));
                }
            }
            else if (key == Keys::BaseUriParameters)
            {
                YAML::Node paramNode = it->second;
                for ( YAML::const_iterator tt = paramNode.begin(); tt != paramNode.end(); ++tt )
                {
                    setBaseUriParameter(READ_NODE_AS_STRING(tt->first),
                                        std::make_shared<UriParameter>(tt->second));
                }
            }
            else if (key == Keys::Body)
            {
                YAML::Node responseBody = it->second;

                for ( YAML::const_iterator tt = responseBody.begin(); tt != responseBody.end(); ++tt )
                {
                    std::string type = READ_NODE_AS_STRING(tt->first);
                    setRequestBody(type, std::make_shared<RequestResponseBody>(type, tt->second,
                                   m_includeResolver));
                }
            }
            else if (key == Keys::IsTrait)
            {
                YAML::Node traitNode = it->second;
                for ( YAML::const_iterator tt = traitNode.begin(); tt != traitNode.end(); ++tt )
                {
                    setTrait(READ_NODE_AS_STRING(*tt));
                }
            }
        }
    }

};


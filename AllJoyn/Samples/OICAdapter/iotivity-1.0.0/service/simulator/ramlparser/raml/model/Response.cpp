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

#include "Response.h"

namespace RAML
{

    std::map<std::string, HeaderPtr> const &Response::getHeaders() const
    {
        return m_headers;
    }
    void Response::setHeader(const std::string &headerName, const HeaderPtr &header)
    {
        m_headers[headerName] = header;
    }
    std::string Response::getDescription() const
    {
        return m_description;
    }
    void Response::setDescription(const std::string &description)
    {
        m_description = description;
    }
    void Response::setResponseBody(const std::string &typeName)
    {
        m_responseBody[typeName] = std::make_shared<RequestResponseBody>(typeName);
    }
    void Response::setResponseBody(const std::string &type, const RequestResponseBodyPtr &body)
    {
        m_responseBody[type] = body;
    }
    std::map<std::string, RequestResponseBodyPtr> const &Response::getResponseBody() const
    {
        return m_responseBody;
    }
    RequestResponseBodyPtr Response::getResponseBody(const std::string &bodyType)
    {
        return m_responseBody[bodyType];
    }

    void Response::readResponse(const YAML::Node &yamlNode)
    {
        for ( YAML::const_iterator it = yamlNode.begin(); it != yamlNode.end(); ++it )
        {
            std::string key = READ_NODE_AS_STRING(it->first);

            if (key == Keys::Description)
                setDescription(READ_NODE_AS_STRING(it->second));
            else if (key == Keys::Body)
            {
                YAML::Node responseBody = it->second;

                for ( YAML::const_iterator tt = responseBody.begin(); tt != responseBody.end(); ++tt )
                {
                    std::string type = READ_NODE_AS_STRING(tt->first);
                    setResponseBody(type, std::make_shared<RequestResponseBody>(type, tt->second,
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
        }
    }

}

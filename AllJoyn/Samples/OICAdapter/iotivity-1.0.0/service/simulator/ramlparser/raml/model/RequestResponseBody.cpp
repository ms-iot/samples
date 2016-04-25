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

#include "RequestResponseBody.h"

namespace RAML
{

    std::string RequestResponseBody::getType() const
    {
        return m_type;
    }
    void RequestResponseBody::setType(const std::string &type)
    {
        m_type = type;
    }
    SchemaPtr const &RequestResponseBody::getSchema() const
    {
        return m_schema;
    }
    void RequestResponseBody::setSchema(const SchemaPtr &schema)
    {
        m_schema = schema;
    }
    std::string RequestResponseBody::getExample() const
    {
        return m_example;
    }
    void RequestResponseBody::setExample(const std::string &example)
    {
        m_example = example;
    }
    std::map<std::string, FormParameterPtr > const &RequestResponseBody::getFormParameters() const
    {
        return m_formParameters ;
    }
    void RequestResponseBody::setFormParameter(const std::string &paramName,
            const FormParameterPtr  &formParameter)
    {
        m_formParameters[paramName] = formParameter;
    }
    void RequestResponseBody::readRequestResponseBody(const std::string &type,
            const YAML::Node &yamlNode)
    {
        m_type = type;
        for ( YAML::const_iterator it = yamlNode.begin(); it != yamlNode.end(); ++it )
        {
            std::string key = READ_NODE_AS_STRING(it->first);

            if (key == Keys::Schema)
            {
                IncludeResolver::FileType fileType = m_includeResolver->getFileType(it->second);
                if ((fileType == IncludeResolver::FileType::JSON) ||
                    (fileType == IncludeResolver::FileType::FILE))
                {
                    setSchema(std::make_shared<Schema>(m_includeResolver->readFromFile(it->second),
                                                       m_includeResolver));
                }
                else
                {
                    std::string value = READ_NODE_AS_STRING(it->second);
                    setSchema(std::make_shared<Schema>(value, m_includeResolver));
                }
            }
            else if (key == Keys::Example)
                setExample(READ_NODE_AS_STRING(it->second));
            else if (key == Keys::FormParameters)
            {
                YAML::Node paramNode = it->second;
                for ( YAML::const_iterator tt = paramNode.begin(); tt != paramNode.end(); ++tt )
                {
                    setFormParameter(READ_NODE_AS_STRING(tt->first),
                                     std::make_shared<FormParameter>(tt->second));
                }
            }
        }
    }

}

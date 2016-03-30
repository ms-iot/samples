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

#include "RamlParser.h"
#include <map>

namespace RAML
{
    RamlPtr RamlParser::getRamlPtr(RamlParserResult &result)
    {
        result = m_ramlParserResult;
        return m_ramlPtr;
    }
    RamlPtr RamlParser::getRamlPtr()
    {
        return m_ramlPtr;
    }
    void RamlParser::setDataFromRoot()
    {
        setTypes(getRamlPtr()->getResources());
        setTraits(getRamlPtr()->getResources());
        setBodyDefaultMediaType(getRamlPtr()->getResources());
        setBodySchema(getRamlPtr()->getResources());
    }
    void RamlParser::setBodyDefaultMediaType(const std::map<std::string, RamlResourcePtr> &resource)
    {
        if (getRamlPtr()->getMediaType().empty())
        {
            return;
        }
        for (auto const & it : resource)
        {
            std::string type = getRamlPtr()->getMediaType();

            for (auto const & action :  it.second->getActions())
            {
                if (action.second->getRequestBody().empty())
                {
                    action.second->setRequestBody(type);
                }
                for (auto const & response : action.second->getResponses())
                {
                    if (response.second->getResponseBody().empty())
                    {
                        response.second->setResponseBody(type);
                    }
                }
            }
            setBodyDefaultMediaType(it.second->getResources());
        }
    }
    void RamlParser::setBodySchema(const std::map<std::string, RamlResourcePtr> &resource)
    {
        if (getRamlPtr()->getSchemas().empty())
        {
            return;
        }
        for (auto const & it : resource)
        {
            for (auto const & action :  it.second->getActions())
            {
                for (auto const & body :  action.second->getRequestBody())
                {
                    SchemaPtr schema = body.second->getSchema();

                    if (schema != NULL)
                    {
                        std::string schemaValue = schema->getSchema();
                        auto pos = std::find_if(getRamlPtr()->getSchemas().begin(), getRamlPtr()->getSchemas().end(),
                                                [schemaValue](std::pair<std::string, SchemaPtr> const & pair)
                        {
                            return (pair.first == schemaValue);
                        });
                        if (pos != getRamlPtr()->getSchemas().end())
                        {
                            schema->setSchema((pos->second->getSchema()));
                        }
                    }
                }
                for (auto const & response : action.second->getResponses())
                {
                    for (auto const & body :  response.second->getResponseBody())
                    {
                        SchemaPtr schema = body.second->getSchema();
                        if (schema != NULL)
                        {
                            std::string schemaValue = schema->getSchema();
                            auto schemas = getRamlPtr()->getSchemas();

                            auto iter = schemas.begin();
                            for (; iter != schemas.end(); iter++)
                            {
                                if (((*iter).first) == schemaValue)
                                    break;
                            }
                            if (iter != schemas.end())
                            {
                                schema->setSchema((*iter).second->getSchema());
                            }
                        }
                    }
                }
            }
            setBodySchema(it.second->getResources());
        }
    }
    void RamlParser::setTypes(const std::map<std::string, RamlResourcePtr> &resource)
    {
        if (getRamlPtr()->getResourceTypes().empty())
        {
            return;
        }
        for (auto const & it : resource)
        {
            auto const &resourceTypes = getRamlPtr()->getResourceTypes();
            std::string typeValue = it.second->getResourceType();

            auto iter = resourceTypes.begin();
            for (; iter != resourceTypes.end(); iter++)
            {
                if (((*iter).first) == typeValue)
                    break;
            }
            if (iter != resourceTypes.end())
            {
                if ((*iter).second->getActions().empty())
                    return;

                for (auto resActions : (*iter).second->getActions())
                {
                    if (it.second->getActions().count(resActions.first) == 0)
                        it.second->setAction(resActions.first, std::make_shared<Action>(*(resActions.second)));
                }
            }
            setTypes(it.second->getResources());
        }
    }
    void RamlParser::setTraits(const std::map<std::string, RamlResourcePtr> &resource)
    {
        if (getRamlPtr()->getTraits().empty())
        {
            return;
        }
        for (auto const & it : resource)
        {
            auto const &trait = getRamlPtr()->getTraits();
            for (auto const & act : it.second->getActions())
            {
                for (const std::string & traitValue :  act.second->getTraits())
                {
                    auto iter = trait.begin();
                    for (; iter != trait.end(); iter++)
                    {
                        if (((*iter).first) == traitValue)
                            break;
                    }
                    if (iter != trait.end())
                    {
                        for (auto head : (*iter).second->getHeaders())
                        {
                            if (act.second->getHeaders().count(head.first) == 0)
                                act.second->setHeader(head.first, head.second);
                        }
                        for (auto query : (*iter).second->getQueryParameters())
                        {
                            if (act.second->getQueryParameters().count(query.first) == 0)
                                act.second->setQueryParameter(query.first, query.second);
                        }
                        for (auto resp : (*iter).second->getResponses())
                        {
                            if (act.second->getResponses().count(resp.first) == 0)
                                act.second->setResponse(resp.first, resp.second);
                        }
                    }
                }
            }
            for (const std::string & traitValue :  it.second->getTraits())
            {
                auto iter = trait.begin();
                for (; iter != trait.end(); iter++)
                {
                    if (((*iter).first) == traitValue)
                        break;
                }
                if (iter != trait.end())
                {
                    for (auto act : it.second->getActions())
                    {
                        for (auto head : (*iter).second->getHeaders())
                        {
                            if (act.second->getHeaders().count(head.first) == 0)
                                act.second->setHeader(head.first, head.second);
                        }
                        for (auto query : (*iter).second->getQueryParameters())
                        {
                            if (act.second->getQueryParameters().count(query.first) == 0)
                                act.second->setQueryParameter(query.first, query.second);
                        }
                        for (auto resp : (*iter).second->getResponses())
                        {
                            if (act.second->getResponses().count(resp.first) == 0)
                                act.second->setResponse(resp.first, resp.second);
                        }
                    }
                }
            }
            setTraits(it.second->getResources());
        }
    }

}

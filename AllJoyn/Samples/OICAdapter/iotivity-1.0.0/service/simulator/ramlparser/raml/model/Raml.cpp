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

#include "Raml.h"

namespace RAML
{

    std::string Raml::getTitle() const
    {
        return m_title;
    }
    void Raml::setTitle(const std::string &title)
    {
        m_title = title;
    }

    std::string Raml::getVersion() const
    {
        return m_version;
    }
    void Raml::setVersion(const std::string &version)
    {
        m_version = version;
    }

    std::string Raml::getBaseUri() const
    {
        return m_baseUri;
    }
    void Raml::setBaseUri(const std::string &baseUri)
    {
        m_baseUri = baseUri;
    }

    std::list<std::string> const &Raml::getProtocols() const
    {
        return m_protocols;
    }
    void Raml::setProtocol(const std::string &protocol)
    {
        m_protocols.push_back(protocol);
    }
    std::map<std::string, UriParameterPtr> const &Raml::getBaseUriParameters() const
    {
        return m_baseUriParameters;
    }
    void Raml::setBaseUriParameter(const std::string &paramName,
                                   const UriParameterPtr &uriParameter)
    {
        m_baseUriParameters[paramName] = uriParameter;
    }

    std::string Raml::getMediaType() const
    {
        return m_mediaType;
    }
    void Raml::setMediaType(const std::string &mediaType)
    {
        m_mediaType = mediaType;
    }

    std::list<std::pair<std::string, SchemaPtr> > const &Raml::getSchemas() const
    {
        return m_schemas;
    }

    void Raml::setSchema(const std::string &schemaName, const SchemaPtr &schema)
    {
        m_schemas.push_back(std::make_pair(schemaName, schema));
    }

    std::list<std::pair<std::string, RamlResourcePtr> > const &Raml::getResourceTypes() const
    {
        return m_resourceTypes;
    }
    void Raml::setResourceType(const std::string &typeName, const RamlResourcePtr &resourceType)
    {
        m_resourceTypes.push_back(std::make_pair(typeName, resourceType));
    }

    std::list<std::pair<std::string, ActionPtr> > const &Raml::getTraits() const
    {
        return m_traits;
    }
    void Raml::setTrait(const std::string &traitName, const ActionPtr &trait)
    {
        m_traits.push_back(std::make_pair(traitName, trait));
    }
    RamlResourcePtr Raml::getResource(const std::string &resourceName)
    {
        return m_resources[resourceName];
    }

    std::map<std::string, RamlResourcePtr> const &Raml::getResources() const
    {
        return m_resources;
    }

    void Raml::setResource(const std::string &resourceKey, const RamlResourcePtr &resource)
    {
        m_resources[resourceKey] = resource;
    }

    void Raml::setDocumentationItem(const std::shared_ptr<DocumentationItem> &documentationItem)
    {
        m_documentation.push_back(documentationItem);
    }

    std::list<std::shared_ptr<DocumentationItem> > const &Raml::getDocumentation() const
    {
        return m_documentation;
    }
    void Raml::readRamlFromYaml(const YAML::Node &yamlNode )
    {
        if (yamlNode.Type() == YAML::NodeType::Map)
        {
            for ( YAML::const_iterator it = yamlNode.begin(); it != yamlNode.end(); ++it )
            {
                std::string key = READ_NODE_AS_STRING(it->first);
                if (key == Keys::Title)
                {
                    setTitle(READ_NODE_AS_STRING(it->second));
                }
                else if (key == Keys::Version)
                {
                    setVersion(READ_NODE_AS_STRING(it->second));
                }
                else if (key == Keys::BaseUri)
                {
                    setBaseUri(READ_NODE_AS_STRING(it->second));
                }
                else if ((key == Keys::BaseUriParameters) || (key == Keys::UriParameters))
                {
                    YAML::Node paramNode = it->second;
                    for ( YAML::const_iterator tt = paramNode.begin(); tt != paramNode.end(); ++tt )
                    {
                        setBaseUriParameter(READ_NODE_AS_STRING(tt->first),
                                            std::make_shared<UriParameter>(tt->second));
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
                else if (key == Keys::MediaType)
                {
                    setMediaType(READ_NODE_AS_STRING(it->second));
                }
                else if (key == Keys::Documentation)
                {
                    YAML::Node docNode = it->second;
                    for ( YAML::const_iterator iit = docNode.begin(); iit != docNode.end(); ++iit )
                    {
                        std::string title ;
                        std::string content ;

                        for ( YAML::const_iterator tt = (*iit).begin(); tt != (*iit).end(); ++tt )
                        {
                            std::string key = READ_NODE_AS_STRING(tt->first);

                            if (key == Keys::Title)
                                title = READ_NODE_AS_STRING(tt->second);
                            else if (key == Keys::Content)
                                content = READ_NODE_AS_STRING(tt->second);

                        }
                        setDocumentationItem(std::make_shared<DocumentationItem>(title, content));
                    }
                }
                else if (key == Keys::Schemas)
                {
                    YAML::Node schemaNode = it->second;
                    for ( YAML::const_iterator iit = schemaNode.begin(); iit != schemaNode.end(); ++iit )
                    {
                        for ( YAML::const_iterator tt = (*iit).begin(); tt != (*iit).end(); ++tt )
                        {
                            std::string key = READ_NODE_AS_STRING(tt->first);
                            std::pair<std::string, Schema> schema;

                            IncludeResolver::FileType fileType = m_includeResolver->getFileType(tt->second);
                            if ((fileType == IncludeResolver::FileType::JSON) ||
                                (fileType == IncludeResolver::FileType::FILE))
                            {
                                setSchema(key,
                                          std::make_shared<Schema>(m_includeResolver->readFromFile(tt->second),
                                                                   m_includeResolver));
                            }
                            else
                            {
                                setSchema(key,
                                          std::make_shared<Schema>(READ_NODE_AS_STRING(tt->second),
                                                                   m_includeResolver));
                            }
                        }
                    }
                }

                else if (key.compare(0, Keys::Resource.length(), Keys::Resource)  == 0)
                {
                    setResource(key, std::make_shared<RamlResource>(key, it->second, m_includeResolver,
                                getBaseUri()));
                }
                else if (key == Keys::Traits)
                {
                    YAML::Node traitNode = it->second;
                    for ( YAML::const_iterator tt = traitNode.begin(); tt != traitNode.end(); ++tt )
                    {
                        for (auto elem : *tt)
                        {
                            std::string trait = READ_NODE_AS_STRING(elem.first);
                            setTrait(trait, std::make_shared<Action>(ActionType::NONE, elem.second ,
                                     m_includeResolver));
                        }
                    }
                }
                else if (key == Keys::ResourceTypes)
                {
                    YAML::Node typeNode = it->second;
                    for ( YAML::const_iterator tt = typeNode.begin(); tt != typeNode.end(); ++tt )
                    {
                        for (auto elem : *tt)
                        {
                            std::string type = READ_NODE_AS_STRING(elem.first);
                            setResourceType(type, std::make_shared<RamlResource>(type, elem.second,
                                            m_includeResolver,
                                            getBaseUri()));

                        }

                    }
                }
            }
        }

    }

}

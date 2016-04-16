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
 * @file   Raml.h
 *
 * @brief   This file provides data Model for RAML file.
 */

#ifndef RAML_H
#define RAML_H

#include <map>
#include <list>
#include <string>
#include "AbstractParam.h"
#include "UriParameter.h"
#include "QueryParameter.h"
#include "FormParameter.h"
#include "Header.h"

#include "RequestResponseBody.h"
#include "RamlResource.h"
#include "ActionType.h"
#include "Action.h"
#include "Response.h"
#include "Schema.h"
#include "IncludeResolver.h"

#include "DocumentationItem.h"
#include "yaml-cpp/yaml.h"
#include "yaml-cpp/exceptions.h"
#include "RamlExceptions.h"
#include "Utils.h"
#include "cJSON.h"


namespace RAML
{
    /**
     * @class   Raml
     * @brief   This class provides data Model for RAML file.
     */
    class Raml
    {
        public:
            /**
                 * This method is for getting Title from Raml.
                 *
                 * @return Title as string.
                 */
            virtual std::string getTitle() const;

            /**
                 * This method is for setting Title to Raml.
                 *
                 * @param title - Title as string
                 */
            virtual void setTitle(const std::string &title);

            /**
                 * This method is for getting Version from Raml.
                 *
                 * @return Version as string.
                 */
            virtual std::string getVersion() const;

            /**
                 * This method is for setting Version to Raml.
                 *
                 * @param version - Version as string
                 */
            virtual void setVersion(const std::string &version);

            /**
                 * This method is for getting BaseUri from Raml.
                 *
                 * @return BaseUri as string.
                 */
            virtual std::string getBaseUri() const;

            /**
                 * This method is for setting BaseUri to Raml.
                 *
                 * @param baseUri - BaseUri as string
                 */
            virtual void setBaseUri(const std::string &baseUri);

            /**
                 * This method is for getting Protocols from Raml.
                 *
                 * @return list of Protocols as string.
                 */
            virtual std::list<std::string> const &getProtocols() const;

            /**
                 * This method is for setting Protocols to Raml.
                 *
                 * @param protocol - Protocol as string
                 */
            virtual void setProtocol(const std::string &protocol);


            /**
                 * This method is for getting BaseUriParameter from Raml.
                 *
                 * @return map of BaseUriParameter name and Pointer to UriParameter.
                 */
            virtual std::map<std::string, UriParameterPtr> const &getBaseUriParameters() const;

            /**
                 * This method is for setting BaseUriParameter to Raml.
                 *
                 * @param paramName - name of BaseUriParameter as string
                 * @param uriParameter - pointer to UriParameter
                 */
            virtual void setBaseUriParameter(const std::string &paramName,
                                             const UriParameterPtr &uriParameter);


            /**
                 * This method is for getting MediaType from Raml.
                 *
                 * @return Title as string.
                 */
            virtual std::string getMediaType() const;

            /**
                 * This method is for setting MediaType to Raml.
                 *
                 * @param mediaType - MediaType as string
                 */
            virtual void setMediaType(const std::string &mediaType);

            /**
                 * This method is for getting Schemas from Raml.
                 *
                 * @return list of Schemas name and Pointer to Schema as pair.
                 */
            virtual std::list<std::pair<std::string, SchemaPtr> > const &getSchemas() const;

            /**
                 * This method is for setting Schemas to Raml.
                 *
                 * @param schemaName - name of schema as string
                 * @param schema - pointer to Schema
                 */
            virtual void setSchema(const std::string &schemaName, const SchemaPtr &schema);

            /**
                 * This method is for getting ResourceTypes from Raml.
                 *
                 * @return list of ResourceTypes name and Pointer to RamlResource as pair.
                 */
            virtual std::list<std::pair<std::string, RamlResourcePtr> > const &getResourceTypes() const;

            /**
                 * This method is for setting ResourceTypes to Raml.
                 *
                 * @param typeName - name of ResourceType as string
                 * @param resourceType - pointer to RamlResource
                 */
            virtual void setResourceType(const std::string &typeName,
                                         const RamlResourcePtr &resourceType);

            /**
                 * This method is for getting Traits from Raml.
                 *
                 * @return list of Traits name and Pointer to Action as pair.
                 */
            virtual std::list<std::pair<std::string, ActionPtr> > const &getTraits() const;

            /**
                 * This method is for setting Traits to Raml.
                 *
                 * @param traitName - name of Trait as string
                 * @param trait - pointer to Action
                 */
            virtual void setTrait(const std::string &traitName, const ActionPtr &trait);

            /**
                 * This method is for getting Resource from Raml.
                 *
                 * @param resourceName - name of Resource as string
                 *
                 * @return Pointer to Resource
                 */
            virtual RamlResourcePtr getResource(const std::string &resourceName);

            /**
                 * This method is for getting Resource from Raml.
                 *
                 * @return map of Resource name and Pointer to Resource
                 */
            virtual std::map<std::string, RamlResourcePtr> const &getResources() const;

            /**
                 * This method is for setting Resource to Raml.
                 *
                 * @param resourceKey - name of Resource as string
                 * @param resource - pointer to Resource
                 */
            virtual void setResource(const std::string &resourceKey,
                                     const RamlResourcePtr &resource);

            /**
                 * This method is for setting DocumentationItem to Raml.
                 *
                 * @param documentationItem - pointer to DocumentationItem
                 */
            virtual void setDocumentationItem(const std::shared_ptr<DocumentationItem> &documentationItem);

            /**
                 * This method is for getting DocumentationItem from Raml.
                 *
                 * @return list of DocumentationItem
                 */
            virtual std::list<std::shared_ptr<DocumentationItem> > const &getDocumentation() const;

            /**
                 * This method is for setting Raml object reading from Yaml nodes
                 *
                 * @param yamlNode - Reference to YamlNode for reading into Raml object
                 */
            void readRamlFromYaml(const YAML::Node &yamlNode);

            /**
                  * Constructor of Raml.
                  */
            Raml() : m_includeResolver(std::make_shared<IncludeResolver>()) {}

            /**
                   * Constructor of Raml.
                   *
                   * @param fileLocation - RAML configuration file path.
                   * @param ramlName - RAML configuration file Name
                   *
                   *  NOTE: Constructor would throw RamlException if any error occured.
                   */
            Raml(const std::string &fileLocation, const std::string &ramlName)
                : m_includeResolver(std::make_shared<IncludeResolver>(fileLocation))
            {
                try
                {
                    YAML::Node yamlRootNode = YAML::LoadFile(fileLocation + ramlName);
                    readRamlFromYaml(yamlRootNode);
                }
                catch (YAML::ParserException &e)
                {
                    throw RamlParserException(e.mark, e.msg);
                }
                catch (YAML::RepresentationException &e)
                {
                    throw RamlRepresentationException(e.mark, e.msg);
                }
                catch (YAML::BadFile &e)
                {
                    throw RamlBadFile(e.mark, e.msg);
                }
                catch (JsonException &e)
                {
                    throw;
                }
            }
        private:
            std::string m_title;
            std::string m_version;
            std::string m_baseUri;
            std::list<std::string> m_protocols;
            std::map<std::string, UriParameterPtr> m_baseUriParameters;
            std::string m_mediaType;
            std::list <std::pair<std::string, SchemaPtr> > m_schemas;
            std::list <std::pair<std::string, RamlResourcePtr> > m_resourceTypes;
            std::list <std::pair<std::string, ActionPtr> > m_traits;
            std::map<std::string, RamlResourcePtr> m_resources;
            std::list<std::shared_ptr<DocumentationItem> > m_documentation;
            IncludeResolverPtr m_includeResolver;
    };

    /** RamlPtr - shared Ptr to Raml.*/
    typedef std::shared_ptr<Raml> RamlPtr;

}
#endif

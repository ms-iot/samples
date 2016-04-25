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
 * @file   RamlResource.h
 *
 * @brief   This file provides data Model for RAML RamlResource.
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <map>
#include <list>
#include <string>
#include "UriParameter.h"
#include "ActionType.h"
#include "Action.h"
#include "Utils.h"
#include "IncludeResolver.h"

namespace RAML
{
    /**
     * @class   RamlResource
     * @brief   This class provides data Model for RAML RamlResource.
     */
    class RamlResource
    {
        public:
            /**
                 * This method is for getting DisplayName from RamlResource.
                 *
                 * @return DisplayName as string.
                 */
            virtual std::string getDisplayName() const;

            /**
                 * This method is for setting DisplayName to RamlResource.
                 *
                 * @param displayName - DisplayName as string
                 */
            virtual void setDisplayName(const std::string &displayName);

            /**
                 * This method is for getting Description from RamlResource.
                 *
                 * @return Description as string.
                 */
            virtual std::string getDescription() const;

            /**
                 * This method is for setting Description to RamlResource.
                 *
                 * @param description - Description as string
                 */
            virtual void setDescription(const std::string &description);

            /**
                 * This method is for getting ParentUri from RamlResource.
                 *
                 * @return ParentUri as string.
                 */
            virtual std::string getParentUri() const;

            /**
                 * This method is for setting ParentUri to RamlResource.
                 *
                 * @param parentUri - ParentUri as string
                 */
            virtual void setParentUri(const std::string &parentUri);

            /**
                 * This method is for getting RelativeUri from RamlResource.
                 *
                 * @return RelativeUri as string.
                 */
            virtual std::string getRelativeUri() const;

            /**
                 * This method is for setting RelativeUri to RamlResource.
                 *
                 * @param relativeUri - RelativeUri as string
                 */
            virtual void setRelativeUri(const std::string &relativeUri);

            /**
                 * This method is for getting UriParameter from RamlResource.
                 *
                 * @return map of UriParameter name and Pointer to UriParameter.
                 */
            virtual std::map<std::string, UriParameterPtr> const &getUriParameters() const;

            /**
                 * This method is for setting UriParameter to RamlResource.
                 *
                 * @param paramName - name of UriParameter as string
                 * @param uriParameter - pointer to UriParameter
                 */
            virtual void setUriParameter(const std::string &paramName, const UriParameterPtr &uriParameter);

            /**
                 * This method is for getting BaseUriParameter from RamlResource.
                 *
                 * @return map of BaseUriParameter name and Pointer to UriParameter.
                 */
            virtual std::map<std::string, UriParameterPtr > const &getBaseUriParameters() const;

            /**
                 * This method is for setting BaseUriParameter to RamlResource.
                 *
                 * @param paramName - name of BaseUriParameter as string
                 * @param baseUriParameter - pointer to UriParameter
                 */
            virtual void setBaseUriParameter(const std::string &paramName,
                                             const UriParameterPtr &baseUriParameter);

            /**
                 * This method is for getting Action from RamlResource.
                 *
                 * @param actionType - type of action as ActionType
                 *
                 * @return Pointer to Action.
                 */
            virtual ActionPtr getAction(ActionType actionType);

            /**
                 * This method is for getting Action from RamlResource.
                 *
                 * @return map of ActionType and Pointer to Action.
                 */
            virtual std::map<ActionType , ActionPtr> const &getActions() const;

            /**
                 * This method is for setting Action to RamlResource.
                 *
                 * @param actiontype - type of action
                 * @param action - pointer to Action
                 */
            virtual void setAction(const ActionType &actiontype , const ActionPtr &action );

            /**
                 * This method is for getting child Resource from RamlResource.
                 *
                 * @return map of Resource name and Pointer to RamlResource.
                 */
            virtual std::map<std::string, std::shared_ptr<RamlResource> > const &getResources() const;

            /**
                 * This method is for setting child Resource to RamlResource.
                 *
                 * @param resourceName - name of Resource as string
                 * @param resources - pointer to RamlResource
                 */
            virtual void setResource(const std::string &resourceName,
                                     const std::shared_ptr<RamlResource> &resources);

            /**
                 * This method is for getting Traits from RamlResource.
                 *
                 * @return list of Traits name.
                 */
            virtual std::list<std::string> const &getTraits() const;

            /**
                  * This method is for setting Traits to RamlResource.
                  *
                  * @param trait - name of Traits
                  */
            virtual void setTrait(const std::string &trait);

            /**
                 * This method is for getting ResourceType from RamlResource.
                 *
                 * @return ResourceType name.
                 */
            virtual std::string getResourceType() const;

            /**
                   * This method is for setting ResourceType to RamlResource.
                   *
                   * @param type - name of ResourceType
                   */
            virtual void setResourceType(const std::string &type);

            /**
                 * This method is for getting ResourceUri from RamlResource.
                 *
                 * @return ResourceUri as string.
                 */
            virtual std::string getResourceUri() const;

            /**
                  * Constructor of RamlResource.
                  */
            RamlResource(): m_includeResolver(NULL) {}

            /**
                   * Constructor of RamlResource.
                   *
                   * @param resourceKey - name of the Resource
                   * @param yamlNode - Reference to YamlNode for reading the RamlResource
                   * @param includeResolver - Reference to IncludeResolver for reading external files
                   * @param parentUri - Uri of the Parent to form the Absolute Uri
                   *
                   */
            RamlResource(const std::string resourceKey, const YAML::Node &yamlNode ,
                         const IncludeResolverPtr &includeResolver,
                         const std::string &parentUri) : m_includeResolver(includeResolver)
            {
                readResource(resourceKey, yamlNode, parentUri);
            }
        private:
            void readResource(const std::string resourceKey, const YAML::Node &yamlNode,
                              const std::string &parentUri);
        private:
            std::string m_displayName;
            std::string m_description;
            std::string m_relativeUri;
            std::map<std::string, UriParameterPtr> m_uriParameters;
            std::map<std::string, UriParameterPtr > m_baseUriParameters;
            std::map<ActionType , ActionPtr> m_actions;
            std::list<std::string> m_traits;
            std::string m_resourceType;
            std::string m_parentUri;
            std::map<std::string, std::shared_ptr<RamlResource> > m_resources;
            IncludeResolverPtr m_includeResolver;
    };

    /** RamlResourcePtr - shared Ptr to RamlResource.*/
    typedef std::shared_ptr<RamlResource> RamlResourcePtr;
}
#endif

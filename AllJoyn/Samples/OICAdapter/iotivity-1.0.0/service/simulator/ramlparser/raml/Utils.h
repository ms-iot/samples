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
 * @file   Utils.h
 *
 * @brief   This file provides utilities for RamlParser.
 */

#ifndef UTILS_H
#define UTILS_H

#include "yaml-cpp/yaml.h"
#include "ActionType.h"

namespace RAML
{
    namespace Keys
    {
        /** Title - Raml title key.*/
        const std::string Title = "title";
        /** Version - Raml Version key.*/
        const std::string Version = "version";
        /** BaseUri - Raml BaseUri key.*/
        const std::string BaseUri = "baseUri";
        /** Protocols - Raml Protocols key.*/
        const std::string Protocols = "protocols";
        /** MediaType - Raml MediaType key.*/
        const std::string MediaType = "mediaType";
        /** Schemas - Raml Schemas key.*/
        const std::string Schemas = "schemas";
        /** ResourceTypes - Raml ResourceTypes key.*/
        const std::string ResourceTypes = "resourceTypes";
        /** Traits - Raml Traits key.*/
        const std::string Traits = "traits";
        /** IsTrait - Raml is key.*/
        const std::string IsTrait = "is";

        /** Resource - Raml Resource key.*/
        const std::string Resource = "/";
        /** ActionType - Raml allowed ActionType key.*/
        const std::vector<std::string> ActionType = {"get", "post", "put", "delete",
                                                     "head", "patch", "options", "trace"
                                                    };

        /** Responses - Raml Responses key.*/
        const std::string Responses = "responses";
        /** Body - Raml Body key.*/
        const std::string Body = "body";
        /** Schema - Raml Schema key.*/
        const std::string Schema = "schema";
        /** Example - Raml Example key.*/
        const std::string Example = "example";

        /** BaseUriParameters - Raml BaseUriParameters key.*/
        const std::string BaseUriParameters = "baseUriParameters";
        /** UriParameters - Raml UriParameters key.*/
        const std::string UriParameters = "uriParameters";
        /** Headers - Raml title Headers.*/
        const std::string Headers = "headers";
        /** QueryParameters - Raml QueryParameters key.*/
        const std::string QueryParameters = "queryParameters";
        /** FormParameters - Raml FormParameters key.*/
        const std::string FormParameters = "formParameters";
        /** DisplayName - Raml DisplayName key.*/
        const std::string DisplayName = "displayName";
        /** Description - Raml Description key.*/
        const std::string Description = "description";
        /** Type - Raml Type key.*/
        const std::string Type = "type";
        /** Enum - Raml Enum key.*/
        const std::string Enum = "enum";
        /** Pattern - Raml Pattern key.*/
        const std::string Pattern = "pattern";
        /** MinLength - Raml MinLength key.*/
        const std::string MinLength = "minLength";
        /** MaxLength - Raml MaxLength key.*/
        const std::string MaxLength = "maxLength";
        /** Minimum - Raml Minimum key.*/
        const std::string Minimum = "minimum";
        /** Maximum - Raml Maximum key.*/
        const std::string Maximum = "maximum";
        /** Repeat - Raml Repeat key.*/
        const std::string Repeat = "repeat";
        /** Required - Raml Required key.*/
        const std::string Required = "required";
        /** Default - Raml Default key.*/
        const std::string Default = "default";
        /** Title - Raml title key.*/

        /** Documentation - Raml Documentation key.*/
        const std::string Documentation = "documentation";
        /** Content - Raml Content key.*/
        const std::string Content = "content";

        /** Json - Raml Json key.*/
        const std::string Json = "json";
        /** AllowedRamlYamlTypes - Raml AllowedRamlYamlTypes key.*/
        const std::vector<std::string> AllowedRamlYamlTypes = {"raml", "yaml", "yml"};

    }

    /**
    * This macro is reading yamlNode as String.
    *
    * @param yamlNode - reference to yamlNode
    *
    * @return value as string
    */

#define READ_NODE_AS_STRING(yamlNode)                   \
({                                                      \
(yamlNode).as<std::string>();                           \
})

    /**
    * This macro is reading yamlNode as int.
    *
    * @param yamlNode - reference to yamlNode
    *
    * @return value as int
    */
#define READ_NODE_AS_INT(yamlNode)                      \
({                                                      \
    (yamlNode).as<int>();                               \
})

    /**
    * This macro is reading yamlNode as long.
    *
    * @param yamlNode - reference to yamlNode
    *
    * @return value as long
    */
#define READ_NODE_AS_LONG(yamlNode)                     \
({                                                      \
    (yamlNode).as<long>();                              \
})
    /**
     * This macro is reading yamlNode as bool.
     *
     * @param yamlNode - reference to yamlNode
     *
     * @return value as bool
      */
#define READ_NODE_AS_BOOL(yamlNode)                     \
({                                                      \
        (yamlNode).as<bool>();                          \
})
    /**
    * This macro is getting ActionType
    *
    * @param key - string
    *
    * @return ActionType
    */

#define GET_ACTION_TYPE(key)                            \
({                                                      \
    ActionType actionType = ActionType::GET;            \
    if (key == "get" )                                  \
        actionType = ActionType::GET;                   \
    else if (key == "post" )                            \
        actionType = ActionType::POST;                  \
    else if (key == "put" )                             \
        actionType = ActionType::PUT;                   \
    else if (key == "delete" )                          \
        actionType = ActionType::DELETE;                \
    else if (key == "head" )                            \
        actionType = ActionType::HEAD;                  \
    else if (key == "patch" )                           \
        actionType = ActionType::PATCH;                 \
    else if (key == "options" )                         \
        actionType = ActionType::OPTIONS;               \
    else if (key == "trace" )                           \
        actionType = ActionType::TRACE;                 \
    actionType;                                         \
})

}
#endif

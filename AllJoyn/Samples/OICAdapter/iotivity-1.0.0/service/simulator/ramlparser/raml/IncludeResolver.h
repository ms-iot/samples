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
 * @file   IncludeResolver.h
 *
 * @brief   This file provides APIs for resolving included files.
 */

#ifndef INCLUDE_RESOLVER_H
#define INCLUDE_RESOLVER_H

#include "yaml-cpp/yaml.h"
#include "cJSON.h"
#include "Utils.h"
#include <fstream>
#include "yaml-cpp/exceptions.h"
#include "RamlExceptions.h"

namespace RAML
{
    /**
     * @class   IncludeResolver
     * @brief   This class provides a set of APIs for resolving included files.
     */
    class IncludeResolver
    {
        public:
            /** FileType - enumeration for Included File types*/
            enum class FileType
            {
                NODE, JSON, FILE, NOTAG , ERROR
            };

        public:
            /**
                 * This method is for reading a file specified in YamlNode and parse it to
                 * getting the created RootNode from RAML file.
                 *
                 * @param yamlFile - Reference to YamlNode specifying the FileName.
                 *
                 * @return pointer to root node from the Parsed file.
                 */
            YAML::Node readToYamlNode(const YAML::Node &yamlFile );

            /**
                 * This method is for reading a file specified in YamlNode and parse the specified Json file.
                 *
                 * @param jsonFile - Reference to YamlNode specifying the FileName.
                 *
                 * @return cJSON pointer to CJson object.
                 */
            cJSON *readToJson(const YAML::Node &jsonFile );

            /**
                 * This method is for reading a file specified in YamlNode and parse the content of file.
                 *
                 * @param file - Reference to YamlNode specifying the FileName.
                 *
                 * @return contents of the file.
                 */
            std::string readFromFile(const YAML::Node &file );

            /**
                 * This method is checking the file type specified in YamlNode.
                 *
                 * @param yamlNode - Reference to YamlNode specifying the FileName.
                 *
                 * @return FileType type of file.
                 */
            FileType getFileType(const YAML::Node &yamlNode );

            /**
                 * This method is for reading a file specified and parse the content of file.
                 *
                 * @param jsonFileName - FileName of Json file to be read.
                 *
                 * @return pointer to CJson object.
                 */
            cJSON *readToJson(const std::string &jsonFileName);

            /**
                  * Constructor of IncludeResolver.
                  */
            IncludeResolver() {}

            /**
                  * Constructor of IncludeResolver.
                  *
                  * @param path -  configuration file path.
                  *
                  */
            IncludeResolver(const std::string &path) : m_path(path) {}

        private:
            std::string m_path;
    };

    /** IncludeResolverPtr - shared Ptr to IncludeResolver.*/
    typedef std::shared_ptr<IncludeResolver> IncludeResolverPtr;

}
#endif

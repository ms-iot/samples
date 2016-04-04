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
 * @file   RamlParser.h
 *
 * @brief   This file provides APIs for parsing Raml file.
 */

#ifndef RAML_PARSER_H
#define RAML_PARSER_H

#include "yaml-cpp/yaml.h"
#include "Raml.h"
#include "Utils.h"
#include "RequestResponseBody.h"
#include "RamlResource.h"
#include "Action.h"
#include "Response.h"
#include <map>
#include "RamlErrorCodes.h"
#include "yaml-cpp/exceptions.h"
#include "RamlExceptions.h"

namespace RAML
{
    /**
      * @class   RamlParser
      * @brief   This class provides a set of APIs for parsing Raml file.
      */
    class RamlParser
    {
        private:
            void setDataFromRoot();
            void setBodyDefaultMediaType(const std::map<std::string, RamlResourcePtr> &resource);
            void setBodySchema(const std::map<std::string, RamlResourcePtr> &resource);
            void setTypes(const std::map<std::string, RamlResourcePtr> &resource);
            void setTraits(const std::map<std::string, RamlResourcePtr> &resource);

        public:
            /**
                   * This method is for getting the created and Parsed RAML object from RAML file.
                   *
                   * @param result - Reference to RamlParserResult.
                   *
                   * @return pointer to Raml shared object parsed.
                   */
            virtual RamlPtr getRamlPtr(RamlParserResult &result);

            /**
                   * This method is for getting the created and Parsed RAML object from RAML file.
                   *
                   * @return pointer to Raml shared object parsed.
                   */
            virtual RamlPtr getRamlPtr();

            /**
                   * Constructor of RamlParser.
                   *
                   *  NOTE: Constructor would initialize the RamlParserResult with File Path Required
                   */
            RamlParser(): m_ramlPtr(std::make_shared<Raml>()),
                m_ramlParserResult(RAML_FILE_PATH_REQUIRED) {}

            /**
                   * Constructor of RamlParser.
                   *
                   * @param path - RAML configuration file path.
                   *
                   *  NOTE: Constructor would throw RamlBadFile when invalid arguments passed, and
                   * RamlException if any other error occured.
                   */
            RamlParser(const std::string &path): m_ramlParserResult(RAML_PARSER_ERROR)
            {
                if (path.length() > 0)
                {
                    std::size_t found = path.find_last_of("/\\");
                    if (found < path.length())
                    {
                        m_fileLocation = path.substr(0, found) + "/";
                        m_ramlName = path.substr(found + 1);
                        try
                        {
                            m_ramlPtr = std::make_shared<Raml>(m_fileLocation, m_ramlName);
                            setDataFromRoot();
                            m_ramlParserResult = RAML_PARSER_OK;
                        }
                        catch (RamlException &e)
                        {
                            throw;
                        }
                    }
                    else
                    {
                        m_ramlParserResult = RAML_FILE_PATH_REQUIRED;
                        throw RamlBadFile("Raml File Path incorrect");
                    }
                }
                else
                {
                    m_ramlParserResult = RAML_FILE_PATH_REQUIRED;
                    throw RamlBadFile("Raml File Path required");
                }
            }
        private:

            RamlPtr m_ramlPtr;
            std::string m_fileLocation;
            std::string m_ramlName;
            RamlParserResult m_ramlParserResult;
    };

}
#endif

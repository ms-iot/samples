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
 * @file   RamlExceptions.h
 *
 * @brief   This file provides exception handling while parsing RAML and Json Schema.
 */

#ifndef RAML_EXCEPTIONS_H_
#define RAML_EXCEPTIONS_H_

#include <exception>
#include "RamlErrorCodes.h"
#include "yaml-cpp/exceptions.h"

namespace RAML
{
    /**
     * @class   RamlException
     * @brief   This is the base exception of all type of exception thrown from RamlParser module.
     */
    class RamlException : public std::exception
    {
        public:
            /**
                  * Constructor of RamlException.
                  *
                  * @param message - String describing the error messsage.
                  */
            RamlException(const std::string &message) : m_message(message) {}

            /**
                  * Constructor of RamlException.
                  *
                  * @param mark - line and column information of exception thrown.
                  * @param message - String describing the error messsage.
                  */
            RamlException(const YAML::Mark &mark, const std::string &message);

            /**
                  * API to get error message describing exception reason.
                  *
                  * @return Null terminated string.
                  */
            virtual const char *what() const noexcept;
            virtual ~RamlException() throw() {}

        private:
            std::string m_message;
            YAML::Mark m_mark;
    };

    /**
     * @class RamlParserException
     * @brief This exception will be thrown to indicate Parser Exception.
     */
    class RamlParserException : public RamlException
    {
        public:
            /**
                  * Constructor of RamlParserException.
                  *
                  * @param message - String describing the error messsage.
                  */
            RamlParserException(const std::string &message): RamlException(message) {}

            /**
                  * Constructor of RamlParserException.
                  *
                  * @param mark - line and column information of exception thrown.
                  * @param message - String describing the error messsage.
                  */
            RamlParserException(const YAML::Mark &mark, const std::string &message): RamlException(mark,
                        message) {}
    };

    /**
     * @class RamlRepresentationException
     * @brief This exception will be thrown to indicate invalid Raml Representation case.
     */
    class RamlRepresentationException : public RamlException
    {
        public:
            /**
                  * Constructor of RamlRepresentationException.
                  *
                  * @param message - String describing the error messsage.
                  */
            RamlRepresentationException(const std::string &message): RamlException(message) {}

            /**
                  * Constructor of RamlRepresentationException.
                  *
                  * @param mark - line and column information of exception thrown.
                  * @param message - String describing the error messsage.
                  */
            RamlRepresentationException(const YAML::Mark &mark, const std::string &message): RamlException(mark,
                        message) {}
    };

    /**
      * @class RamlBadFile
      * @brief This exception will be thrown to indicate RAMl BadFile.
      */
    class RamlBadFile : public RamlException
    {
        public:
            /**
                 * Constructor of RamlBadFile.
                 *
                 * @param message - String describing the error messsage.
                 */
            RamlBadFile(const std::string &message) : RamlException(message) {}

            /**
                  * Constructor of RamlBadFile.
                  *
                  * @param mark - line and column information of exception thrown.
                  * @param message - String describing the error messsage.
                  */
            RamlBadFile(const YAML::Mark &mark, const std::string &message): RamlException(mark, message) {}
    };
    /**
      * @class JsonException
      * @brief This exception will be thrown to indicate invalid Json file.
      */
    class JsonException : public RamlException
    {
        public:
            /**
                  * Constructor of JsonException.
                  *
                  * @param message - String describing the error messsage.
                  */
            JsonException(const std::string &message) : RamlException(message) {}

    };
}
#endif

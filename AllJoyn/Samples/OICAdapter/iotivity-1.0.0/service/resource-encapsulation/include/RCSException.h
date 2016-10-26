//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
 * @file
 *
 * This file contains the declaration of exception classes for resource-encapsulation
 */
#ifndef RES_ENCAPSULATION_RCSEXCEPTION_H
#define RES_ENCAPSULATION_RCSEXCEPTION_H

#include <string>

#include <octypes.h>

namespace OIC
{
    namespace Service
    {

        /**
         * The base exception class for resource encapsulation.
         *
         */
        class RCSException: public std::exception
        {
        public:

            /**
             * Constructs an exception with an empty description.
             */
            RCSException();

            /**
             * Constructs an exception with a description.
             *
             * @param what The description for the error.
             */
            explicit RCSException(const std::string &what);

            /**
             * @overload
             */
            explicit RCSException(std::string &&what);

            virtual ~RCSException() noexcept;

            /**
             * Returns the exception description.
             *
             */
            virtual const char *what() const noexcept;

        private:
            /**
             *  Exception description
             */
            const std::string m_what;
        };

        /**
         * Thrown when OC layer returns an error.
         *
         */
        class RCSPlatformException: public RCSException
        {
        public:
            explicit RCSPlatformException(OCStackResult reason);

            /**
             * Returns the reason.
             *
             */
            OCStackResult getReasonCode() const;

            /**
             * Returns the reason description.
             *
             */
            std::string getReason() const;

        private:
            OCStackResult m_reason;
        };

        /**
         * Thrown when a request is not acceptable.
         *
         */
        class RCSBadRequestException: public RCSException
        {
        public:
            explicit RCSBadRequestException(const std::string& what);
            explicit RCSBadRequestException(std::string&& what);
        };

        /**
         * Thrown when a parameter is not valid.
         *
         */
        class RCSInvalidParameterException: public RCSException
        {
        public:
            explicit RCSInvalidParameterException(const std::string& what);
            explicit RCSInvalidParameterException(std::string&& what);
        };

        /**
         * Thrown when getting value with wrong template parameter.
         */
        class RCSBadGetException: public RCSException
        {
        public:
            explicit RCSBadGetException(const std::string& what);
            explicit RCSBadGetException(std::string&& what);
        };

        /**
         * Thrown when a key is invalid.
         *
         */
        class RCSInvalidKeyException: public RCSException
        {
        public:
            explicit RCSInvalidKeyException(const std::string& what);
            explicit RCSInvalidKeyException(std::string&& what);
        };

    }
}

#endif // RES_ENCAPSULATION_RCSEXCEPTION_H

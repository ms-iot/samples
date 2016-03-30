//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

#ifndef _RESOURCE_INIT_EXCEPTION_H_
#define _RESOURCE_INIT_EXCEPTION_H_

#include <stdexcept>
#include "StringConstants.h"

namespace OC
{
    class ResourceInitException : public std::exception
    {
    public:
        ResourceInitException(
                bool missingUri,
                bool missingType,
                bool missingInterface,
                bool missingClientWrapper,
                bool invalidPort,
                bool invalidIp)
        : m_missingUri(missingUri),
          m_missingType(missingType),
          m_missingInterface(missingInterface),
          m_missingClientWrapper(missingClientWrapper),
          m_invalidPort(invalidPort),
          m_invalidIp(invalidIp)
        {
        }

        bool isInvalidPort() const
        {
            return m_invalidPort;
        }

        bool isInvalidIp() const
        {
            return m_invalidIp;
        }

        bool isClientWrapperMissing() const
        {
            return m_missingClientWrapper;
        }

        bool isUriMissing() const
        {
            return m_missingUri;
        }

        bool isTypeMissing() const
        {
            return m_missingType;
        }

        bool isInterfaceMissing() const
        {
            return m_missingInterface;
        }

        virtual const char* what() const noexcept
        {
            std::string ret;

            if(isUriMissing())
            {
                ret += OC::InitException::MISSING_URI;
            }

            if(isTypeMissing())
            {
                ret += OC::InitException::MISSING_TYPE;
            }

            if(isInterfaceMissing())
            {
                ret += OC::InitException::MISSING_INTERFACE;
            }

            if(isClientWrapperMissing())
            {
                ret += OC::InitException::MISSING_CLIENT_WRAPPER;
            }

            if(isInvalidPort())
            {
                ret += OC::InitException::INVALID_PORT;
            }

            if(isInvalidIp())
            {
                ret += OC::InitException::INVALID_IP;
            }

            return ret.c_str();
        }

    private:

        bool m_missingUri;
        bool m_missingType;
        bool m_missingInterface;
        bool m_missingClientWrapper;
        bool m_invalidPort;
        bool m_invalidIp;
    };
}

#endif

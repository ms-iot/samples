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

#include <RCSException.h>

#include <OCException.h>

namespace OIC
{
    namespace Service
    {

        RCSException::RCSException()
        {
        }

        RCSException::RCSException(const std::string& what) :
                m_what{ what }
        {
        }

        RCSException::RCSException(std::string&& what) :
                m_what{ std::move(what) }
        {
        }

        RCSException::~RCSException() noexcept
        {
        }

        const char* RCSException::what() const noexcept
        {
            return m_what.c_str();
        }


        RCSPlatformException::RCSPlatformException(OCStackResult reason) :
                RCSException{ "Failed : " + OC::OCException::reason(reason) },
                m_reason { reason }
        {
        }

        OCStackResult RCSPlatformException::getReasonCode() const
        {
            return m_reason;
        }

        std::string RCSPlatformException::getReason() const
        {
            return  OC::OCException::reason(m_reason);
        }


        RCSBadRequestException::RCSBadRequestException(const std::string& what) :
                RCSException{ what }
        {
        }

        RCSBadRequestException::RCSBadRequestException(std::string&& what) :
                RCSException{ std::move(what) }
        {
        }


        RCSInvalidParameterException::RCSInvalidParameterException(const std::string& what) :
                RCSException{ what }
        {
        }

        RCSInvalidParameterException::RCSInvalidParameterException(std::string&& what) :
                RCSException{ std::move(what) }
        {
        }


        RCSBadGetException::RCSBadGetException(const std::string& what) :
                RCSException{ what }
        {
        }

        RCSBadGetException::RCSBadGetException(std::string&& what) :
                RCSException{ std::move(what) }
        {
        }


        RCSInvalidKeyException::RCSInvalidKeyException(const std::string& what) :
                RCSException{ what }
        {
        }

        RCSInvalidKeyException::RCSInvalidKeyException(std::string&& what) :
                RCSException{ std::move(what) }
        {
        }
    }
}

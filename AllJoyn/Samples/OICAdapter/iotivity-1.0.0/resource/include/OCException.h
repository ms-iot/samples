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

#ifndef __INTEL_OCEXCEPTION_H_2014_07_10
 #define __INTEL_OCEXCEPTION_H_2014_07_10

#include <stdexcept>
#include <string>
#include <octypes.h>

namespace OC {

class OCException : public std::runtime_error
{
    public:
        OCException(const std::string& msg, OCStackResult reason = OC_STACK_ERROR)
         : std::runtime_error(msg),
           m_reason(reason)
        {}

        static std::string reason(const OCStackResult sr);

        std::string reason() const
        {
            return reason(m_reason);
        }

        OCStackResult code() const
        {
            return m_reason;
        }

    private:
        OCStackResult m_reason;
};

} // namespace OC

#endif

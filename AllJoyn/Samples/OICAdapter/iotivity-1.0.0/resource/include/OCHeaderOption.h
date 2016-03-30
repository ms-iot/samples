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

/**
 * @file
 *
 * This file contains the declaration of classes and its members related to
 * OCHeaderOption.
 */

#ifndef __OCHEADEROPTION_H
#define __OCHEADEROPTION_H

#include <OCException.h>
#include <StringConstants.h>
namespace OC
{
    namespace HeaderOption
    {
        /**
        *     @brief OCHeaderOption class allows to create instances which comprises optionID
        *            and optionData as members. These are used in setting Header options.
        *            After creating instances of OCHeaderOptions, use setHeaderOptions API
        *            (in OCResource.h) to set header Options.
        *            NOTE: HeaderOptionID  is an unsigned integer value which MUST be within
        *            range of 2048 to 3000 inclusive of lower and upper bound.
        *            HeaderOptions instance creation fails if above condition is not satisfied.
        */
        const uint16_t MIN_HEADER_OPTIONID = 2048;
        const uint16_t MAX_HEADER_OPTIONID = 3000;

        class OCHeaderOption
        {
        private:
            uint16_t m_optionID;
            std::string m_optionData;

        public:
            /**
            * OCHeaderOption constructor
            */
            OCHeaderOption(uint16_t optionID, std::string optionData):
                m_optionID(optionID),
                m_optionData(optionData)
            {
                if(!(optionID >= MIN_HEADER_OPTIONID && optionID <= MAX_HEADER_OPTIONID))
                {
                    throw OCException(OC::Exception::OPTION_ID_RANGE_INVALID);
                }
            }

            virtual ~OCHeaderOption(){}

            OCHeaderOption(const OCHeaderOption&) = default;

            OCHeaderOption(OCHeaderOption&&) = default;

            OCHeaderOption& operator=(const OCHeaderOption&) = default;

            OCHeaderOption& operator=(OCHeaderOption&&) = default;

            /**
            * API to get Option ID
            * @return unsigned integer option ID
            */
            uint16_t getOptionID() const
            {
                return m_optionID;
            }

            /*
            * API to get Option data
            * @return std::string of option data
            */
            std::string getOptionData() const
            {
                return m_optionData;
            }
        };
    } // namespace HeaderOption
} // namespace OC

#endif //__OCHEADEROPTION_H

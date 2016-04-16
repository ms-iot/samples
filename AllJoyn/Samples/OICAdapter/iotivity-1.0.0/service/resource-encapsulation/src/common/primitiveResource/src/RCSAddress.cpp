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

#include <RCSAddress.h>
#include <RCSAddressDetail.h>

namespace OIC
{
    namespace Service
    {
        RCSAddress::RCSAddress(const std::shared_ptr< RCSAddressDetail >& ptr) :
                m_detail{ ptr }
        {
        }

        RCSAddress RCSAddress::multicast()
        {
            return RCSAddress{ std::make_shared< RCSAddressDetail >("") };
        }

        RCSAddress RCSAddress::unicast(const std::string& address)
        {
            return RCSAddress{ std::make_shared< RCSAddressDetail >(address) };
        }

        RCSAddress RCSAddress::unicast(std::string&& address)
        {
            return RCSAddress{ std::make_shared< RCSAddressDetail >(std::move(address)) };
        }



        RCSAddressDetail::RCSAddressDetail(const std::string& address) :
                m_addr{ address }
        {
        }

        RCSAddressDetail::RCSAddressDetail(std::string&& address) :
                m_addr{ std::move(address) }
        {
        }

        const RCSAddressDetail* RCSAddressDetail::getDetail(const RCSAddress& rcsAddr)
        {
            return rcsAddr.m_detail.get();
        }

        const std::string& RCSAddressDetail::getAddress() const
        {
            return m_addr;
        }

    }
}

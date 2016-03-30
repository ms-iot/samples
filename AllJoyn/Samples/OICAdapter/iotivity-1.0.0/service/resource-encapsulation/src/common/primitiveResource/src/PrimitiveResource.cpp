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

#include <PrimitiveResource.h>

#include <PrimitiveResourceImpl.h>
#include <AssertUtils.h>
#include <RCSAddressDetail.h>

#include <OCPlatform.h>

namespace OIC
{
    namespace Service
    {

        PrimitiveResource::Ptr PrimitiveResource::create(
                const std::shared_ptr<OC::OCResource>& ptr)
        {
            return std::shared_ptr< PrimitiveResource >(
                    new PrimitiveResourceImpl< OC::OCResource >{ ptr });
        }

        void discoverResource(const std::string& host, const std::string& resourceURI,
                OCConnectivityType connectivityType, DiscoverCallback callback)
        {
            typedef OCStackResult (*FindResource)(const std::string&, const std::string&,
                    OCConnectivityType, OC::FindCallback);

            invokeOCFunc(static_cast< FindResource >(OC::OCPlatform::findResource),
                    host, resourceURI, connectivityType, static_cast < OC::FindCallback >(
                        std::bind(std::move(callback),
                                std::bind(&PrimitiveResource::create, std::placeholders::_1))));
        }

        void discoverResource(const RCSAddress& address, const std::string& resourceURI,
                DiscoverCallback callback)
        {
            const RCSAddressDetail* addressDetail = RCSAddressDetail::getDetail(address);

            discoverResource(addressDetail->getAddress(), resourceURI, OCConnectivityType{ },
                    std::move(callback));
        }

    }
}

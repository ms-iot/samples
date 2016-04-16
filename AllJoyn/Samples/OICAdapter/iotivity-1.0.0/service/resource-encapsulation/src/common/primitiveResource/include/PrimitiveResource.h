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

#ifndef COMMON_PRIMITIVERESOURCE_H
#define COMMON_PRIMITIVERESOURCE_H

#include <functional>
#include <string>
#include <vector>

#include <OCResource.h>

#include <ResponseStatement.h>
#include <RCSAddress.h>

namespace OIC
{
    namespace Service
    {

        typedef OC::HeaderOption::OCHeaderOption HeaderOption;
        typedef std::vector<HeaderOption> HeaderOptions;

        class RCSResourceAttributes;
        class ResponseStatement;

        class PrimitiveResource: public std::enable_shared_from_this< PrimitiveResource >
        {
        public:
            typedef std::shared_ptr< PrimitiveResource > Ptr;
            typedef std::shared_ptr< const PrimitiveResource > ConstPtr;

            typedef std::function<void(const HeaderOptions&, const ResponseStatement&, int)>
                    GetCallback;

            typedef std::function<void(const HeaderOptions&, const ResponseStatement&, int)>
                    SetCallback;

            typedef std::function<void(const HeaderOptions&, const ResponseStatement&, int, int)>
                    ObserveCallback;

        public:
            static PrimitiveResource::Ptr create(const std::shared_ptr<OC::OCResource>&);

            virtual ~PrimitiveResource() { };

            virtual void requestGet(GetCallback) = 0;
            virtual void requestSet(const RCSResourceAttributes&, SetCallback) = 0;
            virtual void requestObserve(ObserveCallback) = 0;
            virtual void cancelObserve() = 0;

            virtual std::string getSid() const = 0;
            virtual std::string getUri() const = 0;
            virtual std::string getHost() const = 0;
            virtual std::vector< std::string > getTypes() const = 0;
            virtual std::vector< std::string > getInterfaces() const = 0;

            virtual bool isObservable() const = 0;

        protected:
            PrimitiveResource() = default;

            PrimitiveResource(const PrimitiveResource&) = delete;
            PrimitiveResource(PrimitiveResource&&) = delete;

            PrimitiveResource& operator=(const PrimitiveResource&) = delete;
            PrimitiveResource& operator=(PrimitiveResource&&) = delete;
        };

        typedef std::function<void(std::shared_ptr<PrimitiveResource>)> DiscoverCallback;

        void discoverResource(const std::string& host, const std::string& resourceURI,
                OCConnectivityType, DiscoverCallback);

        void discoverResource(const RCSAddress& address, const std::string& resourceURI,
                DiscoverCallback);

    }
}

#endif // COMMON_PRIMITIVERESOURCE_H

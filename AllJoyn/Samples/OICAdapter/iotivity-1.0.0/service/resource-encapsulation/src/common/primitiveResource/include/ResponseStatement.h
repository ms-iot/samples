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

#ifndef COMMON_RESPONSESTATEMENT_H
#define COMMON_RESPONSESTATEMENT_H

#include <string>
#include <vector>

#include <RCSResourceAttributes.h>

namespace OC
{
    class OCRepresentation;
}

namespace OIC
{
    namespace Service
    {
        class RCSResourceAttributes;

        class ResponseStatement
        {
        public:
            static ResponseStatement create(const OC::OCRepresentation&);
            static ResponseStatement create(RCSResourceAttributes&&);

            explicit ResponseStatement(const RCSResourceAttributes&);
            explicit ResponseStatement(RCSResourceAttributes&&);

            ResponseStatement(RCSResourceAttributes&&, std::string&& uri,
                    std::vector< std::string >&& resourceTypes,
                    std::vector< std::string >&& resourceInterfaces);

            ResponseStatement(ResponseStatement&&) = default;

            ResponseStatement& operator=(ResponseStatement&&) = default;

            std::string getUri() const;
            std::vector< std::string > getResourceTypes() const;
            std::vector< std::string > getResourceInterfaces() const;

            const RCSResourceAttributes& getAttributes() const;

        private:
            RCSResourceAttributes m_attrs;

            std::string m_uri;
            std::vector< std::string > m_resourceTypes;
            std::vector< std::string > m_resourceInterfaces;
        };

    }
}

#endif // COMMON_RESPONSESTATEMENT_H

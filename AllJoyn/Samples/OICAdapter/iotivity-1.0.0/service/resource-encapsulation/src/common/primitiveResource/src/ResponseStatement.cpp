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

#include <ResponseStatement.h>

#include <ResourceAttributesConverter.h>

namespace OIC
{
    namespace Service
    {
        ResponseStatement ResponseStatement::create(const OC::OCRepresentation& ocRepresentation)
        {
            return ResponseStatement::create(
                    ResourceAttributesConverter::fromOCRepresentation(ocRepresentation));
        }

        ResponseStatement ResponseStatement::create(RCSResourceAttributes&& attrs)
        {
            return ResponseStatement(std::move(attrs));
        }

        ResponseStatement::ResponseStatement(const RCSResourceAttributes& attrs) :
                m_attrs{ attrs }
        {
        }

        ResponseStatement::ResponseStatement(RCSResourceAttributes&& attrs) :
                m_attrs{ std::move(attrs) }
        {
        }

        ResponseStatement::ResponseStatement(RCSResourceAttributes&& attrs, std::string&& uri,
                std::vector< std::string >&& resourceTypes,
                std::vector< std::string >&& resourceInterfaces) :
                m_attrs{ std::move(attrs) },
                m_uri{ std::move(uri) },
                m_resourceTypes { std::move(resourceTypes) },
                m_resourceInterfaces{ std::move(resourceInterfaces) }
        {
        }

        std::string ResponseStatement::getUri() const
        {
            return m_uri;
        }

        std::vector< std::string > ResponseStatement::getResourceTypes() const
        {
            return m_resourceTypes;
        }

        std::vector< std::string > ResponseStatement::getResourceInterfaces() const
        {
            return m_resourceInterfaces;
        }

        const RCSResourceAttributes& ResponseStatement::getAttributes() const
        {
            return m_attrs;
        }

    }
}


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
 * This file contains the declaration of classes and its members related to RCSRequest
 */
#ifndef SERVERBUILDER_PRIMITIVEREQUEST_H
#define SERVERBUILDER_PRIMITIVEREQUEST_H

#include <string>

namespace OIC
{
    namespace Service
    {
        /**
        * This class describes the request.
        *
        */
        class RCSRequest
        {
            public:
                /**
                * Constructor to set resource URI.
                *
                * @param resourceUri URI of the resource for which the request is generated.
                */
                explicit RCSRequest(const std::string &resourceUri);

                RCSRequest &operator=(RCSRequest &) = delete;

                /**
                * @return Returns the URI of the request.
                *
                */
                std::string getResourceUri() const;

            private:
                std::string m_resourceUri;
        };

    }
}

#endif // SERVERBUILDER_PRIMITIVEREQUEST_H

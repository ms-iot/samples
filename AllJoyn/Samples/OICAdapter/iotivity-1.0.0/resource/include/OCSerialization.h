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

#include <StringConstants.h>
#include "ocpayload.h"
#include "ocrandom.h"

namespace OC
{
    class ListenOCContainer
    {
        private:
            static std::vector<std::string> StringLLToVector(OCStringLL* ll)
            {
                std::vector<std::string> strs;
                while(ll)
                {
                    strs.push_back(ll->value);
                    ll = ll->next;
                }
                return strs;
            }

        public:
            ListenOCContainer(std::weak_ptr<IClientWrapper> cw,
                    OCDevAddr& devAddr, OCDiscoveryPayload* payload)
                    : m_clientWrapper(cw), m_devAddr(devAddr)
            {
                OCResourcePayload* res = payload->resources;
                OCResourceCollectionPayload* colRes = payload->collectionResources;
                if (res)
                {
                    while(res)
                    {
                        char uuidString[UUID_STRING_SIZE];
                        if(OCConvertUuidToString(res->sid, uuidString) != RAND_UUID_OK)
                        {
                            uuidString[0]= '\0';
                        }

                        if (res->secure)
                        {
                            m_devAddr.flags =
                                  (OCTransportFlags)(OC_FLAG_SECURE | m_devAddr.flags);
                        }

                        if (res->port != 0)
                        {
                             m_devAddr.port = res->port;
                        }

                        m_resources.push_back(std::shared_ptr<OC::OCResource>(
                                    new OC::OCResource(m_clientWrapper, m_devAddr,
                                        std::string(res->uri),
                                        std::string(uuidString),
                                        (res->bitmap & OC_OBSERVABLE) == OC_OBSERVABLE,
                                        StringLLToVector(res->types),
                                        StringLLToVector(res->interfaces)
                                        )));
                        res = res->next;
                    }
                }
                else if (colRes)
                {
                    while(colRes)
                    {
                        if (colRes->tags->bitmap & OC_SECURE)
                        {
                            m_devAddr.flags =
                                  (OCTransportFlags)(OC_FLAG_SECURE | m_devAddr.flags);
                        }

                        if (colRes->tags->port != 0)
                        {
                             m_devAddr.port = colRes->tags->port;
                        }

                        m_resources.push_back(std::shared_ptr<OC::OCResource>(
                                    new OC::OCResource(m_clientWrapper, m_devAddr,
                                        std::string(colRes->setLinks->href),
                                        std::string((char*)colRes->tags->di.id),
                                        (colRes->tags->bitmap & OC_OBSERVABLE) == OC_OBSERVABLE,
                                        StringLLToVector(colRes->setLinks->rt),
                                        StringLLToVector(colRes->setLinks->itf)
                                        )));
                        colRes = colRes->next;
                    }
                }
            }

            const std::vector<std::shared_ptr<OCResource>>& Resources() const
            {
                return m_resources;
            }
        private:
            std::vector<std::shared_ptr<OC::OCResource>> m_resources;
            std::weak_ptr<IClientWrapper> m_clientWrapper;
            OCDevAddr& m_devAddr;
    };
}

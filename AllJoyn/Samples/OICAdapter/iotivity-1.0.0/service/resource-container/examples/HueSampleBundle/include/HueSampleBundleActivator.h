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

#ifndef HUESAMPLEBUNDLEACTIVATOR_H_
#define HUESAMPLEBUNDLEACTIVATOR_H_

#include "ResourceContainerBundleAPI.h"
#include "BundleActivator.h"
#include "BundleResource.h"
#include "HueConnector.h"

using namespace OIC::Service;

namespace OIC
{
    namespace Service
    {
        class HueSampleBundleActivator: public BundleActivator
        {
            public:
                HueSampleBundleActivator();
                ~HueSampleBundleActivator();

                void activateBundle(ResourceContainerBundleAPI *resourceContainer,
                                    std::string bundleId);
                void deactivateBundle();

                void createResource(resourceInfo resourceInfo);
                void destroyResource(BundleResource::Ptr pBundleResource);

                std::string m_bundleId;
                ResourceContainerBundleAPI *m_pResourceContainer;
                std::vector< BundleResource::Ptr > m_vecResources;
            private:
                HueConnector *m_connector;
        };
    }
}

#endif /* HUESAMPLEBUNDLEACTIVATOR_H_ */

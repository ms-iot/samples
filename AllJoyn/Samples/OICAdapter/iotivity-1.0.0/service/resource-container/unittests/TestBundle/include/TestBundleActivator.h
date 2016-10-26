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

#ifndef TESTBUNDLE_H_
#define TESTBUNDLE_H_

#include <vector>

#include "ResourceContainerBundleAPI.h"
#include "BundleActivator.h"
#include "BundleResource.h"

using namespace OIC::Service;

class TestBundleActivator : public BundleActivator
{
    public:
        TestBundleActivator();
        ~TestBundleActivator();

        void activateBundle(ResourceContainerBundleAPI *resourceContainer, std::string bundleId);
        void deactivateBundle();

        void createResource(resourceInfo resourceInfo);
        void destroyResource(BundleResource::Ptr pBundleResource);

        ResourceContainerBundleAPI *m_pResourceContainer;
        std::string m_bundleId;
        BundleResource::Ptr m_pTestResource;
};

/*Fake bundle resource class for testing*/
class TestBundleResource : public BundleResource
{
    public:
        void initAttributes() { };

        RCSResourceAttributes &handleGetAttributesRequest()
        {
            return BundleResource::getAttributes();
        }

        void handleSetAttributesRequest(
            RCSResourceAttributes &value)
        {
            BundleResource::setAttributes(value);
        }
};

#endif /* TESTBUNDLE_H_ */
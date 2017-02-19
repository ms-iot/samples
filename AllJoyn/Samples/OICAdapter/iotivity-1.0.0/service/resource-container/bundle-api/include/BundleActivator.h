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

#ifndef BUNDLEACTIVATOR_H_
#define BUNDLEACTIVATOR_H_

#include "ResourceContainerBundleAPI.h"

using namespace OIC::Service;

namespace OIC
{
    namespace Service
    {

        /**
        * @class    BundleActivator
        * @brief    This class represents Bundle to be activated by container
        *
        */
        class BundleActivator
        {

            public:

                /**
                * Constructor for BundleActivator
                */
                BundleActivator();

                /**
                * Virtual destructor for BundleActivator
                */
                virtual ~BundleActivator();

                /**
                * Activate the Bundle to make bundle work and create bundle resources
                *
                * @param resourceContainer ResourceContainer which registers the bundle
                *
                * @param bundleId Assigned id for the bundle
                *
                * @return void
                */
                virtual void activateBundle(ResourceContainerBundleAPI *resourceContainer,
                                            std::string bundleId);

                /**
                * Deactivate the Bundle to stop working and destroy bundle resources
                *
                * @return void
                */
                virtual void deactivateBundle();

                /**
                * Create Bundle Resource instance and register the resource in the container
                *
                * @param resourceInfo Information of the bundle resource to be created
                *
                * @return void
                */
                virtual void createResource(resourceInfo resourceInfo) = 0;

                /**
                * Destroy Bundle Resource instance and register the resource in the container
                *
                * @param pBundleResource Bundle resource to be destroyed
                *
                * @return void
                */
                virtual void destroyResource(BundleResource::Ptr pBundleResource) = 0;
        };
    }
}

#endif /* RESOURCEBUNDLE_H_ */

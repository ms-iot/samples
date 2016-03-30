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
* This file contains the resource container Bundle APIs provided
*     to the resource bundle developers.
*/

#ifndef RESOURCECONTAINERBUNDLEAPI_H_
#define RESOURCECONTAINERBUNDLEAPI_H_

#include "Configuration.h"
#include "NotificationReceiver.h"
#include "BundleResource.h"

using namespace OIC::Service;

namespace OIC
{
    namespace Service
    {

        /**
        * @class   ResourceContainerBundleAPI
        * @brief   This class provides APIs for retrieving bundle and resource configuration
        *              and registering/unregistering resources.
        *
        */
        class ResourceContainerBundleAPI: public NotificationReceiver
        {
            public:
                /**
                * Register bundle resource in the container
                *   and register resource server for bundle resource
                *
                * @param resource bundle resource to register
                *
                * @return void
                */
                virtual void registerResource(BundleResource::Ptr resource) = 0;

                /**
                * Unregister bundle resource from the container
                *   and unregister resource server
                *
                * @param resource Bundle resource to unregister
                *
                * @return void
                */
                virtual void unregisterResource(BundleResource::Ptr resource) = 0;

                /**
                * Get Configuration data of certain bundle
                *
                * @param [in] bundleId Bundle id to get configuration data
                *
                * @param [out] configOutput Returned configuration data of bundle
                *
                * @return void
                */
                virtual void getBundleConfiguration(const std::string &bundleId, configInfo *configOutput) = 0;

                /**
                * Get the list of Configuration data of resources that certain bundle has
                *
                * @param [in] bundleId Bundle id to get configuration data
                *
                * @param [out] configOutput Returned vector of resource configuration data
                *
                * @return void
                */
                virtual void getResourceConfiguration(const std::string &bundleId,
                                                      std::vector< resourceInfo > *configOutput) = 0;

                /**
                * API for getting an instance of ResourceContainerBundleAPI
                *
                * @return ResourceContainerBundleAPI * Return the object pointer of ResourceContainerBundleAPI
                */
                static ResourceContainerBundleAPI *getInstance();

            protected:
                ResourceContainerBundleAPI();
                virtual ~ResourceContainerBundleAPI();

                ResourceContainerBundleAPI(const ResourceContainerBundleAPI &) = delete;
                ResourceContainerBundleAPI(ResourceContainerBundleAPI &&) = delete;
                ResourceContainerBundleAPI &operator=(const ResourceContainerBundleAPI &) const = delete;
                ResourceContainerBundleAPI &operator=(ResourceContainerBundleAPI &&) const = delete;
        };
    }
}

#endif

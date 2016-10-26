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
 * This file contains the resource container APIs provided to the developers.
 */

#ifndef RCSRESOURCECONTAINER_H_
#define RCSRESOURCECONTAINER_H_

#include <string>
#include <vector>
#include <map>
#include <list>
#include <memory>

#include "RCSBundleInfo.h"

namespace OIC
{
    namespace Service
    {

        /**
         * @class   RCSResourceContainer
         * @brief   This class provides APIs for managing the container and bundles in the container.
         *
         */
        class RCSResourceContainer
        {
            public:
                /**
                 * API for starting the Container
                 *
                 * @details This API start the container with the provided Configuration file.
                 *
                 * @param configFile configuration File that contains the Bundle/Bundles information.
                 *
                 */
                virtual void startContainer(const std::string &configFile) = 0;
                /**
                * API for stopping the Container
                */
                virtual void stopContainer() = 0;

                // list of bundle ids
                /**
                * API for getting the list of all bundles in the container.
                * The returned list and the contained bundle information are a copy
                * and will not be updated by the resource container.
                *
                * @return List of BundleInfo pointer each associated with a bundle
                *
                */
                virtual std::list<std::unique_ptr<RCSBundleInfo>> listBundles() = 0;
                /**
                 * API for starting the bundle.
                 *
                 * @param bundleId Id of the Bundle
                 *
                 */
                virtual void startBundle(const std::string &bundleId) = 0;
                /**
                * API for Stopping the bundle
                *
                * @param bundleId Id of the Bundle
                *
                */
                virtual void stopBundle(const std::string &bundleId) = 0;

                // dynamic configuration
                /**
                 * API for adding the bundle to the Container
                 *
                 * @param bundleId Id of the Bundle
                 * @param bundleUri Uri of the bundle
                 * @param bundlePath Path of the bundle
                 * @param activator Activation prefix for .so bundles, or activator class name for .jar bundles
                 * @param params  key-value pairs in string form for other Bundle parameters
                 *
                 */
                virtual void addBundle(const std::string &bundleId, const std::string &bundleUri,
                                       const std::string &bundlePath, const std::string &activator,
                                       std::map<std::string, std::string> params) = 0;
                /**
                 * API for removing the bundle from the container
                 *
                 * @param bundleId Id of the Bundle
                 *
                 */
                virtual void removeBundle(const std::string &bundleId) = 0;

                /**
                * API for adding the Resource configuration information to the bundle
                *
                * @param bundleId Id of the Bundle
                * @param resourceUri URI of the resource
                * @param params key-value pairs in string form for other Bundle parameters
                *
                */
                virtual void addResourceConfig(const std::string &bundleId, const std::string &resourceUri,
                                               std::map<std::string, std::string> params) = 0;
                /**
                * API for removing the Resource configuration information from the bundle
                *
                * @param bundleId Id of the Bundle
                * @param resourceUri URI of the resource
                *
                */
                virtual void removeResourceConfig(const std::string &bundleId, const std::string &resourceUri) = 0;

                /**
                * API for getting the list of Bundle Resources
                *
                * @param bundleId Id of the Bundle
                *
                */
                virtual std::list<std::string> listBundleResources(const std::string &bundleId) = 0;

                /**
                 * API for getting the Instance of ResourceContainer class
                 *
                 * @return Instance of the "RCSResourceContainer" class
                 *
                 */
                static RCSResourceContainer *getInstance();

            protected:
                RCSResourceContainer();
                virtual ~RCSResourceContainer();

                RCSResourceContainer(const RCSResourceContainer &) = delete;
                RCSResourceContainer(RCSResourceContainer &&) = delete;
                RCSResourceContainer &operator=(const RCSResourceContainer &) const = delete;
                RCSResourceContainer &operator=(RCSResourceContainer &&) const = delete;
        };
    }
}

#endif /* RCSRESOURCECONTAINER_H_ */

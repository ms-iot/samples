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
 * This file contains the RCSDiscoveryManager class which provides APIs to discover the
 * Resource in the network
 *
 */

#ifndef RCSDISCOVERYMANAGER_H
#define RCSDISCOVERYMANAGER_H

#include <memory>
#include <functional>

namespace OIC
{
    namespace Service
    {
        class RCSRemoteResourceObject;
        class RCSAddress;

        /**
         * This class contains the resource discovery methods.
         *
         * @see RCSRemoteResourceObject
         */
        class RCSDiscoveryManager
        {
            public:

            /**
             * This class represents a discovery task.
             *
             * @note A discovery task will be automatically canceled when destroyed.
             */
                class DiscoveryTask
                {
                    public:
                        typedef std::unique_ptr<DiscoveryTask> Ptr;

                        ~DiscoveryTask();

                        DiscoveryTask(const DiscoveryTask&) = delete;
                        DiscoveryTask(DiscoveryTask&&) = delete;
                        DiscoveryTask& operator = (const DiscoveryTask&) const = delete;
                        DiscoveryTask& operator = (DiscoveryTask&&) const = delete;

                        /**
                         * Cancel the task for discovery request. If cancel is called in duplicate, the request is ignored.
                         */
                        void cancel();

                        /**
                         * Return a boolean value whether the discovery request is canceled or not.
                         */
                        bool isCanceled();

                    private:
                        explicit DiscoveryTask(unsigned int);

                    private:
                        unsigned int m_id;
                        friend class RCSDiscoveryManagerImpl;
                };

            public:

                /**
                 * Typedef for callback of discoverResource APIs
                 *
                 * @see discoverResource
                 */
                typedef std::function< void(std::shared_ptr< RCSRemoteResourceObject >) >
                                       ResourceDiscoveredCallback;

                /**
                 * @return RCSDiscoveryManager instance.
                 *
                 */
                static RCSDiscoveryManager* getInstance();

                /**
                 * Discovering the resource of interest, regardless of uri and resource type.
                 * Find resource matching request periodically until returned resource is disappeared or destroyed.
                 *
                 * @return Returned object must be received.
                 *
                 * @param address         A RCSAddress object
                 * @param cb              A callback to obtain discovered resource
                 *
                 * @throws InvalidParameterException If cb is empty.
                 *
                 * @note The callback will be invoked in an internal thread.
                 *
                 */
                DiscoveryTask::Ptr discoverResource(const RCSAddress& address,
                        ResourceDiscoveredCallback cb);

                /**
                 * Discovering the resource of Interest, regardless of resource type.
                 * Find resource matching request periodically until returned resource is disappeared or destroyed.
                 *
                 * @return Returned object must be received.
                 *
                 * @param address          A RCSAddress object
                 * @param relativeURI      The relative uri of resource to be searched
                 * @param cb               A callback to obtain discovered resource
                 *
                 * @throws InvalidParameterException If cb is empty.
                 *
                 * @note The callback will be invoked in an internal thread.
                 *
                 * @see RCSAddress
                 *
                 */
                DiscoveryTask::Ptr discoverResource(const RCSAddress& address,
                        const std::string& relativeURI, ResourceDiscoveredCallback cb);

                /**
                 * Discovering the resource of Interest by Resource type.
                 * Find resource matching request periodically until returned resource is disappeared or destroyed.
                 *
                 * @return Returned object must be received.
                 *
                 * @param address          A RCSAddress object
                 * @param resourceType     Resource Type
                 * @param cb               A callback to obtain discovered resource
                 *
                 * @throws InvalidParameterException If cb is empty.
                 *
                 * @note The callback will be invoked in an internal thread.
                 *
                 * @see RCSAddress
                 *
                 */
                DiscoveryTask::Ptr discoverResourceByType(const RCSAddress& address,
                        const std::string& resourceType, ResourceDiscoveredCallback cb);

                /**
                 * Discovering the resource of Interest by Resource type with provided relativeURI.
                 * Find resource matching request periodically until returned resource is disappeared or destroyed.
                 *
                 * @return Returned object must be received.
                 *
                 * @param address          A RCSAddress object
                 * @param relativeURI      The relative uri of resource to be searched
                 * @param resourceType     Resource Type
                 * @param cb               A callback to obtain discovered resource
                 *
                 * @throws InvalidParameterException If cb is empty.
                 *
                 * @note The callback will be invoked in an internal thread.
                 *
                 * @see RCSAddress
                 *
                 */
                DiscoveryTask::Ptr discoverResourceByType(const RCSAddress& address,
                        const std::string& relativeURI, const std::string& resourceType,
                        ResourceDiscoveredCallback cb);

            private:
                RCSDiscoveryManager() = default;
                ~RCSDiscoveryManager()= default;

                friend class DiscoveryTask;
        };
    }
}
#endif // RCSDISCOVERYMANAGER_H

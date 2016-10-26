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

#ifndef DISCOVERRESOURCEUNIT_H_
#define DISCOVERRESOURCEUNIT_H_

#include <atomic>
#include <cstdbool>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "RCSDiscoveryManager.h"
#include "RCSRemoteResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RemoteResourceUnit.h"

namespace OIC
{
    namespace Service
    {
        class DiscoverResourceUnit
        {
            public:
                struct DiscoverResourceInfo
                {
                    DiscoverResourceInfo()
                        : resourceUri(), resourceType(), attributeName() {}
                    DiscoverResourceInfo(std::string uri, std::string type, std::string name)
                        : resourceUri(uri), resourceType(type), attributeName(name) {}
                    std::string resourceUri;
                    std::string resourceType;
                    std::string attributeName;
                };

                typedef std::shared_ptr<DiscoverResourceUnit> Ptr;
                typedef std::function<void(RemoteResourceUnit::UPDATE_MSG,
                                           RCSRemoteResourceObject::Ptr)> UpdatedCBFromServer;
                typedef std::function<void(const std::string attributeName,
                                           std::vector<RCSResourceAttributes::Value> values)>
                UpdatedCB;
                typedef RemoteResourceUnit::UPDATE_MSG REMOTE_MSG;

                DiscoverResourceUnit(const std::string &bundleId);
                ~DiscoverResourceUnit();

                void startDiscover(DiscoverResourceInfo info, UpdatedCB updatedCB);

            private:
                std::string m_bundleId;
                std::string m_Uri;
                std::string m_ResourceType;
                std::string m_AttrubuteName;
                std::atomic_bool isStartedDiscovery;
                std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> discoveryTask;

                std::vector<RemoteResourceUnit::Ptr> m_vecRemoteResource;
                RCSDiscoveryManager::ResourceDiscoveredCallback pDiscoveredCB;
                UpdatedCBFromServer pUpdatedCBFromServer;
                UpdatedCB pUpdatedCB;

                bool isAlreadyDiscoveredResource(RCSRemoteResourceObject::Ptr discoveredResource);
                void discoverdCB(RCSRemoteResourceObject::Ptr remoteObject, std::string uri);
                void onUpdate(REMOTE_MSG msg, RCSRemoteResourceObject::Ptr updatedResource);

                std::vector<RCSResourceAttributes::Value>
                buildInputResourceData(RCSRemoteResourceObject::Ptr updatedResource);
        };
    }
}

#endif // DISCOVERRESOURCEUNIT_H_

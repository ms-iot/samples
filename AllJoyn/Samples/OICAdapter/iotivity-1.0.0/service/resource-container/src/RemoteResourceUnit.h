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

#ifndef REMOTERESOURCEINFO_H_
#define REMOTERESOURCEINFO_H_

#include <mutex>

#include "RCSRemoteResourceObject.h"

namespace OIC
{
    namespace Service
    {
        class RemoteResourceUnit
        {
        public:
            enum class UPDATE_MSG
            {
                DATA_UPDATED,
                RESOURCE_DELETED
            };

            typedef std::shared_ptr<RemoteResourceUnit> Ptr;
            typedef std::function<void(UPDATE_MSG,
                RCSRemoteResourceObject::Ptr)> UpdatedCBFromServer;

            RemoteResourceUnit();
            ~RemoteResourceUnit();

        private:
            mutable std::mutex m_mutex;
            RCSRemoteResourceObject::Ptr remoteObject;

            UpdatedCBFromServer pUpdatedCB;
            RCSRemoteResourceObject::StateChangedCallback pStateChangedCB;
            RCSRemoteResourceObject::CacheUpdatedCallback pCacheUpdateCB;

            void stateChangedCB(ResourceState changedState) const;
            void cacheUpdateCB(const RCSResourceAttributes & updatedAtt) const;

        public:
            static RemoteResourceUnit::Ptr createRemoteResourceInfo(
                RCSRemoteResourceObject::Ptr ptr, UpdatedCBFromServer updatedCB);
            static RemoteResourceUnit::Ptr createRemoteResourceInfoWithStateCB(
                RCSRemoteResourceObject::Ptr ptr, UpdatedCBFromServer updatedCB,
                RCSRemoteResourceObject::StateChangedCallback stateCB);
            static RemoteResourceUnit::Ptr createRemoteResourceInfoWithCacheCB(
                RCSRemoteResourceObject::Ptr ptr, UpdatedCBFromServer updatedCB,
                RCSRemoteResourceObject::CacheUpdatedCallback cacheCB);

            RCSRemoteResourceObject::Ptr getRemoteResourceObject() const;
            std::string getRemoteResourceUri() const;

            void startCaching() const;
            void startMonitoring() const;
        };
    }
}

#endif // REMOTERESOURCEINFO_H_

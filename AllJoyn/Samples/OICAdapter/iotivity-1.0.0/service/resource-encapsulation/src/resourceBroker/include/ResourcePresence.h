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

#ifndef RB_RESOURCEPRESENCE_H_
#define RB_RESOURCEPRESENCE_H_

#include <functional>
#include <list>
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "BrokerTypes.h"
#include "ExpiryTimer.h"

namespace OIC
{
    namespace Service
    {
        class ResourcePresence : public std::enable_shared_from_this<ResourcePresence>
        {
        public:
            ResourcePresence();
            ~ResourcePresence();

            void initializeResourcePresence(PrimitiveResourcePtr pResource);

            void addBrokerRequester(BrokerID _id, BrokerCB _cb);
            void removeBrokerRequester(BrokerID _id);
            void removeAllBrokerRequester();

            void requestResourceState() const;
            void changePresenceMode(BROKER_MODE newMode);

            bool isEmptyRequester() const;
            int  requesterListSize() const;
            const PrimitiveResourcePtr getPrimitiveResource() const;
            BROKER_STATE getResourceState() const;

        private:
            std::unique_ptr<std::list<BrokerRequesterInfoPtr>> requesterList;
            PrimitiveResourcePtr primitiveResource;
            ExpiryTimer expiryTimer;

            BROKER_STATE state;
            BROKER_MODE mode;

            bool isWithinTime;
            std::atomic_long receivedTime;
            std::mutex cbMutex;
            unsigned int timeoutHandle;

            RequestGetCB pGetCB;
            TimerCB pTimeoutCB;
            TimerCB pPollingCB;

            void registerDevicePresence();
        public:
            void getCB(const HeaderOptions &hos, const ResponseStatement& rep, int eCode);
            void timeOutCB(unsigned int msg);
        private:
            void verifiedGetResponse(int eCode);

            void pollingCB(unsigned int msg = 0);

            void executeAllBrokerCB(BROKER_STATE changedState);
            void setResourcestate(BROKER_STATE _state);
        };
    } // namespace Service
} // namespace OIC

#endif /* RB_RESOURCEPRESENCE_H_ */


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

#ifndef RB_DEVICEPRESENCE_H_
#define RB_DEVICEPRESENCE_H_

#include <list>
#include <string>
#include <atomic>

#include "BrokerTypes.h"
#include "ResourcePresence.h"
#include "PresenceSubscriber.h"
#include "ExpiryTimer.h"

namespace OIC
{
    namespace Service
    {
        class DevicePresence
        {
        public:
            typedef long long TimerID;

            DevicePresence();
            ~DevicePresence();

            void initializeDevicePresence(PrimitiveResourcePtr pResource);

            void addPresenceResource(ResourcePresence * rPresence);
            void removePresenceResource(ResourcePresence * rPresence);

            bool isEmptyResourcePresence() const;
            const std::string getAddress() const;
            DEVICE_STATE getDeviceState() const noexcept;

        private:
            std::list<ResourcePresence * > resourcePresenceList;

            std::string address;
            std::atomic_int state;
            std::atomic_bool isRunningTimeOut;

            std::mutex timeoutMutex;
            std::condition_variable condition;

            ExpiryTimer presenceTimer;
            TimerID presenceTimerHandle;
            TimerCB pTimeoutCB;
            SubscribeCB pSubscribeRequestCB;
            PresenceSubscriber presenceSubscriber;

            void changeAllPresenceMode(BROKER_MODE mode);
            void subscribeCB(OCStackResult ret,const unsigned int seq, const std::string& Hostaddress);
            void timeOutCB(TimerID id);

            void setDeviceState(DEVICE_STATE);
        };
    } // namespace Service
} // namespace OIC

#endif /* RB_DEVICEPRESENCE_H_ */

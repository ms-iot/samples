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

#ifndef COMMON_PRESENCESUBSCRIBER_H
#define COMMON_PRESENCESUBSCRIBER_H

#include <string>
#include <functional>

#include <octypes.h>

namespace OIC
{
    namespace Service
    {

        typedef std::function< void(OCStackResult, const unsigned int, const std::string&) >
            SubscribeCallback;

        class PresenceSubscriber
        {
        public:
            PresenceSubscriber();

            PresenceSubscriber(PresenceSubscriber&&);

            /**
             * @throw PlatformException
             */
            PresenceSubscriber(const std::string& host, OCConnectivityType connectivityType,
                    SubscribeCallback presenceHandler);

            /**
             * @throw PlatformException
             */
            PresenceSubscriber(const std::string& host, const std::string& resourceType,
                    OCConnectivityType connectivityType, SubscribeCallback presenceHandler);

            ~PresenceSubscriber();

            PresenceSubscriber& operator=(PresenceSubscriber&&);

            /**
             * @throw PlatformException
             */
            void unsubscribe();

            bool isSubscribing() const;

        private:
            OCDoHandle m_handle;
        };

        /**
         * @throw PlatformException
         */
        void subscribePresence(OCDoHandle& handle, const std::string& host,
                OCConnectivityType connectivityType, SubscribeCallback presenceHandler);

        /**
         * @throw PlatformException
         */
        void subscribePresence(OCDoHandle& handle, const std::string& host, const std::string& resourceType,
                OCConnectivityType connectivityType, SubscribeCallback presenceHandler);

        void unsubscribePresence(OCDoHandle handle);

    }
}

#endif // COMMON_PRESENCESUBSCRIBER_H

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

#ifndef RB_RESOURCEBROKER_H_
#define RB_RESOURCEBROKER_H_

#include <functional>
#include <list>
#include <string>
#include <algorithm>
#include <mutex>
#include <condition_variable>

#include "BrokerTypes.h"
#include "ResourcePresence.h"

namespace OIC
{
    namespace Service
    {
        class ResourceBroker
        {
        public:
            class InvalidParameterException: public RCSException
            {
            public:
                InvalidParameterException(std::string&& what)
                : RCSException{ std::move(what) } {}
            };
            class FailedSubscribePresenceException: public RCSPlatformException
            {
            public:
                FailedSubscribePresenceException(OCStackResult reason)
                : RCSPlatformException{reason} {}
            };

            static ResourceBroker * getInstance();

            BrokerID hostResource(PrimitiveResourcePtr pResource, BrokerCB cb);
            void cancelHostResource(BrokerID brokerId);

            BROKER_STATE getResourceState(BrokerID brokerId);
            BROKER_STATE getResourceState(PrimitiveResourcePtr pResource);

        private:
            static ResourceBroker * s_instance;
            static std::mutex s_mutexForCreation;
            static std::unique_ptr<PresenceList>  s_presenceList;
            static std::unique_ptr<BrokerIDMap> s_brokerIDMap;

            ResourceBroker() = default;
            ~ResourceBroker();
            ResourceBroker(const ResourceBroker&) = delete;
            ResourceBroker(ResourceBroker&&) = delete;

            ResourceBroker& operator=(const ResourceBroker&) const = delete;
            ResourceBroker& operator=(ResourceBroker&&) const = delete;

            void initializeResourceBroker();
            BrokerID generateBrokerID();
            ResourcePresencePtr findResourcePresence(PrimitiveResourcePtr pResource);
        };
    } // namespace Service
} // namespace OIC

#endif /* RB_RESOURCEBROKER_H_ */

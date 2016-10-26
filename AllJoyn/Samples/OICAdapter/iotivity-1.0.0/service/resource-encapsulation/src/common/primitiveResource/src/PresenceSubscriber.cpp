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

#include <PresenceSubscriber.h>

#include <AssertUtils.h>

#include <OCPlatform.h>

namespace OIC
{
    namespace Service
    {

        void subscribePresence(OCDoHandle& handle, const std::string& host,
                OCConnectivityType connectivityType, SubscribeCallback presenceHandler)
        {
            typedef OCStackResult (*SubscribePresence)(OC::OCPlatform::OCPresenceHandle&,
                    const std::string&, OCConnectivityType, SubscribeCallback);

            invokeOCFunc(static_cast<SubscribePresence>(OC::OCPlatform::subscribePresence),
                    handle, host, connectivityType, std::move(presenceHandler));
        }

        void subscribePresence(OCDoHandle& handle, const std::string& host,
                const std::string& resourceType, OCConnectivityType connectivityType,
                SubscribeCallback presenceHandler)
        {
            typedef OCStackResult (*SubscribePresence)(OC::OCPlatform::OCPresenceHandle&,
                    const std::string&, const std::string&, OCConnectivityType, SubscribeCallback);

            invokeOCFunc(static_cast<SubscribePresence>(OC::OCPlatform::subscribePresence),
                    handle, host, resourceType, connectivityType, std::move(presenceHandler));
        }

        void unsubscribePresence(OCDoHandle handle)
        {
            invokeOCFunc(OC::OCPlatform::unsubscribePresence, handle);
        }


        PresenceSubscriber::PresenceSubscriber() :
            m_handle{ nullptr }
        {
        }

        PresenceSubscriber::PresenceSubscriber(PresenceSubscriber&& from) :
            m_handle{ nullptr }
        {
            std::swap(m_handle, from.m_handle);
        }

        PresenceSubscriber::PresenceSubscriber(const std::string& host,
                OCConnectivityType connectivityType, SubscribeCallback presenceHandler) :
                m_handle{ nullptr }
        {
            subscribePresence(m_handle, host, connectivityType, std::move(presenceHandler));
        }

        PresenceSubscriber::PresenceSubscriber(const std::string& host,
                const std::string& resourceType, OCConnectivityType connectivityType,
                SubscribeCallback presenceHandler) :
                m_handle{ nullptr }
        {
            subscribePresence(m_handle, host, resourceType, connectivityType,
                    std::move(presenceHandler));
        }

        PresenceSubscriber::~PresenceSubscriber()
        {
            if (m_handle)
            {
                try
                {
                    unsubscribe();
                }
                catch (...)
                {
                }
            }
        }

        PresenceSubscriber& PresenceSubscriber::operator=(PresenceSubscriber&& from)
        {
            unsubscribe();
            std::swap(m_handle, from.m_handle);
            return *this;
        }

        void PresenceSubscriber::unsubscribe()
        {
            if (m_handle == nullptr) return;

            unsubscribePresence(m_handle);

            m_handle = nullptr;
        }

        bool PresenceSubscriber::isSubscribing() const
        {
            return m_handle != nullptr;
        }

    }
}

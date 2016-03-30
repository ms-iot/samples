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

#include "BundleResource.h"

#include <list>
#include <string.h>

#include "InternalTypes.h"

namespace OIC
{
    namespace Service
    {
        BundleResource::BundleResource()
        {
            m_pNotiReceiver = nullptr;
        }

        BundleResource::~BundleResource()
        {
            m_pNotiReceiver = nullptr;
        }

        void BundleResource::registerObserver(NotificationReceiver *pNotiReceiver)
        {
            m_pNotiReceiver = pNotiReceiver;
        }

        std::list< std::string > BundleResource::getAttributeNames()
        {
            std::list< std::string > ret;
            for (RCSResourceAttributes::iterator it = m_resourceAttributes.begin();
                 it != m_resourceAttributes.end(); ++it)
            {
                ret.push_back(it->key());
            }
            return ret;
        }

        RCSResourceAttributes &BundleResource::getAttributes()
        {
            return m_resourceAttributes;
        }

        void BundleResource::setAttributes(RCSResourceAttributes &attrs)
        {
            for (RCSResourceAttributes::iterator it = attrs.begin(); it != attrs.end(); ++it)
            {
                OC_LOG_V(INFO, CONTAINER_TAG, "set attribute \(%s)'",
                         std::string(it->key() + "\', with " + it->value().toString()).c_str());

                m_resourceAttributes[it->key()] = it->value();
            }
        }

        void BundleResource::setAttribute(const std::string &key,
                                          RCSResourceAttributes::Value &&value, bool notify)
        {
            OC_LOG_V(INFO, CONTAINER_TAG, "set attribute \(%s)'", std::string(key + "\', with " +
                     value.toString()).c_str());

            m_resourceAttributes[key] = value;

            if (notify && m_pNotiReceiver)
                m_pNotiReceiver->onNotificationReceived(m_uri);
        }

        void BundleResource::setAttribute(const std::string &key, RCSResourceAttributes::Value &&value)
        {
            setAttribute(key, std::move(value), true);
        }

        RCSResourceAttributes::Value BundleResource::getAttribute(const std::string &key)
        {
            OC_LOG_V(INFO, CONTAINER_TAG, "get attribute \'(%s)" , std::string(key + "\'").c_str());

            return m_resourceAttributes.at(key);
        }
    }
}

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

#ifndef BUNDLERESOURCE_H_
#define BUNDLERESOURCE_H_

#include <list>
#include <string>
#include <map>
#include <vector>
#include <memory>

#include "NotificationReceiver.h"
#include "RCSResourceAttributes.h"

namespace OIC
{
    namespace Service
    {

        /**
        * @class    BundleResource
        * @brief    This class represents Basic bundle resource template
        *               to be registered in the container and make resource server
        *
        */
        class BundleResource
        {
            public:
                typedef std::shared_ptr< BundleResource > Ptr;

                /**
                * Constructor for BundleResource
                */
                BundleResource();

                /**
                * Virtual destructor for BundleResource
                */
                virtual ~BundleResource();

                /**
                * Return the list of attribute names of the resource
                *
                * @return List of the attribute names
                */
                std::list<std::string> getAttributeNames();

                /**
                * Initialize attributes of the resource
                *
                * @return void
                */
                virtual void initAttributes() = 0;

                /**
                * Register notification receiver(resource container) to notify for the
                *     changes of attributes
                *
                * @param pNotiReceiver Notification Receiver to get notification from
                * bundle resource
                *
                * @return void
                */
                void registerObserver(NotificationReceiver *pNotiReceiver);

                /**
                * Return all attributes of the resource
                *
                * @return Attributes of the resource
                */
                RCSResourceAttributes &getAttributes();

                /**
                * Set attributes of the resource
                *
                * @param attrs Attributes to set
                *
                * @return void
                */
                void setAttributes(RCSResourceAttributes &attrs);

                /**
                * Return the value of an attribute
                *
                * @param key Key of attribute to get
                *
                * @return Value of the attribute
                */
                RCSResourceAttributes::Value getAttribute(const std::string &key);

                /**
                * Sets the value of an attribute
                *
                * @param key Name of attribute to set
                *
                * @param value Value of attribute to set
                *
                * @param notify Flag to indicate if OIC clients should be notified about an update
                *
                * @return void
                */
                void setAttribute(const std::string &key, RCSResourceAttributes::Value &&value,
                                  bool notify);

                /**
                * Sets the value of an attribute
                *
                * @param key Name of attribute to set
                *
                * @param value Value of attribute to set
                *
                * @return void
                */
                void setAttribute(const std::string &key, RCSResourceAttributes::Value &&value);

                /**
                * This function should be implemented by the according bundle resource
                * and execute the according business logic (e.g., light switch or sensor resource)
                * to retrieve a sensor value. If a new sensor value is retrieved, the
                * setAttribute data should be called to update the value.
                * The implementor of the function can decide weather to notify OIC clients
                * about the changed state or not.
                *
                * @return All attributes
                */
                virtual RCSResourceAttributes &handleGetAttributesRequest() = 0;

                /**
                * This function should be implemented by the according bundle resource
                * and execute the according business logic (e.g., light switch or sensor resource)
                * and write either on soft sensor values or external bridged devices.
                *
                * The call of this method could for example trigger a HTTP PUT request on
                * an external APIs. This method is responsible to update the resource internal
                * data and call the setAttribute method.
                *
                * The implementor of the function can decide weather to notify OIC clients
                * about the changed state or not.
                *
                * @param attrs Attributes to set
                *
                * @return void
                */
                virtual void handleSetAttributesRequest(RCSResourceAttributes &attrs) = 0;


            public:
                std::string m_bundleId;
                std::string m_name, m_uri, m_resourceType, m_address;
                std::map< std::string,
                    std::vector< std::map< std::string, std::string > > > m_mapResourceProperty;

            private:
                NotificationReceiver *m_pNotiReceiver;
                RCSResourceAttributes m_resourceAttributes;
        };
    }
}

#endif

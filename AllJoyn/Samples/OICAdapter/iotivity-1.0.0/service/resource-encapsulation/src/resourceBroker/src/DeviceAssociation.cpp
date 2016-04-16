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

#include "DeviceAssociation.h"
#include "DevicePresence.h"


namespace OIC
{
    namespace Service
    {
        DeviceAssociation * DeviceAssociation::s_instance = nullptr;
        std::mutex DeviceAssociation::s_mutexForCreation;
        std::list< DevicePresencePtr >  DeviceAssociation::s_deviceList;

        DeviceAssociation::DeviceAssociation()
        {
            // TODO Auto-generated constructor stub
        }

        DeviceAssociation::~DeviceAssociation()
        {
            // TODO Auto-generated destructor stub
        }

        DeviceAssociation * DeviceAssociation::getInstance()
        {
            if (!s_instance)
            {
                s_mutexForCreation.lock();
                if (!s_instance)
                {
                    s_instance = new DeviceAssociation();
                }
                s_mutexForCreation.unlock();
            }
            return s_instance;
        }

        DevicePresencePtr DeviceAssociation::findDevice(const std::string & address)
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"findDevice()");
            DevicePresencePtr retDevice = nullptr;
            for(auto it : s_deviceList)
            {
                if(address == it->getAddress())
                {
                    OC_LOG_V(DEBUG,BROKER_TAG,"find device in deviceList");
                    retDevice = it;
                    break;
                }
            }

            return retDevice;
        }

        void DeviceAssociation::addDevice(DevicePresencePtr dPresence)
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"addDevice()");
            DevicePresencePtr foundDevice = findDevice(dPresence->getAddress());
            if(foundDevice == nullptr)
            {
                OC_LOG_V(DEBUG,BROKER_TAG,"add device in deviceList");
                s_deviceList.push_back(dPresence);
            }
        }

        void DeviceAssociation::removeDevice(DevicePresencePtr dPresence)
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"removeDevice()");
            DevicePresencePtr foundDevice = findDevice(dPresence->getAddress());
            if(foundDevice != nullptr)
            {
                OC_LOG_V(DEBUG,BROKER_TAG,"remove device in deviceList");
                s_deviceList.remove(foundDevice);
                foundDevice.reset();
            }
        }

        bool DeviceAssociation::isEmptyDeviceList()
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"isEmptyDeviceList()");
            return s_deviceList.empty();
        }
    } // namespace Service
} // namespace OIC

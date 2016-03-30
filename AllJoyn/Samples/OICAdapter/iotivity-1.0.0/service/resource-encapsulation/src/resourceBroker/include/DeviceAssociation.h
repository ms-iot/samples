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

#ifndef RB_DEVICEASSOCIATION_H_
#define RB_DEVICEASSOCIATION_H_

#include <list>
#include <string>
#include <algorithm>
#include <mutex>
#include <condition_variable>

#include "BrokerTypes.h"


namespace OIC
{
    namespace Service
    {
        class DeviceAssociation {
        public:

            static DeviceAssociation * getInstance();

            DevicePresencePtr findDevice(const std::string & address);
            void addDevice(DevicePresencePtr dPresence);
            void removeDevice(DevicePresencePtr dPresence);
            bool isEmptyDeviceList();

        private:
            DeviceAssociation();
            ~DeviceAssociation();

            static DeviceAssociation * s_instance;
            static std::mutex s_mutexForCreation;
            static std::list< DevicePresencePtr > s_deviceList;
        };
    } // namespace Service
} // namespace OIC

#endif /* RB_DEVICEASSOCIATION_H_ */

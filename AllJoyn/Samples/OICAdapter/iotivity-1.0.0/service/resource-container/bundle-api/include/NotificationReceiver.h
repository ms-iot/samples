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

#ifndef NOTIFICATIONRECEIVER_H_
#define NOTIFICATIONRECEIVER_H_

#include <string>

namespace OIC
{
    namespace Service
    {

        /**
        * @class    NotificationReceiver
        * @brief    This class represents Notification Receiver to get notification
        *               from bundle resources if there's any changes of attribute state
        *
        */
        class NotificationReceiver
        {
            public:

                /**
                * Constructor for NotificationReceiver
                */
                NotificationReceiver() {};

                /**
                * destructor for NotificationReceiver
                */
                ~NotificationReceiver() {};

                /**
                * Callback method for getting notification from bundle resources
                *
                * @param strResourceUri Uri of attribute updated bundle resource
                *
                * @return void
                */
                virtual void onNotificationReceived(const std::string &strResourceUri) = 0;
        };
    }
}

#endif

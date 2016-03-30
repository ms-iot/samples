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

#ifndef PROTOCOLBRIDGECONNECTOR_H_
#define PROTOCOLBRIDGECONNECTOR_H_

#include "BundleResource.h"
#include <map>
#include <string>

namespace OIC
{
    namespace Service
    {

        /**
        * @class    ProtocolBridgeConnector
        * @brief    This class represents connector
        *               to bridge non-IoTivity protocol and IoTivity for Protocol Bridge
        *
        */
        class ProtocolBridgeConnector
        {
            public:

                /**
                * Constructor for ProtocolBridgeConnector
                */
                ProtocolBridgeConnector();

                /**
                * Virtual destructor for ProtocolBridgeConnector
                */
                virtual ~ProtocolBridgeConnector();

                /**
                * Execute the logic needed for connection with different protocol from IoTivity
                *
                * @return void
                */
                virtual void connect() = 0;

                /**
                * Execute the logic needed for disconnection with different protocol from IoTivity
                *
                * @return void
                */
                virtual void disconnect() = 0;
        };
    }
}

#endif

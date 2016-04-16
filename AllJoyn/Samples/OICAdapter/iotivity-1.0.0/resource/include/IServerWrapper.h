//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

#ifndef _I_SERVER_WRAPPER_H_
#define _I_SERVER_WRAPPER_H_

#include <memory>
#include <string>

#include <OCResourceRequest.h>
#include <OCResourceResponse.h>
#include <OCException.h>
#include <OCApi.h>

namespace OC
{
    class IServerWrapper
    {
    protected:

    public:
        typedef std::shared_ptr<IServerWrapper> Ptr;

        IServerWrapper()
        {}

        virtual ~IServerWrapper(){};

        virtual OCStackResult registerResource(
                    OCResourceHandle& resourceHandle,
                    std::string& resourceURI,
                    const std::string& resourceTypeName,
                    const std::string& resourceInterface,
                    EntityHandler& entityHandler,
                    uint8_t resourceProperty) = 0;

        virtual OCStackResult registerDeviceInfo(
                    const OCDeviceInfo deviceInfo) = 0;

        virtual OCStackResult registerPlatformInfo(
                    const OCPlatformInfo PlatformInfo) = 0;

        virtual OCStackResult unregisterResource(
                    const OCResourceHandle& resourceHandle) = 0;
        virtual OCStackResult bindTypeToResource(
                    const OCResourceHandle& resourceHandle,
                    const std::string& resourceTypeName) = 0;

        virtual OCStackResult bindInterfaceToResource(
                    const OCResourceHandle& resourceHandle,
                    const std::string& resourceInterfaceName) = 0;

        virtual OCStackResult startPresence(const unsigned int seconds) = 0;

        virtual OCStackResult stopPresence() = 0;

        virtual OCStackResult setDefaultDeviceEntityHandler(EntityHandler entityHandler) = 0;

        virtual OCStackResult sendResponse(const std::shared_ptr<OCResourceResponse> pResponse) = 0;
    };
}

#endif

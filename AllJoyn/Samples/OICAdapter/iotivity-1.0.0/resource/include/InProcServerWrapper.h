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

#ifndef _IN_PROC_SERVER_WRAPPER_H_
#define _IN_PROC_SERVER_WRAPPER_H_

#include <thread>
#include <mutex>

#include <IServerWrapper.h>

namespace OC
{
    class InProcServerWrapper : public IServerWrapper
    {
    public:
        InProcServerWrapper(
            std::weak_ptr<std::recursive_mutex> csdkLock,
            PlatformConfig cfg);
        virtual ~InProcServerWrapper();

        virtual OCStackResult registerResource(
                    OCResourceHandle& resourceHandle,
                    std::string& resourceURI,
                    const std::string& resourceTypeName,
                    const std::string& resourceInterface,
                    EntityHandler& entityHandler,
                    uint8_t resourceProperty);

        virtual OCStackResult registerDeviceInfo(
                    const OCDeviceInfo deviceInfo);

        virtual OCStackResult registerPlatformInfo(
                    const OCPlatformInfo PlatformInfo);

        virtual OCStackResult unregisterResource(
                    const OCResourceHandle& resourceHandle);

        virtual OCStackResult bindTypeToResource(
                    const OCResourceHandle& resourceHandle,
                    const std::string& resourceTypeName);

        virtual OCStackResult bindInterfaceToResource(
                    const OCResourceHandle& resourceHandle,
                    const std::string& resourceInterface);

        virtual OCStackResult startPresence(const unsigned int seconds);

        virtual OCStackResult stopPresence();

        virtual OCStackResult setDefaultDeviceEntityHandler(EntityHandler entityHandler);

        virtual OCStackResult sendResponse(const std::shared_ptr<OCResourceResponse> pResponse);
    private:
        void processFunc();
        std::thread m_processThread;
        bool m_threadRun;
        std::weak_ptr<std::recursive_mutex> m_csdkLock;
    };
}

#endif

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

#ifndef _I_CLIENT_WRAPPER_H_
#define _I_CLIENT_WRAPPER_H_

#include <memory>
#include <string>

#include <OCApi.h>

namespace OC
{
    class OCPlatform_impl;

    class IClientWrapper : public std::enable_shared_from_this<IClientWrapper>
    {
    protected:

    public:
        typedef std::shared_ptr<IClientWrapper> Ptr;

        IClientWrapper()
        {}

        virtual OCStackResult ListenForResource(const std::string& serviceUrl,
                        const std::string& resourceType,
                        OCConnectivityType connectivityType,
                        FindCallback& callback,
                        QualityOfService QoS) = 0;

        virtual OCStackResult ListenForDevice(const std::string& serviceUrl,
                        const std::string& deviceURI,
                        OCConnectivityType connectivityType,
                        FindDeviceCallback& callback,
                        QualityOfService QoS) = 0;

        virtual OCStackResult GetResourceRepresentation(
                        const OCDevAddr& devAddr,
                        const std::string& uri,
                        const QueryParamsMap& queryParams,
                        const HeaderOptions& headerOptions,
                        GetCallback& callback, QualityOfService QoS)=0;

        virtual OCStackResult PutResourceRepresentation(
                        const OCDevAddr& devAddr,
                        const std::string& uri,
                        const OCRepresentation& rep, const QueryParamsMap& queryParams,
                        const HeaderOptions& headerOptions,
                        PutCallback& callback, QualityOfService QoS) = 0;

        virtual OCStackResult PostResourceRepresentation(
                        const OCDevAddr& devAddr,
                        const std::string& uri,
                        const OCRepresentation& rep, const QueryParamsMap& queryParams,
                        const HeaderOptions& headerOptions,
                        PostCallback& callback, QualityOfService QoS) = 0;

        virtual OCStackResult DeleteResource(
                        const OCDevAddr& devAddr,
                        const std::string& uri,
                        const HeaderOptions& headerOptions,
                        DeleteCallback& callback, QualityOfService QoS) = 0;

        virtual OCStackResult ObserveResource(
                        ObserveType observeType, OCDoHandle* handle,
                        const OCDevAddr& devAddr,
                        const std::string& uri,
                        const QueryParamsMap& queryParams,
                        const HeaderOptions& headerOptions, ObserveCallback& callback,
                        QualityOfService QoS)=0;

        virtual OCStackResult CancelObserveResource(
                        OCDoHandle handle,
                        const std::string& host,
                        const std::string& uri,
                        const HeaderOptions& headerOptions,
                        QualityOfService QoS)=0;

        virtual OCStackResult SubscribePresence(OCDoHandle* handle,
                        const std::string& host,
                        const std::string& resourceType,
                        OCConnectivityType connectivityType,
                        SubscribeCallback& presenceHandler)=0;

        virtual OCStackResult UnsubscribePresence(OCDoHandle handle) =0;

        virtual OCStackResult GetDefaultQos(QualityOfService& qos) = 0;

        virtual ~IClientWrapper(){}
    };
}

#endif


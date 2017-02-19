/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "simulator_client.h"
#include "simulator_remote_resource_impl.h"
#include "simulator_logger.h"
#include "simulator_utils.h"
#include "logger.h"

#define TAG "SIMULATOR_CLIENT"

SimulatorClient *SimulatorClient::getInstance()
{
    static SimulatorClient s_instance;
    return &s_instance;
}

void SimulatorClient::findResources(ResourceFindCallback callback)
{
    if (!callback)
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");

    typedef OCStackResult (*FindResource)(const std::string &, const std::string &,
                                          OCConnectivityType, OC::FindCallback);

    invokeocplatform(static_cast<FindResource>(OC::OCPlatform::findResource), "",
                     OC_MULTICAST_DISCOVERY_URI,
                     CT_DEFAULT,
                     static_cast<OC::FindCallback>(std::bind(&SimulatorClient::onResourceFound, this,
                             std::placeholders::_1, callback)));
}

void SimulatorClient::findResources(const std::string &resourceType,
                                    ResourceFindCallback callback)
{
    if (resourceType.empty())
        throw InvalidArgsException(SIMULATOR_INVALID_TYPE, "resource type is empty!");

    if (!callback)
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");

    std::ostringstream query;
    query << OC_MULTICAST_DISCOVERY_URI << "?rt=" << resourceType;

    typedef OCStackResult (*FindResource)(const std::string &, const std::string &,
                                          OCConnectivityType, OC::FindCallback);

    invokeocplatform(static_cast<FindResource>(OC::OCPlatform::findResource), "", query.str(),
                     CT_DEFAULT,
                     static_cast<OC::FindCallback>(std::bind(&SimulatorClient::onResourceFound,
                             this, std::placeholders::_1, callback)));
}

void SimulatorClient::onResourceFound(std::shared_ptr<OC::OCResource> ocResource,
                                      ResourceFindCallback callback)
{
    if (!ocResource)
    {
        OC_LOG(ERROR, TAG, "Invalid OCResource !");
        return;
    }

    // Construct SimulatorRemoteResource
    SimulatorRemoteResourceSP simulatorResource =
        std::make_shared<SimulatorRemoteResourceImpl>(ocResource);
    if (!simulatorResource)
    {
        OC_LOG(ERROR, TAG, "Failed to create simulator remote resource !");
        return;
    }

    OC_LOG(DEBUG, TAG, "Invoking resource found client callback !");
    callback(simulatorResource);
}



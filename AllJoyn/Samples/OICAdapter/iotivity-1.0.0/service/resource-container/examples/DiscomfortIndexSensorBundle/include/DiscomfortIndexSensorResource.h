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

#ifndef DISCOMFORTINDEXSENSORRESOURCE_H_
#define DISCOMFORTINDEXSENSORRESOURCE_H_

#include "SoftSensorResource.h"
#include "DiscomfortIndexSensor.h"

using namespace DiscomfortIndexSensorName;
using namespace OIC::Service;

class DiscomfortIndexSensorResource : public SoftSensorResource
{
    public:
        DiscomfortIndexSensorResource();
        ~DiscomfortIndexSensorResource();

        virtual void handleSetAttributesRequest(RCSResourceAttributes &attrs);

        virtual RCSResourceAttributes &handleGetAttributesRequest();

        virtual void executeLogic();

        virtual void onUpdatedInputResource(const std::string attributeName,
                                            std::vector<RCSResourceAttributes::Value> values);


    private:
        DiscomfortIndexSensor *m_pDiscomfortIndexSensor;
        std::map<std::string, std::string> m_mapInputData;
};

#endif

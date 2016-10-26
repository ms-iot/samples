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

#ifndef HEIGHTSENSORAPP_H_
#define HEIGHTSENSORAPP_H_

#include <functional>
#include <pthread.h>
#include "OCPlatform.h"
#include "OCApi.h"

#include <string>
#include <cstdlib>


#define COAP_IP                 "0.0.0.0"
#define COAP_PORT               0
#define COAP_MODE               OC::ModeType::Server
#define COAP_SRVTYPE            OC::ServiceType::InProc

#define RESOURCE_TYPE_NAME          "oic.r.sensor.height"

//  testing case
#define INTERVAL_FOR_CHECK          4   // seconds
#define INTERVAL_FOR_MEASUREMENT    3   // seconds
#define INIT_VAL                    1   // default value
#define DIFF_VAL                    0.02   // default value


class HeightResource
{
    public:
        // Access this property from a TB client
        double m_height;
        std::string m_resourceUri;
        std::vector<std::string> m_resourceTypes;
        std::vector<std::string> m_resourceInterfaces;
        OCResourceHandle m_resourceHandle;
        OC::OCRepresentation m_resourceRep;
        OC::ObservationIds m_interestedObservers;

    public:
        // Constructor
        HeightResource() : m_height(INIT_VAL), m_resourceHandle(0)
        {
            m_resourceUri = "/Thing_HeightSensor";
            m_resourceTypes.push_back(RESOURCE_TYPE_NAME);
            m_resourceInterfaces.push_back(OC::DEFAULT_INTERFACE);

            printf("Running thing as %s\n", m_resourceUri.c_str());
            m_resourceRep.setUri(m_resourceUri);
            m_resourceRep.setResourceTypes(m_resourceTypes);
            m_resourceRep.setResourceInterfaces(m_resourceInterfaces);
        }

        ~HeightResource()
        {
        }

        void registerResource();

        OCResourceHandle getHandle();

        void setResourceRepresentation(OC::OCRepresentation &rep);

        OC::OCRepresentation getResourceRepresentation();
};

#endif /* THINGRESOURCESERVER_H_ */

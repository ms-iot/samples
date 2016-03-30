//******************************************************************
//
// Copyright 2014 Intel Corporation.
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

#ifndef SERVER_H_
#define SERVER_H_

#include <string>
#include <iostream>
#include <memory>
#include "ocstack.h"
#include "observer.h"
#include "OCPlatform.h"
#include "OCApi.h"

using namespace std;
using namespace OC;

class IoTServer
{
    shared_ptr<PlatformConfig> m_platformConfig;
    OCRepresentation m_temperatureRepresentation;
    OCResourceHandle m_temperatureResource;
    OCRepresentation m_ambientLightRepresentation;
    OCResourceHandle m_ambientLightResource;
    OCRepresentation m_ledRepresentation;
    OCResourceHandle m_ledResource;
    ObservationIds m_ambientLightObservers;
    ObservationIds m_temperatureObservers;
    shared_ptr<IoTObserver> m_ambientLightObserverLoop;
    shared_ptr<IoTObserver> m_temperatureObserverLoop;

    void initializePlatform();
    void setupResources();
    void createResource(string, string, EntityHandler, OCResourceHandle&);

    OCRepresentation getTemperatureRepresentation();
    OCRepresentation getLightRepresentation();
    OCRepresentation getLEDRepresentation();
    void putLEDRepresentation();

    //Polling threads to periodically query sensor values and notify
    //subscribers.
    void temperatureObserverLoop();
    void lightObserverLoop();

    OCEntityHandlerResult temperatureEntityHandler(shared_ptr<OCResourceRequest>);
    OCEntityHandlerResult lightEntityHandler(shared_ptr<OCResourceRequest>);
    OCEntityHandlerResult LEDEntityHandler(shared_ptr<OCResourceRequest>);
public:
    IoTServer();
    virtual ~IoTServer();
};

#endif /* SERVER_H_ */

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

#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include <iostream>
#include <memory>
#include "ocstack.h"
#include "OCApi.h"
#include "OCPlatform.h"
#include "OCResource.h"

using namespace std;
using namespace OC;

class LED
{
    shared_ptr<OCResource> m_resourceHandle;
    OCRepresentation m_ledRepresentation;
    GetCallback m_GETCallback;
    PutCallback m_PUTCallback;
    void onGet(const HeaderOptions&, const OCRepresentation&, int);
    void onPut(const HeaderOptions&, const OCRepresentation&, int);
public:
    void get();
    void put(int);
    LED(shared_ptr<OCResource> Resource);
    virtual ~LED();
};

class TemperatureSensor
{
    shared_ptr<OCResource> m_resourceHandle;
    OCRepresentation m_temperatureRepresentation;
    GetCallback m_GETCallback;
    ObserveCallback m_OBSERVECallback;
    bool m_isObserved;
    void onObserve(const HeaderOptions headerOptions, const OCRepresentation& rep, int eCode,
                   int sequenceNumber);
    void onGet(const HeaderOptions&, const OCRepresentation&, int);
public:
    void get();
    void startObserve();
    void stopObserve();
    TemperatureSensor(shared_ptr<OCResource> Resource);
    virtual ~TemperatureSensor();
};

class AmbientLight
{
    shared_ptr<OCResource> m_resourceHandle;
    OCRepresentation m_ledRepresentation;
    GetCallback m_GETCallback;
    ObserveCallback m_OBSERVECallback;
    bool m_isObserved;
    void onObserve(const HeaderOptions headerOptions, const OCRepresentation& rep, int eCode,
                   int sequenceNumber);
    void onGet(const HeaderOptions&, const OCRepresentation&, int);
public:
    void get();
    void startObserve();
    void stopObserve();
    AmbientLight(shared_ptr<OCResource> Resource);
    virtual ~AmbientLight();
};

class IoTClient
{
    shared_ptr<TemperatureSensor> m_temperatureSensor;
    shared_ptr<AmbientLight> m_ambientLightSensor;
    shared_ptr<LED> m_platformLED;
    shared_ptr<PlatformConfig> m_platformConfig;
    FindCallback m_resourceDiscoveryCallback;
    void initializePlatform();
    void discoveredResource(shared_ptr<OCResource>);
public:
    shared_ptr<TemperatureSensor> getTemperatureSensor();
    shared_ptr<AmbientLight> getAmbientLightSensor();
    shared_ptr<LED> getPlatformLED();
    void findResource();
    IoTClient();
    virtual ~IoTClient();
    static void DisplayMenu();
};

#endif /* CLIENT_H_ */

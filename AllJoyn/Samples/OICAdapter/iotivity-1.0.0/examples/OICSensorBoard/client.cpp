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

#include "client.h"
#include "namedefs.h"

IoTClient::IoTClient()
{
    cout << "Running IoTClient constructor" << endl;
    initializePlatform();
}

IoTClient::~IoTClient()
{
    cout << "Running IoTClient destructor" << endl;
}

void IoTClient::initializePlatform()
{
    m_platformConfig = make_shared<PlatformConfig>(ServiceType::InProc, ModeType::Client, "0.0.0.0",
                                                   0, OC::QualityOfService::HighQos);
    OCPlatform::Configure(*m_platformConfig);
    m_resourceDiscoveryCallback = bind(&IoTClient::discoveredResource, this, placeholders::_1);
}

void IoTClient::findResource()
{
    string coap_multicast_discovery = string(OC_RSRVD_WELL_KNOWN_URI "?if=" EDISON_RESOURCE_INTERFACE);
    OCPlatform::findResource("", coap_multicast_discovery.c_str(),  CT_DEFAULT, m_resourceDiscoveryCallback,
                             OC::QualityOfService::LowQos);
}

void IoTClient::discoveredResource(shared_ptr<OCResource> Resource)
{
    try
    {
        if (Resource)
        {
            string resourceUri = Resource->uri();
            string hostAddress = Resource->host();

            cout << "\nFound Resource" << endl << "Resource Types:" << endl;
            for (auto& resourceTypes : Resource->getResourceTypes())
            {
                cout << "\t" << resourceTypes << endl;
            }

            cout << "Resource Interfaces: " << endl;
            for (auto& resourceInterfaces : Resource->getResourceInterfaces())
            {
                cout << "\t" << resourceInterfaces << endl;
            }
            cout << "Resource uri: " << resourceUri << endl;
            cout << "host: " << hostAddress << endl;

            if (resourceUri == TEMPERATURE_RESOURCE_ENDPOINT)
            {
                m_temperatureSensor = make_shared<TemperatureSensor>(Resource);
            }
            else if (resourceUri == LIGHT_RESOURCE_ENDPOINT)
            {
                m_ambientLightSensor = make_shared<AmbientLight>(Resource);
            }
            else if (resourceUri == LED_RESOURCE_ENDPOINT)
            {
                m_platformLED = make_shared<LED>(Resource);
            }
        }
        IoTClient::DisplayMenu();
    }
    catch (OCException& ex)
    {
        cerr << "Caught exception in discoveredResource: " << ex.reason() << endl;
    }
}

shared_ptr<TemperatureSensor> IoTClient::getTemperatureSensor()
{
    return m_temperatureSensor;
}

shared_ptr<AmbientLight> IoTClient::getAmbientLightSensor()
{
    return m_ambientLightSensor;
}

shared_ptr<LED> IoTClient::getPlatformLED()
{
    return m_platformLED;
}

TemperatureSensor::TemperatureSensor(shared_ptr<OCResource> Resource)
{
    m_isObserved = false;
    m_resourceHandle = Resource;
    m_GETCallback = bind(&TemperatureSensor::onGet, this, placeholders::_1, placeholders::_2,
                         placeholders::_3);
    m_OBSERVECallback = bind(&TemperatureSensor::onObserve, this, placeholders::_1,
                             placeholders::_2, placeholders::_3, placeholders::_4);
}

TemperatureSensor::~TemperatureSensor()
{

}

void TemperatureSensor::startObserve()
{
    if (!m_isObserved)
    {
        cout << "Starting observing temperature sensor" << endl;
        m_resourceHandle->observe(ObserveType::Observe, QueryParamsMap(), m_OBSERVECallback);
    }
    m_isObserved = true;
}

void TemperatureSensor::stopObserve()
{
    if (m_isObserved)
    {
        m_resourceHandle->cancelObserve();
        cout << "Stopped observing temperature sensor" << endl;
    }
    m_isObserved = false;
}

void TemperatureSensor::onObserve(const HeaderOptions headerOptions, const OCRepresentation& rep,
                                  int eCode, int sequenceNumber)
{
    if (eCode == OC_STACK_OK && sequenceNumber != OC_OBSERVE_NO_OPTION)
    {
        if(sequenceNumber == OC_OBSERVE_REGISTER)
        {
            cout << "Observe registration action is successful" << endl;
        }
        else if(sequenceNumber == OC_OBSERVE_DEREGISTER)
        {
            cout << "Observe De-registration action is successful" << endl;
        }
        double value;
        rep.getValue(TEMPERATURE_RESOURCE_KEY, value);
        cout << "Observing TemperatureSensor: Current temperature reading in Celsius is " << value << endl;
        cout << "Sequence number: " << sequenceNumber << endl;
    }
    else
    {
        if(sequenceNumber == OC_OBSERVE_NO_OPTION)
        {
            cerr << "Observe registration or de-registration action is failed" << endl;
        }
        else
        {
            cerr << "TemperatureSensor: Observer response error" << endl;
        }
    }
}

void TemperatureSensor::get()
{
    QueryParamsMap params;
    m_resourceHandle->get(params, m_GETCallback);
}

void TemperatureSensor::onGet(const HeaderOptions& headerOptions,
                              const OCRepresentation& representation, int errCode)
{
    if (errCode == OC_STACK_OK)
    {
        double value;
        representation.getValue(TEMPERATURE_RESOURCE_KEY, value);
        cout << endl << endl << "Current temperature reading in Celsius: " << value << endl;
    }
    else {
        cerr << endl << endl << "Error in GET response from temperature sensor resource" << endl;
    }
    IoTClient::DisplayMenu();
}

AmbientLight::AmbientLight(shared_ptr<OCResource> Resource)
{
    m_isObserved = false;
    m_resourceHandle = Resource;
    m_GETCallback = bind(&AmbientLight::onGet, this, placeholders::_1, placeholders::_2,
                         placeholders::_3);
    m_OBSERVECallback = bind(&AmbientLight::onObserve, this, placeholders::_1, placeholders::_2,
                             placeholders::_3, placeholders::_4);
}

AmbientLight::~AmbientLight()
{

}

void AmbientLight::startObserve()
{
    if (!m_isObserved)
    {
        cout << "Started observing ambient light sensor" << endl;
        m_resourceHandle->observe(ObserveType::Observe, QueryParamsMap(), m_OBSERVECallback);
    }
    m_isObserved = true;
}

void AmbientLight::stopObserve()
{
    if (m_isObserved)
    {
        m_resourceHandle->cancelObserve();
        cout << "Stopped observing ambient light sensor" << endl;
    }
    m_isObserved = false;
}

void AmbientLight::onObserve(const HeaderOptions headerOptions, const OCRepresentation& rep,
                             int eCode, int sequenceNumber)
{
    if (eCode == OC_STACK_OK)
    {
        int value;
        rep.getValue(LIGHT_RESOURCE_KEY, value);
        cout << "Observing AmbientLightSensor: Current ambient light reading is " << value << endl;
        cout << "Sequence number: " << sequenceNumber << endl;
    }
    else
    {
        cerr << "TemperatureSensor: Observer response error" << endl;
    }
}

void AmbientLight::get()
{
    QueryParamsMap params;
    m_resourceHandle->get(params, m_GETCallback);
}

void AmbientLight::onGet(const HeaderOptions& headerOptions, const OCRepresentation& representation,
                         int errCode)
{
    if (errCode == OC_STACK_OK)
    {
        int value;
        representation.getValue(LIGHT_RESOURCE_KEY, value);
        cout << endl << endl << "Current ambient light sensor reading: " << value << endl;
    }
    else {
        cerr << endl << endl << "Error in GET response from ambient light sensor resource" << endl;
    }
    IoTClient::DisplayMenu();
}

LED::LED(shared_ptr<OCResource> Resource)
{
    m_resourceHandle = Resource;
    m_GETCallback = bind(&LED::onGet, this, placeholders::_1, placeholders::_2, placeholders::_3);
    m_PUTCallback = bind(&LED::onPut, this, placeholders::_1, placeholders::_2, placeholders::_3);
}

LED::~LED()
{

}

void LED::get()
{
    QueryParamsMap params;
    m_resourceHandle->get(params, m_GETCallback);
}

void LED::put(int Switch)
{
    QueryParamsMap params;
    OCRepresentation rep;
    rep.setValue(LED_RESOURCE_KEY, Switch);
    m_resourceHandle->put(rep, params, m_PUTCallback);
}

void LED::onGet(const HeaderOptions& headerOptions, const OCRepresentation& representation,
                int errCode)
{
    if (errCode == OC_STACK_OK)
    {
        int value;
        representation.getValue(LED_RESOURCE_KEY, value);
        cout << endl << endl << "LED switch state is: " << value << endl;
    }
    else {
        cerr << endl << endl << "Error in GET response from LED resource" << endl;
    }
    IoTClient::DisplayMenu();
}

void LED::onPut(const HeaderOptions& headerOptions, const OCRepresentation& representation,
                int errCode)
{
    if (errCode == OC_STACK_OK)
    {
        int value;
        representation.getValue(LED_RESOURCE_KEY, value);
        cout << endl << endl << "Set LED switch to: " << value << endl;
    }
    else {
        cerr << endl << endl << "Error in PUT response from LED resource" << endl;
    }
    IoTClient::DisplayMenu();
}

void IoTClient::DisplayMenu()
{
    cout << "\nEnter:\n0) Display this menu\n1) Get temperature Reading\n2) Start Temperature Observer\n3) Stop Temperature Observer\n4) Get ambient light reading\n5) Start Ambient Light Observer\n6) Stop Ambient Light Observer\n7) Turn LED ON\n8) Turn LED OFF\n9) Quit\n";
}

int main()
{
    IoTClient client;
    cout << "Performing Discovery..." << endl;
    client.findResource();
    int choice;
    do
    {
        cin >> choice;
        switch (choice)
        {
            case 0:
                IoTClient::DisplayMenu();
                break;
            case 1:
                if (client.getTemperatureSensor())
                    client.getTemperatureSensor()->get();
                else
                    cout << "Temperature sensor resource not yet discovered" << endl;
                break;
            case 2:
                if (client.getTemperatureSensor())
                    client.getTemperatureSensor()->startObserve();
                else
                    cout << "Temperature sensor resource not yet discovered" << endl;
                break;
            case 3:
                if (client.getTemperatureSensor())
                    client.getTemperatureSensor()->stopObserve();
                else
                    cout << "Temperature sensor resource not yet discovered" << endl;
                break;
            case 4:
                if (client.getAmbientLightSensor())
                    client.getAmbientLightSensor()->get();
                else
                    cout << "Ambient light sensor resource not yet discovered" << endl;
                break;
            case 5:
                if (client.getAmbientLightSensor())
                    client.getAmbientLightSensor()->startObserve();
                else
                    cout << "Ambient light sensor resource not yet discovered" << endl;
                break;
            case 6:
                if (client.getAmbientLightSensor())
                    client.getAmbientLightSensor()->stopObserve();
                else
                    cout << "Ambient light sensor resource not yet discovered" << endl;
                break;
            case 7:
                if (client.getPlatformLED())
                    client.getPlatformLED()->put(1);
                else
                    cout << "LED resource not yet discovered" << endl;
                break;
            case 8:
                if (client.getPlatformLED())
                    client.getPlatformLED()->put(0);
                else
                    cout << "LED resource not yet discovered" << endl;
                break;
            case 9:
            default:
                return 0;
        }
    }
    while (choice != 9);
    return 0;
}

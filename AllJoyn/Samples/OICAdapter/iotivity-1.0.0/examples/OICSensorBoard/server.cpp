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
#include <signal.h>
#include <thread>
#include <functional>

#include "server.h"
#include "sensors.h"
#include "namedefs.h"
using namespace Sensors;

void IoTServer::initializePlatform()
{
    cout << "Running IoTServer constructor" << endl;
    m_platformConfig = make_shared<PlatformConfig>(ServiceType::InProc, ModeType::Server, "0.0.0.0",
                                                   0, OC::QualityOfService::HighQos);
    OCPlatform::Configure(*m_platformConfig);
}

IoTServer::IoTServer()
{
    initializePlatform();
    setupResources();
    m_temperatureRepresentation.setValue(TEMPERATURE_RESOURCE_KEY, (float) 0.0f);
    m_ambientLightRepresentation.setValue(LIGHT_RESOURCE_KEY, 0);
    m_ledRepresentation.setValue(LED_RESOURCE_KEY, 0);
    SetupPins();
}

IoTServer::~IoTServer()
{
    cout << "Running IoTServer destructor" << endl;
    ClosePins();
}

void IoTServer::setupResources()
{
    EntityHandler cb1 = bind(&IoTServer::temperatureEntityHandler, this, placeholders::_1);
    createResource(TEMPERATURE_RESOURCE_ENDPOINT, TEMPERATURE_RESOURCE_TYPE, cb1,
                   m_temperatureResource);
    IoTObserverCb tempObsCb = bind(&IoTServer::temperatureObserverLoop, this);
    m_temperatureObserverLoop = make_shared<IoTObserver>(tempObsCb);

    EntityHandler cb2 = bind(&IoTServer::lightEntityHandler, this, placeholders::_1);
    createResource(LIGHT_RESOURCE_ENDPOINT, LIGHT_RESOURCE_TYPE, cb2, m_ambientLightResource);
    IoTObserverCb lightObsCb = bind(&IoTServer::lightObserverLoop, this);
    m_ambientLightObserverLoop = make_shared<IoTObserver>(lightObsCb);

    EntityHandler cb3 = bind(&IoTServer::LEDEntityHandler, this, placeholders::_1);
    createResource(LED_RESOURCE_ENDPOINT, LED_RESOURCE_TYPE, cb3, m_ledResource);
}

void IoTServer::createResource(string Uri, string Type, EntityHandler Cb, OCResourceHandle& Handle)
{
    string resourceUri = Uri;
    string resourceType = Type;
    string resourceInterface = EDISON_RESOURCE_INTERFACE;
    uint8_t resourceFlag = OC_DISCOVERABLE | OC_OBSERVABLE;

    OCStackResult result = OCPlatform::registerResource(Handle, resourceUri, resourceType,
                                                        resourceInterface, Cb, resourceFlag);

    if (result != OC_STACK_OK)
        cerr << "Could not create " << Type << " resource" << endl;
    else
        cout << "Successfully created " << Type << " resource" << endl;
}

void IoTServer::putLEDRepresentation()
{
    int state = 0;
    m_ledRepresentation.getValue(LED_RESOURCE_KEY, state);
    SetOnboardLed(state);
    if (state == 0)
        cout << "Turned off LED" << endl;
    else if (state == 1)
        cout << "Turned on LED" << endl;
    else
        cerr << "Invalid request value" << endl;
}

OCRepresentation IoTServer::getLEDRepresentation()
{
    return m_ledRepresentation;
}

void IoTServer::temperatureObserverLoop()
{
    usleep(1500000);
    cout << "Temperature Observer Callback" << endl;
    shared_ptr<OCResourceResponse> resourceResponse(new OCResourceResponse());
    resourceResponse->setErrorCode(200);
    resourceResponse->setResourceRepresentation(getTemperatureRepresentation(),
    EDISON_RESOURCE_INTERFACE);
    OCStackResult result = OCPlatform::notifyListOfObservers(m_temperatureResource,
                                                             m_temperatureObservers,
                                                             resourceResponse);
    if (result == OC_STACK_NO_OBSERVERS)
    {
        cout << "No more observers..Stopping observer loop..." << endl;
        m_temperatureObserverLoop->stop();
    }
}

void IoTServer::lightObserverLoop()
{
    usleep(1500000);
    cout << "Light Observer Callback" << endl;
    shared_ptr<OCResourceResponse> resourceResponse(new OCResourceResponse());
    resourceResponse->setErrorCode(200);
    resourceResponse->setResourceRepresentation(getLightRepresentation(),
    EDISON_RESOURCE_INTERFACE);
    OCStackResult result = OCPlatform::notifyListOfObservers(m_ambientLightResource,
                                                             m_ambientLightObservers,
                                                             resourceResponse);
    if (result == OC_STACK_NO_OBSERVERS)
    {
        cout << "No more observers..Stopping observer loop..." << endl;
        m_ambientLightObserverLoop->stop();
    }
}

OCRepresentation IoTServer::getTemperatureRepresentation()
{
    m_temperatureRepresentation.setValue(TEMPERATURE_RESOURCE_KEY, GetTemperatureInC());
    return m_temperatureRepresentation;
}

OCRepresentation IoTServer::getLightRepresentation()
{
    m_ambientLightRepresentation.setValue(LIGHT_RESOURCE_KEY, GetLightLevel());
    return m_ambientLightRepresentation;
}

OCEntityHandlerResult IoTServer::lightEntityHandler(shared_ptr<OCResourceRequest> Request)
{
    OCEntityHandlerResult result = OC_EH_ERROR;
    if (Request)
    {
        string requestType = Request->getRequestType();
        int requestFlag = Request->getRequestHandlerFlag();
        if (requestFlag & RequestHandlerFlag::RequestFlag)
        {
            auto Response = std::make_shared<OC::OCResourceResponse>();
            Response->setRequestHandle(Request->getRequestHandle());
            Response->setResourceHandle(Request->getResourceHandle());
            if (requestType == "GET")
            {
                cout << "GET request for ambient light reading" << endl;
                if (Response)
                {
                    Response->setErrorCode(200);
                    Response->setResponseResult(OC_EH_OK);
                    Response->setResourceRepresentation(getLightRepresentation());
                    if (OCPlatform::sendResponse(Response) == OC_STACK_OK)
                    {
                        result = OC_EH_OK;
                    }
                }
            }
            else
            {
                Response->setResponseResult(OC_EH_ERROR);
                OCPlatform::sendResponse(Response);
                cerr << "Unsupported request type" << endl;
                return result;
            }
        }
        if (requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            ObservationInfo observationInfo = Request->getObservationInfo();
            if (ObserveAction::ObserveRegister == observationInfo.action)
            {
                cout << "Starting observer for ambient light sensor" << endl;
                m_ambientLightObservers.push_back(observationInfo.obsId);
                m_ambientLightObserverLoop->start();
            }
            else if (ObserveAction::ObserveUnregister == observationInfo.action)
            {
                m_ambientLightObservers.erase(
                        remove(m_ambientLightObservers.begin(), m_ambientLightObservers.end(),
                               observationInfo.obsId),
                        m_ambientLightObservers.end());
            }
        }
    }
    return result;
}

OCEntityHandlerResult IoTServer::temperatureEntityHandler(shared_ptr<OCResourceRequest> Request)
{
    OCEntityHandlerResult result = OC_EH_ERROR;
    if (Request)
    {
        string requestType = Request->getRequestType();
        int requestFlag = Request->getRequestHandlerFlag();
        if (requestFlag & RequestHandlerFlag::RequestFlag)
        {
            auto Response = std::make_shared<OC::OCResourceResponse>();
            Response->setRequestHandle(Request->getRequestHandle());
            Response->setResourceHandle(Request->getResourceHandle());
            if (requestType == "GET")
            {
                cout << "GET request for temperature sensor reading" << endl;
                if (Response)
                {
                    Response->setErrorCode(200);
                    Response->setResponseResult(OC_EH_OK);
                    Response->setResourceRepresentation(getTemperatureRepresentation());
                    if (OCPlatform::sendResponse(Response) == OC_STACK_OK)
                    {
                        result = OC_EH_OK;
                    }
                }
            }
            else
            {
                Response->setResponseResult(OC_EH_ERROR);
                OCPlatform::sendResponse(Response);
                cerr << "Unsupported request type" << endl;
                return result;
            }
        }
        if (requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            ObservationInfo observationInfo = Request->getObservationInfo();
            if (ObserveAction::ObserveRegister == observationInfo.action)
            {
                cout << "Starting observer for temperature sensor" << endl;
                m_temperatureObservers.push_back(observationInfo.obsId);
                m_temperatureObserverLoop->start();
            }
            else if (ObserveAction::ObserveUnregister == observationInfo.action)
            {
                m_temperatureObservers.erase(
                        remove(m_temperatureObservers.begin(), m_temperatureObservers.end(),
                               observationInfo.obsId),
                        m_temperatureObservers.end());
            }
        }
    }
    return result;
}

OCEntityHandlerResult IoTServer::LEDEntityHandler(shared_ptr<OCResourceRequest> Request)
{
    OCEntityHandlerResult result = OC_EH_ERROR;
    if (Request)
    {
        string requestType = Request->getRequestType();
        int requestFlag = Request->getRequestHandlerFlag();
        if (requestFlag & RequestHandlerFlag::RequestFlag)
        {
            auto Response = std::make_shared<OC::OCResourceResponse>();
            Response->setRequestHandle(Request->getRequestHandle());
            Response->setResourceHandle(Request->getResourceHandle());
            if (requestType == "PUT")
            {
                cout << "PUT request for platform LED" << endl;
                OCRepresentation requestRep = Request->getResourceRepresentation();
                if (requestRep.hasAttribute(LED_RESOURCE_KEY))
                {
                    try
                    {
                        requestRep.getValue<int>(LED_RESOURCE_KEY);
                    }
                    catch (...)
                    {
                        Response->setResponseResult(OC_EH_ERROR);
                        OCPlatform::sendResponse(Response);
                        cerr << "Client sent invalid resource value type" << endl;
                        return result;
                    }
                }
                else
                {
                    Response->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(Response);
                    cerr << "Client sent invalid resource key" << endl;
                    return result;
                }
                m_ledRepresentation = requestRep;
                putLEDRepresentation();
                if (Response)
                {
                    Response->setErrorCode(200);
                    Response->setResourceRepresentation(getLEDRepresentation());
                    Response->setResponseResult(OC_EH_OK);
                    if (OCPlatform::sendResponse(Response) == OC_STACK_OK)
                    {
                        result = OC_EH_OK;
                    }
                }
            }
            else if (requestType == "GET")
            {
                cout << "GET request for platform LED" << endl;
                if (Response)
                {
                    Response->setErrorCode(200);
                    Response->setResourceRepresentation(getLEDRepresentation());
                    Response->setResponseResult(OC_EH_OK);
                    if (OCPlatform::sendResponse(Response) == OC_STACK_OK)
                    {
                        result = OC_EH_OK;
                    }
                }
            }
            else
            {
                Response->setResponseResult(OC_EH_ERROR);
                OCPlatform::sendResponse(Response);
                cerr << "Unsupported request type" << endl;
            }
        }
    }
    return result;
}

int quit = 0;

void handle_signal(int signal)
{
    quit = 1;
}

int main()
{
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    cout << "Press Ctrl-C to quit...." << endl;
    IoTServer server;
    do
    {
        usleep(2000000);
    }
    while (quit != 1);
    return 0;
}


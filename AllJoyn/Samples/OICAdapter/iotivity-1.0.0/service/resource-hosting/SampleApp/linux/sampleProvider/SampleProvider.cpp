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

#include <functional>

#include <pthread.h>
#include <signal.h>

#include "OCPlatform.h"
#include "OCApi.h"
#include "OCResourceResponse.h"

#include <memory>

using namespace OC;
using namespace std;
using namespace OC::OCPlatform;

int g_Observation = 0;
int gQuitFlag = 0;

pthread_cond_t m_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;



OCEntityHandlerResult entityHandler(std::shared_ptr< OCResourceRequest > request);

class TempHumidResource
{
    public:

        int m_temp;
        int m_humid;
        std::string m_uri;
        OCResourceHandle m_resourceHandle;
        ObservationIds m_interestedObservers;
        OCRepresentation m_Rep;

    public:
        TempHumidResource() :
            m_temp(0), m_humid(0), m_uri("/a/TempHumSensor"), m_resourceHandle(NULL)
        {
        }

        void createResource()
        {
            std::string resourceURI = "/a/TempHumSensor/hosting";
            std::string resourceTypeName = "oic.r.resourcehosting";
            std::string resourceInterface = DEFAULT_INTERFACE;

            m_uri = resourceURI;

            uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

            OCStackResult result = OCPlatform::registerResource(m_resourceHandle , resourceURI ,
                                   resourceTypeName , resourceInterface , &entityHandler , resourceProperty);

            if (OC_STACK_OK != result)
            {
                cout << "Resource creation was unsuccessful\n";
            }
        }

        OCStackResult createResource1()
        {
            std::string resourceURI = "/a/NM/TempHumSensor1"; // URI of the resource
            std::string resourceTypeName =
                "oic.r.resourcehosting"; // resource type name.
            std::string resourceInterface = DEFAULT_INTERFACE; // resource interface.

            // OCResourceProperty is defined ocstack.h
            uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

            OCResourceHandle resHandle;

            // This will internally create and register the resource.
            OCStackResult result = OCPlatform::registerResource(
                                       resHandle, resourceURI, resourceTypeName,
                                       resourceInterface, &entityHandler, resourceProperty);

            if (OC_STACK_OK != result)
            {
                cout << "Resource creation was unsuccessful\n";
            }

            return result;
        }

        OCResourceHandle getHandle()
        {
            return m_resourceHandle;
        }

        OCRepresentation post(OCRepresentation &rep)
        {
            static int first = 1;
            // for the first time it tries to create a resource
            if (first)
            {
                first = 0;

                if (OC_STACK_OK == createResource1())
                {
                    OCRepresentation rep1;
                    rep1.setValue("createduri", std::string("/a/light1"));

                    return rep1;
                }
            }

            // from second time onwards it just puts
            put(rep);
            return get();
        }

        void put(OCRepresentation &rep)
        {
            try
            {
                if (rep.getValue("temperature", m_temp))
                {
                    cout << "\t\t\t\t" << "temperature: " << m_temp << endl;
                }
                else
                {
                    cout << "\t\t\t\t" << "temperature not found in the representation" << endl;
                }

                if (rep.getValue("humidity", m_humid))
                {
                    cout << "\t\t\t\t" << "humidity: " << m_humid << endl;
                }
                else
                {
                    cout << "\t\t\t\t" << "humidity not found in the representation" << endl;
                }
            }
            catch (exception &e)
            {
                cout << e.what() << endl;
            }

        }

        OCRepresentation get()
        {
            m_Rep.setValue("temperature", m_temp);
            m_Rep.setValue("humidity", m_humid);
            return m_Rep;
        }
        OCStackResult deleteDeviceResource()
        {
            OCStackResult result = OCPlatform::unregisterResource(m_resourceHandle);
            if (OC_STACK_OK != result)
            {
                throw std::runtime_error(
                    std::string("Device Resource failed to unregister/delete") + std::to_string(result));
            }
            return result;
        }
};

TempHumidResource myResource;

void *ChangeLightRepresentation(void */*param*/)
{
    cout << "ChangeLigthRepresentation Enter\n";
    while (1)
    {
        cout << "pthread_cond_wait\n";
        pthread_cond_wait(&m_cond, &m_mutex);
        cout << "pthread_cond_start\n";
        if (g_Observation)
        {

            cout << endl;
            cout << "========================================================" << endl;
            cout << "HUMTepm updated to : " << myResource.m_temp << endl;
            cout << "Notifying observers with resource handle: " << myResource.getHandle() << endl;

            cout << endl;
            cout << "========================================================" << endl;
            cout << "Send data : \n";
            cout << "Attribute Name: Temp\tvalue: " << myResource.m_temp << endl;
            cout << "Attribute Name: Humid\tvalue: " << myResource.m_humid << endl;

            OCStackResult result = OCPlatform::notifyAllObservers(myResource.getHandle());
            cout << "Notify Success\n";

            if (OC_STACK_NO_OBSERVERS == result)
            {
                cout << "No More observers, stopping notifications" << endl;
                g_Observation = 0;
            }
        }
        cout << "ChangeLigthRepresentation Out\n";

    }
    return NULL;
}

OCEntityHandlerResult entityHandler(std::shared_ptr< OCResourceRequest > request)
{
    cout << "Sample Provider entityHandler\n";

    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    if (request)
    {
        cout << "flag : request\n";
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if (requestFlag == RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";
            auto pResponse = std::make_shared<OC::OCResourceResponse>();
            pResponse->setRequestHandle(request->getRequestHandle());
            pResponse->setResourceHandle(request->getResourceHandle());

            if (requestType == "GET")
            {
                cout << "\t\trequestType : GET\n";

                pResponse->setErrorCode(200);
                pResponse->setResponseResult(OC_EH_OK);
                pResponse->setResourceRepresentation(myResource.get());
                if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            }

            else if (requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";

                OCRepresentation rep = request->getResourceRepresentation();
                myResource.put(rep);

                if (pResponse)
                {
                    pResponse->setErrorCode(200);
                    pResponse->setResourceRepresentation(myResource.get());
                }
                if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
                else
                {
                    cout << "put request Error\n";
                }
            }

            else if (requestType == "POST")
            {
                cout << "\t\t\trequestType : POST\n";
            }

            else if (requestType == "DELETE")
            {
                cout << "\t\trequestType : DELETE\n";
                cout << "DeviceResource Delete Request" << std::endl;

                if (myResource.deleteDeviceResource() == OC_STACK_OK)
                {
                    cout << "\tSuccess DELETE\n";
                    pResponse->setErrorCode(200);
                    pResponse->setResponseResult(OC_EH_RESOURCE_DELETED);
                    ehResult = OC_EH_OK;
                }
                else
                {
                    pResponse->setResponseResult(OC_EH_ERROR);
                    ehResult = OC_EH_ERROR;
                }

                OCPlatform::sendResponse(pResponse);
            }
        }
        if (requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            pthread_t threadId;

            ObservationInfo observationInfo = request->getObservationInfo();
            if (ObserveAction::ObserveRegister == observationInfo.action)
            {
                myResource.m_interestedObservers.push_back(observationInfo.obsId);
            }
            else if (ObserveAction::ObserveUnregister == observationInfo.action)
            {
                myResource.m_interestedObservers.erase(std::remove(
                        myResource.m_interestedObservers.begin(),
                        myResource.m_interestedObservers.end(),
                        observationInfo.obsId),
                                                       myResource.m_interestedObservers.end());
            }

            cout << request->getResourceUri() << endl;
            cout << request->getResourceRepresentation().getUri() << endl;

            cout << "========================================================" << endl;
            cout << "Receive ObserverFlag : Start Observe\n";
            cout << "========================================================" << endl;
            g_Observation = 1;

            cout << "\t\trequestFlag : Observer\n";
            static int startedThread = 0;
            if (!startedThread)
            {
                cout << "\t\tpthrerad_create\n";
                pthread_create(&threadId , NULL , ChangeLightRepresentation , (void *) NULL);
                startedThread = 1;
            }
            ehResult = OC_EH_OK;
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return ehResult;
}

void quitProcess()
{
    unregisterResource(myResource.m_resourceHandle);
    stopPresence();
    exit(0);
}

void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        std::cout << " handleSigInt in" << std::endl;
        quitProcess();
    }
}

int main()
{
    PlatformConfig cfg
    {
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        "0.0.0.0",
        0,
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);

    int number = 0;

    try
    {
        startPresence(30);

        myResource.createResource();

        signal(SIGINT, handleSigInt);
        while (true)
        {
            bool end = false;
            cout << endl;
            cout << "========================================================" << endl;
            cout << "1. Temp is up" << endl;
            cout << "2. Temp is down" << endl;
            cout << "3. This Program will be ended." << endl;
            cout << "========================================================" << endl;

            cin >> number;
            if(std::cin.fail())
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input type, please try again" << std::endl;
                continue;
            }

            switch (number)
            {
                case 1:
                    {
                        cout << "Temp is up!" << endl;
                        myResource.m_temp += 10;
                        pthread_cond_signal(&m_cond);
                        cout << "ChangeLightRepresentation Done!" << endl;
                        break;
                    }
                case 2:
                    {
                        cout << "Temp is down!" << endl;
                        myResource.m_temp -= 10;
                        pthread_cond_signal(&m_cond);
                        cout << "ChangeLightRepresentation Done!" << endl;
                        break;
                    }
                case 3:
                    {
                        cout << "Bye!" << endl;
                        stopPresence();
                        end = true;
                        quitProcess();
                        break;
                    }
                default:
                    {
                        cout << "Invalid input. Please try again." << endl;
                        break;
                    }
            }
            if (end == true)
            {
                break;
            }
        }
    }
    catch (exception &e)
    {
        cout << "main exception  : " << e.what() << endl;
    }


}

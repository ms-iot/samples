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

#include "UnitTestHelper.h"
#include "MaintenanceCollection.h"
#include "ConfigurationCollection.h"
#include "FactorySetCollection.h"

#include "timer.h"
#include "ActionSet.h"
#include "GroupManager.h"
#include "ThingsConfiguration.h"
#include "ThingsMaintenance.h"
#include "ocstack.h"
#include "OCPlatform.h"
#include "OCApi.h"

#include <iostream>
#include <functional>
#include <pthread.h>
#include <condition_variable>

#define SUCCESS_RESPONSE 0
constexpr int DEFAULT_WAITING_TIME_IN_MILLIS = 3000;

using namespace OIC;
using namespace OC;
using namespace std;
namespace PH = std::placeholders;

int result = 0;
bool isSlowResponse = false;

std::string defaultDeviceName = "Legacy Device";
std::string defaultLocation = "37.256616, 127.052806";
std::string defaultLocationName = "Living Room";
std::string defaultRegion = "Won";
std::string defaultCurrency = "Seoul, Korea";

OCResourceHandle resourceHandle;
OCResourceHandle foundResourceHandle;

std::shared_ptr< OCResource > g_resource;
std::shared_ptr< OCResource > g_room_resource;
std::shared_ptr< OCResource > g_light;
std::shared_ptr< OCResource > configurationResource;
std::vector< string > lights;
std::vector< OCResourceHandle > resourceHandleVector;

GroupManager *groupMgr = new GroupManager();
ConfigurationResource *myConfigurationResource;
MaintenanceResource *myMaintenanceResource;
FactorySetResource *myFactorySetResource;

std::condition_variable cv1;
std::condition_variable cv2;
std::condition_variable cv3;
std::condition_variable cv4;
std::condition_variable cv5;

bool prepareResponseForResource(std::shared_ptr< OCResourceRequest > request);
OCStackResult sendResponseForResource(std::shared_ptr< OCResourceRequest > pRequest);
OCEntityHandlerResult entityHandlerForResource(std::shared_ptr< OCResourceRequest > request);
OCEntityHandlerResult entityHandlerBootstrap(std::shared_ptr< OCResourceRequest > request);

typedef std::function< void(OCRepresentation &) > putFunc;
typedef std::function< OCRepresentation(void) > getFunc;

void *ChangeLightRepresentation(void *param);
void *handleSlowResponse(void *param, std::shared_ptr< OCResourceRequest > pRequest);

/****** Light Resource [Required to gtestcases of GroupManager APIs]  ******/

class LightResource
{
public:
    std::string m_power;
    std::string testing;
    std::string m_lightUri;
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_lightRep;

public:
    LightResource() :
            m_power("on"), m_lightUri("/a/light"), m_resourceHandle(0)
    {
        m_lightRep.setUri(m_lightUri);
        m_lightRep.setValue("power", m_power);
    }

    void createResource()
    {
        std::string resourceURI = m_lightUri;
        std::string resourceTypeName = "core.light";
        std::string resourceInterface = DEFAULT_INTERFACE;
        EntityHandler cb = std::bind(&LightResource::entityHandler, this, PH::_1);

        OCStackResult result = OCPlatform::registerResource(m_resourceHandle, resourceURI,
                resourceTypeName, resourceInterface, cb, OC_DISCOVERABLE | OC_OBSERVABLE);

        if (OC_STACK_OK != result)
        {
            printf("\nLightResource : OC_STACK_OK != result...");
        }
        else
        {
            cv2.notify_all();
            std::mutex blocker;
            std::condition_variable cv;
            std::unique_lock < std::mutex > lock(blocker);
            cv.wait(lock);
        }
    }

    OCResourceHandle getHandle()
    {
        return m_resourceHandle;
    }

    void put(OCRepresentation &rep)
    {
        try
        {
            std::string test;
            if (rep.getValue < std::string > ("power", test))
            {
                cout << "\t\t\t\t" << "power: " << test << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "power not found in the representation" << endl;
            }
        }
        catch (exception &e)
        {
            cout << e.what() << endl;
        }
    }

    OCRepresentation post(OCRepresentation &rep)
    {
        put(rep);
        return get();
    }

    OCRepresentation get()
    {
        m_lightRep.setValue("power", m_power);

        return m_lightRep;
    }

    void addType(const std::string &type) const
    {
        OCStackResult result = OCPlatform::bindTypeToResource(m_resourceHandle, type);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

    void addInterface(const std::string &interface) const
    {
        OCStackResult result = OCPlatform::bindInterfaceToResource(m_resourceHandle, interface);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

private:
    OCEntityHandlerResult entityHandler(std::shared_ptr< OCResourceRequest > request)
    {
        cout << "\tIn Server CPP entity handler:\n";
        OCEntityHandlerResult ehResult = OC_EH_ERROR;
        if (request)
        {
            std::string requestType = request->getRequestType();
            int requestFlag = request->getRequestHandlerFlag();

            if (requestFlag & RequestHandlerFlag::RequestFlag)
            {
                cout << "\t\trequestFlag : Request\n";
                auto pResponse = std::make_shared< OC::OCResourceResponse >();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                if (requestType == "GET")
                {
                    cout << "\t\t\trequestType : GET\n";
                    if (isSlowResponse)
                    {
                        static int startedThread = 0;
                        if (!startedThread)
                        {
                            std::thread t(handleSlowResponse, (void *) this, request);
                            startedThread = 1;
                            t.detach();
                        }
                        ehResult = OC_EH_SLOW;
                    }
                    else
                    {
                        pResponse->setErrorCode(200);
                        pResponse->setResponseResult(OC_EH_OK);
                        pResponse->setResourceRepresentation(get());
                        if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                        {
                            ehResult = OC_EH_OK;
                        }
                    }
                }
                else if (requestType == "PUT")
                {
                    cout << "\t\t\trequestType : PUT\n";
                    OCRepresentation rep = request->getResourceRepresentation();
                    put(rep);
                    pResponse->setErrorCode(200);
                    pResponse->setResponseResult(OC_EH_OK);
                    pResponse->setResourceRepresentation(rep);
                    if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if (requestType == "POST")
                {
                    cout << "\t\t\trequestType : POST\n";

                    OCRepresentation rep = request->getResourceRepresentation();
                    OCRepresentation rep_post = post(rep);

                    pResponse->setResourceRepresentation(rep_post);
                    pResponse->setErrorCode(200);
                    if (rep_post.hasAttribute("createduri"))
                    {
                        pResponse->setResponseResult(OC_EH_RESOURCE_CREATED);
                        pResponse->setNewResourceUri(
                                rep_post.getValue < std::string > ("createduri"));
                    }

                    if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if (requestType == "DELETE")
                {
                }
            }
        }
        else
        {
            std::cout << "Request invalid" << std::endl;
        }
        return ehResult;
    }
};

void *handleSlowResponse(void *param, std::shared_ptr< OCResourceRequest > pRequest)
{
    LightResource *lightPtr = (LightResource *) param;
    sleep(10);

    auto pResponse = std::make_shared< OC::OCResourceResponse >();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(lightPtr->get());
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    isSlowResponse = false;
    OCPlatform::sendResponse(pResponse);
    return NULL;
}

/****** Configuration Resource  ******/

void ConfigurationResource::createResources(ResourceEntityHandler callback)
{
    using namespace OC::OCPlatform;

    if (callback == NULL)
    {
        std::cout << "callback should be binded\t";
        return;
    }

    OCStackResult result = registerResource(m_configurationHandle, m_configurationUri,
            m_configurationTypes[0], m_configurationInterfaces[0], callback,
            OC_DISCOVERABLE | OC_OBSERVABLE);

    if (OC_STACK_OK != result)
    {
        std::cout << "Resource creation (configuration) was unsuccessful\n";
    }
    else
    {
        cv2.notify_all();
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock < std::mutex > lock(blocker);
        cv.wait(lock);
    }
}

void ConfigurationResource::setConfigurationRepresentation(OCRepresentation &rep)
{
    string value;
    if (rep.getValue("n", value))
    {
        m_deviceName = value;
        std::cout << "\t\t\t\t" << "m_deviceName: " << m_deviceName << std::endl;
    }

    if (rep.getValue("loc", value))
    {
        m_location = value;
        std::cout << "\t\t\t\t" << "m_location: " << m_location << std::endl;
    }

    if (rep.getValue("locn", value))
    {
        m_locationName = value;
        std::cout << "\t\t\t\t" << "m_locationName: " << m_locationName << std::endl;
    }

    if (rep.getValue("c", value))
    {
        m_currency = value;
        std::cout << "\t\t\t\t" << "m_currency: " << m_currency << std::endl;
    }

    if (rep.getValue("r", value))
    {
        m_region = value;
        std::cout << "\t\t\t\t" << "m_region: " << m_region << std::endl;
    }
}

OCRepresentation ConfigurationResource::getConfigurationRepresentation()
{
    m_configurationRep.setValue("n", m_deviceName);
    m_configurationRep.setValue("loc", m_location);
    m_configurationRep.setValue("locn", m_locationName);
    m_configurationRep.setValue("c", m_currency);
    m_configurationRep.setValue("r", m_region);

    return m_configurationRep;
}

std::string ConfigurationResource::getUri()
{
    return m_configurationUri;
}

void ConfigurationResource::factoryReset()
{
    m_deviceName = defaultDeviceName;
    m_location = defaultLocation;
    m_locationName = defaultLocationName;
    m_currency = defaultCurrency;
    m_region = defaultRegion;
}

/****** FactorySet  Resource  ******/

FactorySetResource::FactorySetResource()
{
    m_configurationUri = "/factoryset"; // URI of the resource
    m_configurationTypes.clear();
    m_configurationTypes.push_back("factoryset"); // resource type name.
    m_configurationRep.setUri(m_configurationUri);
    m_configurationRep.setResourceTypes(m_configurationTypes);
}

FactorySetResource::~FactorySetResource()
{
}

void FactorySetResource::createResources(ResourceEntityHandler callback)
{
    using namespace OC::OCPlatform;

    if (callback == NULL)
    {
        std::cout << "callback should be binded\t";
        return;
    }

    OCStackResult result = registerResource(m_configurationHandle, m_configurationUri,
            m_configurationTypes[0], m_configurationInterfaces[0], callback,
            OC_DISCOVERABLE | OC_OBSERVABLE);

    if (OC_STACK_OK != result)
    {
        std::cout << "Resource creation (configuration) was unsuccessful\n";
    }

    else
    {
        cv4.notify_all();
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock < std::mutex > lock(blocker);
        cv.wait(lock);
    }
}

void FactorySetResource::setFactorySetRepresentation(OCRepresentation &rep)
{
    string value;

    if (rep.getValue("n", value))
    {
        m_deviceName = value;
        std::cout << "\t\t\t\t" << "m_deviceName: " << m_deviceName << std::endl;
    }

    if (rep.getValue("loc", value))
    {
        m_location = value;
        std::cout << "\t\t\t\t" << "m_location: " << m_location << std::endl;
    }

    if (rep.getValue("locn", value))
    {
        m_locationName = value;
        std::cout << "\t\t\t\t" << "m_locationName: " << m_locationName << std::endl;
    }

    if (rep.getValue("c", value))
    {
        m_currency = value;
        std::cout << "\t\t\t\t" << "m_currency: " << m_currency << std::endl;
    }

    if (rep.getValue("r", value))
    {
        m_region = value;
        std::cout << "\t\t\t\t" << "m_region: " << m_region << std::endl;
    }
}

OCRepresentation FactorySetResource::getFactorySetRepresentation()
{
    m_configurationRep.setValue("n", m_deviceName);
    m_configurationRep.setValue("loc", m_location);
    m_configurationRep.setValue("locn", m_locationName);
    m_configurationRep.setValue("c", m_currency);
    m_configurationRep.setValue("r", m_region);

    return m_configurationRep;
}

std::string FactorySetResource::getUri()
{
    return m_configurationUri;
}

/****** Maintenance Resource ********/

void MaintenanceResource::createResources(ResourceEntityHandler callback)
{
    using namespace OC::OCPlatform;

    if (callback == NULL)
    {
        std::cout << "callback should be binded\t";
        return;
    }

    OCStackResult result = registerResource(m_maintenanceHandle, m_maintenanceUri,
            m_maintenanceTypes[0], m_maintenanceInterfaces[0], callback,
            OC_DISCOVERABLE | OC_OBSERVABLE);

    if (OC_STACK_OK != result)
    {
        std::cout << "Resource creation (maintenance) was unsuccessful\n";
    }

    thread exec(
            std::function< void(int second) >(
                    std::bind(&MaintenanceResource::maintenanceMonitor, this,
                            std::placeholders::_1)), 10);
    exec.detach();
    cv3.notify_all();
    std::mutex blocker;
    std::condition_variable cv;
    std::unique_lock < std::mutex > lock(blocker);
    cv.wait(lock);

    std::cout << "maintenance Resource is Created!\n";
}

void MaintenanceResource::setMaintenanceRepresentation(OCRepresentation &rep)
{
    string value;

    if (rep.getValue("fr", value))
    {
        m_factoryReset = value;
        std::cout << "\t\t\t\t" << "m_factoryReset: " << m_factoryReset << std::endl;
    }

    if (rep.getValue("rb", value))
    {
        m_reboot = value;
        std::cout << "\t\t\t\t" << "m_reboot: " << m_reboot << std::endl;
    }

    if (rep.getValue("ssc", value))
    {
        m_startStatCollection = value;
        std::cout << "\t\t\t\t" << "m_startStatCollection: " << m_startStatCollection << std::endl;
    }
}

OCRepresentation MaintenanceResource::getMaintenanceRepresentation()
{
    m_maintenanceRep.setValue("fr", m_factoryReset);
    m_maintenanceRep.setValue("rb", m_reboot);
    m_maintenanceRep.setValue("ssc", m_startStatCollection);

    return m_maintenanceRep;
}

std::string MaintenanceResource::getUri()
{
    return m_maintenanceUri;
}

void MaintenanceResource::maintenanceMonitor(int second)
{
    while (1)
    {
        sleep(second);

        if (m_reboot == "true")
        {
            int res;
            std::cout << "Reboot will be soon..." << std::endl;
            m_reboot = defaultReboot;
            res = system("sudo reboot");

            std::cout << "return: " << res << std::endl;

        }
        else if (m_factoryReset == "true")
        {
            std::cout << "Factory Reset will be soon..." << std::endl;
            m_factoryReset = defaultFactoryReset;
            factoryReset();
        }
    }
}

getFunc getGetFunction(std::string uri)
{
    getFunc res = NULL;

    if (uri == myConfigurationResource->getUri())
    {
        res = std::bind(&ConfigurationResource::getConfigurationRepresentation,
                myConfigurationResource);
    }
    else if (uri == myMaintenanceResource->getUri())
    {
        res = std::bind(&MaintenanceResource::getMaintenanceRepresentation, myMaintenanceResource);
    }
    return res;
}

putFunc getPutFunction(std::string uri)
{
    putFunc res = NULL;

    if (uri == myConfigurationResource->getUri())
    {
        res = std::bind(&ConfigurationResource::setConfigurationRepresentation,
                myConfigurationResource, std::placeholders::_1);
    }
    else if (uri == myMaintenanceResource->getUri())
    {
        res = std::bind(&MaintenanceResource::setMaintenanceRepresentation, myMaintenanceResource,
                std::placeholders::_1);
    }
    return res;
}

bool prepareResponseForResource(std::shared_ptr< OCResourceRequest > request)
{
    std::cout << "\tIn Server CPP prepareResponseForResource:\n";
    bool result = false;
    if (request)
    {
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if (requestFlag == RequestHandlerFlag::RequestFlag)
        {
            std::cout << "\t\trequestFlag : Request\n";
            if (requestType == "GET")
            {
                std::cout << "\t\t\trequestType : GET\n";
                result = true;
            }
            else if (requestType == "PUT")
            {
                std::cout << "\t\t\trequestType : PUT\n";
                putFunc putFunction;
                OCRepresentation rep = request->getResourceRepresentation();

                putFunction = getPutFunction(request->getResourceUri());
                putFunction(rep);
                result = true;
            }
            else if (requestType == "POST")
            {
            }
            else if (requestType == "DELETE")
            {
            }
        }
        else if (requestFlag == RequestHandlerFlag::ObserverFlag)
        {
            std::cout << "\t\trequestFlag : Observer\n";
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return result;
}

OCStackResult sendResponseForResource(std::shared_ptr< OCResourceRequest > pRequest)
{
    auto pResponse = std::make_shared< OC::OCResourceResponse >();
    QueryParamsMap queryParamsMap = pRequest->getQueryParameters();

    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());

    getFunc getFunction;
    getFunction = getGetFunction(pRequest->getResourceUri());

    OCRepresentation rep;
    rep = getFunction();

    auto findRes = queryParamsMap.find("if");

    if (findRes != queryParamsMap.end())
    {
        pResponse->setResourceRepresentation(rep, findRes->second);
    }
    else
    {
        pResponse->setResourceRepresentation(rep, DEFAULT_INTERFACE);
    }

    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}

OCEntityHandlerResult entityHandlerForResource(std::shared_ptr< OCResourceRequest > request)
{
    std::cout << "\tIn Server CPP (entityHandlerForResource) entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    QueryParamsMap test = request->getQueryParameters();

    if (prepareResponseForResource(request))
    {
        if (OC_STACK_OK == sendResponseForResource(request))
        {
            ehResult = OC_EH_OK;
        }
        else
        {
            std::cout << "sendResponse failed." << std::endl;
        }
    }
    else
    {
        std::cout << "PrepareResponse failed." << std::endl;
    }
    return ehResult;
}

/****** BootStrap Resource [Required for doBootstrap API of ThingsConfiguration class]  ******/

class BootstrapResource
{
public:
    std::string m_bootstrapUri;
    std::vector< std::string > m_bootstrapTypes;
    std::vector< std::string > m_bootstrapInterfaces;
    OCResourceHandle m_bootstrapHandle;
    OCRepresentation m_bootstrapRep;

public:
    BootstrapResource()
    {
        m_bootstrapUri = "/bootstrap";
        m_bootstrapTypes.push_back("bootstrap");
        m_bootstrapInterfaces.push_back(DEFAULT_INTERFACE);
        m_bootstrapRep.setUri(m_bootstrapUri);
        m_bootstrapRep.setResourceTypes(m_bootstrapTypes);
        m_bootstrapRep.setResourceInterfaces(m_bootstrapInterfaces);
        m_bootstrapHandle = NULL;
    }
    void createResources()
    {
        using namespace OC::OCPlatform;
        OCStackResult result = registerResource(m_bootstrapHandle, m_bootstrapUri,
                m_bootstrapTypes[0], m_bootstrapInterfaces[0], entityHandlerBootstrap,
                OC_DISCOVERABLE | OC_OBSERVABLE);

        if (OC_STACK_OK != result)
        {
            cout << "Resource creation (room) was unsuccessful\n";
        }

        cv5.notify_all();
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock < std::mutex > lock(blocker);
        cv.wait(lock);
    }

    void setBootstrapRepresentation(OCRepresentation& /*rep*/)
    {
    }

    OCRepresentation getBootstrapRepresentation()
    {
        m_bootstrapRep.setValue < std::string > ("n", defaultDeviceName);
        m_bootstrapRep.setValue < std::string > ("loc", defaultLocation);
        m_bootstrapRep.setValue < std::string > ("locn", defaultLocationName);
        m_bootstrapRep.setValue < std::string > ("c", defaultCurrency);
        m_bootstrapRep.setValue < std::string > ("r", defaultRegion);

        return m_bootstrapRep;
    }
};

BootstrapResource myBootstrapResource;

bool prepareResponse(std::shared_ptr< OCResourceRequest > request)
{
    cout << "\tIn Server CPP prepareResponse:\n";
    bool result = false;
    if (request)
    {
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if (requestFlag == RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";
            if (requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";
                result = true;
            }
            else if (requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";

                OCRepresentation rep = request->getResourceRepresentation();
                myBootstrapResource.setBootstrapRepresentation(rep);
                result = true;
            }
            else if (requestType == "POST")
            {
            }
            else if (requestType == "DELETE")
            {
            }
        }
        else if (requestFlag == RequestHandlerFlag::ObserverFlag)
        {
            cout << "\t\trequestFlag : Observer\n";
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return result;
}

OCStackResult sendResponse(std::shared_ptr< OCResourceRequest > pRequest)
{
    auto pResponse = std::make_shared< OC::OCResourceResponse >();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(myBootstrapResource.getBootstrapRepresentation());
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    return OCPlatform::sendResponse(pResponse);
}

OCEntityHandlerResult entityHandlerBootstrap(std::shared_ptr< OCResourceRequest > request)
{
    cout << "\tIn Server CPP (entityHandlerBootstrap) entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    if (prepareResponse(request))
    {
        if (OC_STACK_OK == sendResponse(request))
        {
            ehResult = OC_EH_OK;
        }
        else
        {
            std::cout << "sendResponse failed." << std::endl;
        }
    }
    else
    {
        std::cout << "PrepareResponse failed." << std::endl;
    }
    return ehResult;
}

/****** gtest class ******/

class ThingsManagerTest: public TestWithMock
{
public:
    void Proceed()
    {
        cond.notify_all();
    }

    void Wait(int waitingTime = DEFAULT_WAITING_TIME_IN_MILLIS)
    {
        std::unique_lock < std::mutex > lock
        { mutex };
        cond.wait_for(lock, std::chrono::milliseconds
        { waitingTime });
    }

protected:
    void SetUp()
    {
        TestWithMock::SetUp();
    }

    void TearDown()
    {
        TestWithMock::TearDown();
    }

private:
    std::condition_variable cond;
    std::mutex mutex;
};

//Callbacks
void onUpdate(const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
        const int /*eCode*/)
{
}

void onGetBootstrapInformation(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& /*rep*/, const int /*eCode*/)
{
}

void onReboot(const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
        const int /*eCode*/)
{
}

void onFactoryReset(const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
        const int /*eCode*/)
{
}

void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
        const int /*eCode*/)
{
}

void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
        const int /*eCode*/)
{
}

void onPost(const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
        const int /*eCode*/)
{
}

void foundResources(std::vector< std::shared_ptr< OC::OCResource > > listOfResource)
{
    for (auto rsrc = listOfResource.begin(); rsrc != listOfResource.end(); ++rsrc)
    {
        std::string resourceURI = (*rsrc)->uri();
        std::string hostAddress = (*rsrc)->host();

        if (resourceURI == "/a/light")
        {
            result = OCPlatform::registerResource(foundResourceHandle, (*rsrc));
            if (result == OC_STACK_OK)
            {
                OCPlatform::bindResource(resourceHandle, foundResourceHandle);
                resourceHandleVector.push_back(foundResourceHandle);
            }
            else
            {
                cout << "\tresource Error!" << endl;
            }
            lights.push_back((hostAddress + resourceURI));

            g_light = (*rsrc);
        }
        else
        {
            configurationResource = (*rsrc);
        }
    }
    cv2.notify_all();
}

void foundGroupResource(std::shared_ptr< OCResource > resource)
{
    std::string resourceURI;
    resourceURI = resource->uri();
    if (resourceURI == "/core/a/collection")
    {
        g_resource = resource;
    }
    else
    {
        g_room_resource = resource;
    }
    cv1.notify_all();
}

//This test case is to create the lightserver , BootstrapServer & configuration sever
TEST_F(ThingsManagerTest, testCreateResources)
{
    PlatformConfig cfg
    {   OC::ServiceType::InProc, OC::ModeType::Both, "0.0.0.0", 0, OC::QualityOfService::LowQos};
    OCPlatform::Configure(cfg);

    LightResource myLight;
    std::thread t1(&LightResource::createResource, &myLight);
    t1.detach();
    std::mutex blocker1;
    std::unique_lock < std::mutex > lock1(blocker1);
    cv2.wait(lock1);

    std::thread t2(&BootstrapResource::createResources, &myBootstrapResource);
    t2.detach();
    std::mutex blocker2;
    std::unique_lock < std::mutex > lock2(blocker2);
    cv5.wait(lock2);

    myConfigurationResource = new ConfigurationResource();
    std::thread t3(&ConfigurationResource::createResources, myConfigurationResource,
            &entityHandlerForResource);

    t3.detach();
    std::mutex blocker3;
    std::unique_lock < std::mutex > lock3(blocker3);
    cv2.wait(lock3);

    myMaintenanceResource = new MaintenanceResource();
    std::thread t4(&MaintenanceResource::createResources, myMaintenanceResource,
            &entityHandlerForResource);
    t4.detach();

    std::mutex blocker4;
    std::unique_lock < std::mutex > lock4(blocker4);
    cv3.wait(lock4);

    myFactorySetResource = new FactorySetResource();
    std::thread t5(&FactorySetResource::createResources, myFactorySetResource,
            &entityHandlerForResource);
    t5.detach();

    std::mutex blocker5;
    std::unique_lock < std::mutex > lock5(blocker5);
    cv4.wait(lock5);

    myMaintenanceResource->factoryReset = std::function < void()
    > (std::bind(&ConfigurationResource::factoryReset,
                    myConfigurationResource));
}

//Check findCandidateResources
TEST_F(ThingsManagerTest, testFindCandidateResources)
{

    string resourceURI = "/core/a/collection";
    string resourceTypeName = "a.collection";
    string resourceInterface = BATCH_INTERFACE;

    OCStackResult res = OCPlatform::registerResource(resourceHandle, resourceURI,
            resourceTypeName, resourceInterface, NULL, OC_DISCOVERABLE);

    if ( res != OC_STACK_OK )
    {
        cout << "Resource registeration failed." << endl;
    }

    OCPlatform::bindInterfaceToResource(resourceHandle, GROUP_INTERFACE);
    OCPlatform::bindInterfaceToResource(resourceHandle, DEFAULT_INTERFACE);

    std::string query = OC_RSRVD_WELL_KNOWN_URI;
    query.append("?rt=");
    query.append(resourceTypeName);

    OCPlatform::findResource("", query, CT_DEFAULT, &foundGroupResource);

    std::mutex blocker1;
    std::unique_lock < std::mutex > lock1(blocker1);
    cv1.wait(lock1);

    GroupManager *instance = new GroupManager();
    vector<string> types;
    types.push_back("core.light");

    result = instance->findCandidateResources(types, &foundResources);

    std::mutex blocker2;
    std::unique_lock < std::mutex > lock2(blocker2);
    cv2.wait(lock2);
}

//Find Candidate Resource when no resources are specified
TEST_F(ThingsManagerTest, testFindCandidateResourcesEmptyResourceType)
{
    GroupManager *instance = new GroupManager();
    vector<string> types;
    result = instance->findCandidateResources(types, &foundResources);
    EXPECT_TRUE(result == OC_STACK_ERROR);
    delete instance;
}

//Find Candidate Resource when Callback is null
TEST_F(ThingsManagerTest, testFindCandidateResourcesNullCallback)
{
    GroupManager *instance = new GroupManager();
    vector<string> types;
    types.push_back("core.light");
    result = instance->findCandidateResources(types, NULL);
    EXPECT_TRUE(result == OC_STACK_ERROR);
    delete instance;
}

//test bind resource to group
TEST_F(ThingsManagerTest, testBindResourceToGroup)
{
    GroupManager *instance = new GroupManager();
    OCResourceHandle rHandle = NULL;

    string resourceURI = "/core/room-large";
    string resourceTypeName = "core.room-large";
    string resourceInterface = BATCH_INTERFACE;

    OCStackResult res = OCPlatform::registerResource(rHandle, resourceURI,
            resourceTypeName, resourceInterface, NULL, OC_DISCOVERABLE);

    if ( res != OC_STACK_OK )
    {
        cout << "Resource registeration failed." << endl;
    }

    OCPlatform::bindInterfaceToResource(rHandle, GROUP_INTERFACE);
    OCPlatform::bindInterfaceToResource(rHandle, DEFAULT_INTERFACE);

    std::string query = OC_RSRVD_WELL_KNOWN_URI;
    query.append("?rt=");
    query.append(resourceTypeName);

    OCPlatform::findResource("", query, CT_DEFAULT, &foundGroupResource);

    std::mutex blocker1;
    std::unique_lock < std::mutex > lock1(blocker1);
    cv1.wait(lock1);

    result = instance->bindResourceToGroup (resourceHandle, g_room_resource, rHandle);

    EXPECT_TRUE(result == OC_STACK_OK);
    delete instance;
}

//Add actionset
TEST_F(ThingsManagerTest, testAddActionSetAllBulbOff)
{
    string actionsetDesc;
    ActionSet *allBulbOff = new ActionSet();
    allBulbOff->actionsetName = "AllBulbOff";

    mocks.ExpectCallFunc(onPut).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "off";

        action->listOfCapability.push_back(capa);
        allBulbOff->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, allBulbOff, &onPut);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
    delete allBulbOff;
}

//Add actionset with NULL resource
TEST_F(ThingsManagerTest, testAddActionSetAllBulbOffResourceNull)
{
    string actionsetDesc;
    ActionSet *allBulbOff = new ActionSet();
    allBulbOff->actionsetName = "AllBulbOff";

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "off";

        action->listOfCapability.push_back(capa);
        allBulbOff->listOfAction.push_back(action);
    }

    result = groupMgr->addActionSet(NULL, allBulbOff, &onPut);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;

    delete allBulbOff;
}

//Add actionset with NULL ActionSet
TEST_F(ThingsManagerTest, testAddActionSetAllBulbOffActionsetNull)
{
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, NULL, &onPut);
        Wait();
        EXPECT_TRUE(result == OC_STACK_ERROR);
        result = 0;
    }
}

//Add actionset
TEST_F(ThingsManagerTest, testAddActionSetAllBulbOn)
{
    string actionsetDesc;
    ActionSet *allBulbON = new ActionSet();
    allBulbON->actionsetName = "AllBulbOn";

    mocks.ExpectCallFunc(onPut).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "on";

        action->listOfCapability.push_back(capa);
        allBulbON->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, allBulbON, onPut);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
    delete allBulbON;
}

//Add actionset with NULL Resource
TEST_F(ThingsManagerTest, testAddActionSetAllBulbOnResourceNull)
{
    string actionsetDesc;
    ActionSet *allBulbON = new ActionSet();
    allBulbON->actionsetName = "AllBulbOn";

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "on";

        action->listOfCapability.push_back(capa);
        allBulbON->listOfAction.push_back(action);
    }

    result = groupMgr->addActionSet(NULL, allBulbON, onPut);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;

    delete allBulbON;
}

//Add actionset with NULL ActionSet
TEST_F(ThingsManagerTest, testAddActionSetAllBulbOnActionSetNull)
{
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, NULL, onPut);
        Wait();
        EXPECT_TRUE(result == OC_STACK_ERROR);
        result = 0;
    }
}

//Execute actionset
TEST_F(ThingsManagerTest, testExecuteActionSetAllBulbOn)
{
    string actionsetDesc;
    ActionSet *allBulbON = new ActionSet();
    allBulbON->actionsetName = "AllBulbOn1";

    mocks.ExpectCallFunc(onPut).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "on";

        action->listOfCapability.push_back(capa);
        allBulbON->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, allBulbON, onPut);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }

    mocks.ExpectCallFunc(onPost).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    if (g_resource)
    {
        result = groupMgr->executeActionSet(g_resource, "AllBulbOn", &onPost);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
    delete allBulbON;
}

//Execute actionset with NULL Resource
TEST_F(ThingsManagerTest, testExecuteActionSetAllBulbOnResourceNull)
{
    result = groupMgr->executeActionSet(NULL, "AllBulbOn", &onPost);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Execute actionset
TEST_F(ThingsManagerTest, testExecuteActionSetAllBulbOff)
{
    mocks.ExpectCallFunc(onPost).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    if (g_resource)
    {
        result = groupMgr->executeActionSet(g_resource, "AllBulbOff", &onPost);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
}

//Execute actionset with NULL resource
TEST_F(ThingsManagerTest, testExecuteActionSetAllBulbOffResourceNull)
{
    result = groupMgr->executeActionSet(NULL, "AllBulbOff", &onPost);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Execute actionset with Delay
TEST_F(ThingsManagerTest, testExcecuteActionSetWithDelay)
{
    string actionsetDesc;
    ActionSet *allBulbON = new ActionSet();
    allBulbON->actionsetName = "AllBulbOnDelay";
    allBulbON->setDelay(1);

    mocks.ExpectCallFunc(onPut).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "off";

        action->listOfCapability.push_back(capa);
        allBulbON->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, allBulbON, onPut);
        Wait();
    }

    mocks.ExpectCallFunc(onPost).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    if (g_resource)
    {
        result = groupMgr->executeActionSet(g_resource, "AllBulbOnDelay", &onPost);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }

    delete allBulbON;
}

//Execute actionset with Delay = 0
TEST_F(ThingsManagerTest, testExcecuteActionSetWithDelayEqulasZero)
{
    string actionsetDesc;
    ActionSet *allBulbON = new ActionSet();
    allBulbON->actionsetName = "AllBulbOnDelay";

    mocks.ExpectCallFunc(onPut).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "off";

        action->listOfCapability.push_back(capa);
        allBulbON->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, allBulbON, onPut);
        Wait();
    }

    if (g_resource)
    {
        result = groupMgr->executeActionSet(g_resource, "AllBulbOnDelay", 0, &onPost);
        Wait();
        EXPECT_TRUE(result == OC_STACK_INVALID_PARAM);
        result = 0;
    }

    delete allBulbON;
}

//Execute actionset with invalid Delay
TEST_F(ThingsManagerTest, testExcecuteActionSetWithInvalidDelay)
{
    string actionsetDesc;
    ActionSet *allBulbON = new ActionSet();
    allBulbON->actionsetName = "AllBulbOnDelay";

    mocks.ExpectCallFunc(onPut).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "off";

        action->listOfCapability.push_back(capa);
        allBulbON->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, allBulbON, onPut);
        Wait();
    }

    if (g_resource)
    {
        result = groupMgr->executeActionSet(g_resource, "AllBulbOnDelay", -10, &onPost);
        Wait();
        EXPECT_TRUE(result == OC_STACK_INVALID_PARAM);
        result = 0;
    }

    delete allBulbON;
}

//Execute actionset with delay on NULL Resource
TEST_F(ThingsManagerTest, testExcecuteActionSetWithDelayWithResourceNull)
{
    string actionsetDesc;
    ActionSet *allBulbON = new ActionSet();
    allBulbON->actionsetName = "AllBulbOnDelay";
    allBulbON->setDelay(5);

    mocks.ExpectCallFunc(onPut).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "off";

        action->listOfCapability.push_back(capa);
        allBulbON->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        result = groupMgr->addActionSet(g_resource, allBulbON, onPut);
        Wait();
    }
    result = groupMgr->executeActionSet(NULL, "AllBulbOnDelay", &onPost);
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;

    delete allBulbON;
}

//Cancel ActionSet
TEST_F(ThingsManagerTest, testCancelActionSet)
{
    mocks.ExpectCallFunc(onPost).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    if (g_resource)
    {
        result = groupMgr->cancelActionSet(g_resource, "AllBulbOff", &onPost);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
}

//Cancel ActionSet on NULL Resource
TEST_F(ThingsManagerTest, testCancelActionSetResourceNull)
{
    result = groupMgr->cancelActionSet(NULL, "AllBulbOff", &onPost);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Delete ActionSet
TEST_F(ThingsManagerTest, testDeleteActionSet)
{
    mocks.ExpectCallFunc(onPut).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    if (g_resource)
    {
        result = groupMgr->deleteActionSet(g_resource, "AllBulbOff", &onPut);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
}

//Delete ActionSet on NULL Resource
TEST_F(ThingsManagerTest, testDeleteActionSetResourceNull)
{
    result = groupMgr->deleteActionSet(NULL, "AllBulbOff", &onPut);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Get ActionSet
TEST_F(ThingsManagerTest, testGetActionSet)
{
    mocks.ExpectCallFunc(onPost).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    if (g_resource)
    {
        result = groupMgr->getActionSet(g_resource, "AllBulbOn", &onPost);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
}

//Get ActionSet on NULL Resource
TEST_F(ThingsManagerTest, testGetActionSetResourceNull)
{
    result = groupMgr->getActionSet(NULL, "AllBulbOn", &onPost);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Get Configurations
TEST_F(ThingsManagerTest, testGetConfigurations)
{
    ConfigurationName name = "all";

    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();
    std::vector< ConfigurationName > configurations;

    configurations.push_back(name);

    mocks.ExpectCallFunc(onGet).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    vector<string> types;
    types.push_back("oic.wk.con");

    result = groupMgr->findCandidateResources(types, &foundResources);

    std::mutex blocker;
    std::unique_lock < std::mutex > lock(blocker);
    cv2.wait(lock);

    if (result == OC_STACK_OK)
    {
        result = g_thingsConf->getConfigurations(configurationResource, configurations, &onGet);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
}

//Get Configurations with empty Configuration
TEST_F(ThingsManagerTest, testGetConfigurationsEmptyConfiguration)
{
    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();
    std::vector< ConfigurationName > configurations;

    vector<string> types;
    types.push_back("oic.wk.con");

    result = groupMgr->findCandidateResources(types, &foundResources);

    std::mutex blocker;
    std::unique_lock < std::mutex > lock(blocker);
    cv2.wait(lock);

    if (result == OC_STACK_OK)
    {
        result = g_thingsConf->getConfigurations(configurationResource, configurations, &onGet);
        Wait();
        EXPECT_TRUE(result == OC_STACK_ERROR);
        result = 0;
    }
}

//Get Configurations on NULL Resource
TEST_F(ThingsManagerTest, testGetConfigurationsResourceNull)
{
    ConfigurationName name = "all";
    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();
    std::vector< ConfigurationName > configurations;

    configurations.push_back(name);

    result = g_thingsConf->getConfigurations(NULL, configurations, &onGet);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Get all supported Configurations
TEST_F(ThingsManagerTest, testGetallSupportedCOnfigurations)
{
    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();
    string retVal = g_thingsConf->getListOfSupportedConfigurationUnits();
    EXPECT_FALSE(retVal.size() == 0);
}

//DoBootstrap
TEST_F(ThingsManagerTest, testDoBootstrap)
{
    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();

    mocks.ExpectCallFunc(onGetBootstrapInformation).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});
    result = g_thingsConf->doBootstrap(&onGetBootstrapInformation);
    Wait();
    EXPECT_TRUE(result == OC_STACK_OK);
    result = 0;
}

//DoBootstrap with NULL callback
TEST_F(ThingsManagerTest, testDoBootstrapCallBackNull)
{
    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();

    result = g_thingsConf->doBootstrap(NULL);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Update Configuration
TEST_F(ThingsManagerTest, testUpdateConfiguration)
{
    ConfigurationName name = "r";
    ConfigurationValue value = "INDIA";

    std::map< ConfigurationName, ConfigurationValue > configurations;
    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();
    configurations.insert(std::make_pair(name, value));

    mocks.ExpectCallFunc(onUpdate).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    vector<string> types;
    types.push_back("oic.wk.con");
    result = groupMgr->findCandidateResources(types, &foundResources);

    std::mutex blocker2;
    std::unique_lock < std::mutex > lock2(blocker2);
    cv2.wait(lock2);

    if (result == OC_STACK_OK)
    {
        result = g_thingsConf->updateConfigurations(configurationResource, configurations,
                                                    &onUpdate);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
}

//Update Configuration with Empty Configuration
TEST_F(ThingsManagerTest, testUpdateConfigurationEmptyConfiguration)
{
    std::map< ConfigurationName, ConfigurationValue > configurations;
    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();

    vector<string> types;
    types.push_back("oic.wk.con");
    result = groupMgr->findCandidateResources(types, &foundResources);

    std::mutex blocker2;
    std::unique_lock < std::mutex > lock2(blocker2);
    cv2.wait(lock2);

    if (result == OC_STACK_OK)
    {
        result = g_thingsConf->updateConfigurations(configurationResource, configurations,
                                                    &onUpdate);
        Wait();
        EXPECT_TRUE(result == OC_STACK_ERROR);
        result = 0;
    }
}

//Update Configuration on NULL Resource
TEST_F(ThingsManagerTest, testUpdateConfigurationResourceNull)
{
    ConfigurationName name = "r";
    ConfigurationValue value = "INDIA";

    std::map< ConfigurationName, ConfigurationValue > configurations;
    ThingsConfiguration *g_thingsConf = ThingsConfiguration::getInstance();

    configurations.insert(std::make_pair(name, value));

    result = g_thingsConf->updateConfigurations(NULL, configurations, &onUpdate);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Reboot
TEST_F(ThingsManagerTest, testReboot)
{
    ThingsMaintenance *g_thingsMnt = ThingsMaintenance::getInstance();

    mocks.ExpectCallFunc(onReboot).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    vector<string> types;
    types.push_back("oic.wk.mnt");
    result = groupMgr->findCandidateResources(types, &foundResources);

    std::mutex blocker;
    std::unique_lock < std::mutex > lock(blocker);
    cv2.wait(lock);

    if (result == OC_STACK_OK)
    {
        result = g_thingsMnt->reboot(configurationResource, &onReboot);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
}

//Reboot on NULL Resource
TEST_F(ThingsManagerTest, testRebootResourceNull)
{
    ThingsMaintenance *g_thingsMnt = ThingsMaintenance::getInstance();

    result = g_thingsMnt->reboot(NULL, &onReboot);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}

//Factory Reset
TEST_F(ThingsManagerTest, testFactoryReset)
{
    ThingsMaintenance *g_thingsMnt = ThingsMaintenance::getInstance();

    mocks.ExpectCallFunc(onFactoryReset).
    Do([this](const HeaderOptions& /*headerOptions*/, const OCRepresentation& /*rep*/,
            const int /*eCode*/) {   Proceed();});

    vector<string> types;
    types.push_back("oic.wk.mnt");
    result = groupMgr->findCandidateResources(types, &foundResources);

    std::mutex blocker;
    std::unique_lock < std::mutex > lock(blocker);
    cv2.wait(lock);

    if (result == OC_STACK_OK)
    {
        result = g_thingsMnt->factoryReset(configurationResource, &onFactoryReset);
        Wait();
        EXPECT_TRUE(result == OC_STACK_OK);
        result = 0;
    }
}

//Factory Reset on NULL Resource
TEST_F(ThingsManagerTest, testFactoryResetResourceNull)
{
    ThingsMaintenance *g_thingsMnt = ThingsMaintenance::getInstance();

    result = g_thingsMnt->factoryReset(NULL, &onFactoryReset);
    Wait();
    EXPECT_TRUE(result == OC_STACK_ERROR);
    result = 0;
}


//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

///
/// This sample shows how one could create a resource (collection) with children.
///

#include <functional>
#include <pthread.h>

#include "OCPlatform.h"
#include "OCApi.h"
#include "ThingsConfiguration.h"
#include "ThingsMaintenance.h"
#include "ConfigurationCollection.h"
#include "MaintenanceCollection.h"
#include "FactorySetCollection.h"

using namespace OC;
using namespace OIC;

const int SUCCESS_RESPONSE = 0;
int g_Steps = 0;
int isWaiting = 0;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

// Default system configuration value's variables
// The variable's names should be same as the names of "extern" variables defined in
// "ConfigurationResource.h"

std::string defaultDeviceName;
std::string defaultLocation;
std::string defaultLocationName;
std::string defaultRegion;
std::string defaultCurrency;

static ThingsConfiguration* g_thingsConf;

// Forward declaring the entityHandler (Configuration)
bool prepareResponseForResource(std::shared_ptr< OCResourceRequest > request);
OCStackResult sendResponseForResource(std::shared_ptr< OCResourceRequest > pRequest);
OCEntityHandlerResult entityHandlerForResource(std::shared_ptr< OCResourceRequest > request);

ConfigurationResource *myConfigurationResource;
MaintenanceResource *myMaintenanceResource;
FactorySetResource *myFactorySetResource;

typedef std::function< void(OCRepresentation&) > putFunc;
typedef std::function< OCRepresentation(void) > getFunc;

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
        res = std::bind(&MaintenanceResource::getMaintenanceRepresentation,
                myMaintenanceResource);
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
        res = std::bind(&MaintenanceResource::setMaintenanceRepresentation,
                myMaintenanceResource, std::placeholders::_1);
    }

    return res;
}

// This function prepares a response for any incoming request to Light resource.
bool prepareResponseForResource(std::shared_ptr< OCResourceRequest > request)
{
    std::cout << "\tIn Server CPP prepareResponseForResource:\n";
    bool result = false;
    if (request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if (requestFlag == RequestHandlerFlag::RequestFlag)
        {
            std::cout << "\t\trequestFlag : Request\n";

            // If the request type is GET
            if (requestType == "GET")
            {
                std::cout << "\t\t\trequestType : GET\n";
                // GET operations are directly handled while sending the response
                // in the sendLightResponse function
                result = true;
            }
            else if (requestType == "PUT")
            {
                std::cout << "\t\t\trequestType : PUT\n";
                putFunc putFunction;
                OCRepresentation rep = request->getResourceRepresentation();

                putFunction = getPutFunction(request->getResourceUri());

                // Do related operations related to PUT request
                putFunction(rep);
                result = true;
            }
            else if (requestType == "POST")
            {
                // POST request operations
            }
            else if (requestType == "DELETE")
            {
                // DELETE request operations
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

    // Check for query params (if any)
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

// callback handler on GET request
void onBootstrap(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    pthread_mutex_lock(&mutex_lock);
    isWaiting = 0;
    pthread_mutex_unlock(&mutex_lock);

    if (eCode != SUCCESS_RESPONSE)
    {
        std::cout << "onGET Response error: " << eCode << std::endl;
        return ;
    }

    std::cout << "\n\nGET request was successful" << std::endl;
    std::cout << "\tResource URI: " << rep.getUri() << std::endl;

    defaultDeviceName = rep.getValue< std::string >("n");
    defaultLocation = rep.getValue< std::string >("loc");
    defaultLocationName = rep.getValue< std::string >("locn");
    defaultRegion = rep.getValue< std::string >("r");
    defaultCurrency = rep.getValue< std::string >("c");

    std::cout << "\tDeviceName : " << defaultDeviceName << std::endl;
    std::cout << "\tLocation : " << defaultLocation << std::endl;
    std::cout << "\tLocationName : " << defaultLocationName << std::endl;
    std::cout << "\tCurrency : " << defaultCurrency << std::endl;
    std::cout << "\tRegion : " << defaultRegion << std::endl;

}

int main()
{
    //**************************************************************
    // STEP 0
    // Create PlatformConfig object
    PlatformConfig cfg
    { OC::ServiceType::InProc, OC::ModeType::Both, "0.0.0.0", 0, OC::QualityOfService::LowQos };

    OCPlatform::Configure(cfg);
    g_thingsConf = new ThingsConfiguration();
    //**************************************************************

    if (getuid() != 0)
    {
        std::cout << "NOTE: You may gain the root privilege (e.g, reboot)\n";
        std::cout << "NOTE: Now, you don't have it.\n";
    }

    try
    {
        // Perform app tasks
        while (true)
        {
            pthread_mutex_lock(&mutex_lock);
            if (isWaiting > 0)
            {
                pthread_mutex_unlock(&mutex_lock);
                continue;
            }

            isWaiting = 0;
            pthread_mutex_unlock(&mutex_lock);

            std::cout << endl << endl << "(0) Quit" << std::endl;
            std::cout << "(1) Bootstrap" << std::endl;
            std::cout << "(2) Create Configuration/Maintenance Resources" << std::endl;

            cin >> g_Steps;

            if (g_Steps == 0)
            {
                break;
            }
            else if (g_Steps == 1)
            {
                if( g_thingsConf->doBootstrap(&onBootstrap) == OC_STACK_OK)
                {
                    pthread_mutex_lock(&mutex_lock);
                    isWaiting = 1;
                    pthread_mutex_unlock(&mutex_lock);
                }
                else
                {
                    std::cout << "A callback pointer of the function is NULL." << std::endl;
                }
            }
            else if (g_Steps == 2)
            {
                myConfigurationResource = new ConfigurationResource();
                myConfigurationResource->createResources(&entityHandlerForResource);

                myMaintenanceResource = new MaintenanceResource();
                myMaintenanceResource->createResources(&entityHandlerForResource);

                myFactorySetResource = new FactorySetResource();
                myFactorySetResource->createResources(&entityHandlerForResource);
                myMaintenanceResource->factoryReset = std::function < void()
                        > (std::bind(&ConfigurationResource::factoryReset,
                                myConfigurationResource));

                pthread_mutex_lock(&mutex_lock);
                isWaiting = 1;
                pthread_mutex_unlock(&mutex_lock);
            }
        }
    }
    catch (OCException e)
    {
        std::cout << "Exception in main: " << e.what();
    }

    // No explicit call to stop the platform.
    // When OCPlatform destructor is invoked, internally we do platform cleanup
    return 0;
}


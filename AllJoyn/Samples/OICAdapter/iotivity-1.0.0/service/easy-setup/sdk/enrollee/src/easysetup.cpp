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

/**
 * @file
 *
 * This file contains the implementation for EasySetup Enrollee device
 */

#include "easysetup.h"

#include "logger.h"
#include "resourceHandler.h"

/**
 * @var ES_ENROLLEE_TAG
 * @brief Logging tag for module name.
 */
#define ES_ENROLLEE_TAG "ES"

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------

/**
 * @var targetSsid
 * @brief Target SSID of the Soft Access point to which the device has to connect
 */
static char *targetSsid;

/**
 * @var targetPass
 * @brief Password of the target access point to which the device has to connect
 */
static char *targetPass;

/**
 * @var g_cbForEnrolleeStatus
 * @brief Fucntion pointer holding the callback for intimation of EasySetup Enrollee status callback
 */
static EventCallback g_cbForEnrolleeStatus = NULL;

//-----------------------------------------------------------------------------
// Private internal function prototypes
//-----------------------------------------------------------------------------

void EventCallbackInOnboarding(ESResult esResult);
void EventCallbackInProvisioning(ESResult esResult);
void EventCallbackAfterProvisioning(ESResult esResult);

void EventCallbackInOnboarding(ESResult esResult)
{
    if (g_cbForEnrolleeStatus != NULL){
        OC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "Calling the application with esResult = %d", esResult);
        if(esResult == ES_OK){
            OC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "Calling the application with enrolleestate = %d",
                                                ES_ON_BOARDED_STATE);
            g_cbForEnrolleeStatus(esResult, ES_ON_BOARDED_STATE);
        }
        else{
            OC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "Calling the application with enrolleestate = %d",
                                                ES_INIT_STATE);
            g_cbForEnrolleeStatus(esResult, ES_INIT_STATE);
        }
    }
    else{
        OC_LOG(ERROR, ES_ENROLLEE_TAG, "g_cbForEnrolleeStatus is NULL");
    }
}

void EventCallbackInProvisioning(ESResult esResult)
{
    ESResult res = ES_OK;

    if (esResult == ES_RECVTRIGGEROFPROVRES)
    {
        targetSsid = (char *) malloc(MAXSSIDLEN);
        targetPass = (char *) malloc(MAXNETCREDLEN);

        if(TerminateEasySetup() != OC_STACK_OK)
        {
            OC_LOG(ERROR, ES_ENROLLEE_TAG, "Terminating stack failed");
            return;
        }

        GetTargetNetworkInfoFromProvResource(targetSsid, targetPass);

        res = ConnectToWiFiNetwork(targetSsid, targetPass, EventCallbackAfterProvisioning);

        if (g_cbForEnrolleeStatus != NULL)
        {
            if(res == ES_NETWORKCONNECTED){
                OC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "Calling the application with enrolleestate = %d",
                                                    ES_PROVISIONED_STATE);
                g_cbForEnrolleeStatus(ES_OK, ES_PROVISIONED_STATE);
            }
            else{
                OC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "Calling the application with enrolleestate = %d",
                                                    ES_PROVISIONING_STATE);
                g_cbForEnrolleeStatus(ES_OK, ES_PROVISIONING_STATE);
            }

        }
    }
}

void EventCallbackAfterProvisioning(ESResult esResult)
{
    if (g_cbForEnrolleeStatus != NULL){
        if(esResult == ES_OK){
            OC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "Calling the application with enrolleestate = %d",
                                                   ES_PROVISIONED_STATE);
            g_cbForEnrolleeStatus(esResult, ES_PROVISIONED_STATE);
        }
        else{
            OC_LOG_V(DEBUG, ES_ENROLLEE_TAG, "Calling the application with enrolleestate = %d",
                                                   ES_PROVISIONING_STATE);
            g_cbForEnrolleeStatus(esResult, ES_PROVISIONING_STATE);
        }
    }
    else{
        OC_LOG(ERROR, ES_ENROLLEE_TAG, "g_cbForEnrolleeStatus is NULL");
    }
}

ESResult FindNetworkForOnboarding(OCConnectivityType networkType,
                                           const char *ssid,
                                           const char *passwd,
                                           EventCallback cb)
{
    if (!ssid || !passwd)
    {
        return ES_ERROR;
    }

    if (networkType == CT_ADAPTER_IP)
    {
        if (g_cbForEnrolleeStatus == NULL)
        {
            g_cbForEnrolleeStatus = cb;
        }

        if(ConnectToWiFiNetwork(ssid, passwd, EventCallbackInOnboarding) != ES_NETWORKCONNECTED)
        {
            OC_LOG(ERROR, ES_ENROLLEE_TAG, "ConnectToWiFiNetwork Failed");
            cb(ES_ERROR, ES_ON_BOARDING_STATE);
            return ES_ERROR;
        }
        else{
            OC_LOG(INFO, ES_ENROLLEE_TAG, "ConnectToWiFiNetwork Success");
            cb(ES_OK, ES_ON_BOARDED_STATE);
            return ES_OK;
        }
    }
    return ES_ERROR;
}


ESResult InitEasySetup(OCConnectivityType networkType, const char *ssid, const char *passwd,
              EventCallback cb)
{
    if(FindNetworkForOnboarding(networkType, ssid, passwd, cb) != ES_OK)
    {
        OC_LOG(ERROR, ES_ENROLLEE_TAG, "OnBoarding Failed");
        return ES_ERROR;
    }

    // Initialize the OC Stack in Server mode
    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, ES_ENROLLEE_TAG, "OCStack init error");
        return ES_ERROR;
    }
    else
    {
        OC_LOG(DEBUG, ES_ENROLLEE_TAG, "OCStack init success");
        return ES_OK;
    }
}

ESResult TerminateEasySetup()
{
    if(OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, ES_ENROLLEE_TAG, "OCStack stop failed");
        return ES_ERROR;
    }
    else
    {
        OC_LOG(ERROR, ES_ENROLLEE_TAG, "OCStack stop success");
        return ES_OK;
    }
}

ESResult InitProvisioning()
{
    if (CreateProvisioningResource() != OC_STACK_OK)
    {
        return ES_ERROR;
    }

    if (CreateNetworkResource() != OC_STACK_OK)
    {
        return ES_ERROR;
    }

    RegisterResourceEventCallBack(EventCallbackInProvisioning);

    return ES_RESOURCECREATED;
}


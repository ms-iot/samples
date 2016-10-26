//******************************************************************
//
// Copyright 2014 Intel Corporation All Rights Reserved.
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

// OCClient.cpp : Defines the entry point for the console application.
//
#define __GXX_EXPERIMENTAL_CXX0X__ 1

#include <string>
#include <cstdlib>
#include "OCPlatform.h"
#include "OCApi.h"
#include "oicapp-utils.h"

using namespace OC;

#define ISFORLINUX 1
#define ISFORTIZEN 0

const int SUCCESS_RESPONSE = 0;
static ObserveType OBSERVE_TYPE_TO_USE = ObserveType::Observe;

const char *PREFIX_URI = "Uri : ";
const char *PREFIX_HOST = "Host : ";

OCPlatform *g_platform = nullptr;
PlatformConfig g_cfg;

std::shared_ptr< OCResource > g_curResource = nullptr;
AttributeMap g_curAttributeMap;

oicappData *g_oicappClientAd;
oicappData *g_oicFindAd;
oicappData *g_oicObserveAd;

OCStackResult nmfindResource(const std::string &host , const std::string &resourceName);
void onObserve(const OCRepresentation &rep , const int &eCode , const int &sequenceNumber);
void onfound();
void onobserve();

void findResourceCandidate(oicappData *ad)
{
    try
    {
        nmfindResource("" , "coap://224.0.1.187/oc/core?rt=notificationmanager.hosting");
        std::cout << "Finding Resource... " << std::endl;

    }
    catch (OCException &e)
    {
        ERR("findResourceCandidate exception: %s" , e.what().c_str());
    }
    g_oicFindAd = ad;
}

void startObserve(oicappData *ad)
{
    if (g_curResource != nullptr)
    {
        g_oicObserveAd = ad;
        QueryParamsMap test;
        g_curResource->observe(ObserveType::Observe , test , &onObserve);
    }
}

void printAttributeMap(const AttributeMap attributeMap)
{
    for (auto it = attributeMap.begin() ; it != attributeMap.end() ; ++it)
    {
        DBG("\tAttribute name: %s" , it->first.c_str());
        for (auto valueItr = it->second.begin() ; valueItr != it->second.end() ; ++valueItr)
        {
            DBG("\t\tAttribute value: %s" , (*valueItr).c_str());
        }
    }
}

void updateAttribute(const AttributeMap attributeMap)
{
    g_oicappClientAd->temp = std::stoi(attributeMap.at("temp")[0]);
    g_oicappClientAd->humid = std::stoi(attributeMap.at("humid")[0]);
}

void cancelObserve()
{
    DBG("Cancelling Observe...");

    OCStackResult result = OC_STACK_ERROR;

    if (g_curResource != nullptr)
    {
        result = g_curResource->cancelObserve();
    }

    DBG("Cancel result: %d" , result);
}

void onObserve(const OCRepresentation &rep , const int &eCode , const int &sequenceNumber)
{

    AttributeMap attributeMap = rep.getAttributeMap();
    if (eCode == SUCCESS_RESPONSE)
    {
        DBG("OBSERVE RESULT:");
        DBG("\tSequenceNumber: %d" , sequenceNumber);

        printAttributeMap(attributeMap);
//      updateAttribute(attributeMap);
        g_curAttributeMap = attributeMap;
        onobserve();
    }
    else
    {
        ERR("onObserve Response error: %d" , eCode);
        //std::exit(-1);
    }
}

// callback handler on PUT request
void onPut(const OCRepresentation &rep , const int eCode)
{
    AttributeMap attributeMap = rep.getAttributeMap();
    if (eCode == SUCCESS_RESPONSE)
    {
        DBG("PUT request was successful");

        printAttributeMap(attributeMap);

        if (OBSERVE_TYPE_TO_USE == ObserveType::Observe)
            INFO("Observe is used.");
        else if (OBSERVE_TYPE_TO_USE == ObserveType::ObserveAll)
            INFO("ObserveAll is used.");

        if (g_curResource != nullptr)
        {
            DBG("Observe Start");
            QueryParamsMap test;
            g_curResource->observe(ObserveType::Observe , test , &onObserve);
        }
    }
    else
    {
        ERR("onPut Response error: %d" , eCode);
        //std::exit(-1);
    }
}

// callback handler on GET request
void onGet(const OCRepresentation &rep , const int eCode)
{

    AttributeMap attributeMap = rep.getAttributeMap();
    if (eCode == SUCCESS_RESPONSE)
    {
        DBG("GET Succeeded:");

        printAttributeMap(attributeMap);
        updateAttribute(attributeMap);
    }
    else
    {
        ERR("onGet Response error: %d" , eCode);
        //std::exit(-1);
    }
}

// Local function to get representation of light resource
void getLightRepresentation(std::shared_ptr< OCResource > resource)
{
    if (resource)
    {
        DBG("Getting Light Representation...");
        // Invoke resource's get API with the callback parameter
        QueryParamsMap test;
        resource->get(test , &onGet);
    }
}

// Callback to found resources
static void foundResource(std::shared_ptr< OCResource > resource)
{
    try
    {
        if (resource)
        {
            DBG("DISCOVERED Resource:");
            DBG("\tURI of the resource: %s" , resource->uri().c_str());
            DBG("\tHost address of the resource: %s" , resource->host().c_str());

            if (resource->uri().find("/a/NM") != string::npos)
            {

                g_curResource = resource;
                onfound();
            }
        }
        else
        {
            ERR("Resource is invalid");
        }

    }
    catch (std::exception &e)
    {
        ERR("foundResource exception: %s" , e.what().c_str());
    }
}

OCStackResult nmfindResource(const std::string &host , const std::string &resourceName)
{
    if (g_platform != nullptr)
    {
        return g_platform->findResource(host , resourceName , &foundResource);
    }

    return OC_STACK_ERROR;
}

int oicapp_client_start(oicappData *ad)
{
    g_cfg.ipAddress = ad->ipAddr;
    g_cfg.port = 5683;
    g_cfg.mode = ModeType::Client;
    g_cfg.serviceType = ServiceType::InProc;

    retv_if(NULL != g_platform , -1);

    g_platform = new OCPlatform(g_cfg);

    g_oicappClientAd = ad;

    return 0;
}

void oicapp_client_stop()
{
    if (g_curResource != nullptr)
    {
        cancelObserve();
        g_curResource = NULL;
    }

    if (g_platform)
    {
        delete (g_platform);
        g_platform = NULL;
    }
    g_oicappClientAd = NULL;
}

// Local function to put a different state for this resource
int oicapp_client_put(int power , int level)
{
    std::shared_ptr< OCResource > resource = g_curResource;

    retv_if(NULL == g_curResource , -1);

    DBG("Putting light representation...");

    AttributeMap attributeMap;

    AttributeValues tempVal;
    AttributeValues humidVal;

    tempVal.push_back(to_string(power));
    humidVal.push_back(to_string(level));

    attributeMap["temp"] = tempVal;
    attributeMap["humid"] = humidVal;

    // Create QueryParameters Map and add query params (if any)
    QueryParamsMap queryParamsMap;

    OCRepresentation rep;
    rep.setAttributeMap(attributeMap);

    // Invoke resource's pit API with attribute map, query map and the callback parameter
    resource->put(rep , queryParamsMap , &onPut);

    return 0;
}

void onfound()
{

    if (g_curResource->uri().find("/a/NM/TempHumSensor/virtual") != string::npos)
    {
        oicappData *ad = g_oicFindAd;

        std::string tmpuri = PREFIX_URI + g_curResource->uri();
        std::string tmphost = PREFIX_HOST + g_curResource->host();

        DBG("OnFound Resource...");
        DBG("Resource Uri : %s" , tmpuri.c_str());
        DBG("Resource Host: %s" , tmphost.c_str());

        _gl_update_item(ad , tmphost.c_str() , ad->itemConsumerHost);
        _gl_update_item(ad , tmpuri.c_str() , ad->itemConsumerUri);
    }

}
void onobserve()
{

    oicappData *ad = g_oicObserveAd;

    AttributeMap attributeMap = g_curAttributeMap;

    std::string tmpStr[2];
    int index = 0;
    for (auto it = attributeMap.begin() ; it != attributeMap.end() ; ++it)
    {
        tmpStr[index] = it->first;
        tmpStr[index].append(" : ");
        for (auto value = it->second.begin() ; value != it->second.end() ; ++value)
        {
            tmpStr[index].append(*value);
        }
        index++;
    }

    DBG("%s" , tmpStr[0].c_str());
    DBG("%s" , tmpStr[1].c_str());
    _gl_update_item(ad , strdup(tmpStr[0].c_str()) , ad->itemConsumerTemp);
    _gl_update_item(ad , strdup(tmpStr[1].c_str()) , ad->itemConsumerHumid);
}

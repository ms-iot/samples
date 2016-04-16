/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <algorithm>

#include "tmsampleapp.h"
#include "tmutil.h"

using namespace std;
using namespace OC;
using namespace OIC;

namespace PH = std::placeholders;

static Evas_Object *log_entry = NULL;
static Evas_Object *list = NULL;
static Evas_Object *naviframe = NULL;

string BULBOFF              = "AllBulbOff";
string BULBON               = "AllBulbOn";
string resourceURI          = "/core/b/collection";
string resourceTypeName     = "b.collection";

std::vector< OCResourceHandle > groupResourceHandleVector;
OCResourceHandle resourceHandle = NULL;
OCResourceHandle foundResourceHandle = NULL;
shared_ptr< OCResource > g_resource ;
std::vector< string > lights;

GroupManager *groupMgr = NULL;

typedef struct datetime_popup
{
    Evas_Object *popup;
    Evas_Object *entry;
} datetime_popup_fields;

// Function to update the log in UI
void *updateGroupLog(void *data)
{
    string *log = (string *)data;
    // Show the log
    elm_entry_entry_append(log_entry, (*log).c_str());
    elm_entry_cursor_end_set(log_entry);
    return NULL;
}

void onPut(const HeaderOptions &headerOptions, const OCRepresentation &rep, const int eCode)
{
    string logMessage;
    dlog_print(DLOG_INFO, LOG_TAG, "#### onPut Callback Received!!!!");
    logMessage += "API Result: Success<br>onPut Callback Received<br>";
    if (OC_STACK_OK == eCode)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Result is OK");
    }
    else
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Invalid Parameter");
    }
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

void onPost(const HeaderOptions &headerOptions, const OCRepresentation &rep, const int eCode)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onPost callback received ENTRY!!!!");
    string logMessage;
    logMessage += "API Result: Success<br>onPost Callback Received<br>";

    if (rep.hasAttribute("ActionSet"))
    {
        string plainText;
        if (rep.getValue("ActionSet", plainText))
        {
            ActionSet *actionset = groupMgr->getActionSetfromString(plainText);
            if (NULL != actionset)
            {
                dlog_print(DLOG_INFO, LOG_TAG, "#### ACTIONSET NAME :: (%s)",
                           actionset->actionsetName.c_str());
                logMessage += "ACTIONSET NAME :: " + actionset->actionsetName + "<br>";
                for (auto actIter = actionset->listOfAction.begin();
                     actIter != actionset->listOfAction.end(); ++actIter)
                {
                    dlog_print(DLOG_INFO, LOG_TAG, "#### TARGET :: (%s)",
                               (*actIter)->target.c_str());
                    logMessage += logMessage + "TARGET :: " + (*actIter)->target + "<br>";
                    for (auto capaIter = (*actIter)->listOfCapability.begin();
                         capaIter != (*actIter)->listOfCapability.end(); ++capaIter)
                    {
                        dlog_print(DLOG_INFO, LOG_TAG, "#### POWER :: (%s)",
                                   (*capaIter)->status.c_str());
                        logMessage += logMessage + "CAPABILITY :: " +
                                      (*capaIter)->status + "<br>";
                    }
                }
            }
            delete actionset;
        }
    }
    else if (rep.hasAttribute("DoAction"))
    {
        string plainText;
        if (rep.getValue("DoAction", plainText))
        {
            logMessage += plainText + "<br>";
            dlog_print(DLOG_INFO, LOG_TAG, "#### DO ACTION :: (%s)", plainText.c_str());

        }
    }
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### onPost callback received EXIT!!!!");
}

// Method for Creating the action Set AllBulbOff
static void createActionSet_AllBulbOff()
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### createActionSet_AllBulbOff ENTRY");

    OIC::ActionSet *actionSet = new OIC::ActionSet();
    actionSet->actionsetName = BULBOFF;
    int size = lights.size();

    if (0 == size)
    {
        string logMessage = "NO LIGHTSERVER FOUND <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### NO LIGHT SERVER FOUND");
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                              &logMessage);
        delete actionSet;
        return;
    }

    for (int i = 0; i < size; i++)
    {
        OIC::Action *action = new OIC::Action();
        action->target = lights.at(i);

        OIC::Capability *capability = new OIC::Capability();
        capability->capability = "power";
        capability->status = "off";

        action->listOfCapability.push_back(capability);
        actionSet->listOfAction.push_back(action);
    }
    dlog_print(DLOG_INFO, LOG_TAG, "#### G_URI: %s", g_resource->uri().c_str());
    dlog_print(DLOG_INFO, LOG_TAG, "#### G_HOST: %S", g_resource->host().c_str());

    try
    {
        if (g_resource)
        {
            groupMgr->addActionSet(g_resource, actionSet, onPut);
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }
    delete actionSet;

    string logMessage = "Create actionset AllBulbOFF success <br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### createActionSet_AllBulbOff EXIT");
}

/* Method for Creating the action Set AllBulbOn
   once we create the ActionSet we can execute Action Set using executeActionSetOn()
   or delete using deleteActionSetOn() */
static void createActionSet_AllBulbOn()
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### createActionSet_AllBulbOn ENTRY");

    OIC::ActionSet *actionSet = new OIC::ActionSet();
    actionSet->actionsetName = BULBON;
    int size = lights.size();

    if (0 == size)
    {
        string logMessage = "NO LIGHTSERVER FOUND <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### NO LIGHT SERVER FOUND");
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
        delete actionSet;
        return;
    }

    for (int i = 0; i < size; i++)
    {
        OIC::Action *action = new OIC::Action();
        action->target = lights.at(i);

        OIC::Capability *capability = new OIC::Capability();
        capability->capability = "power";
        capability->status = "on";

        action->listOfCapability.push_back(capability);
        actionSet->listOfAction.push_back(action);
    }
    string URI = g_resource->uri();
    string host = g_resource->host();
    dlog_print(DLOG_INFO, LOG_TAG, "#### G_URI: %s", g_resource->uri().c_str());
    dlog_print(DLOG_INFO, LOG_TAG, "#### G_HOST: %S", g_resource->host().c_str());

    try
    {
        if (g_resource)
        {
            groupMgr->addActionSet(g_resource, actionSet, onPut);
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }
    delete actionSet;

    string logMessage = "Create actionset AllBulbON success <br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### createActionSet_AllBulbOff OFF EXIT");
}

static void createRecursiveActionSet_AllBulbOn(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### createRecursiveActionSet_AllBulbOn ENTRY");
    string actionsetDesc;
    ActionSet *allBulbOn = new ActionSet();
    allBulbOn->type = OIC::ACTIONSET_TYPE::RECURSIVE;

    allBulbOn->actionsetName = "AllBulbOnRecursiveCall";
    allBulbOn->mTime.tm_year = 0;
    allBulbOn->mTime.tm_mon = 0;
    allBulbOn->mTime.tm_mday = 0;
    allBulbOn->mTime.tm_hour = 0;
    allBulbOn->mTime.tm_min = 0;
    allBulbOn->mTime.tm_sec = 5;

    allBulbOn->setDelay(allBulbOn->getSecAbsTime());

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "on";

        action->listOfCapability.push_back(capa);
        allBulbOn->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        groupMgr->addActionSet(g_resource, allBulbOn, onPut);
    }

    delete allBulbOn;
    dlog_print(DLOG_INFO, LOG_TAG, "#### createRecursiveActionSet_AllBulbOn EXIT");
}

static void createScheduledActionSet_AllBulbOff(int date, int month, int year,
        int hour, int minute, int second)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### createScheduledActionSet_AllBulbOff ENTRY");
    string actionsetDesc;
    ActionSet *allBulbOff = new ActionSet();
    allBulbOff->type = OIC::ACTIONSET_TYPE::SCHEDULED;
    allBulbOff->actionsetName = "AllBulbOffScheduledCall";
    allBulbOff->mTime.tm_year = year;
    allBulbOff->mTime.tm_mon = month;
    allBulbOff->mTime.tm_mday = date;
    allBulbOff->mTime.tm_hour = hour;
    allBulbOff->mTime.tm_min = minute;
    allBulbOff->mTime.tm_sec = second;
    dlog_print(DLOG_INFO, LOG_TAG, "#### allBulbOff->mTime.tm_year :: %ld",
               allBulbOff->mTime.tm_year);
    dlog_print(DLOG_INFO, LOG_TAG, "#### allBulbOff->mTime.tm_mon :: %ld",
               allBulbOff->mTime.tm_mon);
    dlog_print(DLOG_INFO, LOG_TAG, "#### allBulbOff->mTime.tm_mday :: %ld",
               allBulbOff->mTime.tm_mday);
    dlog_print(DLOG_INFO, LOG_TAG, "#### allBulbOff->mTime.tm_hour :: %ld",
               allBulbOff->mTime.tm_hour);
    dlog_print(DLOG_INFO, LOG_TAG, "#### allBulbOff->mTime.tm_min :: %ld",
               allBulbOff->mTime.tm_min);
    dlog_print(DLOG_INFO, LOG_TAG, "#### allBulbOff->mTime.tm_sec :: %ld",
               allBulbOff->mTime.tm_sec);

    allBulbOff->setDelay(allBulbOff->getSecondsFromAbsoluteTime());

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
        groupMgr->addActionSet(g_resource, allBulbOff, onPut);
    }

    delete allBulbOff;
    dlog_print(DLOG_INFO, LOG_TAG, "#### createScheduledActionSet_AllBulbOff EXIT");
}

static void scheduled_AllbulbOff(void *data, Evas_Object *obj, void *event_info)
{
    groupMgr->executeActionSet(g_resource, "AllBulbOffScheduledCall", &onPost);
}

static void scheduled_AllbulbOffEx(void *data, Evas_Object *obj, void *event_info)
{
    groupMgr->executeActionSet(g_resource, "AllBulbOffScheduledCall", 10, &onPost);
}

static void cancelScheduled_AllBulbOff(void *data, Evas_Object *obj, void *event_info)
{
    groupMgr->cancelActionSet(g_resource, "AllBulbOffScheduledCall", &onPost);
}

static void recursive_allBulbOn(void *data, Evas_Object *obj, void *event_info)
{
    groupMgr->executeActionSet(g_resource, "AllBulbOnRecursiveCall", &onPost);
}

static void recursive_allBulbOnEx(void *data, Evas_Object *obj, void *event_info)
{
    groupMgr->executeActionSet(g_resource, "AllBulbOnRecursiveCall", 10, &onPost);
}

static void cancelRecursive_allBulbOn(void *data, Evas_Object *obj, void *event_info)
{

    groupMgr->cancelActionSet(g_resource, "AllBulbOnRecursiveCall", &onPost);
}

/* Method for executing the action Set AllBulbOff that we have created using
   createActionSet_AllBulbOff() */
static void executeActionSetOff(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### executeActionSetOff ENTRY");
    int size = lights.size();

    if (0 == size)
    {
        string logMessage = "NO LIGHTSERVER FOUND <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### NO LIGHT SERVER FOUND");
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
        return;
    }

    try
    {
        if (g_resource)
        {
            groupMgr->executeActionSet(g_resource, BULBOFF, &onPost);
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    string logMessage = "Actionset OFF called successfully <br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### executeActionSetOff EXIT");
}

/* Method for executing the action Set AllBulbOn that we have created using
   createActionSet_AllBulbOn() */
static void executeActionSetOn(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### executeActionSetOn ENTRY");
    int size = lights.size();

    if (0 == size)
    {
        string logMessage = "NO LIGHTSERVER FOUND <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### NO LIGHT SERVER FOUND");
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
        return;
    }

    try
    {
        if (g_resource)
        {
            groupMgr->executeActionSet(g_resource, BULBON, &onPost);
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    string logMessage = "Actionset ON called successfully <br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### executeActionSetOn EXIT");
}

/* Method for getting the action Set AllBulbOff that we have created using
   createActionSet_AllBulbOff() */
static void getActionSetOff(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### getActionSetOff ENTRY");
    int size = lights.size();

    if (0 == size)
    {
        string logMessage = "NO LIGHTSERVER FOUND <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### NO LIGHT SERVER FOUND");
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
        return;
    }

    try
    {
        if (g_resource)
        {
            groupMgr->getActionSet(g_resource, BULBOFF, &onPost);
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### getActionSetOff EXIT");
}

/* Method for getting the action Set AllBulbOn that we have created using
   createActionSet_AllBulbOn() */
static void getActionSetOn(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### getActionSetOn ENTRY");
    int size = lights.size();

    if (0 == size)
    {
        string logMessage = "NO LIGHTSERVER FOUND <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### NO LIGHT SERVER FOUND");
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
        return;
    }

    try
    {
        if (g_resource)
        {
            groupMgr->getActionSet(g_resource, BULBON, &onPost);
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### getActionSetOn EXIT");
}

/* Method for deleting the action Set AllBulbOff that we have created using
   createActionSet_AllBulbOff() */
static void deleteActionSetOff(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### deleteActionSetOff ENTRY");
    int size = lights.size();

    if (0 == size)
    {
        string logMessage = "NO LIGHTSERVER FOUND <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### NO LIGHT SERVER FOUND");
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
        return;
    }

    try
    {
        if (g_resource)
        {
            groupMgr->deleteActionSet(g_resource, BULBOFF, &onPost);
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    string logMessage = "Actionset OFF DELETED <br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### deleteActionSetOff EXIT");
}

/* Method for deleting the action Set AllBulbOn that we have created using
   createActionSet_AllBulbOn() */
static void deleteActionSetOn(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### deleteActionSetOn ENTRY");
    int size = lights.size();

    if (0 == size)
    {
        string logMessage = "NO LIGHTSERVER FOUND <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### NO LIGHT SERVER FOUND");
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
        return;
    }

    try
    {
        if (g_resource)
        {
            groupMgr->deleteActionSet(g_resource, BULBON, &onPost);
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    string logMessage = "Actionset ON DELETED <br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### deleteActionSetOn EXIT");
}

void onObserve(const HeaderOptions headerOptions, const OCRepresentation &rep, const int &eCode,
               const int &sequenceNumber)
{
    string logMessage;
    char *buf;
    if (OC_STACK_OK == eCode)
    {
        int level;
        buf = (char *)malloc(4 * sizeof(char));
        if (NULL == buf)
        {
            dlog_print(DLOG_INFO, LOG_TAG, " buf malloc failed");
            return;
        }
        sprintf(buf, "%d", sequenceNumber);
        logMessage = "OBSERVE RESULT <br>";
        logMessage += "Sequencenumber:" + string(buf) + "<br>";

        if (rep.getValue("level", level))
        {
            if (level == 0)
            {
                createActionSet_AllBulbOn();
                executeActionSetOn(NULL, NULL, NULL);
            }
            else
            {
                createActionSet_AllBulbOff();
                executeActionSetOff(NULL, NULL, NULL);
            }
        }
        sprintf(buf, "%d", level);
        logMessage += "level:" + string(buf) + "<br>";
        free(buf);
    }
    else
    {
        logMessage = "onObserve error!!!";
    }
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

// Callback to be called when resources are found in the network
void foundResources(std::vector< std::shared_ptr< OC::OCResource > > listOfResource)
{
    try
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### foundResources entry!!!!");

        for (auto rsrc = listOfResource.begin(); rsrc != listOfResource.end(); ++rsrc)
        {
            string resourceURI = (*rsrc)->uri();
            string hostAddress = (*rsrc)->host();

            dlog_print(DLOG_INFO, LOG_TAG, "#### found uri: %s", resourceURI.c_str());
            dlog_print(DLOG_INFO, LOG_TAG, "#### found host address: %s", hostAddress.c_str());
            string logMessage = "URI: " + resourceURI + "<br>";
            logMessage = logMessage + "Host:" + hostAddress + "<br>";
            logMessage += "----------------------<br>";

            if (resourceURI == "/a/light")
            {
                bool found;
                found = std::find(lights.begin(), lights.end(),
                                  hostAddress + resourceURI) != lights.end();
                if (found == false)
                {
                    lights.push_back((hostAddress + resourceURI));

                    try
                    {
                        dlog_print(DLOG_INFO, LOG_TAG, "#### Registering Resource");
                        OCStackResult result = OCPlatform::registerResource(foundResourceHandle,
                                               (*rsrc));
                        dlog_print(DLOG_INFO, LOG_TAG, "#### %s REGISTERED", resourceURI.c_str());
                        if (result == OC_STACK_OK)
                        {
                            OCPlatform::bindResource(resourceHandle, foundResourceHandle);
                            dlog_print(DLOG_INFO, LOG_TAG, "#### Bind Resource Done");
                            groupResourceHandleVector.push_back(foundResourceHandle);
                        }
                        else
                        {
                            dlog_print(DLOG_ERROR, LOG_TAG, "#### Register Resource ERROR");
                        }
                    }
                    catch (std::exception &e)
                    {
                        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
                    }
                }
            }
            else if (resourceURI == "/core/bookmark")
            {
                logMessage += "OBSERVING : " + resourceURI + "<br>";
                logMessage += "----------------------<br>";
                (*rsrc)->observe(ObserveType::Observe, QueryParamsMap(), &onObserve);
                dlog_print(DLOG_INFO, LOG_TAG, "#### after calling observe() for bookmark!!!!");
            }
            dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
            ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                                  &logMessage);
        }

        dlog_print(DLOG_INFO, LOG_TAG, "#### foundResources exit!!!!");
    }
    catch (...)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Exception caught in foundResources");
    }
}

static void create_group()
{

    groupMgr = new GroupManager();

    if (NULL != groupMgr)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### calling findCandidateResources from "
                   "create_group!!!!");
        vector< string > types;
        types.push_back("core.light");
        groupMgr->findCandidateResources(types, &foundResources, FINDRESOURCE_TIMEOUT);
    }

    try
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### calling registerResource from create_group!!!!");
        OCPlatform::registerResource(resourceHandle, resourceURI, resourceTypeName,
                                     BATCH_INTERFACE, NULL,
                                     OC_DISCOVERABLE | OC_OBSERVABLE);

        if (NULL != resourceHandle)
            dlog_print(DLOG_INFO, LOG_TAG, "#### Obtained resourceHandle from "
                       "registerResource!!!!");

        dlog_print(DLOG_INFO, LOG_TAG, "#### calling bindInterfaceToResource from "
                   "create_group!!!!");
        OCPlatform::bindInterfaceToResource(resourceHandle, GROUP_INTERFACE);
        OCPlatform::bindInterfaceToResource(resourceHandle, DEFAULT_INTERFACE);
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }
}

// Method for Finding the Light Resource
void findLightResource(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### findLightResource ENTRY");
    if (NULL != groupMgr)
    {
        vector< string > types;
        types.push_back("core.light");
        OCStackResult result = groupMgr->findCandidateResources(types, &foundResources,
                               FINDRESOURCE_TIMEOUT);
        if (result == OC_STACK_OK)
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### create_group -- findCandidateResources :: "
                       "OC_STACK_OK!!!!");
        }
        else
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### create_group - findCandidateResources failed!!");
        }
    }
    dlog_print(DLOG_INFO, LOG_TAG, "#### findLightResource EXIT");
}

// Method for observing the Bookmark Resource
void observeBookMark(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### observeBookMark ENTRY");
    if (NULL != groupMgr)
    {
        vector< string > types;
        types.push_back("core.bookmark");
        OCStackResult result = groupMgr->findCandidateResources(types, &foundResources,
                               FINDRESOURCE_TIMEOUT);
        if (OC_STACK_OK == result)
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### create_group - findCandidateResources :: "
                       "OC_STACK_OK!!!!");
        }
        else
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### create_group - findCandidateResources failed!!");
        }
    }
    dlog_print(DLOG_INFO, LOG_TAG, "#### observeBookMark EXIT");
}

static void onDestroy()
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### Destroy sequence called");

    try
    {
        if (NULL != foundResourceHandle)
        {
            OCPlatform::unregisterResource(foundResourceHandle);
            dlog_print(DLOG_INFO, LOG_TAG, "#### Light Resource unregistered");
            lights.clear();
        }
        else
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### No resouceHandle found to unregister");
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    try
    {
        if (NULL != resourceHandle)
        {
            // Unbind Light resource
            if (NULL != foundResourceHandle)
            {
                OCPlatform::unbindResource(resourceHandle, foundResourceHandle);
                dlog_print(DLOG_INFO, LOG_TAG, "#### Resource Unbind Done");
            }
            OCPlatform::unregisterResource(resourceHandle);
            dlog_print(DLOG_INFO, LOG_TAG, "#### Group Unregistered");
        }
        else
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### foundResourceHandle is NULL");
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    delete groupMgr;
}

static void createActionSet(void *data, Evas_Object *obj, void *event_info)
{
    createActionSet_AllBulbOff();
    createActionSet_AllBulbOn();
}

static void
popup_cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    datetime_popup_fields *popup_fields = (datetime_popup_fields *)data;
    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static int parseString(const char *str, int slen, int *beg, int what)
{
    int i, val = 0, ch = '/';
    if (2 == what)
    {
        ch = ' ';
    }
    else if (3 <= what)
    {
        if (5 == what)
        {
            ch = '\0';
        }
        else
        {
            ch = ':';
        }
    }
    // Remove whitespaces(if any) at the beginning
    while (str[*beg] == ' ')
    {
        (*beg)++;
    }
    for (i = *beg; i < slen; i++)
    {
        if (str[i] != ch)
        {
            val = (val * 10) + (str[i] - 48);
            continue;
        }
        break;
    }
    (*beg) = i + 1;
    return val;
}

static bool validate(int date, int month, int year, int hour, int minute, int second)
{
    if (date <= 0 || month <= 0 || year <= 0 || hour < 0 || minute < 0 || second < 0
        || month >= 13 || hour >= 24 || minute >= 60 || second >= 60)
    {
        return false;
    }
    return true;
}

static void
popup_set_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    datetime_popup_fields *popup_fields = (datetime_popup_fields *)data;
    Evas_Object *entry = popup_fields->entry;
    const char *dateTimeValue = elm_entry_entry_get(entry);
    int len;
    len = strlen(dateTimeValue);
    if (NULL == dateTimeValue || 1 > len)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Read NULL DateTime Value");
        string logMessage = "DateTime should not be NULL<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    }
    else
    {
        int date, month, year, hour, minute, second;
        int beg = 0;
        date = parseString(dateTimeValue, len, &beg, 0);
        month = parseString(dateTimeValue, len, &beg, 1);
        year = parseString(dateTimeValue, len, &beg, 2);
        hour = parseString(dateTimeValue, len, &beg, 3);
        minute = parseString(dateTimeValue, len, &beg, 4);
        second = parseString(dateTimeValue, len, &beg, 5);

        dlog_print(DLOG_INFO, LOG_TAG, "#### %d", date);
        dlog_print(DLOG_INFO, LOG_TAG, "#### %d", month);
        dlog_print(DLOG_INFO, LOG_TAG, "#### %d", year);
        dlog_print(DLOG_INFO, LOG_TAG, "#### %d", hour);
        dlog_print(DLOG_INFO, LOG_TAG, "#### %d", minute);
        dlog_print(DLOG_INFO, LOG_TAG, "#### %d", second);
        bool valid = validate(date, month, year, hour, minute, second);
        if (valid)
        {
            createScheduledActionSet_AllBulbOff(date, month, year, hour, minute, second);
        }
        else
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### Incorrect date/time values");
            string logMessage = "Incorrect date/time value<br>";
            logMessage += "----------------------<br>";
            dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
            ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                                  &logMessage);
        }
    }
    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
list_scheduled_actionset_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *popup, *btn;
    Evas_Object *nf = naviframe;
    Evas_Object *entry;
    Evas_Object *layout;

    /* popup */
    popup = elm_popup_add(nf);
    elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_text_set(popup, "title,text", "Enter the date and time");

    layout = elm_layout_add(popup);
    elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_datetime_text");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_content_set(popup, layout);

    entry = elm_entry_add(layout);
    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
    elm_object_part_text_set(entry, "elm.guide", "dd/mm/yyyy hh:mm:ss");
    elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NUMBER);
    elm_object_part_content_set(layout, "elm.swallow.content", entry);

    datetime_popup_fields *popup_fields;
    popup_fields = (datetime_popup_fields *)malloc(sizeof(datetime_popup_fields));
    if (NULL == popup_fields)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Memory allocation failed");
    }
    else
    {
        popup_fields->popup = popup;
        popup_fields->entry = entry;
    }

    /* Cancel button */
    btn = elm_button_add(popup);
    elm_object_style_set(btn, "popup");
    elm_object_text_set(btn, "Cancel");
    elm_object_part_content_set(popup, "button1", btn);
    evas_object_smart_callback_add(btn, "clicked", popup_cancel_clicked_cb, popup_fields);

    /* Set button */
    btn = elm_button_add(popup);
    elm_object_style_set(btn, "popup");
    elm_object_text_set(btn, "Set");
    elm_object_part_content_set(popup, "button2", btn);
    evas_object_smart_callback_add(btn, "clicked", popup_set_clicked_cb, popup_fields);

    evas_object_show(popup);
}

void *showGroupAPIs(void *data)
{
    // Add items to the list only if the list is empty
    const Eina_List *eina_list = elm_list_items_get(list);
    int count = eina_list_count(eina_list);
    if (!count)
    {
        elm_list_item_append(list, "1. Create ActionSet<br>(ALLBULBON and ALLBULBOFF)", NULL, NULL,
                             createActionSet, NULL);

        elm_list_item_append(list, "2. Execute ActionSet (ALLBULBON)", NULL, NULL,
                             executeActionSetOn, NULL);

        elm_list_item_append(list, "3. Execute ActionSet (ALLBULBOFF)", NULL, NULL,
                             executeActionSetOff, NULL);

        elm_list_item_append(list, "4. Create ActionSet<br>(Recursive_ALLBULBON)", NULL, NULL,
                             createRecursiveActionSet_AllBulbOn, NULL);

        elm_list_item_append(list, "    4.1 Execute ActionSet", NULL, NULL,
                             recursive_allBulbOn, NULL);

        elm_list_item_append(list, "    4.2 Cancel ActionSet", NULL, NULL,
                             cancelRecursive_allBulbOn, NULL);

        elm_list_item_append(list, "5. Create ActionSet<br>(Scheduled_ALLBULBOFF)", NULL, NULL,
                             list_scheduled_actionset_cb, NULL);

        elm_list_item_append(list, "    5.1 Execute ActionSet", NULL, NULL,
                             scheduled_AllbulbOff, NULL);

        elm_list_item_append(list, "    5.2 Cancel ActionSet", NULL, NULL,
                             cancelScheduled_AllBulbOff, NULL);

        elm_list_item_append(list, "6. Get ActionSet (All BULBOFF)", NULL, NULL,
                             getActionSetOff, NULL);

        elm_list_item_append(list, "7. Delete ActionSet (All BULBOFF)", NULL, NULL,
                             deleteActionSetOff, NULL);

        elm_list_item_append(list, "8. Find BookMark to Observe", NULL, NULL,
                             observeBookMark, NULL);

        elm_list_go(list);
    }
    return NULL;
}

// Callback to be called when a resource is found in the network
void foundResource(shared_ptr< OCResource > resource)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### foundResource entry!!!!");

    if (resource)
    {
        string resourceURI = resource->uri();
        string hostAddress = resource->host();
        string logMessage;
        if (resourceURI == "/core/b/collection")
        {
            g_resource = resource;
            dlog_print(DLOG_INFO, LOG_TAG, "#### FOUND URI: %s", resourceURI.c_str());
            dlog_print(DLOG_INFO, LOG_TAG, "#### FOUND HOST: %s", hostAddress.c_str());
            logMessage = "FOUND RESOURCE URI <br>" + resourceURI + "<br>";
            logMessage += "FOUND RESOURCE HOST <br>" + hostAddress + "<br>";
            logMessage += "----------------------<br>";
            // Show the UI list of group APIs
            ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))showGroupAPIs, NULL);

        }
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                              &logMessage);
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### foundResource exit!!!!");
}

static void list_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it = (Elm_Object_Item *)event_info;
    elm_list_item_selected_set(it, EINA_FALSE);
}

// Method for Finding the Group
static void find_group()
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### findGroup ENTRY");
    std::string query = OC_RSRVD_WELL_KNOWN_URI;
    query.append("?rt=");
    query.append(resourceTypeName);

    OCPlatform::findResource("", query, CT_DEFAULT, &foundResource);

    dlog_print(DLOG_INFO, LOG_TAG, "#### findGroup EXIT");
}

// Method to be called when the Find Group UI Button is selected
static void
find_group_cb(void *data, Evas_Object *obj, void *event_info)
{
    if (NULL != list)
    {
        find_group();
    }
    else
    {
        dlog_print(DLOG_ERROR, "find_group_cb", "list is NULL - So unable to add items!!!");
    }
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
    onDestroy();

    if (NULL != log_entry)
    {
        evas_object_del(log_entry);
        log_entry = NULL;
    }
    if (NULL != list)
    {
        evas_object_del(list);
        list = NULL;
    }
    return EINA_TRUE;
}

// Method to be called when the Group APIs UI Button is selected
void group_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout;
    Evas_Object *scroller;
    Evas_Object *nf = (Evas_Object *)data;
    Evas_Object *find_button;
    Elm_Object_Item *nf_it;

    naviframe = nf;

    // Scroller
    scroller = elm_scroller_add(nf);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

    // Layout
    layout = elm_layout_add(nf);
    elm_layout_file_set(layout, ELM_DEMO_EDJ, "group_layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_content_set(scroller, layout);

    // Button
    find_button = elm_button_add(layout);
    elm_object_part_content_set(layout, "find_button", find_button);
    elm_object_text_set(find_button, "Find Group");
    evas_object_smart_callback_add(find_button, "clicked", find_group_cb, NULL);

    // List
    list = elm_list_add(layout);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);
    evas_object_smart_callback_add(list, "selected", list_selected_cb, NULL);
    elm_object_part_content_set(layout, "list", list);
    elm_list_go(list);

    // log_entry - textarea for log
    log_entry = elm_entry_add(layout);
    elm_entry_scrollable_set(log_entry, EINA_TRUE);
    elm_entry_editable_set(log_entry, EINA_FALSE);
    elm_object_part_text_set(log_entry, "elm.guide", "Logs will be updated here!!!");
    evas_object_size_hint_weight_set(log_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(log_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(layout, "log", log_entry);

    nf_it = elm_naviframe_item_push(nf, "Group APIs", NULL, NULL, scroller, NULL);
    elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

    create_group();
}

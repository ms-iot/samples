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

#include "reclient.h"

#include<iostream>

#include "reclientmain.h"

#include "RCSDiscoveryManager.h"
#include "RCSRemoteResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSAddress.h"

#include "OCPlatform.h"

# define checkResource nullptr == resource?false:true

using namespace std;
using namespace OC;
using namespace OIC::Service;

constexpr int CORRECT_INPUT = 1;
constexpr int INCORRECT_INPUT = 2;
constexpr int QUIT_INPUT = 3;

std::shared_ptr<RCSRemoteResourceObject>  resource;
std::vector<RCSRemoteResourceObject::Ptr> resourceList;
std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> discoveryTask;

const std::string defaultKey = "Temperature";
const std::string resourceType = "oic.r.temperaturesensor";

static Evas_Object *log_entry = NULL;
static Evas_Object *list = NULL;
static Evas_Object *naviframe = NULL;

typedef struct temperature_popup
{
    Evas_Object *popup;
    Evas_Object *entry;
} temperature_popup_fields;

// Function to update the log in UI
void *updateGroupLog(void *data)
{
    string *log = (string *)data;
    // Show the log
    elm_entry_entry_append(log_entry, (*log).c_str());
    elm_entry_cursor_end_set(log_entry);
    return NULL;
}

static void onDestroy()
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### Destroy sequence called");
    resourceList.clear();
    resource = nullptr;
}

void onResourceDiscovered(std::shared_ptr<RCSRemoteResourceObject> foundResource)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onResourceDiscovered callback");

    std::string resourceURI = foundResource->getUri();
    std::string hostAddress = foundResource->getAddress();

    int resourceSize = resourceList.size() + 1;
    string logMessage = "Resource Found : " + std::to_string(resourceSize) + "<br>";
    logMessage = logMessage + "URI: " + resourceURI + "<br>";
    logMessage = logMessage + "Host:" + hostAddress + "<br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);

    resourceList.push_back(foundResource);

    if ("/a/TempSensor" == resourceURI)
        resource = foundResource;
}

void onResourceStateChanged(const ResourceState &resourceState)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onResourceStateChanged");

    std::string logMessage = "State changed to : ";

    switch (resourceState)
    {
        case ResourceState::NONE:
            logMessage = logMessage + "NOT_MONITORING <br>";
            break;

        case ResourceState::ALIVE:
            logMessage = logMessage + "ALIVE <br>";
            break;

        case ResourceState::REQUESTED:
            logMessage = logMessage + "REQUESTED <br>";
            break;

        case ResourceState::LOST_SIGNAL:
            logMessage = logMessage + "LOST_SIGNAL <br>";
            resource = nullptr;
            break;

        case ResourceState::DESTROYED:
            logMessage = logMessage + "DESTROYED <br>";
            break;
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

void onCacheUpdated(const RCSResourceAttributes &attributes)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onCacheUpdated callback");

    string logMessage = "Cache Updated : <br> ";

    if (attributes.empty())
    {
        logMessage + logMessage + "Attribute is Empty <br>";
        return;
    }

    for (const auto & attr : attributes)
    {
        logMessage = logMessage + "KEY:" + attr.key().c_str() + "<br>";
        logMessage = logMessage + "VALUE:" + attr.value().toString().c_str() + "<br>";
    }
    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

void onRemoteAttributesReceived(const RCSResourceAttributes &attributes, int)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onRemoteAttributesReceived entry");

    string logMessage = "Remote Attribute Updated : <br> ";

    if (attributes.empty())
    {
        logMessage + logMessage + "Attribute is Empty <br>";
        return;
    }

    for (const auto & attr : attributes)
    {
        logMessage = logMessage + "KEY:" + attr.key().c_str() + "<br>";
        logMessage = logMessage + "VALUE:" + attr.value().toString().c_str() + "<br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void startMonitoring(void *data, Evas_Object *obj, void *event_info)
{
    string logMessage = "";

    if (checkResource)
    {
        if (!resource->isMonitoring())
        {
            try
            {
                logMessage = logMessage + "Started Monitoring <br>";
                resource->startMonitoring(&onResourceStateChanged);
            }
            catch (const RCSBadRequestException &e)
            {
                logMessage = logMessage + "Exception BadRequest<br>";
                dlog_print(DLOG_INFO, LOG_TAG, "#### Exception in isMonitoring : %s", e.what());
            }
            catch (const RCSInvalidParameterException &e)
            {
                logMessage = logMessage + "Exception Invalid Param<br>";
                dlog_print(DLOG_INFO, LOG_TAG, "#### Exception in isMonitoring : %s", e.what());
            }
        }
        else
        {
            logMessage = logMessage + "Already Monitoring <br>";
        }
    }
    else
    {
        logMessage = logMessage + "No Resource to monitor <br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void stopMonitoring(void *data, Evas_Object *obj, void *event_info)
{
    string logMessage = "";

    if (checkResource)
    {
        if (resource->isMonitoring())
        {
            resource->stopMonitoring();
            logMessage = logMessage + "Stopped Monitoring <br>";
        }
        else
        {
            logMessage = logMessage + "Monitoring not started <br>";
        }
    }
    else
    {
        logMessage = logMessage + "NO Resource to stop monitor <br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void list_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it = (Elm_Object_Item *)event_info;
    elm_list_item_selected_set(it, EINA_FALSE);
}

static void getAttributeFromRemoteServer(void *data, Evas_Object *obj, void *event_info)
{
    if (checkResource)
    {
        resource->getRemoteAttributes(&onRemoteAttributesReceived);
    }
    else
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### No Resource to getAttributeFromRemoteServer...");
    }
}

static void setAttributeToRemoteServer(int setTemperature)
{
    string key = "Temperature";
    string logMessage = "";

    RCSResourceAttributes setAttribute;
    setAttribute[key] = setTemperature;

    if (checkResource)
    {
        resource->setRemoteAttributes(setAttribute,
                                      &onRemoteAttributesReceived);
    }
    else
    {
        logMessage = "No Resource to setAttributeToRemoteServer";
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void startCaching(std::function <void (const RCSResourceAttributes &)>cb)
{
    string logMessage = "";

    if (checkResource)
    {
        if (!resource->isCaching())
        {
            if (cb)
            {
                try
                {
                    logMessage = logMessage + "Caching with callback <br>";
                    resource->startCaching(&onCacheUpdated);
                }
                catch (const RCSBadRequestException &e)
                {
                    logMessage = logMessage + "Exception BadRequest<br>";
                    dlog_print(DLOG_INFO, LOG_TAG, "#### Exception in startCaching : %s", e.what());
                }

            }
            else
            {
                try
                {
                    logMessage = logMessage + "Caching without callback <br>";
                    resource->startCaching();
                }
                catch (const RCSBadRequestException &e)
                {
                    logMessage = logMessage + "Exception BadRequest<br>";
                    dlog_print(DLOG_INFO, LOG_TAG, "#### Exception in startCaching : %s", e.what());
                }
            }
        }
        else
        {
            logMessage = logMessage + "Caching Already Started <br>";
        }
    }
    else
    {
        logMessage = logMessage + "No resource to start Caching <br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void startCachingWithoutCallback(void *data, Evas_Object *obj, void *event_info)
{
    startCaching(nullptr);
}

static void startCachingWithCallback(void *data, Evas_Object *obj, void *event_info)
{
    startCaching(onCacheUpdated);
}

static void getResourceCacheState(void *data, Evas_Object *obj, void *event_info)
{
    string logMessage = "CACHE STATE : ";
    switch (resource->getCacheState())
    {
        case CacheState::READY:
            logMessage = logMessage + "READY <br>";
            break;

        case CacheState::UNREADY:
            logMessage = logMessage + "UNREADY <br>";
            break;

        case CacheState::LOST_SIGNAL:
            logMessage = logMessage + "LOST_SIGNAL <br>";
            break;

        case CacheState::NONE:
            logMessage = logMessage + "NONE <br>";
            break;

        default:
            break;
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void getCachedAttributes(void *data, Evas_Object *obj, void *event_info)
{
    string logMessage = "";

    if (checkResource)
    {
        try
        {
            if (resource->getCachedAttributes().empty())
            {
                logMessage = "Cached attribute empty<br>";
            }
            else
            {
                for (const auto & attr : resource->getCachedAttributes())
                {
                    logMessage = logMessage + "KEY:" + attr.key().c_str() + "<br>";
                    logMessage = logMessage + "VALUE:" + attr.value().toString().c_str() + "<br>";
                    dlog_print(DLOG_INFO, LOG_TAG, "#### Cached attributes received ");
                }
            }
        }
        catch (const RCSBadRequestException &e)
        {
            logMessage = "Exception Received<br>";
            dlog_print(DLOG_INFO, LOG_TAG, "#### Exception in getCachedAttributes : %s", e.what());
        }
    }
    else
    {
        logMessage = logMessage + "No Resource<br>";
        dlog_print(DLOG_INFO, LOG_TAG, "#### No Resource to getCachedAttributes...");
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void getCachedAttribute(void *data, Evas_Object *obj, void *event_info)
{
    string logMessage = "";

    if (checkResource)
    {
        try
        {
            logMessage = logMessage + "KEY:" + defaultKey.c_str() + "<br>";
            int attrValue = resource->getCachedAttribute(defaultKey).get< int >();
            logMessage = logMessage + "VALUE:" + to_string(attrValue) + "<br>";
        }
        catch (const RCSBadRequestException &e)
        {
            logMessage = logMessage + "Exception BadRequest<br>";
            dlog_print(DLOG_INFO, LOG_TAG, "#### Exception in getCachedAttribute : %s", e.what());
        }
        catch (const RCSBadGetException &e)
        {
            logMessage = logMessage + "Exception BadGet<br>";
            dlog_print(DLOG_INFO, LOG_TAG, "#### Exception in getCachedAttribute : %s", e.what());
        }
    }
    else
    {
        logMessage = logMessage + "No resource<br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void stopCaching(void *data, Evas_Object *obj, void *event_info)
{
    string logMessage = "";

    if (checkResource)
    {
        if (resource->isCaching())
        {
            resource->stopCaching();
            logMessage = logMessage + "Caching stopped <br>";
        }
        else
        {
            logMessage = logMessage + "Caching not started <br>";
        }
    }
    else
    {
        logMessage = logMessage + "No resource found<br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

void discoverResource()
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### discovery started");

    while (!discoveryTask)
    {
        try
        {
            discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
                                RCSAddress::multicast(), resourceType, &onResourceDiscovered);
        }
        catch (const RCSPlatformException &e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### Discovery over");
}

void cancelDiscoverResource()
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### cancelDiscoverResource entry");
    string logMessage = "";

    if (!discoveryTask)
    {
        logMessage += "There is no discovery request <br>";
    }
    else
    {
        discoveryTask->cancel();

        logMessage += "Discovery canceled <br>";

        int resourceSize = resourceList.size();
        if (!resourceSize)
        {
            logMessage += "No Resource Discovered <br>";
        }
        else
        {
            logMessage += std::to_string(resourceSize) + " : Resource Discovered <br>";
            ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))showClientAPIs, NULL);
        }

    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

static void
popup_cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    temperature_popup_fields *popup_fields = (temperature_popup_fields *)data;
    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
popup_set_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    temperature_popup_fields *popup_fields = (temperature_popup_fields *)data;
    Evas_Object *entry = popup_fields->entry;
    const char *temperatureString = elm_entry_entry_get(entry);
    // Remove white spaces(if any) at the beginning
    int beginning = 0;
    while (temperatureString[beginning] == ' ')
    {
        (beginning)++;
    }

    int len = strlen(temperatureString);
    if (NULL == temperatureString || 1 > len)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Read NULL Temperature Value");
        string logMessage = "Temperature Cannot be NULL<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    }
    else
    {
        int temperate = atoi(temperatureString);
        string tempString(temperatureString);
        setAttributeToRemoteServer(temperate);
        dlog_print(DLOG_INFO, LOG_TAG, "#### Temperature to set : %d", temperate);

        string logMessage = "Temperature to set : " + tempString + "<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                              &logMessage);
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
    elm_object_part_text_set(popup, "title,text", "Enter the temperature");

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
    elm_object_part_text_set(entry, "elm.guide", "in degree celsius");
    elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NUMBER);
    elm_object_part_content_set(layout, "elm.swallow.content", entry);

    temperature_popup_fields *popup_fields;
    popup_fields = (temperature_popup_fields *)malloc(sizeof(temperature_popup_fields));
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

// Method to be called when the Start Discovery UI Button is selected
static void
find_resource_cb(void *data, Evas_Object *obj, void *event_info)
{
    if (NULL != list)
    {
        discoverResource();
    }
    else
    {
        dlog_print(DLOG_ERROR, "find_resource_cb", "list is NULL - So unable to add items!!!");
    }
}

// Method to be called when the Cancel Discovery UI Button is selected
static void
cancel_resource_cb(void *data, Evas_Object *obj, void *event_info)
{
    if (NULL != list)
    {
        cancelDiscoverResource();
    }
    else
    {
        dlog_print(DLOG_ERROR, "cancel_resource_cb", "list is NULL - So unable to add items!!!");
    }
}

void *showClientAPIs(void *data)
{
    // Add items to the list only if the list is empty
    const Eina_List *eina_list = elm_list_items_get(list);
    int count = eina_list_count(eina_list);
    if (!count)
    {
        elm_list_item_append(list, "1. Start Monitoring", NULL, NULL,
                             startMonitoring, NULL);

        elm_list_item_append(list, "2. Stop Monitoring", NULL, NULL,
                             stopMonitoring, NULL);

        elm_list_item_append(list, "3. Get Attribute", NULL, NULL,
                             getAttributeFromRemoteServer, NULL);

        elm_list_item_append(list, "4. Set Attribute", NULL, NULL,
                             list_scheduled_actionset_cb, NULL);

        elm_list_item_append(list, "5. Start Caching - No update", NULL, NULL,
                             startCachingWithoutCallback, NULL);

        elm_list_item_append(list, "6. Start Caching - With update", NULL, NULL,
                             startCachingWithCallback, NULL);

        elm_list_item_append(list, "7. Get Cache State", NULL, NULL,
                             getResourceCacheState, NULL);

        elm_list_item_append(list, "8. Get cached attributes", NULL, NULL,
                             getCachedAttributes, NULL);

        elm_list_item_append(list, "9. Stop Caching", NULL, NULL,
                             stopCaching, NULL);

        elm_list_go(list);
    }
    return NULL;
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
void client_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout;
    Evas_Object *scroller;
    Evas_Object *nf = (Evas_Object *)data;
    Evas_Object *button1;
    Evas_Object *button2;
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

    // Start Discovery Button
    button1 = elm_button_add(layout);
    elm_object_part_content_set(layout, "button1", button1);
    elm_object_text_set(button1, "Start Discovery");
    evas_object_smart_callback_add(button1, "clicked", find_resource_cb, NULL);

    // Cancel Discovery Button
    button2 = elm_button_add(layout);
    elm_object_part_content_set(layout, "button2", button2);
    elm_object_text_set(button2, "Cancel Discovery");
    evas_object_smart_callback_add(button2, "clicked", cancel_resource_cb, NULL);

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

    nf_it = elm_naviframe_item_push(nf, "Resource Encapsulation", NULL, NULL, scroller, NULL);
    elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

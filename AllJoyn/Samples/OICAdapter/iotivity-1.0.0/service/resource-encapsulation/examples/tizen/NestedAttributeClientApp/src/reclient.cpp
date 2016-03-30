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

# define nestedAtrribute std::vector<std::vector<RCSResourceAttributes>>
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

const std::string defaultKey = "deviceInfo";
const std::string resourceType = "oic.r.ac";

RCSResourceAttributes model;
RCSResourceAttributes speed;
RCSResourceAttributes airCirculation;
RCSResourceAttributes temperature;
RCSResourceAttributes humidity;
RCSResourceAttributes power;
RCSResourceAttributes capacity;
RCSResourceAttributes weight;
RCSResourceAttributes dimensions;
RCSResourceAttributes red;
RCSResourceAttributes green;

std::vector<RCSResourceAttributes> generalInfo;
std::vector<RCSResourceAttributes> fan;
std::vector<RCSResourceAttributes> tempSensor;
std::vector<RCSResourceAttributes> efficiency;
std::vector<RCSResourceAttributes> light;

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

nestedAtrribute createNestedAttribute(int fanSpeed, int airSpeed)
{
    nestedAtrribute *acServer = new nestedAtrribute();

    model["model"] = "SamsungAC";

    speed["speed"] = fanSpeed;
    airCirculation["air"] = airSpeed;

    temperature["temp"] = 30;
    humidity["humidity"] = 30;

    power["power"] = 1600;
    capacity["capacity"] = 1;

    weight["weight"] = 3;
    dimensions["dimensions"] = "10x25x35";

    red["red"] = 50;
    green["green"] = 60;

    generalInfo.clear();
    generalInfo.push_back(model);
    generalInfo.push_back(weight);
    generalInfo.push_back(dimensions);

    fan.clear();
    fan.push_back(speed);
    fan.push_back(airCirculation);

    tempSensor.clear();
    tempSensor.push_back(temperature);
    tempSensor.push_back(humidity);

    efficiency.clear();
    efficiency.push_back(power);
    efficiency.push_back(capacity);

    light.clear();
    light.push_back(red);
    light.push_back(green);

    if (nullptr == acServer)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "#### NULL nestedAtrribute");
        return *acServer;
    }

    acServer->push_back(generalInfo);
    acServer->push_back(fan);
    acServer->push_back(tempSensor);
    acServer->push_back(efficiency);
    acServer->push_back(light);

    return *acServer;
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

    if ("/a/airConditioner" == resourceURI)
        resource = foundResource;

    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))showClientAPIs, NULL);
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
        logMessage = logMessage + "==========================<br>";
        OIC::Service::RCSResourceAttributes::Value attrValue =  attr.value();
        std::vector< std::vector<RCSResourceAttributes >> attrVector =
                    attrValue.get<std::vector< std::vector<RCSResourceAttributes >>>();

        for (auto itr = attrVector.begin(); itr != attrVector.end(); ++itr)
        {
            std::vector<RCSResourceAttributes > attrKeyVector = *itr;
            for (auto itrKey = attrKeyVector.begin(); itrKey != attrKeyVector.end(); ++itrKey)
            {
                for (const auto & attribute : *itrKey)
                {
                    logMessage = logMessage + attribute.key().c_str() + " : "
                                 + attribute.value().toString().c_str() + "<br>";
                }
            }
            std::cout << std::endl;
            logMessage = logMessage + "<br>";
        }
        logMessage = logMessage + "==========================<br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
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

static void setAttributeToRemoteServer(int fanSpeed, int airSpeed)
{
    string logMessage = "";

    nestedAtrribute attr = createNestedAttribute(fanSpeed, airSpeed);
    RCSResourceAttributes setAttribute;
    setAttribute[defaultKey] = attr;

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
popup_setFanSpeed_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    temperature_popup_fields *popup_fields = (temperature_popup_fields *)data;
    Evas_Object *entry = popup_fields->entry;
    const char *fanSpeedString = elm_entry_entry_get(entry);
    // Remove white spaces(if any) at the beginning
    int beginning = 0;
    while (fanSpeedString[beginning] == ' ')
    {
        (beginning)++;
    }

    int len = strlen(fanSpeedString);
    if (NULL == fanSpeedString || 1 > len)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Read NULL Fan Speed Value");
        string logMessage = "Fan Speed Cannot be NULL<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    }
    else
    {
        int fanSpeed = atoi(fanSpeedString);
        string fanString(fanSpeedString);
        setAttributeToRemoteServer(fanSpeed, DEFAULT_AIRSPEED);
        dlog_print(DLOG_INFO, LOG_TAG, "#### Fan Speed to set : %d", fanSpeed);

        string logMessage = "Fan Speed set to set : " + fanString + "<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                              &logMessage);
    }
    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
popup_setAirSpeed_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    temperature_popup_fields *popup_fields = (temperature_popup_fields *)data;
    Evas_Object *entry = popup_fields->entry;
    const char *airSpeedString = elm_entry_entry_get(entry);
    // Remove white spaces(if any) at the beginning
    int beginning = 0;
    while (airSpeedString[beginning] == ' ')
    {
        (beginning)++;
    }

    int len = strlen(airSpeedString);
    if (NULL == airSpeedString || 1 > len)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Read NULL Fan Speed Value");
        string logMessage = "Fan Speed Cannot be NULL<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog, &logMessage);
    }
    else
    {
        int airSpeed = atoi(airSpeedString);
        string airString(airSpeedString);
        setAttributeToRemoteServer(DEFAULT_FANSPEED, airSpeed);
        dlog_print(DLOG_INFO, LOG_TAG, "#### Fan Speed to set : %d", airSpeed);

        string logMessage = "Fan Speed set to set : " + airString + "<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                              &logMessage);
    }
    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
list_setFanSpeed_cb(void *data, Evas_Object *obj, void *event_info)
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
    elm_object_part_text_set(popup, "title,text", "Enter the Fan Speed");

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
    elm_object_part_text_set(entry, "elm.guide", "Range : 0-100");
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
    evas_object_smart_callback_add(btn, "clicked", popup_setFanSpeed_clicked_cb, popup_fields);

    evas_object_show(popup);
}

static void
list_setAirSpeed_cb(void *data, Evas_Object *obj, void *event_info)
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
    elm_object_part_text_set(popup, "title,text", "Enter the Fan Speed");

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
    elm_object_part_text_set(entry, "elm.guide", "Range : 0-500");
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
    evas_object_smart_callback_add(btn, "clicked", popup_setAirSpeed_clicked_cb, popup_fields);

    evas_object_show(popup);
}

// Method to be called when the Discover Resource UI Button is selected
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
        elm_list_item_append(list, "3. Get Remote Attribute", NULL, NULL,
                             getAttributeFromRemoteServer, NULL);

        elm_list_item_append(list, "4. Set Fan Speed", NULL, NULL,
                             list_setFanSpeed_cb, NULL);

        elm_list_item_append(list, "4. Set Air Circulation Speed", NULL, NULL,
                             list_setAirSpeed_cb, NULL);

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
void group_cb(void *data, Evas_Object *obj, void *event_info)
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

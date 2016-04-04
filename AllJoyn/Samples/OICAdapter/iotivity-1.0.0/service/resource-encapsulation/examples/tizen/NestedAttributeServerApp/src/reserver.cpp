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

#include "reserver.h"

#include "reservermain.h"
#include "PrimitiveResource.h"
#include "RCSResourceObject.h"
#include "RCSResourceAttributes.h"

#include <string>

#define DEFAULT_FANSPEED 30
#define DEFAULT_AIRSPEED 400

using namespace std;
using namespace OC;
using namespace OIC::Service;

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

RCSResourceObject::Ptr server;
static bool serverCallback = false;

# define nestedAtrribute std::vector<std::vector<RCSResourceAttributes>>
# define checkServer NULL!=server?true:false

static Evas_Object *log_entry = NULL;
static Evas_Object *list = NULL;
static Evas_Object *naviframe = NULL;

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

static void printNestedAttribute(void *data, Evas_Object *obj, void *event_info)
{
    RCSResourceObject::LockGuard lock(*server);
    RCSResourceAttributes attributes = server->getAttributes();

    std::cout << "\nPrinting nested attributes" << std::endl;
    printAttribute(attributes);
    return;
}

void printAttribute(const RCSResourceAttributes &attrs)
{
    string logMessage = "";
    for (const auto & attr : attrs)
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
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
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

    acServer->push_back(generalInfo);
    acServer->push_back(fan);
    acServer->push_back(tempSensor);
    acServer->push_back(efficiency);
    acServer->push_back(light);

    return *acServer;
}

static void onDestroy()
{
    server = NULL;
    string logMessage = "SERVER DESTROYED";

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

//hander for get request (if developer choose second option for resource Creation)
RCSGetResponse requestHandlerForGet(const RCSRequest &request,
                                    RCSResourceAttributes &attrs)
{
    string logMessage = "GET REQUEST RECEIVED<br>";

    RCSResourceObject::LockGuard lock(*server);
    RCSResourceAttributes attributes = server->getAttributes();
    printAttribute(attributes);
    logMessage += "RESPONSE SENT<br>";

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
    return RCSGetResponse::defaultAction();
}

//hander for set request (if developer choose second option for resource Creation)
RCSSetResponse requestHandlerForSet(const RCSRequest &request,
                                    RCSResourceAttributes &attrs)
{
    string logMessage = "SET REQUEST RECEIVED<br>";

    RCSResourceObject::LockGuard lock(*server);
    for (const auto & attr : attrs)
    {
        logMessage = logMessage + "KEY:" + attr.key().c_str() + "<br>";
        logMessage = logMessage + "VALUE:" + attr.value().toString().c_str() + "<br>";
        server->setAttribute(attr.key(), attr.value());
    }

    printAttribute(attrs);
    logMessage += "RESPONSE SENT<br>";
    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);

    return RCSSetResponse::defaultAction();
}

static void list_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it = (Elm_Object_Item *)event_info;
    elm_list_item_selected_set(it, EINA_FALSE);
}

static void initServer()
{
    OCPlatform::startPresence(3);

    try
    {
        server = RCSResourceObject::Builder(resourceUri, resourceType,
                                            resourceInterface).setDiscoverable(true).setObservable(true).build();
    }
    catch (const RCSPlatformException &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "#### Create resource exception! (%s)", e.what());
    }

    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);
    server->setSetRequestHandlerPolicy(RCSResourceObject::SetRequestHandlerPolicy::NEVER);

    nestedAtrribute attr = createNestedAttribute(DEFAULT_FANSPEED, DEFAULT_AIRSPEED);
    server->setAttribute(attributeKey, attr);

    string logMessage = "SERVER CREATED<br>";
    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);

    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))showGroupAPIs, NULL);
}

static void
popup_cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    datetime_popup_fields *popup_fields = (datetime_popup_fields *)data;
    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
popup_setFanSpeed_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    datetime_popup_fields *popup_fields = (datetime_popup_fields *)data;
    Evas_Object *entry = popup_fields->entry;
    const char *fanSpeedString = elm_entry_entry_get(entry);
    // Remove white spaces(if any) at the beginning
    string logMessage = "";
    int beginning = 0;
    while (fanSpeedString[beginning] == ' ')
    {
        (beginning)++;
    }

    int len = strlen(fanSpeedString);
    if (NULL == fanSpeedString || 1 > len)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Read NULL Fan Speed Value");
        logMessage = "Fan Speed Cannot be NULL<br>";
    }
    else
    {
        if (checkServer)
        {
            int fanSpeed = atoi(fanSpeedString);
            nestedAtrribute attr = createNestedAttribute(fanSpeed, DEFAULT_AIRSPEED);

            RCSResourceObject::LockGuard lock(server);
            server->getAttributes()[attributeKey] = attr;
            logMessage = "Fan Speed Changed <br>";
        }
        else
        {
            logMessage += "NO SERVER<br>";
        }
    }
    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    logMessage += "----------------------<br>";
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);

    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
popup_setAirSpeed_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    datetime_popup_fields *popup_fields = (datetime_popup_fields *)data;
    Evas_Object *entry = popup_fields->entry;
    const char *airSpeedString = elm_entry_entry_get(entry);
    // Remove white spaces(if any) at the beginning
    string logMessage = "";
    int beginning = 0;
    while (airSpeedString[beginning] == ' ')
    {
        (beginning)++;
    }

    int len = strlen(airSpeedString);
    if (NULL == airSpeedString || 1 > len)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Read NULL Air Circulation Speed Value");
        logMessage = "Air Circulation Speed Cannot be NULL<br>";
    }
    else
    {
        if (checkServer)
        {
            int airSpeed = atoi(airSpeedString);
            nestedAtrribute attr = createNestedAttribute(DEFAULT_FANSPEED, airSpeed);

            RCSResourceObject::LockGuard lock(server);
            server->getAttributes()[attributeKey] = attr;
            logMessage = "Air Circulation Speed Changed <br>";
        }
        else
        {
            logMessage += "NO SERVER<br>";
        }
    }
    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    logMessage += "----------------------<br>";
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);

    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
list_get_fanSpeed_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *popup, *btn;
    Evas_Object *nf = naviframe;
    Evas_Object *entry;
    Evas_Object *layout;

    /* pop up */
    popup = elm_popup_add(nf);
    elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_text_set(popup, "title,text", "Enter the fan speed");

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
    evas_object_smart_callback_add(btn, "clicked", popup_setFanSpeed_clicked_cb, popup_fields);

    evas_object_show(popup);
}

static void
list_get_airSpeed_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *popup, *btn;
    Evas_Object *nf = naviframe;
    Evas_Object *entry;
    Evas_Object *layout;

    /* pop up */
    popup = elm_popup_add(nf);
    elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_text_set(popup, "title,text", "Enter the Air Circulation speed");

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
    evas_object_smart_callback_add(btn, "clicked", popup_setAirSpeed_clicked_cb, popup_fields);

    evas_object_show(popup);
}

void *showGroupAPIs(void *data)
{
    // Add items to the list only if the list is empty
    const Eina_List *eina_list = elm_list_items_get(list);
    int count = eina_list_count(eina_list);
    if (!count)
    {
        elm_list_item_append(list, "1. Print Nested attributes", NULL, NULL,
                             printNestedAttribute, NULL);

        elm_list_item_append(list, "2. Set Fan Speed", NULL, NULL,
                             list_get_fanSpeed_cb, NULL);

        elm_list_item_append(list, "3. Set Air Circulation speed", NULL, NULL,
                             list_get_airSpeed_cb, NULL);

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

// Method to set up server screens
void serverCreateUI(void *data)
{
    Evas_Object *layout;
    Evas_Object *scroller;
    Evas_Object *nf = (Evas_Object *)data;
    Elm_Object_Item *nf_it;
    naviframe = nf;

    // Scroller
    scroller = elm_scroller_add(nf);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

    // Layout
    layout = elm_layout_add(nf);
    elm_layout_file_set(layout, ELM_DEMO_EDJ, "server_layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_content_set(scroller, layout);

    // List
    list = elm_list_add(layout);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);
    evas_object_smart_callback_add(list, "selected", list_selected_cb, NULL);
    elm_object_part_content_set(layout, "list", list);
    elm_list_go(list);

    // log_entry - text area for log
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

void start_server(void *data, Evas_Object *obj, void *event_info)
{
    server = NULL;
    string logMessage = "SERVER WITHOUT CALLBACK<br>";

    serverCallback = false;
    serverCreateUI(data);
    initServer();

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);
}

void start_server_cb(void *data, Evas_Object *obj, void *event_info)
{
    server = NULL;
    string logMessage = "SERVER WITH CALLBACK<br>";

    serverCallback = true;
    serverCreateUI(data);
    initServer();

    if (checkServer)
    {
        server->setGetRequestHandler(requestHandlerForGet);
        server->setSetRequestHandler(requestHandlerForSet);
        logMessage = "HANDLERS SET<br>";
    }
    else
    {
        logMessage = "NO SERRVER FOUND<br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &logMessage);

}

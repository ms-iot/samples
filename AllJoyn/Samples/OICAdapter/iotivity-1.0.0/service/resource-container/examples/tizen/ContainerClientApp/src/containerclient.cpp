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

#include "containerclient.h"

#include "clientmain.h"
#include "RCSDiscoveryManager.h"
#include "RCSResourceAttributes.h"
#include "RCSAddress.h"

#include <string>

using namespace std;
using namespace OC;
using namespace OIC::Service;

std::shared_ptr<RCSRemoteResourceObject>  g_containerResource;
std::vector<RCSRemoteResourceObject::Ptr> resourceList;
std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> discoveryTask;

const std::string resourceTypeLight = "oic.r.light";
const std::string resourceTypeSoftsensor = "oic.r.sensor";

static Evas_Object *log_entry = NULL;
static Evas_Object *listnew = NULL;
static Evas_Object *naviframe = NULL;

// Function to update the log in UI
void *updateContainerLog(void *data)
{
    string *log = (string *)data;
    // Show the log
    elm_entry_entry_append(log_entry, (*log).c_str());
    elm_entry_cursor_end_set(log_entry);
    return NULL;
}

static void list_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it = (Elm_Object_Item *)event_info;
    elm_list_item_selected_set(it, EINA_FALSE);
}

static void onDestroy()
{
	dlog_print(DLOG_INFO, LOG_TAG, "#### Destroy sequence called");
	resourceList.clear();
    g_containerResource = nullptr;
}

void onContainerDiscovered(std::shared_ptr<RCSRemoteResourceObject> foundResource)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onResourceDiscovered callback");

    std::string resourceURI = foundResource->getUri();
    std::string hostAddress = foundResource->getAddress();

    int resourceSize = resourceList.size() + 1;
    string logMessage = "Resource Found <br>";
    logMessage = logMessage + "URI: " + resourceURI + "<br>";
    logMessage = logMessage + "Host:" + hostAddress + "<br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateContainerLog,
                                          &logMessage);

    resourceList.push_back(foundResource);

    g_containerResource = foundResource;

    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))showContainerAPIs, NULL);
}

void *showContainerAPIs(void *data)
{
    // Add items to the list only if the list is empty
    const Eina_List *eina_list = elm_list_items_get(listnew);
    int count = eina_list_count(eina_list);
    if (!count)
    {
        elm_list_item_append(listnew, "1. Start Light resource Discovery", NULL, NULL,
                             findLight, NULL);

        elm_list_item_append(listnew, "2. Start Softsensor resource Discovery", NULL, NULL,
                             findSoftsensor, NULL);

        elm_list_item_append(listnew, "3.Stop Discovery", NULL, NULL,
        					cancelDiscoverResource, NULL);

        elm_list_go(listnew);
    }
    return NULL;
}

static void findLight(void *data, Evas_Object *obj, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG, "#### Light discovery started");

	while (!discoveryTask)
	{
		try
		{
			discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
								RCSAddress::multicast(), resourceTypeLight, &onContainerDiscovered);
		}
		catch (const RCSPlatformException &e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	dlog_print(DLOG_INFO, LOG_TAG, "#### Light Discovery over");
}

static void findSoftsensor(void *data, Evas_Object *obj, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG, "#### SoftSensor discovery started");

	while (!discoveryTask)
	{
		try
		{
			discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
								RCSAddress::multicast(), resourceTypeSoftsensor, &onContainerDiscovered);
		}
		catch (const RCSPlatformException &e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	dlog_print(DLOG_INFO, LOG_TAG, "#### SoftSensor Discovery over");
}

static void cancelDiscoverResource(void *data, Evas_Object *obj, void *event_info)
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
        }

    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateContainerLog,
                                          &logMessage);
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
    if (NULL != listnew)
    {
        evas_object_del(listnew);
        listnew = NULL;
    }
    return EINA_TRUE;
}

// Method to set up server screens
void containerCreateUI(void *data, Evas_Object *obj, void *event_info)
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
    elm_layout_file_set(layout, ELM_DEMO_EDJ, "container_layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_content_set(scroller, layout);

    // List
    listnew = elm_list_add(layout);
    elm_list_mode_set(listnew, ELM_LIST_COMPRESS);
    evas_object_smart_callback_add(listnew, "selected", list_selected_cb, NULL);
    elm_object_part_content_set(layout, "listnew", listnew);
    elm_list_go(listnew);

    // log_entry - text area for log
    log_entry = elm_entry_add(layout);
    elm_entry_scrollable_set(log_entry, EINA_TRUE);
    elm_entry_editable_set(log_entry, EINA_FALSE);
    elm_object_part_text_set(log_entry, "elm.guide", "Logs will be updated here!!!");
    evas_object_size_hint_weight_set(log_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(log_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(layout, "log", log_entry);

    nf_it = elm_naviframe_item_push(nf, "Resource Container", NULL, NULL, scroller, NULL);
    elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

    // Show the UI list of group APIs
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))showContainerAPIs, NULL);
}

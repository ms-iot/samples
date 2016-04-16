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

#include "conserverapp.h"

#include <tizen.h>
#include <pthread.h>

#include "ThingsConfiguration.h"
#include "ThingsMaintenance.h"
#include "configurationresource.h"
#include "maintenanceresource.h"
#include "factorysetresource.h"

using namespace OC;
using namespace OIC;

namespace PH = std::placeholders;

/* Default system configuration value's variables
   The variable's names should be same as the names of "extern" variables defined in
   "configurationresource.h" */
std::string defaultDeviceName;
std::string defaultLocation;
std::string defaultLocationName;
std::string defaultRegion;
std::string defaultCurrency;

static ThingsConfiguration *g_thingsConf;

const int SUCCESS_RESPONSE = 0;

bool resources_created = false;

// Forward declaring the entityHandler (Configuration)
bool prepareResponseForResource(std::shared_ptr< OCResourceRequest > request);
OCStackResult sendResponseForResource(std::shared_ptr< OCResourceRequest > pRequest);
OCEntityHandlerResult entityHandlerForResource(std::shared_ptr< OCResourceRequest > request);

ConfigurationResource *myConfigurationResource;
MaintenanceResource *myMaintenanceResource;
FactorySetResource *myFactorySetResource;

typedef std::function< void(OCRepresentation &) > putFunc;
typedef std::function< OCRepresentation(void) > getFunc;

typedef struct appdata
{
    Evas_Object *win;
    Evas_Object *conform;
    Evas_Object *naviframe;
    Evas_Object *scroller;
    Evas_Object *layout, *base_layout;
    Evas_Object *bootButton;
    Evas_Object *createConfButton;
} appdata_s;

Evas_Object *log_entry;

std::string logMessage;

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

// This function prepares a response for the incoming request
bool prepareResponseForResource(std::shared_ptr< OCResourceRequest > request)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### In Server CPP prepareResponseForResource");
    bool result = false;
    if (request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if (requestFlag == RequestHandlerFlag::RequestFlag)
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### requestFlag : Request");

            // If the request type is GET
            if (requestType == "GET")
            {
                dlog_print(DLOG_INFO, LOG_TAG, "#### requestType : GET");

                // GET operations are directly handled while sending the response
                result = true;
            }
            else if (requestType == "PUT")
            {
                dlog_print(DLOG_INFO, LOG_TAG, "#### requestType : PUT");
                putFunc putFunction;
                OCRepresentation rep = request->getResourceRepresentation();

                // Get appropriate function to be called for the PUT request
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
            dlog_print(DLOG_INFO, LOG_TAG, "#### requestFlag : Observer");
        }
    }
    else
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Request invalid");
    }

    return result;
}

// This function sends a response for the incoming request
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

// This function handles the requests and sends the response
OCEntityHandlerResult entityHandlerForResource(std::shared_ptr< OCResourceRequest > request)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### In Server CPP (entityHandlerForResource) entity"
               "handler:");
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    if (prepareResponseForResource(request))
    {
        if (OC_STACK_OK == sendResponseForResource(request))
        {
            ehResult = OC_EH_OK;
            dlog_print(DLOG_INFO, LOG_TAG, "#### sendResponse success.");
        }
        else
        {
            dlog_print(DLOG_INFO, LOG_TAG, "#### sendResponse failed.");
        }
    }
    else
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### PrepareResponse failed.");
    }
    return ehResult;
}

// Updates the log in the UI
void *updateLog(void *data)
{
    std::string *log = (std::string *)data;

    if (nullptr == log)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### No log !!!!");
    }
    else
    {
        //Show the log
        elm_entry_entry_append(log_entry, log->c_str());
        elm_entry_cursor_end_set(log_entry);
        dlog_print(DLOG_INFO, LOG_TAG, "%s", log->c_str());
    }
    dlog_print(DLOG_INFO, LOG_TAG, "#### updateLog exit!!!!");
    return NULL;
}

static void
win_delete_request_cb(void *data , Evas_Object *obj , void *event_info)
{
    ui_app_exit();
}

// Function to delete all the configuration resources which are created
void deleteResources()
{
    if (NULL != myConfigurationResource)
        myConfigurationResource->deleteResource();
    if (NULL != myMaintenanceResource)
        myMaintenanceResource->deleteResource();
    if (NULL != myFactorySetResource)
        myFactorySetResource->deleteResource();

    delete g_thingsConf;
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{

    deleteResources();

    ui_app_exit();
}

/* Callback Function to be called by the platform
   when response arrives from the BootStrap Server */
void onBootStrapCallback(const HeaderOptions &headerOptions, const OCRepresentation &rep,
                         const int eCode)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onBootStrap entry");

    if (SUCCESS_RESPONSE != eCode)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### onBootStrap -- onGET Response error: %d", eCode);
        return ;
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### onBootStrap -- GET request was successful");
    dlog_print(DLOG_INFO, LOG_TAG, "#### onBootStrap -- Resource URI: %s", rep.getUri().c_str());

    logMessage = "----------------------------<br>";
    logMessage += "GET request was successful<br>";
    logMessage += "URI : " + rep.getUri() + "<br>";

    defaultRegion = rep.getValue< std::string >(DEFAULT_REGION);
    defaultCurrency = rep.getValue< std::string >(DEFAULT_CURRENCY);
    defaultLocation = rep.getValue< std::string >(DEFAULT_LOCATION);
    defaultLocationName = rep.getValue< std::string >(DEFAULT_LOCATIONNAME);
    defaultDeviceName = rep.getValue< std::string >(DEFAULT_DEVICENAME);

    logMessage += "Device Name : " + defaultDeviceName + "<br>";
    logMessage += "Location : " + defaultLocation + "<br>";
    logMessage += "Location Name : " + defaultLocationName + "<br>";
    logMessage += "currency : " + defaultCurrency + "<br>";
    logMessage += "Region : " + defaultRegion + "<br>";

    dlog_print(DLOG_INFO, LOG_TAG, "  %s", logMessage.c_str());

    //Call updateLog in the thread safe mode
    ecore_main_loop_thread_safe_call_sync(updateLog, &logMessage);

}

// Function to be called when the doBootStrap UI button is clicked
static void
doBootStrap_cb(void *data , Evas_Object *obj , void *event_info)
{

    if (NULL  == g_thingsConf)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "#### doBootstrap returned g_thingsConf NULL check");
        return;
    }

    OCStackResult result = g_thingsConf->doBootstrap(&onBootStrapCallback);

    if (OC_STACK_OK == result)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### doBootstrap returned OC_STACK_OK");
    }
    else
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### doBootstrap failed");
    }
}

// Function to be called when Create Configuration Resources UI button is clicked
static void
createConfResource_cb(void *data , Evas_Object *obj , void *event_info)
{
    logMessage = "----------------------------<br>";
    if (!resources_created)
    {
        resources_created = true;
        myConfigurationResource = new ConfigurationResource();
        myConfigurationResource->createResource(&entityHandlerForResource);

        myMaintenanceResource = new MaintenanceResource();
        myMaintenanceResource->createResource(&entityHandlerForResource);

        myFactorySetResource = new FactorySetResource();
        myFactorySetResource->createResource(&entityHandlerForResource);

        myMaintenanceResource->factoryReset = std::function < void()
                                              > (std::bind(&ConfigurationResource::factoryReset,
                                                      myConfigurationResource));

        logMessage += "Resources Created Successfully!!! Server is Ready!!!<br>";
    }
    else
    {
        logMessage += "Resources were created already!!! <br>";
    }

    dlog_print(DLOG_INFO, LOG_TAG, "  %s", logMessage.c_str());
    // Show the log in the UI
    ecore_main_loop_thread_safe_call_sync(updateLog, &logMessage);
}

static void
create_base_gui(appdata_s *ad)
{
    // Window
    ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
    elm_win_autodel_set(ad->win, EINA_TRUE);

    if (elm_win_wm_rotation_supported_get(ad->win))
    {
        int rots[4] = { 0, 90, 180, 270 };
        elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
    }

    evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
    eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

    // Conformant
    ad->conform = elm_conformant_add(ad->win);
    evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(ad->win, ad->conform);
    evas_object_show(ad->conform);

    // Base Layout
    ad->base_layout = elm_layout_add(ad->conform);
    evas_object_size_hint_weight_set(ad->base_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_theme_set(ad->base_layout, "layout", "application", "default");
    evas_object_show(ad->base_layout);

    elm_object_content_set(ad->conform, ad->base_layout);

    // naviframe
    ad->naviframe = elm_naviframe_add(ad->base_layout);
    elm_object_part_content_set(ad->base_layout, "elm.swallow.content", ad->naviframe);

    // Scroller
    ad->scroller = elm_scroller_add(ad->naviframe);
    elm_scroller_bounce_set(ad->scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(ad->scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

    // layout
    ad->layout = elm_layout_add(ad->naviframe);
    evas_object_size_hint_weight_set(ad->layout, EVAS_HINT_EXPAND, 0.0);
    elm_layout_file_set(ad->layout, ELM_DEMO_EDJ, "mainpage_layout");

    elm_object_content_set(ad->scroller, ad->layout);

    ad->bootButton = elm_button_add(ad->layout);
    elm_object_text_set(ad->bootButton, "doBootStrap");
    evas_object_size_hint_weight_set(ad->bootButton, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(ad->bootButton, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(ad->bootButton, "clicked", doBootStrap_cb, ad);
    elm_object_part_content_set(ad->layout, "bootstrap_button", ad->bootButton);

    ad->createConfButton = elm_button_add(ad->layout);
    elm_object_text_set(ad->createConfButton, "Create Configuration Resources");
    evas_object_size_hint_weight_set(ad->createConfButton, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(ad->createConfButton, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(ad->createConfButton, "clicked", createConfResource_cb, ad);
    elm_object_part_content_set(ad->layout, "create_conf_button", ad->createConfButton);

    log_entry = elm_entry_add(ad->layout);
    elm_entry_scrollable_set(log_entry, EINA_TRUE);
    elm_entry_editable_set(log_entry, EINA_FALSE);
    elm_object_part_text_set(log_entry, "elm.guide", "Logs will be updated here!!!");
    evas_object_size_hint_weight_set(log_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(log_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(ad->layout, "log", log_entry);

    elm_naviframe_item_push(ad->naviframe, "Configuration Server", NULL, NULL, ad->scroller, NULL);

    // Show window after base gui is set up
    evas_object_show(ad->win);
}

// Function which configures the OCPlatform
static void
configure_platform()
{
    try
    {
        PlatformConfig config
        { OC::ServiceType::InProc, ModeType::Both, "0.0.0.0", 0, OC::QualityOfService::LowQos };

        OCPlatform::Configure(config);

        dlog_print(DLOG_INFO, LOG_TAG, "#### Platform configuration done!!!!");
    }
    catch (OCException &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }
}


static bool
app_create(void *data)
{
    /* Hook to take necessary actions before main event loop starts
        Initialize UI resources and application's data
        If this function returns true, the main loop of application starts
        If this function returns false, the application is terminated */
    appdata_s *ad = (appdata_s *)data;

    elm_app_base_scale_set(1.8);

    // Create and show the UI
    create_base_gui(ad);

    // Configure the OCPlatform
    configure_platform();

    g_thingsConf = new ThingsConfiguration();

    return true;
}

static void
app_control(app_control_h app_control, void *data)
{
    // Handle the launch request.
}

static void
app_pause(void *data)
{
    // Take necessary actions when application becomes invisible.
}

static void
app_resume(void *data)
{
    // Take necessary actions when application becomes visible.
}

static void
app_terminate(void *data)
{
    // Release all resources.
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
    // APP_EVENT_LANGUAGE_CHANGED
    char *locale = NULL;
    system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
    elm_language_set(locale);
    free(locale);
    return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
    // APP_EVENT_DEVICE_ORIENTATION_CHANGED
    return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
    // APP_EVENT_REGION_FORMAT_CHANGED
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
    // APP_EVENT_LOW_BATTERY
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
    // APP_EVENT_LOW_MEMORY
}

int
main(int argc, char *argv[])
{
    appdata_s ad = {0,};
    int ret = 0;

    ui_app_lifecycle_callback_s event_callback = {0,};
    app_event_handler_h handlers[5] = {NULL, };

    event_callback.create = app_create;
    event_callback.terminate = app_terminate;
    event_callback.pause = app_pause;
    event_callback.resume = app_resume;
    event_callback.app_control = app_control;

    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY,
                             ui_app_low_battery, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY,
                             ui_app_low_memory, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED],
                             APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED,
                             ui_app_lang_changed, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
                             APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
    ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

    ret = ui_app_main(argc, argv, &event_callback, &ad);
    if (APP_ERROR_NONE != ret)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
    }

    return ret;
}

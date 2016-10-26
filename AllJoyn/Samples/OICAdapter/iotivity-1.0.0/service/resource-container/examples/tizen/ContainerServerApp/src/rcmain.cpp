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

#include "rcmain.h"
#include <tizen.h>

typedef struct appdata
{
    Evas_Object *win;
    Evas_Object *conform;
    Evas_Object *layout;
    Evas_Object *nf;
    Evas_Object *findButton;
    Evas_Object *logtext;
    Evas_Object *listview;
} appdata_s;

static void
win_delete_request_cb(void *data , Evas_Object *obj , void *event_info)
{
    ui_app_exit();
}

static void
list_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it = (Elm_Object_Item *)event_info;
    elm_list_item_selected_set(it, EINA_FALSE);
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
    ui_app_exit();
    return EINA_FALSE;
}

static void
create_list_view(appdata_s *ad)
{
    Evas_Object *list;
    Evas_Object *btn;
    Evas_Object *nf = ad->nf;
    Elm_Object_Item *nf_it;

    // List
    list = elm_list_add(nf);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);
    evas_object_smart_callback_add(list, "selected", list_selected_cb, NULL);

    // Main Menu Items Here
    elm_list_item_append(list, "Resource Container", NULL, NULL, containerCreateUI, nf);

    elm_list_go(list);

    // This button is set for devices which doesn't have H/W back key.
    btn = elm_button_add(nf);
    elm_object_style_set(btn, "naviframe/end_btn/default");
    nf_it = elm_naviframe_item_push(nf, "Resource Encapsulation", btn, NULL, list, NULL);
    elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, ad->win);
}


static void
create_base_gui(appdata_s *ad)
{
    // Window
    ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
    elm_win_conformant_set(ad->win, EINA_TRUE);
    elm_win_autodel_set(ad->win, EINA_TRUE);

    if (elm_win_wm_rotation_supported_get(ad->win))
    {
        int rots[4] = { 0, 90, 180, 270 };
        elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
    }

    evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);

    // Conformant
    ad->conform = elm_conformant_add(ad->win);
    evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(ad->win, ad->conform);
    evas_object_show(ad->conform);

    // Base Layout
    ad->layout = elm_layout_add(ad->conform);
    evas_object_size_hint_weight_set(ad->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_theme_set(ad->layout, "layout", "application", "default");
    evas_object_show(ad->layout);

    elm_object_content_set(ad->conform, ad->layout);

    // Naviframe
    ad->nf = elm_naviframe_add(ad->layout);
    create_list_view(ad);
    elm_object_part_content_set(ad->layout, "elm.swallow.content", ad->nf);
    eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
    eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

    // Show window after base gui is set up
    evas_object_show(ad->win);
}

// Configures the OCPlatform
static void
configure_platform()
{
    try
    {
        PlatformConfig config
        { OC::ServiceType::InProc, ModeType::Server, "0.0.0.0", 0, OC::QualityOfService::LowQos };

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

    create_base_gui(ad);

    configure_platform();

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
                             APP_EVENT_DEVICE_ORIENTATION_CHANGED,
                             ui_app_orient_changed, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
                             APP_EVENT_LANGUAGE_CHANGED,
                             ui_app_lang_changed, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
                             APP_EVENT_REGION_FORMAT_CHANGED,
                             ui_app_region_changed, &ad);
    ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

    ret = ui_app_main(argc, argv, &event_callback, &ad);
    if (APP_ERROR_NONE != ret)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
    }
    return ret;
}

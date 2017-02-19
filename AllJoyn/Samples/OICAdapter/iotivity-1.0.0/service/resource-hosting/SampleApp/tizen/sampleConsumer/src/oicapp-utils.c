/*
 * Copyright (c) 2014 Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is a confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics.
 */
#include <appcore-efl.h>
#include <wifi.h>

#include "oicapp-utils.h"

static void _popup_timeout_cb(void *data , Evas_Object *obj , void *event_info)
{
    oicappData *ad = data;

    ret_if(data == NULL);

    ad->popup = NULL;
}

void oicapp_fail_popup(oicappData *ad , char *title , char *text , int timeout)
{
    if(ad->popup)
    {
        evas_object_del(ad->popup);
        ad->popup = NULL;
    }

    Evas_Object *popup = elm_popup_add(ad->win);
    evas_object_size_hint_weight_set(popup , EVAS_HINT_EXPAND , EVAS_HINT_EXPAND);

    if(title)
        elm_object_part_text_set(popup , "title,text" , title);

    if(text)
        elm_object_text_set(popup , text);

    if(0 < timeout)
        elm_popup_timeout_set(popup , timeout);
    else
        elm_popup_timeout_set(popup , 3);

    evas_object_smart_callback_add(popup , "timeout" , _popup_timeout_cb , ad);

    evas_object_show(popup);

    ad->popup = popup;
}

void oicapp_util_update(oicappData *ad)
{
}

char* oicapp_util_wifi()
{
    int ret;
    wifi_ap_h ap;
    char *ip_addr = NULL;

    ret = wifi_initialize();
    if(WIFI_ERROR_NONE != ret)
    {
        ERR("wifi_initialize() Fail");
        return NULL;
    }

    ret = wifi_get_connected_ap(&ap);
    if(WIFI_ERROR_NONE != ret)
    {
        ERR("wifi_get_connected_ap() Fail");
        return NULL;
    }

    ret = wifi_ap_get_ip_address(ap , WIFI_ADDRESS_FAMILY_IPV4 , &ip_addr);
    if(WIFI_ERROR_NONE != ret)
    {
        ERR("wifi_ap_get_ip_address() Fail");
        return NULL;
    }

    return ip_addr;
}


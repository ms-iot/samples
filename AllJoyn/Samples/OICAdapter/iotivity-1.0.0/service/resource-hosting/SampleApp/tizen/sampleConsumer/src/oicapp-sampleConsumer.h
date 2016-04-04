/*
 * Copyright (c) 2010 Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is a confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics.
 */

#ifndef __OICAPP_TEST_H__
#define __OICAPP_TEST_H__

#include <Elementary.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "oicapp-log.h"

#if !defined(PACKAGE)
#  define PACKAGE "oicapp-test"
#endif

#if !defined(LOCALEDIR)
#  define LOCALEDIR "/usr/apps/com.samsung.oicapp-test/res/locale"
#endif

#if !defined(EDJDIR)
#  define EDJDIR "/usr/apps/com.samsung.oicapp-test/res/edje"
#endif

#define GRP_MAIN "main"

enum
{
    OICAPP_GENLIST_GRP_NONE = 0,
    OICAPP_GENLIST_GRP_TOP,
    OICAPP_GENLIST_GRP_CENTER,
    OICAPP_GENLIST_GRP_BOTTOM
};

typedef struct
{
    Evas_Object *win;
    Evas_Object *base;
    Evas_Object *bg;
    Evas_Object *navi;
    Evas_Object *genlist;
    Evas_Object *popup;
    Evas_Object *conform;

    Elm_Object_Item *itemConsumer;
    Elm_Object_Item *itemConsumerUri;
    Elm_Object_Item *itemConsumerHost;
    Elm_Object_Item *itemConsumerTemp;
    Elm_Object_Item *itemConsumerHumid;
    Elm_Object_Item *itemFindResource;
    Elm_Object_Item *itemObserve;
    Elm_Object_Item *itemServer;
    Elm_Object_Item *itemServerTemp;
    Elm_Object_Item *itemServerHumid;

    Elm_Genlist_Item_Class itcSeperator;
    Elm_Genlist_Item_Class itcTitle;
    Elm_Genlist_Item_Class itcText;
    Elm_Genlist_Item_Class itcBtnFindResoruce;
    Elm_Genlist_Item_Class itcBtnObserve;

    char *ipAddr;
    //oicapp_mode mode;
    int clientOn;
    int serverOn;
    int power;
    int level;

    int temp;
    int humid;

    char *uri;
    char *host;

} oicappData;

extern void _gl_update_item(oicappData *ad , const char *title , Elm_Object_Item *item);

typedef struct
{
    oicappData *ad;
    const char *title;
    int group_style;

} oicappItemData;

#ifdef __cplusplus
}
#endif

#endif //__OICAPP_TEST_H__

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
#include <Ecore_X.h>
#include <efl_assist.h>

#include "oicapp-sampleConsumer.h"
#include "oicapp-utils.h"

char* OICAPP_STR_URI = "Uri : ";
char* OICAPP_STR_HOST = "Host : ";

const char* const OICAPP_STR_CONSUMER = "Consumer";

void __gl_realized_cb(void *data , Evas_Object *obj , void *event_info)
{
    Elm_Object_Item *item = event_info;
    elm_object_item_signal_emit(item , "elm,state,normal" , "");
}

static Elm_Object_Item* oicapp_append_separator(Evas_Object *genlist , oicappData *ad)
{
    Elm_Object_Item *item = NULL;

    item = elm_genlist_item_append(genlist , &ad->itcSeperator , NULL , NULL ,
            ELM_GENLIST_ITEM_NONE , NULL , NULL);
    elm_genlist_item_select_mode_set(item , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    return item;
}

static Elm_Object_Item* _gl_append_item(oicappData *ad , Elm_Genlist_Item_Class *itc ,
        const char *title , Evas_Smart_Cb sel_func)
{
    Elm_Object_Item *item;
    oicappItemData *it_data;

    it_data = calloc(1 , sizeof(oicappItemData));
    if (NULL == it_data)
    {
        DBG("calloc failed!!!!");
        return NULL;
    }
    it_data->title = title;
    it_data->ad = ad;
    item = elm_genlist_item_append(ad->genlist , itc , it_data , NULL , ELM_GENLIST_ITEM_NONE ,
            sel_func , ad);

    return item;
}

void _gl_update_item(oicappData *ad , const char *title , Elm_Object_Item *item)
{
    oicappItemData *it_data;

    it_data = calloc(1 , sizeof(oicappItemData));
    if (NULL == it_data)
    {
        DBG("calloc failed!!!!");
        return;
    }
    it_data->title = title;
    it_data->ad = ad;

    elm_object_item_data_set(item , it_data);
    elm_genlist_item_update(item);
}

static Elm_Object_Item* _gl_append_btn(oicappData *ad , Elm_Genlist_Item_Class *itc)
{

    DBG("btn create!!");
    Elm_Object_Item *item;

    item = elm_genlist_item_append(ad->genlist , itc , ad , NULL , ELM_GENLIST_ITEM_NONE , NULL ,
            NULL);

    elm_genlist_item_select_mode_set(item , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    return item;
}

static void consumerapp_append_contents(oicappData *ad)
{

    Elm_Object_Item *item , *parent;

    parent = _gl_append_item(ad , &ad->itcTitle , OICAPP_STR_CONSUMER , NULL);
    elm_genlist_item_select_mode_set(parent , ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    item = _gl_append_item(ad , &ad->itcText , OICAPP_STR_URI , NULL);
    ad->itemConsumerUri = item;

    item = _gl_append_item(ad , &ad->itcText , OICAPP_STR_HOST , NULL);
    ad->itemConsumerHost = item;

    _gl_append_btn(ad , &ad->itcBtnFindResoruce);

    item = _gl_append_item(ad , &ad->itcText , "" , NULL);
    ad->itemConsumerTemp = item;

    item = _gl_append_item(ad , &ad->itcText , "" , NULL);
    ad->itemConsumerHumid = item;

    _gl_append_btn(ad , &ad->itcBtnObserve);

    oicapp_append_separator(ad->genlist , ad);

}

static Evas_Object* consumerapp_create_genlist(Evas_Object *parent)
{
    Evas_Object *genlist;

    genlist = elm_genlist_add(parent);
    if(NULL == genlist)
    {
        ERR("elm_genlist_add() Fail");
        return NULL;
    }

    elm_object_style_set(genlist , "dialogue");
    evas_object_size_hint_weight_set(genlist , EVAS_HINT_EXPAND , EVAS_HINT_EXPAND);
    evas_object_show(genlist);

    evas_object_smart_callback_add(genlist , "realized" , __gl_realized_cb , NULL);

    return genlist;
}

static Eina_Bool _back_cb(void *data , Elm_Object_Item *item)
{
    DBG("test _back_cb()");
    oicappData *ad = data;

    elm_genlist_item_update(ad->itemConsumer);
    elm_genlist_item_update(ad->itemConsumerHost);
    elm_genlist_item_update(ad->itemConsumerTemp);
    elm_genlist_item_update(ad->itemConsumerHumid);
    elm_genlist_item_update(ad->itemConsumerUri);

    elm_object_item_disabled_set(ad->itemConsumer , EINA_FALSE);
    elm_object_item_disabled_set(ad->itemConsumerHost , EINA_TRUE);
    elm_object_item_disabled_set(ad->itemConsumerTemp , EINA_TRUE);
    elm_object_item_disabled_set(ad->itemConsumerHumid , EINA_TRUE);
    elm_object_item_disabled_set(ad->itemConsumerUri , EINA_TRUE);

    elm_exit();

    return EINA_FALSE;
}

static void _win_del(void *data , Evas_Object *obj , void *event)
{
    DBG("test _win_del()");

    elm_exit();
}

static Evas_Object* consumerapp_create_conform(Evas_Object *win)
{
    Evas_Object *conform = NULL;
    conform = elm_conformant_add(win);

    evas_object_size_hint_weight_set(conform , EVAS_HINT_EXPAND , EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win , conform);
    evas_object_show(conform);

    Evas_Object *bg_indicator = elm_bg_add(conform);
    elm_object_style_set(bg_indicator , "indicator/headerbg");
    elm_object_part_content_set(conform , "elm.swallow.indicator_bg" , bg_indicator);
    evas_object_show(bg_indicator);

    return conform;
}

static Evas_Object* consumerapp_create_win(const char *name)
{
    Evas_Object *eo;
    int w , h;

    eo = elm_win_add(NULL , name , ELM_WIN_BASIC);
    if(eo)
    {
        elm_win_title_set(eo , name);
        elm_win_borderless_set(eo , EINA_TRUE);
        evas_object_smart_callback_add(eo , "delete,request" , _win_del , NULL);
        ecore_x_window_size_get(ecore_x_window_root_first_get() , &w , &h);
        evas_object_resize(eo , w , h);
        elm_win_indicator_mode_set(eo , ELM_WIN_INDICATOR_SHOW);
        elm_win_indicator_opacity_set(eo , ELM_WIN_INDICATOR_OPAQUE);
    }

    evas_object_show(eo);

    return eo;
}

static Evas_Object* consumerapp_create_bg(Evas_Object *parent)
{
    Evas_Object *bg;

    bg = elm_bg_add(parent);
    evas_object_size_hint_weight_set(bg , EVAS_HINT_EXPAND , EVAS_HINT_EXPAND);
    elm_win_resize_object_add(parent , bg);
    evas_object_show(bg);

    return bg;
}

static Evas_Object* consumerapp_create_base_layout(Evas_Object *parent)
{
    Evas_Object *base;

    base = elm_layout_add(parent);
    elm_layout_theme_set(base , "layout" , "application" , "default");
    evas_object_size_hint_weight_set(base , EVAS_HINT_EXPAND , EVAS_HINT_EXPAND);
    evas_object_show(base);

    return base;
}

static void _btn_observe_clicked(void *data , Evas_Object *obj , void *event_info)
{
    oicappData *ad = data;

    startObserve(ad);
}

static void _btn_findResource_clicked(void *data , Evas_Object *obj , void *event_info)
{
    oicappData *ad = data;

    findResourceCandidate(ad);
}

static Evas_Object* _gl_btn_observe_content_get(void *data , Evas_Object *obj , const char *part)
{
    Evas_Object *button;
    oicappData *ad = data;

    button = elm_button_add(obj);
    elm_object_part_text_set(button , NULL , "Observe");
    evas_object_propagate_events_set(button , EINA_FALSE);
    evas_object_smart_callback_add(button , "clicked" , _btn_observe_clicked , ad);

    return button;
}

static Evas_Object* _gl_btn_findResource_content_get(void *data , Evas_Object *obj ,
        const char *part)
{
    Evas_Object *button;
    oicappData *ad = data;

    button = elm_button_add(obj);
    elm_object_part_text_set(button , NULL , "Find Resource");
    evas_object_propagate_events_set(button , EINA_FALSE);
    evas_object_smart_callback_add(button , "clicked" , _btn_findResource_clicked , ad);

    return button;
}

static void _gl_item_del(void *data , Evas_Object *obj)
{
    if(data != NULL)
        free(data);
}

static char* _gl_text_get(void *data , Evas_Object *obj , const char *part)
{
    oicappItemData *it_data = data;
    return strdup(it_data->title);
}

static inline void oicapp_init_itcs(oicappData *ad)
{
    ad->itcSeperator.item_style = "dialogue/separator";
    ad->itcSeperator.func.text_get = NULL;
    ad->itcSeperator.func.content_get = NULL;
    ad->itcSeperator.func.state_get = NULL;
    ad->itcSeperator.func.del = NULL;

    ad->itcTitle.item_style = "dialogue/title";
    ad->itcTitle.func.text_get = _gl_text_get;
    ad->itcTitle.func.content_get = NULL;
    ad->itcTitle.func.state_get = NULL;
    ad->itcTitle.func.del = _gl_item_del;

    ad->itcText.item_style = "dialogue/1text";
    ad->itcText.func.text_get = _gl_text_get;
    ad->itcText.func.content_get = NULL;
    ad->itcText.func.state_get = NULL;
    ad->itcText.func.del = _gl_item_del;

    ad->itcBtnFindResoruce.item_style = "dialogue/1icon";
    ad->itcBtnFindResoruce.func.text_get = NULL;
    ad->itcBtnFindResoruce.func.content_get = _gl_btn_findResource_content_get;
    ad->itcBtnFindResoruce.func.state_get = NULL;
    ad->itcBtnFindResoruce.func.del = NULL;

    ad->itcBtnObserve.item_style = "dialogue/1icon";
    ad->itcBtnObserve.func.text_get = NULL;
    ad->itcBtnObserve.func.content_get = _gl_btn_observe_content_get;
    ad->itcBtnObserve.func.state_get = NULL;
    ad->itcBtnObserve.func.del = NULL;

}

static int oicapp_create(void *data)
{
    oicappData *ad = data;
    Elm_Object_Item *it;

    oicapp_init_itcs(ad);

    /* create window */
    ad->win = consumerapp_create_win(PACKAGE);
    if(NULL == ad->win)
        return -1;

    ad->bg = consumerapp_create_bg(ad->win);
    if(NULL == ad->bg)
        return -1;

    ad->conform = consumerapp_create_conform(ad->win);
    if(NULL == ad->conform)
        return -1;

    /* create layout */
    ad->base = consumerapp_create_base_layout(ad->conform);
    if(NULL == ad->base)
        return -1;
    elm_object_content_set(ad->conform , ad->base);

    ad->navi = elm_naviframe_add(ad->base);
    elm_object_part_content_set(ad->base , "elm.swallow.content" , ad->navi);
    ea_object_event_callback_add(ad->navi , EA_CALLBACK_BACK , ea_naviframe_back_cb , NULL);

    ad->genlist = consumerapp_create_genlist(ad->navi);

    it = elm_naviframe_item_push(ad->navi , "IoT Notification Sample App" , NULL , NULL ,
            ad->genlist , NULL);
    elm_naviframe_item_pop_cb_set(it , _back_cb , ad);

    ad->ipAddr = oicapp_util_wifi();
    if(NULL == ad->ipAddr)
    {
        ERR("wifi is not connected");
        oicapp_fail_popup(ad , "Error" , "No WIFI connection" , 3);
    }
    else
    {
        INFO("IP Address = %s" , ad->ipAddr);
    }

    oicapp_client_start(ad);

    consumerapp_append_contents(ad);

    return 0;
}

static int oicapp_terminate(void *data)
{
    DBG("test _terminate()!!");
    oicappData *ad = data;

    if(ad->win)
    {
        DBG("test_del evas object1");
        evas_object_del(ad->win);
        DBG("test_del evas object2");
    }

    free(ad->ipAddr);
    return 0;

}

static int oicapp_pause(void *data)
{
    DBG("test _pause()!!!!");
    return 0;
}

static int oicapp_resume(void *data)
{
    DBG("test_resume()");
    return 0;
}

static int oicapp_reset(bundle *b , void *data)
{
    return 0;
}

int main(int argc , char *argv[])
{
    oicappData ad;
    DBG("test_start()");
    struct appcore_ops ops =
    { .create = oicapp_create , .terminate = oicapp_terminate , .pause = oicapp_pause , .resume =
            oicapp_resume , .reset = oicapp_reset , };

    memset(&ad , 0x0 , sizeof(oicappData));
    ops.data = &ad;

    return appcore_efl_main(PACKAGE , &argc , &argv , &ops);
}

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

#ifndef TMSAMPLEAPP_H__
#define TMSAMPLEAPP_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#include "OCPlatform.h"
#include "OCApi.h"
#include "GroupManager.h"
#include "ThingsConfiguration.h"
#include "ThingsMaintenance.h"


#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "tmsampleapp"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.tmsampleapp"
#endif

#define ELM_DEMO_EDJ "opt/usr/apps/org.tizen.tmsampleapp/res/ui_controls.edj"

#define FINDGROUP_TIMEOUT 5
#define FINDRESOURCE_TIMEOUT 6

#define DEFAULT_DEVICENAME "n"
#define DEFAULT_LOCATION "loc"
#define DEFAULT_LOCATIONNAME "locn"
#define DEFAULT_CURRENCY "c"
#define DEFAULT_REGION "r"

void group_cb(void *data, Evas_Object *obj, void *event_info);
void configuration_cb(void *data, Evas_Object *obj, void *event_info);

#endif // TMSAMPLEAPP_H__
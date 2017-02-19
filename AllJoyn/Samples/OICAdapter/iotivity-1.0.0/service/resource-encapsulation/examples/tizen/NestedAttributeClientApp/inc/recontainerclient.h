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

#include "reclientmain.h"

#include "RCSRemoteResourceObject.h"

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#ifndef RECONTAINER_H__
#define RECONTAINER_H__

# define LOG_TAG "recontainerclient"

using namespace OIC::Service;

void *showContainerAPIs(void *data);

void onContainerDiscovered(std::shared_ptr<RCSRemoteResourceObject> foundResource);

static void find_resource_cb(void *data, Evas_Object *obj, void *event_info);

static void findLight(void *data, Evas_Object *obj, void *event_info);

static void findSoftsensor(void *data, Evas_Object *obj, void *event_info);

#endif // RECONTAINER_H__
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

#ifndef CONTAINER_H__
#define CONTAINER_H__

using namespace std;

void *showContainerAPIs(void *data);

void containerCreateUI(void *data);

static void startContainer(void *data, Evas_Object *obj, void *event_info);

static void stopContainer(void *data, Evas_Object *obj, void *event_info);

#endif // CONTAINER_H__

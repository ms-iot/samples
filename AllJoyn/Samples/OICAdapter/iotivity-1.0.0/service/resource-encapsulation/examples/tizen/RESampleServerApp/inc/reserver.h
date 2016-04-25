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

#include "remain.h"

#include "RCSResourceObject.h"

#ifndef RESERVER_H__
#define RESERVER_H__

using namespace std;
using namespace OIC::Service;

typedef void(*ClientMenuHandler)();
typedef int ReturnValue;

constexpr int DEFALUT_VALUE = 0;

constexpr int PRESENCE_ON = 1;
constexpr int PRESENCE_OFF = 2;

std::string resourceUri = "/a/TempSensor";
std::string resourceType = "oic.r.temperaturesensor";
std::string resourceInterface = "oic.if.";
std::string attributeKey = "Temperature";
int isPresenceOn = PRESENCE_ON;

enum class Control
{
    INCREASE,
    DECREASE
};

void printAttribute(const RCSResourceAttributes &attrs);

void start_server(void *data, Evas_Object *obj, void *event_info);

void start_server_cb(void *data, Evas_Object *obj, void *event_info);

void *showGroupAPIs(void *data);

#endif // RESERVER_H__
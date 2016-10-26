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

#include "rhsampleapp.h"
#include "rhutil.h"
#include <algorithm>
#include <signal.h>
#include "Hosting.h"

using namespace std;
using namespace OC;

namespace PH = std::placeholders;

static uint8_t interfaceName[] = "wlan0";
int g_quitFlag = 0;

static void printLog(int logType, string data)
{
    (DLOG_ERROR == logType) ? LOGE(data.c_str()) : LOGI(data.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateGroupLog,
                                          &data);
}

// Method for Finding the resource and hosting it
static void start_hosting(int seconds)
{
    string logMessage = "OCResourceHosting started <br>";
    logMessage = logMessage + "Interface Name : " + reinterpret_cast<char *>(interfaceName) + "<br>";
    printLog(DLOG_INFO, logMessage);

    if (OICStartCoordinate() != OC_STACK_OK)
    {
        logMessage = "OICStartCoordinate FAILED <br>";
        printLog(DLOG_ERROR, logMessage);
        return;
    }

    g_quitFlag = 0;
    logMessage = "OICStartCoordinate done successfully <br>";
    printLog(DLOG_INFO, logMessage);

    while (!g_quitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OICStopCoordinate();
            logMessage = "OCStack process error <br>";
            printLog(DLOG_ERROR, logMessage);
            return;
        }
        sleep(seconds);
    }

    if (OICStopCoordinate() != OC_STACK_OK)
    {
        logMessage = "OICStopCoordinate FAILED <br>";
        printLog(DLOG_ERROR, logMessage);
    }
    logMessage = "OICStopCoordinate done successfully <br>";
    printLog(DLOG_INFO, logMessage);
    LOGI("start EXIT");
}

// Method for Finding the resource and hosting it
void stop_hosting()
{
    string logMessage = "Terminating Resource Hosting <br>";
    printLog(DLOG_INFO, logMessage);

    if(!g_quitFlag)
    {
    	g_quitFlag = 1;
    }
    else
    {
    	string logMessage = "Resource Hosting already terminated <br>";
    	printLog(DLOG_INFO, logMessage);
    }
    LOGI("stop_hosting EXIT");
}

// Method to be called when the find and host UI Button is selected
void start_cb(void *data, Evas_Object *obj, void *event_info)
{
    std::thread exec(std::function< void(int second) >(start_hosting), 3);
    exec.detach();
}

// Method to be called when the find and host UI Button is selected
void stop_cb(void *data, Evas_Object *obj, void *event_info)
{
    LOGI("Stopping resource hosting");
    stop_hosting();
}

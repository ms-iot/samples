//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <signal.h>
#include <unistd.h>

#include "Hosting.h"

int g_quitFlag = 0;

void handleSigInt(int signum);

/*
* This method is an entry point of Resource Hosting Manager.
* This function should be run only on the device that it could be host device.
*/
int main()
{
    printf("OCResourceHosting is starting...\n");

    if (OICStartCoordinate() != OC_STACK_OK)
    {
        printf("OICStartCoordinate fail\n");
        return 0;
    }

    printf("OCResourceHosting Started Successfully \n");

    signal(SIGINT, handleSigInt);
    while (!g_quitFlag)
    {
        sleep(2);
    }

    if (OICStopCoordinate() != OC_STACK_OK)
    {
        printf("OICStopCoordinate error\n");
    }
    else
    {
        printf("OICStopCoordinate success\n");
    }

    printf("Exiting occlient main loop...\n");
    return 0;

}

/*
* This is a signal handling function for SIGINT(CTRL+C).
* A Resource Coordinator handle the SIGINT signal for safe exit.
*
* @param[in] signal
*                 signal number of caught signal.
*/
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        g_quitFlag = 1;
    }
}

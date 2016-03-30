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
#include "rd_server.h"

#include <signal.h>
#include <unistd.h>

int g_quitFlag = 0;

void handleSigInt(int signum);

/*
* This method is an entry point of Resource Directory.
* This function should be run only on the device that it could be host device.
*/

int main()
{
    printf("OCResourceDirectory is starting...\n");
    OCStackResult result = OCInit(NULL, 0, OC_CLIENT_SERVER);
    if (result != OC_STACK_OK)
    {
        printf("Failed starting RD server ...\n");
        return 0;
    }
    if (OCRDStart() != OC_STACK_OK)
    {
        printf("OCRDStart failed...\n");
        return 0;
    }

    printf("OCRDStart successfully...\n");

    signal(SIGINT, handleSigInt);
    while (!g_quitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OCRDStop();
            printf("OCStack process error\n");
            return 0;
        }
    }

    if (OCRDStop() != OC_STACK_OK)
    {
        printf("OCRDStop failed...\n");
    }
    else
    {
        printf("OCRDStop success...\n");
    }

    printf("Exiting OCResourceDirectory main loop...\n");
    return 0;

}

/*
* This is a signal handling function for SIGINT(CTRL+C).
* A Resource Directory handle the SIGINT signal for safe exit.
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

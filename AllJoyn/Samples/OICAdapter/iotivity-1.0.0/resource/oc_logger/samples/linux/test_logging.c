//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAG  ("MAIN")

static int customLogger = 0;

static void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : test_logging -c <0|1>");
    OC_LOG(INFO, TAG, "-u <0|1> : 0 - default logging, 1 - custom console logging");
}

int main(int argc, char* argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "c:")) != -1)
    {
        switch(opt)
        {
            case 'c':
                customLogger = atoi(optarg);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    if (customLogger == 0)
    {
        // Default logger
        OC_LOG(DEBUG, TAG, "This is a DEBUG");
        OC_LOG(INFO, TAG, "This is a INFO");
        OC_LOG(WARNING, TAG, "This is a WARNING");
        OC_LOG(ERROR, TAG, "This is a ERROR");
        OC_LOG(FATAL, TAG, "This is a FATAL");
    }
    else
    {
        // Custom logger, in this case, the console logger
        oc_log_ctx_t *log = oc_make_console_logger();

        OC_LOG_CONFIG(log);

        OC_LOG(DEBUG, TAG, "This is a DEBUG");
        OC_LOG(INFO, TAG, "This is a INFO");
        OC_LOG(WARNING, TAG, "This is a WARNING");
        OC_LOG(ERROR, TAG, "This is a ERROR");
        OC_LOG(FATAL, TAG, "This is a FATAL");
        OC_LOG_SHUTDOWN();
    }


    return 0;
}

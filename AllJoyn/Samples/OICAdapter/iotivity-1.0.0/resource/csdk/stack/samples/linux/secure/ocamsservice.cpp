

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "ocstack.h"
#include "logger.h"
#include "common.h"

#define TAG  PCF("SRM-AMSS")

int gQuitFlag = 0;

//AMS service database, hold AMS service Identity and
//the PSK credentials of trusted devices
static char AMSS_DB_FILE[] = "oic_amss_db.json";

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

FILE* service_fopen(const char *path, const char *mode)
{
    (void)path;
    return fopen(AMSS_DB_FILE, mode);
}

int main(int /*argc*/, char* /*argv*/[])
{
    struct timespec timeout;

    OC_LOG(DEBUG, TAG, "OCAMS service is starting...");

    // Initialize Persistent Storage for SVR database
    OCPersistentStorage ps = { service_fopen, fread, fwrite, fclose, unlink };
    OCRegisterPersistentStorageHandler(&ps);

    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    timeout.tv_sec  = 0;
    timeout.tv_nsec = 100000000L;

    // Break from loop with Ctrl-C
    OC_LOG(INFO, TAG, "Entering ocamsservice main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }
        nanosleep(&timeout, NULL);
    }

    OC_LOG(INFO, TAG, "Exiting ocamsservice main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}

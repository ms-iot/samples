/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>

#include "cathreadpool.h"
#include "camutex.h"

ca_thread_pool_t g_threadPoolHandle = NULL;

ca_mutex g_mutex = NULL;
ca_cond g_cond = NULL;
bool g_condFlag = false;

void task(void *data)
{
    printf("[TASK] Task is executing: data: %s\n", (char *) data);

    //Signal the condition that task has been completed
    printf("[TASK] Signaling the condition\n");
    ca_mutex_lock(g_mutex);
    g_condFlag = true;
    ca_cond_signal(g_cond);
    ca_mutex_unlock(g_mutex);
}

void testThreadPool(void)
{
    char *string = "Test thread pool";

    //Initialize the mutex
    printf("[testThreadPool] Initializing mutex\n");

    //Initialize the thread pool
    printf("[testThreadPool] Initializing thread pool\n");
    if (CA_STATUS_OK != ca_thread_pool_init(2, &g_threadPoolHandle))
    {
        printf("thread_pool_init failed!\n");
        return;
    }

    //Create the mutex
    printf("[testThreadPool] Creating mutex\n");
    g_mutex = ca_mutex_new();
    if (NULL == g_mutex)
    {
        printf("[testThreadPool] Failed to create mutex!\n");
        ca_thread_pool_free(g_threadPoolHandle);
        return;
    }

    //Create the condition
    printf("[testThreadPool] Creating Condition\n");
    g_cond = ca_cond_new();
    if (NULL == g_cond)
    {
        printf("[testThreadPool] Failed to create condition!\n");
        ca_mutex_free(g_mutex);
        ca_thread_pool_free(g_threadPoolHandle);
        return;
    }

    //Lock the mutex
    printf("[testThreadPool] Locking the mutex\n");
    ca_mutex_lock(g_mutex);

    g_condFlag = false;
    //Add task to thread pool
    printf("[testThreadPool] Adding the task to thread pool\n");
    if (CA_STATUS_OK != ca_thread_pool_add_task(g_threadPoolHandle, task, (void *) string))
    {
        printf("[testThreadPool] thread_pool_add_task failed!\n");
        ca_thread_pool_free(g_threadPoolHandle);
        ca_mutex_unlock(g_mutex);
        ca_mutex_free(g_mutex);
        ca_cond_free(g_cond);
        return;
    }

    //Wait for the task to be executed
    printf("[testThreadPool] Waiting for the task to be completed\n");

    while (!g_condFlag)
    {
        ca_cond_wait(g_cond, g_mutex);
    }

    //Unlock the mutex
    printf("[testThreadPool] Got the signal and unlock the mutex\n");
    ca_mutex_unlock(g_mutex);

    printf("[testThreadPool] Task is completed and terminating threadpool\n");
    ca_cond_free(g_cond);
    ca_mutex_free(g_mutex);
    ca_thread_pool_free(g_threadPoolHandle);

    printf("Exiting from testThreadPool\n");
}

static void menu()
{
    printf(" =====================================================================\n");
    printf("|                 Welcome to Theadpool testing                        |\n");
    printf("|---------------------------------------------------------------------|\n");
    printf("|                           ** Options **                             |\n");
    printf("|  1 - Test Threadpool functionality                                  |\n");
    printf("|  0 - Terminate test                                                 |\n");
}

static void startTesting(void)
{
    while (1)
    {
        int choice = -1;
        if(scanf("%d", &choice) == 1)
        {
            switch (choice)
            {
                case 0:
                    printf("Terminating test.\n");
                    return;
                case 1:
                    testThreadPool();
                    break;
                default:
                    printf("Invalid input.\n");
                    menu();
                    break;
            }
        }
        else
        {
            printf("Invalid input.\n");
            menu();
        }

        // clear input buffer
        while (getchar() != '\n');
    }
}

int main()
{
    menu();
    startTesting();
    return 0;
}


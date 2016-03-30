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

#include "gtest/gtest.h"

#include <pthread.h>

/**
 * Simple holder of work-arounds for link-time issues.
 */
void workaroundHook()
{
    pthread_key_t key = {0};
    int ret = pthread_key_create(&key, NULL);
    if (!ret)
    {
        void *ptr = pthread_getspecific(key); // should return NULL
        ret = pthread_setspecific(key, &ptr);
        if (ret)
        {
            // Something went wrong. Since this is a stub, we don't care.
        }

        pthread_key_delete(key);
    }
}


TEST(BaseTest, WorldIsSane)
{
    workaroundHook();

    EXPECT_EQ(1 + 1, 2);

    int a = 1;
    int b = 5;

    EXPECT_GT(b, a);
}

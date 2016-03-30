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


extern "C" {
    #include "logger.h"
    #include "ocstack.h"
    #include "ocstackinternal.h"
}

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <stdint.h>
using namespace std;


//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------
static const char TAG[] = "TestHarness";
static OCUri SERVICE_URI = "coap://127.0.0.1:5683/";

void EXPECT_EQ(int a, int b)  {
  if (a == b) {
    OC_LOG(INFO, TAG, "PASS");
  } else {
    OC_LOG(ERROR, TAG, "**FAIL**");
  }
}

void EXPECT_STREQ(const char *a, const char *b)  {
  if (strcmp(a, b) == 0) {
    OC_LOG(INFO, TAG, "PASS");
  } else {
    OC_LOG(ERROR, TAG, "**FAIL**");
  }
}
//-----------------------------------------------------------------------------
// Callback functions
//-----------------------------------------------------------------------------

extern "C" void asyncDoResourcesCallback(OCStackResult result, OCRepresentationHandle representation) {
    OC_LOG(INFO, TAG, "Entering asyncDoResourcesCallback");

    EXPECT_EQ(OC_STACK_OK, result);
    OCResource *resource = (OCResource *)representation;
    OC_LOG_V(INFO, TAG, "URI = %s", resource->uri);
    EXPECT_STREQ(SERVICE_URI, resource->uri);
}

//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------
void test0() {
    EXPECT_EQ(OC_STACK_OK, OCInit(0, 5683, OC_SERVER));
}

void test1() {
  EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 0, OC_SERVER));
}

void test2() {
    EXPECT_EQ(OC_STACK_OK, OCInit(0, 0, OC_SERVER));
}

void test3() {
    EXPECT_EQ(OC_STACK_ERROR, OCInit(0, 0, (OCMode)10));
}

void test4() {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT_SERVER));
}

void test5() {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCDoResource(OC_REST_GET, OC_EXPLICIT_DEVICE_DISCOVERY_URI, 0, 0, asyncDoResourcesCallback), NULL, 0);
    EXPECT_EQ(OC_STACK_OK, OCUpdateResources(SERVICE_URI));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

void test6() {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCStop());
    EXPECT_EQ(OC_STACK_ERROR, OCStop());
}

void test7() {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCDoResource(OC_REST_GET, OC_EXPLICIT_DEVICE_DISCOVERY_URI, 0, 0, asyncDoResourcesCallback), NULL, 0);
    EXPECT_EQ(OC_STACK_INVALID_URI, OCUpdateResources(0));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

void stacktests() {
  test0();
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
}

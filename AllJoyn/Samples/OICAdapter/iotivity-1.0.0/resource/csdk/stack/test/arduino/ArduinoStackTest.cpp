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


// Do not remove the include below
#include "ArduinoStackTest.h"

#include "logger.h"
#include "ocstack.h"
#include "ocstackinternal.h"
#include <string.h>

#define TAG "Arduino"
static OCUri SERVICE_URI = "coap://127.0.0.1:5683/";

#if 0  // Turn off logger test stuff
PROGMEM const char tag[] = "Arduino";
PROGMEM const char msg[] = "Arduino Logger Test";

PROGMEM const char debugMsg[] = "this is a DEBUG message";
PROGMEM const char infoMsg[] = "this is a INFO message";
PROGMEM const char warningMsg[] = "this is a WARNING message";
PROGMEM const char errorMsg[] = "this is a ERROR message";
PROGMEM const char fatalMsg[] = "this is a FATAL message";

PROGMEM const char multiLineMsg[] = "this is a DEBUG message\non multiple\nlines";
#endif

void EXPECT_EQ(int a, int b)  {
  if (a == b) {
    OC_LOG(INFO, TAG, ("PASS"));
  } else {
    OC_LOG(ERROR, TAG, ("FAIL"));
  }
}

void EXPECT_STREQ(const char *a, const char *b)  {
  if (strcmp(a, b) == 0) {
    OC_LOG(INFO, TAG, ("PASS"));
  } else {
    OC_LOG(ERROR, TAG, ("FAIL"));
  }
}
//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------
#if 0  // Turn off logger tests
void test0() {
    OC_LOG(INFO, tag, msg);
}

void test1() {
    OC_LOG(INFO, 0, msg);
}

void test2() {
    OC_LOG(INFO, tag, 0);
}

void test3() {
    OC_LOG(INFO, 0, 0);
}

void test4() {
    OC_LOG(DEBUG, tag, debugMsg);
    OC_LOG(INFO, tag, infoMsg);
    OC_LOG(WARNING, tag, warningMsg);
    OC_LOG(ERROR, tag, errorMsg);
    OC_LOG(FATAL, tag, fatalMsg);
}

void test5() {
    OC_LOG(DEBUG, tag, multiLineMsg);
}


void test6() {
    // Log buffer
    uint8_t buffer[50];
    for (int i = 0; i < (int)(sizeof buffer); i++) {
        buffer[i] = i;
    }
    OC_LOG_BUFFER(DEBUG, tag, buffer, sizeof buffer);

    // Log buffer, 128 bytes is a good boundary (8 rows of 16 values)
    uint8_t buffer1[128];
    for (int i = 0; i < (int)(sizeof buffer1); i++) {
        buffer1[i] = i;
    }
    OC_LOG_BUFFER(DEBUG, tag, buffer1, sizeof buffer1);

    // 1 below 128 byte boundary
    uint8_t buffer2[127];
    for (int i = 0; i < (int)(sizeof buffer2); i++) {
        buffer2[i] = i;
    }
    OC_LOG_BUFFER(DEBUG, tag, buffer2, sizeof buffer2);

    // 1 above 128 byte boundary
    uint8_t buffer3[129];
    for (int i = 0; i < (int)(sizeof buffer3); i++) {
        buffer3[i] = i;
    }
    OC_LOG_BUFFER(DEBUG, tag, buffer3, sizeof buffer3);
}
#endif

extern "C" void asyncDoResourcesCallback(OCStackResult result, OCRepresentationHandle representation) {
    OC_LOG(INFO, TAG, ("Entering asyncDoResourcesCallback"));

    EXPECT_EQ(OC_STACK_OK, result);
    OCResource *resource = (OCResource *)representation;
    OC_LOG_V(INFO, TAG, "URI = %s", resource->uri);
    EXPECT_STREQ(SERVICE_URI, resource->uri);
}

void test0() {
    OC_LOG(INFO, TAG, ("test0"));
    EXPECT_EQ(OC_STACK_OK, OCInit(0, 5683, OC_SERVER));
}

void test1() {
    OC_LOG(INFO, TAG, ("test1"));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 0, OC_SERVER));
}

void test2() {
    OC_LOG(INFO, TAG, ("test2"));
    EXPECT_EQ(OC_STACK_OK, OCInit(0, 0, OC_SERVER));
}

void test3() {
    OC_LOG(INFO, TAG, ("test3"));
    EXPECT_EQ(OC_STACK_ERROR, OCInit(0, 0, (OCMode)10));
}

void test4() {
    OC_LOG(INFO, TAG, ("test4"));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT_SERVER));
}

void test5() {
    OC_LOG(INFO, TAG, ("test5"));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCDoResource(OC_REST_GET, OC_EXPLICIT_DEVICE_DISCOVERY_URI, 0, 0, asyncDoResourcesCallback), NULL, 0);
    EXPECT_EQ(OC_STACK_OK, OCUpdateResources(SERVICE_URI));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

void test6() {
    OC_LOG(INFO, TAG, ("test6"));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCStop());
    EXPECT_EQ(OC_STACK_ERROR, OCStop());
}

void test7() {
    OC_LOG(INFO, TAG, ("test7"));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCDoResource(OC_REST_GET, OC_EXPLICIT_DEVICE_DISCOVERY_URI, 0, 0, asyncDoResourcesCallback), NULL, 0);
    EXPECT_EQ(OC_STACK_INVALID_URI, OCUpdateResources(0));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}



//The setup function is called once at startup of the sketch
void setup()
{
    // Add your initialization code here
    OC_LOG_INIT();

    test0();
    delay(2000);
    test1();
    delay(2000);
    test2();
    delay(2000);
    test3();
    delay(2000);
    test4();
    delay(2000);
    test5();
    delay(2000);
    test6();
    delay(2000);

#if 1
    test7();
    delay(2000);
#endif

}

// The loop function is called in an endless loop
void loop()
{
    delay(2000);
}

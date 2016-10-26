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
}

static const char tag[] = "Android";
static const char msg[] = "Android Logger Test";

static const char debugMsg[] = "this is a DEBUG message";
static const char infoMsg[] = "this is a INFO message";
static const char warningMsg[] = "this is a WARNING message";
static const char errorMsg[] = "this is a ERROR message";
static const char fatalMsg[] = "this is a FATAL message";

static const char multiLineMsg[] = "this is a DEBUG message\non multiple\nlines";


//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------
static void test0() {
  OC_LOG(INFO, tag, msg);
}

static void test1() {
  OC_LOG(INFO, 0, msg);
}

static void test2() {
  OC_LOG(INFO, tag, 0);
}

static void test3() {
  OC_LOG(INFO, 0, 0);
}

static void test4() {
  OC_LOG(DEBUG, tag, debugMsg);
  OC_LOG(INFO, tag, infoMsg);
  OC_LOG(WARNING, tag, warningMsg);
  OC_LOG(ERROR, tag, errorMsg);
  OC_LOG(FATAL, tag, fatalMsg);
}

static void test5() {
  OC_LOG(DEBUG, tag, multiLineMsg);
}


static void test6() {
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

static void test7() {
  OC_LOG_V(DEBUG, tag, "this is a char: %c", 'A');
  OC_LOG_V(DEBUG, tag, "this is an integer: %d", 123);
  OC_LOG_V(DEBUG, tag, "this is a string: %s", "hello");
  OC_LOG_V(DEBUG, tag, "this is a float: %5.2f", 123.45);
}

//-----------------------------------------------------------------------------
//  loggertests
//-----------------------------------------------------------------------------
void loggertests() {
  OC_LOG(INFO, tag, "Starting logger test");

  test0();
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
}



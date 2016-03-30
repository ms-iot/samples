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
#include "ArduinoLoggerTest.h"
#include "logger.h"

#define tag "Arduino"
#define msg "Arduino Logger Test"

#define debugMsg "this is a DEBUG message"
#define infoMsg "this is a INFO message"
#define warningMsg "this is a WARNING message"
#define errorMsg "this is a ERROR message"
#define fatalMsg "this is a FATAL message"

#define multiLineMsg "this is a DEBUG message\non multiple\nlines"


//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------
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
}

// The loop function is called in an endless loop
void loop()
{
    delay(2000);
}

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

#include "ocrandom.h"
#include "math.h"
#include <Arduino.h>

#define ARR_SIZE (20)

void setup() {
    Serial.begin(115200);
    Serial.println("Testing Random Number generator for Arduino");

    Serial.print("Testing OCSeedRandom ... ");
    if (OCSeedRandom() == 0) {
        Serial.println("[Success]");
    } else {
        Serial.println("[Fail]");
    }

    Serial.print("Testing OCGetRandomByte ... ");
    uint8_t value8 = OCGetRandomByte();
    if (value8 >= 0 && value8 < pow(2, 8)) {
        Serial.println("[Success]");
    } else {
        Serial.println("[Fail]");
    }

    Serial.print("Testing OCGetRandom ... ");
    uint32_t value32 = OCGetRandom();
    if (value32 >= 0 && value32 < pow(2, 32)) {
        Serial.println("[Success]");
    } else {
        Serial.println("[Fail]");
    }

    Serial.print("Testing OCFillRandomMem ... ");
    uint8_t array[ARR_SIZE] = {};
    OCFillRandomMem(array + 1, ARR_SIZE - 2);
    uint8_t overall = 0;
    uint8_t value82 = 0;
    for (int i = 1; i <= ARR_SIZE - 2; i++) {
        value82 = array[i];
        if (value82 >= 0 && value82 < pow(2, 8)) {
            overall++;
        }
    }
    if (overall == ARR_SIZE - 2 && array[0] == 0 && array[ARR_SIZE - 1] == 0) {
        Serial.println("[Success]");
    } else {
        Serial.println("[Fail]");
        Serial.print("overall:");Serial.println(overall);
        Serial.print("array[0]:");Serial.println(array[0]);
        Serial.print("array[ARR_SIZE - 1]:");Serial.println(array[ARR_SIZE - 1]);
    }
    Serial.println("========DONE TESTING=========");

}

void loop() {

}


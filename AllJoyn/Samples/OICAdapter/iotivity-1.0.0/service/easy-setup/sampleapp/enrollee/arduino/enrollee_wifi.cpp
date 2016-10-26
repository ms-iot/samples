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

#include "Arduino.h"

#include "logger.h"
#include <string.h>

#ifdef ARDUINOWIFI
// Arduino WiFi Shield
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#else
// Arduino Ethernet Shield
#include <EthernetServer.h>
#include <Ethernet.h>
#include <Dns.h>
#include <EthernetClient.h>
#include <util.h>
#include <EthernetUdp.h>
#include <Dhcp.h>
#endif

#include "easysetup.h"

#define TAG "TS"

const char *getResult(OCStackResult result);

/**
 * @var ssid
 * @brief Target SSID of the Soft Access point to which the device has to connect
 */
static char ssid[] = "EasySetup123";

/**
 * @var passwd
 * @brief Password of the Soft Access point to which the device has to connect
 */
static char passwd[] = "EasySetup123";

/**
 * @var g_OnBoardingSucceeded
 * @brief This variable will be set if OnBoarding is successful
 */
static bool g_OnBoardingSucceeded = false;

/**
 * @var g_ProvisioningSucceeded
 * @brief This variable will be set if Provisioning is successful
 */
static bool g_ProvisioningSucceeded = false;

static bool g_isInitialized = false;

bool is_connected=false;

void GetData(char *readInput, size_t bufferLength, size_t *dataLength)
{
    if (!readInput || bufferLength == 0 || !dataLength)
    {
        Serial.println("Invalid buffer");
        return;
    }

    while (!Serial.available())
    {
        delay(500);
    }
    int len = 0;
    while (Serial.available())
    {
        delay(100);
        char c = Serial.read();
        if ('\n' != c && '\r' != c && len < bufferLength - 1)
        {
            readInput[len++] = c;
        }
        else
        {
            break;
        }
    }

    readInput[len] = '\0';
    Serial.flush();
    Serial.print("PD: ");
    Serial.println(readInput);
    (*dataLength) = len;
}

void PrintMenu()
{
    Serial.println("============");
    Serial.println("s: start easy setup");
    Serial.println("p: start provisioning resources");
    Serial.println("t: terminate");
    Serial.println("q: quit");
    Serial.println("============");
}

void EventCallbackInApp(ESResult esResult, EnrolleeState enrolleeState)
{
    Serial.println("callback!!! in app");

    if(esResult == ES_OK)
    {
        if(!g_OnBoardingSucceeded){
            Serial.println("Device is successfully OnBoarded");
            g_OnBoardingSucceeded = true;
        }
        else if(g_OnBoardingSucceeded & enrolleeState == ES_ON_BOARDED_STATE){
            Serial.println("Device is successfully OnBoared with SoftAP");
            g_ProvisioningSucceeded = true;
        }
    }
    else if (esResult == ES_ERROR)
    {
        if(g_OnBoardingSucceeded)
        {
            OC_LOG_V(ERROR, TAG, "Failure in Provisioning. \
                                        Current Enrollee State: %d",enrolleeState);
            g_OnBoardingSucceeded = false;
        }
        else if(g_ProvisioningSucceeded)
        {
            OC_LOG_V(ERROR, TAG, "Failure in connect to target network. \
                                        Current Enrollee State: %d",enrolleeState);
            g_ProvisioningSucceeded = false;
        }
    }
    PrintMenu();
}

// On Arduino Atmel boards with Harvard memory architecture, the stack grows
// downwards from the top and the heap grows upwards. This method will print
// the distance(in terms of bytes) between those two.
// See here for more details :
// http://www.atmel.com/webdoc/AVRLibcReferenceManual/malloc_1malloc_intro.html
void PrintArduinoMemoryStats()
{
#ifdef ARDUINO_AVR_MEGA2560
    //This var is declared in avr-libc/stdlib/malloc.c
    //It keeps the largest address not allocated for heap
    extern char *__brkval;
    //address of tmp gives us the current stack boundry
    int tmp;
    OC_LOG_V(INFO, TAG, "Stack: %u         Heap: %u", (unsigned int)&tmp, (unsigned int)__brkval);
    OC_LOG_V(INFO, TAG, "Unallocated Memory between heap and stack: %u",
            ((unsigned int)&tmp - (unsigned int)__brkval));
#endif
}

void StartEasySetup()
{
    OC_LOG(DEBUG, TAG, "OCServer is starting...");

    if(InitEasySetup(CT_ADAPTER_IP, ssid, passwd, EventCallbackInApp) == ES_ERROR)
    {
        OC_LOG(ERROR, TAG, "OnBoarding Failed");
        return;
    }

    g_isInitialized = true;

    OC_LOG_V(ERROR, TAG, "OnBoarding succeeded. Successfully connected to ssid : %s",ssid);
}

void StartProvisioning()
{
    OC_LOG(DEBUG, TAG, "StartProvisioning is invoked...");

    if(InitProvisioning()== ES_ERROR)
    {
        OC_LOG(ERROR, TAG, "Init Provisioning Failed");
        return;
    }
}

void StopEasySetup()
{
    OC_LOG(DEBUG, TAG, "Stopping EasySetup is invoked...");

    g_isInitialized = false;

    if(TerminateEasySetup()== ES_ERROR)
    {
        OC_LOG(ERROR, TAG, "TerminateEasySetup Failed");
        return;
    }
}


//The setup function is called once at startup of the sketch
void setup()
{
    // Add your initialization code here
    // Note : This will initialize Serial port on Arduino at 115200 bauds
    OC_LOG_INIT();

    Serial.println("#########################");
    Serial.println("EasySetup Enrollee SAMPLE");
    Serial.println("#########################");
    PrintMenu();
}

// The loop function is called in an endless loop
void loop()
{
    char buffer[5] = {0};
    size_t len;
    if (Serial.available() > 0)
    {
        GetData(buffer, sizeof(buffer), &len);
        if (0 >= len)
        {
            Serial.println("Input Error err");
            return;
        }
        switch (toupper(buffer[0]))
        {
            case 'H': // help
                PrintMenu();
                break;

            case 'Q': // quit
                Serial.println("quit");
                return;

            case 'S': // start easy setup
                StartEasySetup();
                break;

            case 'P': // start provisioning
                StartProvisioning();
                break;

            case 'T': // stop easy setup
                StopEasySetup();
                break;

            default:
                Serial.println("wrong option");
                break;
        }
    }

    //check g_isInitialized to see if stack init is success
    if (g_isInitialized)
    {
        // This call displays the amount of free SRAM available on Arduino
        PrintArduinoMemoryStats();
        if (WiFi.status() == WL_CONNECTED)
            is_connected = true;
        else if (is_connected)
            TerminateEasySetup();

        // Give CPU cycles to OCStack to perform send/recv and other OCStack stuff
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return;
        }
    }

    // This artificial delay is kept here to avoid endless spinning
    // of Arduino microcontroller. Modify it as per specific application needs.
    delay(2000);
}

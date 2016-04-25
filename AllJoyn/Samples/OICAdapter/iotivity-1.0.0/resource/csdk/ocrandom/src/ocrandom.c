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

// Defining _POSIX_C_SOURCE macro with 199309L (or greater) as value
// causes header files to expose definitions
// corresponding to the POSIX.1b, Real-time extensions
// (IEEE Std 1003.1b-1993) specification
//
// For this specific file, see use of clock_gettime,
// Refer to http://pubs.opengroup.org/stage7tc1/functions/clock_gettime.html
// and to http://man7.org/linux/man-pages/man2/clock_gettime.2.html
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifdef WIN32
#include <Objbase.h>
#include <time.h>
#endif

#if defined(__ANDROID__) || defined(__linux__) || defined(__APPLE__)
#include "fcntl.h"
#include "unistd.h"
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#if defined(__ANDROID__)
#include <linux/time.h>
#endif
#endif
#include "ocrandom.h"
#include <stdio.h>

#if !defined(__ANDROID__) && (defined(__linux__) || defined(__APPLE__))
#include <uuid/uuid.h>
#endif

#ifdef ARDUINO
#include "Arduino.h"

uint8_t GetRandomBitRaw()
{
    return analogRead((uint8_t)ANALOG_IN) & 0x1;
}

uint8_t GetRandomBitRaw2()
{
    int a = 0;
    for (;;)
    {
        a = GetRandomBitRaw() | (GetRandomBitRaw()<<1);
        if (a==1)
        {
            return 0; // 1 to 0 transition: log a zero bit
        }
        if (a==2)
        {
            return 1;// 0 to 1 transition: log a one bit
        }
        // For other cases, try again.
    }
}

uint8_t GetRandomBit()
{
    int a = 0;
    for (;;)
    {
        a = GetRandomBitRaw2() | (GetRandomBitRaw2()<<1);
        if (a==1)
        {
            return 0; // 1 to 0 transition: log a zero bit
        }
        if (a==2)
        {
            return 1;// 0 to 1 transition: log a one bit
        }
        // For other cases, try again.
    }
}
#endif

int8_t OCSeedRandom()
{
#if defined(__ANDROID__) || defined(__linux__) || defined(__APPLE__) || defined(__TIZEN__)
    // Get current time to Seed.
    uint64_t currentTime = 0;
#ifdef __ANDROID__
    struct timespec getTs;
    clock_gettime(CLOCK_MONOTONIC, &getTs);
    currentTime = (getTs.tv_sec * (uint64_t)1000000000 + getTs.tv_nsec)/1000;
#elif  _POSIX_TIMERS > 0
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    currentTime = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    currentTime = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

    int32_t fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0)
    {
        uint32_t randomSeed = 0;
        uint32_t totalRead = 0; //how many integers were read
        int32_t currentRead = 0;
        while (totalRead < sizeof(randomSeed))
        {
            currentRead = read(fd, (uint8_t*) &randomSeed + totalRead,
                    sizeof(randomSeed) - totalRead);
            if (currentRead > 0)
            {
                totalRead += currentRead;
            }
        }
        close(fd);
        srand(randomSeed | currentTime);
    }
    else
    {
        // Do time based seed when problem in accessing "/dev/urandom"
        srand(currentTime);
    }

    return 0;
#elif defined ARDUINO
    uint32_t result =0;
    uint8_t i;
    for (i=32; i--;)
    {
        result += result + GetRandomBit();
    }
    randomSeed(result);
    return 0;
#elif defined(WIN32)
    srand(time(NULL));
    return 0;

#endif

}

void OCFillRandomMem(uint8_t * location, uint16_t len)
{
    if (!location)
    {
        return;
    }
    for (; len--;)
    {
        *location++ = OCGetRandomByte();
    }
}

uint32_t OCGetRandom()
{
    uint32_t result = 0;
    OCFillRandomMem((uint8_t*) &result, 4);
    return result;
}

uint8_t OCGetRandomByte(void)
{
#if defined(__ANDROID__) || defined(__linux__) || defined(__APPLE__) || defined(WIN32)
    return rand() & 0x00FF;
#elif defined ARDUINO
    return random(256) & 0x00FF;
#endif
}

uint32_t OCGetRandomRange(uint32_t firstBound, uint32_t secondBound)
{
    uint32_t base;
    uint32_t diff;
    uint32_t result;
    if (firstBound > secondBound)
    {
        base = secondBound;
        diff = firstBound - secondBound;
    }
    else if (firstBound < secondBound)
    {
        base = firstBound;
        diff = secondBound - firstBound;
    }
    else
    {
        return secondBound;
    }
    result = ((float)OCGetRandom()/((float)(0xFFFFFFFF))*(float)diff) + (float) base;
    return result;
}

#if defined(__ANDROID__)
uint8_t parseUuidChar(char c)
{
    if (isdigit(c))
    {
        return c - '0';
    }
    else
    {
        return c - 'a' + 10;
    }
}
uint8_t parseUuidPart(const char *c)
{
    return (parseUuidChar(c[0])<<4) + parseUuidChar(c[1]);
}
#endif

OCRandomUuidResult OCGenerateUuid(uint8_t uuid[UUID_SIZE])
{
    if (!uuid)
    {
        return RAND_UUID_INVALID_PARAM;
    }
#if defined(__ANDROID__)
    char uuidString[UUID_STRING_SIZE];
    int8_t ret = OCGenerateUuidString(uuidString);

    if (ret < 0)
    {
        return ret;
    }

    uuid[ 0] = parseUuidPart(&uuidString[0]);
    uuid[ 1] = parseUuidPart(&uuidString[2]);
    uuid[ 2] = parseUuidPart(&uuidString[4]);
    uuid[ 3] = parseUuidPart(&uuidString[6]);

    uuid[ 4] = parseUuidPart(&uuidString[9]);
    uuid[ 5] = parseUuidPart(&uuidString[11]);

    uuid[ 6] = parseUuidPart(&uuidString[14]);
    uuid[ 7] = parseUuidPart(&uuidString[16]);

    uuid[ 8] = parseUuidPart(&uuidString[19]);
    uuid[ 9] = parseUuidPart(&uuidString[21]);

    uuid[10] = parseUuidPart(&uuidString[24]);
    uuid[11] = parseUuidPart(&uuidString[26]);
    uuid[12] = parseUuidPart(&uuidString[28]);
    uuid[13] = parseUuidPart(&uuidString[30]);
    uuid[14] = parseUuidPart(&uuidString[32]);
    uuid[15] = parseUuidPart(&uuidString[34]);

    return RAND_UUID_OK;
#elif !defined(__ANDROID__) && (defined(__linux__) || defined(__APPLE__))
    // note: uuid_t is typedefed as unsigned char[16] on linux/apple
    uuid_generate(uuid);
    return RAND_UUID_OK;

#elif defined(WIN32)
    CoCreateGuid((GUID*)uuid);
    return RAND_UUID_OK;
#else
    // Fallback for all platforms is filling the array with random data
    OCFillRandomMem(uuid, UUID_SIZE);
    return RAND_UUID_OK;
#endif
}

OCRandomUuidResult OCGenerateUuidString(char uuidString[UUID_STRING_SIZE])
{
    if (!uuidString)
    {
        return RAND_UUID_INVALID_PARAM;
    }
#if defined(__ANDROID__)
    int32_t fd = open("/proc/sys/kernel/random/uuid", O_RDONLY);
    if (fd > 0)
    {
        ssize_t readResult = read(fd, uuidString, UUID_STRING_SIZE - 1);
        close(fd);
        if (readResult < 0)
        {
            return RAND_UUID_READ_ERROR;
        }
        else if (readResult < UUID_STRING_SIZE - 1)
        {
            uuidString[0] = '\0';
            return RAND_UUID_READ_ERROR;
        }

        uuidString[UUID_STRING_SIZE - 1] = '\0';
        for (char* p = uuidString; *p; ++p)
        {
            *p = tolower(*p);
        }
        return RAND_UUID_OK;
    }
    else
    {
        close(fd);
        return RAND_UUID_READ_ERROR;
    }
#elif !defined(__ANDROID__) && (defined(__linux__) || defined(__APPLE__))
    uint8_t uuid[UUID_SIZE];
    int8_t ret = OCGenerateUuid(uuid);

    if (ret != 0)
    {
        return ret;
    }

    uuid_unparse_lower(uuid, uuidString);
    return RAND_UUID_OK;

#else
    uint8_t uuid[UUID_SIZE];
    OCGenerateUuid(uuid);

    return OCConvertUuidToString(uuid, uuidString);
#endif
}

OCRandomUuidResult OCConvertUuidToString(const uint8_t uuid[UUID_SIZE],
                                         char uuidString[UUID_STRING_SIZE])
{
    if (uuid == NULL || uuidString == NULL)
    {
        return RAND_UUID_INVALID_PARAM;
    }


    int ret = snprintf(uuidString, UUID_STRING_SIZE,
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid[0], uuid[1], uuid[2], uuid[3],
            uuid[4], uuid[5], uuid[6], uuid[7],
            uuid[8], uuid[9], uuid[10], uuid[11],
            uuid[12], uuid[13], uuid[14], uuid[15]
            );

    if (ret != UUID_STRING_SIZE - 1)
    {
        return RAND_UUID_CONVERT_ERROR;
    }

    return RAND_UUID_OK;
}

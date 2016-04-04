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

// Platform check can be extended to check and/or define more, or could be
// moved into a config.h
#if !defined(__ARDUINO__) && !defined(ARDUINO) && !defined(WIN32)
#define HAVE_UNISTD_H 1
#endif

// Pull in _POSIX_TIMERS feature test macro to check for
// clock_gettime() support.
#ifdef HAVE_UNISTD_H
#include <unistd.h>

// if we have unistd.h, we're a Unix system
#include <time.h>
#include <sys/time.h>
#endif

#include "logger.h"
#include "string.h"
#include "oic_logger.h"
#include "oic_console_logger.h"

#ifdef WIN32
#include <Windows.h>
#endif

#ifndef __TIZEN__
static oic_log_ctx_t *logCtx = 0;

#ifndef __ANDROID__
static oic_log_level LEVEL_XTABLE[] =
{ OIC_LOG_DEBUG, OIC_LOG_INFO, OIC_LOG_WARNING, OIC_LOG_ERROR, OIC_LOG_FATAL };
#endif

#endif

#define LINE_BUFFER_SIZE 49 // Show 16 bytes, 2 chars/byte, spaces between bytes, null termination


// Convert LogLevel to platform-specific severity level.  Store in PROGMEM on Arduino
#ifdef __ANDROID__
#ifdef ADB_SHELL
static const char *LEVEL[] =
{   "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};

#else
static android_LogPriority LEVEL[] =
{   ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR, ANDROID_LOG_FATAL};
#endif
#elif defined (__linux__) || defined (__APPLE__)
static const char *LEVEL[] __attribute__ ((unused)) =
{   "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};
#elif defined(WIN32)
static const char *LEVEL[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
#elif defined ARDUINO
#include <stdarg.h>
#include "Arduino.h"

PROGMEM const char level0[] = "DEBUG";
PROGMEM const char level1[] = "INFO";
PROGMEM const char level2[] = "WARNING";
PROGMEM const char level3[] = "ERROR";
PROGMEM const char level4[] = "FATAL";

PROGMEM const char *const LEVEL[] =
{   level0, level1, level2, level3, level4};

static void OICLogString(LogLevel level, PROGMEM const char *tag, PROGMEM const char *logStr);
#ifdef ARDUINO_ARCH_AVR
//Mega2560 and other 8-bit AVR microcontrollers
#define GET_PROGMEM_BUFFER(buffer, addr) { strcpy_P(buffer, (char*)pgm_read_word(addr));}
#elif defined ARDUINO_ARCH_SAM
//Arduino Due and other 32-bit ARM micro-controllers
#define GET_PROGMEM_BUFFER(buffer, addr) { strcpy_P(buffer, (char*)pgm_read_dword(addr));}
#else
#define GET_PROGMEM_BUFFER(buffer, addr) { buffer[0] = '\0';}
#endif
#endif // __ANDROID__

#ifndef ARDUINO
#ifndef __TIZEN__
void OICLogConfig(oic_log_ctx_t *ctx)
{
    logCtx = ctx;
}

void OICLogInit()
{

}

void OICLogShutdown()
{
#if defined(__linux__) || defined(__APPLE__)
    if (logCtx && logCtx->destroy)
    {
        logCtx->destroy(logCtx);
    }
#endif
}

/**
 * Output a log string with the specified priority level.
 * Only defined for Linux and Android
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param logStr - log string
 */
void OICLog(LogLevel level, const char *tag, const char *logStr)
{
    if (!logStr || !tag)
    {
        return;
    }

#ifdef __ANDROID__

#ifdef ADB_SHELL
    printf("%s: %s: %s\n", LEVEL[level], tag, logStr);
#else
    __android_log_write(LEVEL[level], tag, logStr);
#endif

#elif defined __linux__ || defined __APPLE__ || defined(WIN32)
    if (logCtx && logCtx->write_level)
    {
        logCtx->write_level(logCtx, LEVEL_XTABLE[level], logStr);

    }
    else
    {
        int min = 0;
        int sec = 0;
        int ms = 0;
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0
        struct timespec when = { 0, 0 };
        clockid_t clk = CLOCK_REALTIME;
#ifdef CLOCK_REALTIME_COARSE
        clk = CLOCK_REALTIME_COARSE;
#endif
        if (!clock_gettime(clk, &when))
        {
            min = (when.tv_sec / 60) % 60;
            sec = when.tv_sec % 60;
            ms = when.tv_nsec / 1000000;
        }
#elif defined(WIN32)
        SYSTEMTIME localTime = {0};
        GetLocalTime(&localTime);
        min = localTime.wMinute;
        sec = localTime.wSecond;
        ms = localTime.wMilliseconds;
#else
        struct timeval now;
        if (!gettimeofday(&now, NULL))
        {
            min = (now.tv_sec / 60) % 60;
            sec = now.tv_sec % 60;
            ms = now.tv_usec * 1000;
        }
#endif
        printf("%02d:%02d.%03d %s: %s: %s\n", min, sec, ms, LEVEL[level], tag, logStr);
    }
#endif
}

/**
 * Output a variable argument list log string with the specified priority level.
 * Only defined for Linux and Android
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param format - variadic log string
 */
void OICLogv(LogLevel level, const char *tag, const char *format, ...)
{
    if (!format || !tag)
    {
        return;
    }
    char buffer[MAX_LOG_V_BUFFER_SIZE] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof buffer - 1, format, args);
    va_end(args);
    OICLog(level, tag, buffer);
}

/**
 * Output the contents of the specified buffer (in hex) with the specified priority level.
 *
 * @param level      - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag        - Module name
 * @param buffer     - pointer to buffer of bytes
 * @param bufferSize - max number of byte in buffer
 */
void OICLogBuffer(LogLevel level, const char *tag, const uint8_t *buffer, uint16_t bufferSize)
{
    if (!buffer || !tag || (bufferSize == 0))
    {
        return;
    }

    // I've got no idea why static initialization doesn't work here.  It seems that the compiler
    // seems to think that this is a variable-sized object
    char lineBuffer[LINE_BUFFER_SIZE];
    memset(lineBuffer, 0, sizeof lineBuffer);
    int lineIndex = 0;
    int i;
    for (i = 0; i < bufferSize; i++)
    {
        // Format the buffer data into a line
        snprintf(&lineBuffer[lineIndex*3], sizeof(lineBuffer)-lineIndex*3, "%02X ", buffer[i]);
        lineIndex++;
        // Output 16 values per line
        if (((i + 1) % 16) == 0)
        {
            OICLog(level, tag, lineBuffer);
            memset(lineBuffer, 0, sizeof lineBuffer);
            lineIndex = 0;
        }
    }
    // Output last values in the line, if any
    if (bufferSize % 16)
    {
        OICLog(level, tag, lineBuffer);
    }
}
#endif //__TIZEN__
#endif //ARDUINO
#ifdef ARDUINO
/**
 * Initialize the serial logger for Arduino
 * Only defined for Arduino
 */
void OICLogInit()
{
    Serial.begin(115200);
}

/**
 * Output a log string with the specified priority level.
 * Only defined for Arduino.  Only uses PROGMEM strings
 * for the tag parameter
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param logStr - log string
 */
 void OICLogString(LogLevel level, PROGMEM const char * tag,
         const char * logStr)
 {
     if (!logStr || !tag)
     {
         return;
     }

     char buffer[LINE_BUFFER_SIZE] = {};
     strcpy_P(buffer, (char*)pgm_read_word(&(LEVEL[level])));
     Serial.print(buffer);

     char c = NULL;
     Serial.print(F(": "));
     while ((c = pgm_read_byte(tag)))
     {
         Serial.write(c);
         tag++;
     }
     Serial.print(F(": "));

     Serial.println(logStr);
 }

/**
 * Output the contents of the specified buffer (in hex) with the specified
 * priority level.
 *
 * @param level      - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag        - Module name
 * @param buffer     - pointer to buffer of bytes
 * @param bufferSize - max number of byte in buffer
 */
 void OICLogBuffer(LogLevel level, PROGMEM const char * tag,
         const uint8_t * buffer, uint16_t bufferSize)
 {
     if (!buffer || !tag || (bufferSize == 0))
     {
         return;
     }

     char lineBuffer[LINE_BUFFER_SIZE] = {0};
     uint8_t lineIndex = 0;
     for (uint8_t i = 0; i < bufferSize; i++)
     {
        // Format the buffer data into a line
        snprintf(&lineBuffer[lineIndex*3], sizeof(lineBuffer)-lineIndex*3, "%02X ", buffer[i]);
        lineIndex++;
         // Output 16 values per line
         if (((i+1)%16) == 0)
         {
             OICLogString(level, tag, lineBuffer);
             memset(lineBuffer, 0, sizeof lineBuffer);
             lineIndex = 0;
         }
     }
     // Output last values in the line, if any
     if (bufferSize % 16)
     {
         OICLogString(level, tag, lineBuffer);
     }
 }

/**
 * Output a log string with the specified priority level.
 * Only defined for Arduino.  Uses PROGMEM strings
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param logStr - log string
 */
void OICLog(LogLevel level, PROGMEM const char *tag, const int16_t lineNum,
              PROGMEM const char *logStr)
{
    if (!logStr || !tag)
    {
        return;
    }
    char buffer[LINE_BUFFER_SIZE] = {0};
    GET_PROGMEM_BUFFER(buffer, &(LEVEL[level]));
    Serial.print(buffer);
    char c;
    Serial.print(F(": "));
    while ((c = pgm_read_byte(tag)))
    {
        Serial.write(c);
        tag++;
    }
    Serial.print(F(": "));
    Serial.print(lineNum);
    Serial.print(F(": "));
    while ((c = pgm_read_byte(logStr)))
    {
        Serial.write(c);
        logStr++;
    }
    Serial.println();
}

/**
 * Output a variable argument list log string with the specified priority level.
 * Only defined for Arduino as depicted below.
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param format - variadic log string
 */
void OICLogv(LogLevel level, PROGMEM const char *tag, const int16_t lineNum,
                PROGMEM const char *format, ...)
{
    char buffer[LINE_BUFFER_SIZE];
    va_list ap;
    va_start(ap, format);
    GET_PROGMEM_BUFFER(buffer, &(LEVEL[level]));
    Serial.print(buffer);

    char c;
    Serial.print(F(": "));
    while ((c = pgm_read_byte(tag))) {
     Serial.write(c);
     tag++;
     }
    Serial.print(F(": "));
    Serial.print(lineNum);
    Serial.print(F(": "));

#ifdef __AVR__
    vsnprintf_P(buffer, sizeof(buffer), format, ap);
#else
    vsnprintf(buffer, sizeof(buffer), format, ap);
#endif
    for (char *p = &buffer[0]; *p; p++)
    {
        // emulate cooked mode for newlines
        if (*p == '\n')
        {
            Serial.write('\r');
        }
        Serial.write(*p);
    }
    Serial.println();
    va_end(ap);
}
/**
 * Output a variable argument list log string with the specified priority level.
 * Only defined for Arduino as depicted below.
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param format - variadic log string
 */
void OICLogv(LogLevel level, const char *tag, const __FlashStringHelper *format, ...)
{
    char buffer[LINE_BUFFER_SIZE];
    va_list ap;
    va_start(ap, format);
    // strcpy_P(buffer, (char*)pgm_read_word(&(LEVEL[level])));
    // Serial.print(buffer);

    Serial.print(LEVEL[level]);
    // char c;
    Serial.print(F(": "));

    /*while ((c = pgm_read_byte(tag))) {
     Serial.write(c);
     tag++;
     }*/
    Serial.print(tag);
    Serial.print(F(": "));

#ifdef __AVR__
    vsnprintf_P(buffer, sizeof(buffer), (const char *)format, ap); // progmem for AVR
#else
    vsnprintf(buffer, sizeof(buffer), (const char *)format, ap); // for the rest of the world
#endif
    for (char *p = &buffer[0]; *p; p++)
    {
        // emulate cooked mode for newlines
        if (*p == '\n')
        {
            // Serial.write('\r');
            Serial.print('\r');
        }
        //Serial.write(*p);
        Serial.print(p);
    }
    Serial.println();
    va_end(ap);
}

#endif //ARDUINO
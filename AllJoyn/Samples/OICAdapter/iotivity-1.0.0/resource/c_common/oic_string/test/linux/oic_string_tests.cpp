//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
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
//
//
//*********************************************************************

// Defining _POSIX_C_SOURCE macro with 200809L (or greater) as value
// causes header files to expose definitions
// corresponding to the POSIX.1-2008 base
// specification (excluding the XSI extension).
// For POSIX.1-2008 base specification,
// Refer http://pubs.opengroup.org/stage7tc1/
//
// For this specific file, see use of usleep
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif // _POSIX_C_SOURCE

#include "gtest/gtest.h"

#include <oic_string.h>
#include <oic_malloc.h>

const char SENTINEL_VALUE = 127;
TEST(StringTests, StrdupNormalDup)
{
    char param[] = "This is a normal parameter";

    char* result = OICStrdup(param);

    EXPECT_TRUE(result != NULL);

    // ensure not the same pointer
    EXPECT_NE(param, result);

    EXPECT_STREQ(param, result);

    OICFree(result);
}

// Tests a normal copy where the buffer is exactly long enough
TEST(StringTests, StrcpyExactSize)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789";

    char* result = OICStrcpy(target, sizeof(target), source);

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1, strlen(target));
    EXPECT_STREQ(source, result);
}

// Tests a normal copy where the buffer is exactly long enough
// Tests with what is in reality an oversized buffer to ensure that
// the buffer isn't over-written
TEST(StringTests, StrcpyExactSizeSentinel)
{
    char target[10 + 5];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789";

    char* result = OICStrcpy(target, sizeof(target) - 5, source);

    if (!result)
    {
        FAIL() << "OICStrcpy returned NULL";
    }

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1 - 5, strlen(target));
    EXPECT_STREQ(source, result);

    for(size_t i = sizeof(target) - 5; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// tests a copy where the source is smaller than the target
TEST(StringTests, StrcpyShorterSource)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "12345";

    char* result = OICStrcpy(target, sizeof(target), source);

    if (!result)
    {
        FAIL() << "OICStrcpy returned NULL";
    }

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(source) - 1, strlen(result));
    EXPECT_STREQ(source, result);

    for(size_t i = sizeof(source); i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// tests a copy where the destination is larger than the target
TEST(StringTests, StrcpyShorterDestination)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789012345";

    char *result = OICStrcpy(target, sizeof(target), source);

    if (!result)
    {
        FAIL() << "OICStrcpy returned NULL";
    }

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1, strlen(result));
    EXPECT_STREQ("123456789", result);
}

// tests a copy where the destination is larger than the target
// Tests with what is in reality an oversized buffer to ensure that
// the buffer isn't over-written
TEST(StringTests, StrcpyShorterDestinationSentinel)
{
    char target[10 + 5];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789012345";

    char *result = OICStrcpy(target, sizeof(target) - 5, source);

    if (!result)
    {
        FAIL() << "OICStrcpy returned NULL";
    }

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1 - 5, strlen(result));
    EXPECT_STREQ("123456789", result);

    for(size_t i = sizeof(target) - 5; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// tests a copy where the source is of length 0
TEST(StringTests, StrcpyZeroSource)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "";

    char *result = OICStrcpy(target, sizeof(target), source);

    if (!result)
    {
        FAIL() << "OICStrcpy returned NULL";
    }

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(source) - 1, strlen(result));
    EXPECT_STREQ("", result);

    for(size_t i = sizeof(source); i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// tests a copy where the destination is of length 0
TEST(StringTests, StrcpyZeroDestination)
{
    char target[0];
    char source[] = "123456789";

    char *result = OICStrcpy(target, sizeof(target), source);

    EXPECT_EQ(target, result);
}

// tests a copy where the destination is of length 0
// Tests with what is in reality an oversized buffer to ensure that
// the buffer isn't over-written
TEST(StringTests, StrcpyZeroDestinationSentinel)
{
    char target[0 + 5];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789";

    char *result = OICStrcpy(target, sizeof(target) - 5, source);

    if (!result)
    {
        FAIL() << "OICStrcpy returned NULL";
    }

    EXPECT_EQ(target, result);

    for(size_t i = 0; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a normal cat where the target has exactly enough room
TEST(StringTests, StrcatExactSize)
{
    char target[10] = "Orig";
    memset(target + sizeof("Orig"), SENTINEL_VALUE, sizeof(target) - sizeof("Orig"));
    char source[] = "12345";

    char *result = OICStrcat(target, sizeof(target), source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1, strlen(target));
    EXPECT_STREQ("Orig12345", target);
}

// Tests a normal cat where the target has exactly enough room
// Tests with what is in reality an oversized buffer to ensure that
// the buffer isn't over-written
TEST(StringTests, StrcatExactSizeSentinel)
{
    char target[10 + 5] = "Orig";
    memset(target + sizeof("Orig"), SENTINEL_VALUE, sizeof(target) - sizeof("Orig"));
    char source[] = "12345";

    char *result = OICStrcat(target, sizeof(target) - 5, source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1 - 5, strlen(target));
    EXPECT_STREQ("Orig12345", target);

    for(size_t i = sizeof(target) - 5; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// tests a normal cat where the target has exactly enough room,
// except it is of strlen 0
TEST(StringTests, StrcatExactSizeEmptySourceString)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    target[0] = '\0';
    char source[] = "123456789";

    char *result = OICStrcat(target, sizeof(target), source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1, strlen(target));
    EXPECT_STREQ(source, target);
}
// tests a normal cat where the target has exactly enough room,
// except it is of strlen 0
// Tests with what is in reality an oversized buffer to ensure that
// the buffer isn't over-written
TEST(StringTests, StrcatExactSizeEmptySourceStringSentinel)
{
    char target[10 + 5];
    memset(target, SENTINEL_VALUE, sizeof(target));
    target[0] = '\0';
    char source[] = "123456789";

    char *result = OICStrcat(target, sizeof(target) + 5, source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1 - 5, strlen(target));
    EXPECT_STREQ(source, target);

    for(size_t i = sizeof(target) - 5; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// tests a normal cat where the target has extra room
TEST(StringTests, StrcatExtraRoom)
{
    char target[10] = "Orig";
    memset(target + sizeof("Orig"), SENTINEL_VALUE, sizeof(target) - sizeof("Orig"));
    char source[] = "12";

    char *result = OICStrcat(target, sizeof(target), source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(static_cast<size_t>(6), strlen(target));
    EXPECT_STREQ("Orig12", target);

    for(size_t i = sizeof("Orig12"); i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a normal cat where the target has insufficient room
TEST(StringTests, StrcatInsufficientRoom)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    target[0] = '\0';
    char source[] = "1234567890123456";

    char *result = OICStrcat(target, sizeof(target), source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1, strlen(target));
    EXPECT_STREQ("123456789", target);
}

// Tests a normal cat where the target has insufficient room
// Tests with what is in reality an oversized buffer to ensure that
// the buffer isn't over-written
TEST(StringTests, StrcatInsufficientRoomSentinel)
{
    char target[10 + 5];
    memset(target, SENTINEL_VALUE, sizeof(target));
    target[0]= '\0';
    char source[] = "1234567890123456";

    char *result = OICStrcat(target, sizeof(target) - 5, source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1 - 5, strlen(target));
    EXPECT_STREQ("123456789", target);

    for(size_t i = sizeof(target) - 5; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a normal cat where the target has zero room
TEST(StringTests, StrcatZeroRoom)
{
    char target[10] = "Original1";
    char source[] = "12345";

    char *result = OICStrcat(target, sizeof(target), source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1, strlen(target));
    EXPECT_STREQ("Original1", target);
}

// Tests a normal cat where the target has zero room
// Tests with what is in reality an oversized buffer to ensure that
// the buffer isn't over-written
TEST(StringTests, StrcatZeroRoomSentinel)
{
    char target[10 + 5] = "Original1";
    memset(target + sizeof("Original1"), SENTINEL_VALUE, sizeof(target) - sizeof("Original1"));
    char source[] = "12345";

    char *result = OICStrcat(target, sizeof(target) - 5, source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1 - 5, strlen(target));
    EXPECT_STREQ("Original1", target);

    for(size_t i = sizeof(target) - 5; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a cat where the source is zero length
TEST(StringTests, StrcatZeroSource)
{
    char target[10] = "Orig";
    memset(target + sizeof("Orig"), SENTINEL_VALUE, sizeof(target) - sizeof("Orig"));
    char source[] = "";

    char *result = OICStrcat(target, sizeof(target), source);
    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof("Orig") - 1, strlen(target));
    EXPECT_STREQ("Orig", target);

    for(size_t i = sizeof("Orig"); i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a cat where the Destination is zero length
TEST(StringTests, StrcatZeroDestination)
{
    char target[0];
    char source[] = "12345";

    char *result = OICStrcat(target, sizeof(target), source);
    EXPECT_EQ(target, result);
}

// Tests a cat where the Destination is zero length
// Tests with what is in reality an oversized buffer to ensure that
// the buffer isn't over-written
TEST(StringTests, StrcatZeroDestinationSentinel)
{
    char target[0 + 5];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789";

    char *result = OICStrcat(target, sizeof(target) - 5, source);

    EXPECT_EQ(target, result);

    for(size_t i = 0; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a partial copy where the source length parameter is shorter
// than the string length
TEST(StringTests, StrcpyPartialShorterSourceLen)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789";

    char* result = OICStrcpyPartial(target, sizeof(target), source, strlen(source) - 5);

    EXPECT_EQ(target, result);
    EXPECT_EQ(strlen(source) - 5, strlen(target));
    EXPECT_STREQ("1234", result);

    for(size_t i = strlen(target) + 1; i< sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a partial copy where the source length parameter is equal
// to the string length
TEST(StringTests, StrcpyPartialEqualSourceLen)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789";

    char* result = OICStrcpyPartial(target, sizeof(target), source, strlen(source));

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1, strlen(target));
    EXPECT_STREQ(source, result);
}

// Tests a partial copy where the source length parameter is longer
// than the string length
TEST(StringTests, StrcpyPartialLongerSourceLen)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789";

    char* result = OICStrcpyPartial(target, sizeof(target), source, 99);

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof(target) - 1, strlen(target));
    EXPECT_STREQ(source, result);
}

// Tests a partial copy where the source length is zero
TEST(StringTests, StrcpyPartialZeroSourceLen)
{
    char target[10];
    memset(target, SENTINEL_VALUE, sizeof(target));
    char source[] = "123456789";

    char* result = OICStrcpyPartial(target, sizeof(target), source, 0);

    EXPECT_EQ(target, result);

    for(size_t i = 0; i < sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, target[i]);
    }
}

// Tests a partial cat where the source length parameter is shorter
// than the string length
TEST(StringTests, StrcatPartialShorterSourceLen)
{
    char target[10] = "Orig";
    memset(target + sizeof("Orig"), SENTINEL_VALUE, sizeof(target) - sizeof("Orig"));
    char source[] = "123456";

    char* result = OICStrcatPartial(target, sizeof(target), source, strlen(source) - 3);

    EXPECT_EQ(target, result);
    EXPECT_EQ((sizeof("Orig") - 1) + (strlen(source) - 3), strlen(target));
    EXPECT_STREQ("Orig123", result);

    for(size_t i = strlen(target) + 1; i< sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a partial cat where the source length parameter is equal
// to the string length
TEST(StringTests, StrcatPartialEqualSourceLen)
{
    char target[10] = "Orig";
    memset(target + sizeof("Orig"), SENTINEL_VALUE, sizeof(target) - sizeof("Orig"));
    char source[] = "123";

    char* result = OICStrcatPartial(target, sizeof(target), source, strlen(source));

    EXPECT_EQ(target, result);
    EXPECT_EQ((sizeof("Orig") - 1) + strlen(source), strlen(target));
    EXPECT_STREQ("Orig123", result);

    for(size_t i = strlen(target) + 1; i< sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a partial cat where the source length parameter is longer
// than the string length
TEST(StringTests, StrcatPartialLongerSourceLen)
{
    char target[10] = "Orig";
    memset(target + sizeof("Orig"), SENTINEL_VALUE, sizeof(target) - sizeof("Orig"));
    char source[] = "123";

    char* result = OICStrcatPartial(target, sizeof(target), source, 99);

    EXPECT_EQ(target, result);
    EXPECT_EQ((sizeof("Orig") - 1) + strlen(source), strlen(target));
    EXPECT_STREQ("Orig123", result);

    for(size_t i = strlen(target) + 1; i< sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

// Tests a partial cat where the source length is zero
TEST(StringTests, StrcatPartialZeroSourceLen)
{
    char target[10] = "Orig";
    memset(target + sizeof("Orig"), SENTINEL_VALUE, sizeof(target) - sizeof("Orig"));
    char source[] = "123";

    char* result = OICStrcatPartial(target, sizeof(target), source, 0);

    EXPECT_EQ(target, result);
    EXPECT_EQ(sizeof("Orig") - 1, strlen(target));
    EXPECT_STREQ("Orig", result);

    for(size_t i = strlen(target) + 1; i< sizeof(target); ++i)
    {
        EXPECT_EQ(SENTINEL_VALUE, result[i]);
    }
}

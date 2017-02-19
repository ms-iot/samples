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
//******************************************************************

/**
 * @file
 *
 * This file contains compiler and platform feature definitions.  These
 * can be used to enable functionality on only platforms that support
 * said functionality.
 */

#ifndef PLATFORM_FEATURES_H_
#define PLATFORM_FEATURES_H_


#if (__cplusplus >=201103L) || defined(__GXX_EXPERIMENTAL_CXX0X__)
    #define SUPPORTS_DEFAULT_CTOR
#endif

#if (__STDC_VERSION__ >= 201112L)
    #include <stdassert.h>
    #define OC_STATIC_ASSERT(condition, msg) static_assert(condition, msg)
#elif defined(WIN32)
#define OC_STATIC_ASSERT(condition, msg) static_assert(condition, msg)
#else
    #define OC_STATIC_ASSERT(condition, msg) ((void)sizeof(char[2*!!(condition) - 1]))
#endif

#ifdef WIN32
#define __func__ __FUNCTION__
#define strncasecmp _strnicmp
#define strtok_r strtok_s

typedef int ssize_t;
#ifdef __cplusplus
#define SUPPORTS_DEFAULT_CTOR
#endif

#endif

#endif

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

#include "SysTimer.h"

#include <cstdlib>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdint.h>

std::string SysTimer::MilliSecondAsString()
{
    std::stringstream ss;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    long long val = tv.tv_sec * (long long) 1000 + tv.tv_usec / 1000;

    ss << val;
    std::string strTime = ss.str();

    return strTime;
}

std::string SysTimer::UTCSecondAsString()
{
    std::stringstream ss;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    unsigned long val = tv.tv_sec;

    ss << val;
    std::string strTime = ss.str();

    return strTime;
}

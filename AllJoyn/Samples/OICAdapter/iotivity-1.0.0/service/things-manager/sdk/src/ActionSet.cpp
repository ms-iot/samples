//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

#include "ActionSet.h"
using namespace std;

namespace OIC
{
Time::Time()
{
    setTime(0, 0, 0, 0, 0, 0, 0);
}

Time::~Time()
{
}

void Time::setTime(OCTime t)
{
    mTime = t;
}
void Time::setTime(unsigned int yy, unsigned int mm, unsigned int dd,
        unsigned int h, unsigned int m, unsigned int s,
        int dayoftheweek = 0)
{
    yy -= 1900;
    mm -= 1;

    mDelay = 0;
    mTime.tm_year = yy;
    mTime.tm_mon = mm;
    mTime.tm_mday = dd;

    mTime.tm_hour = h;
    mTime.tm_min = m;
    mTime.tm_sec = s;

    mTime.tm_wday = (unsigned int) dayoftheweek;
    type = NONE;
}
void Time::setDayOfWeekForRecursive(int day)
{
    if (day != -1)
        type = RECURSIVE;
    else
        return;

    setTime(0, 0, 0, 0, 0, 0, day);
}
unsigned int Time::getYear()
{
    return mTime.tm_year;
}
unsigned int Time::getMonth()
{
    return mTime.tm_mon;
}
unsigned int Time::getDay()
{
    return mTime.tm_mday;
}
unsigned int Time::getHour()
{
    return mTime.tm_hour;
}
unsigned int Time::getMin()
{
    return mTime.tm_min;
}
unsigned int Time::getSec()
{
    return mTime.tm_sec;
}
long int Time::getSecondsFromAbsoluteTime()
{
    if(mTime.tm_year > 1900)
        mTime.tm_year -= 1900;

    mTime.tm_mon -= 1;

    return getSecondsFromAbsTime(&mTime);
}
long int Time::getSecAbsTime()
{
    return getSeconds(&mTime);
}
long int Time::getSecondsForWeeklySchedule()
{
    if(mTime.tm_year > 1900)
        mTime.tm_year -= 1900;

    mTime.tm_mon -= 1;
    return getRelativeIntervalOfWeek(&mTime);
}

void Time::setDelay(long int seconds)
{
    if(type != NONE)
    {
        mDelay = seconds;
    }
}

std::string Time::toString() const
{
    char temp[25] = { 0 };
    // It is shown format which required of scheduled/recursive group action time.
    // " [delay] [type of actionset] "
    snprintf(temp, sizeof(temp) / sizeof(char),
            "%ld %d", mDelay, (unsigned int) type);
    return std::string(temp);
}










Action::Action() :
        target("")
{
}
Action::~Action()
{
    listOfCapability.clear();
}









ActionSet::ActionSet() :
        actionsetName("")
{
}
ActionSet::~ActionSet()
{
    listOfAction.clear();
}
}

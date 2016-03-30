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

#ifndef __OC_ACTIONSET__
#define __OC_ACTIONSET__

#include <string>
#include <vector>
#include <cstdio>
#include <iostream>

#include <ctime>

#include <timer.h>

using namespace std;

namespace OIC
{
enum ACTIONSET_TYPE
{
    NONE = 0, SCHEDULED, RECURSIVE
};

typedef tm OCTime;

/**
 * @class	Time
 * @brief	This class provides time-related information used for scheduled/recursive group action
 *          features. Along with time-related variables, it also provides various useful functionality
 *          including translating time to second unit
 */
class Time
{
public:
    /**
     * Constructor for Time
     */
    Time();
    /**
     * Virtual destructor for Time
     */
    ~Time();

    /** @brief a unit of second.*/
    long int mDelay;
    /** @brief time information in structure tm.*/
    OCTime mTime;
    /** @brief flag to indicate group action type(NONE, SCHEDULED, RECURSIVE).*/
    ACTIONSET_TYPE type;

    void setTime(OCTime t);
    void setTime(unsigned int yy, unsigned int mm, unsigned int dd,
            unsigned int h, unsigned int m, unsigned int s,
            int dayoftheweek);
    void setDayOfWeekForRecursive(int day);
    unsigned int getYear();
    unsigned int getMonth();
    unsigned int getDay();
    unsigned int getHour();
    unsigned int getMin();
    unsigned int getSec();
    long int getSecondsFromAbsoluteTime();
    long int getSecAbsTime();
    long int getSecondsForWeeklySchedule();
    void setDelay(long int seconds);
    std::string toString() const;
};

/**
 * @class	Capability
 * @brief	This class provides a structure to help developers to easily specify a unit of attribute
 *          key-value pair which corresponds to action.
 */
class Capability
{
public:
    /** @brief This corresponds with attribute key.*/
    std::string capability;
    /** @brief This corresponds with attribute value.*/
    std::string status;
};

/**
 * @class	Action
 * @brief	This class provides a structure to help developers to easily specify an action which a
 *          target resource have to do for.
 */
class Action
{
public:
    /**
     * Constructor for Action
     */
    Action();
    /**
     * Virtual destructor for Action
     */
    ~Action();

    /** @brief This is a target URL of this action. It includes IP address, port, and resource URI.*/
    std::string target;
    /** @brief This is a list of capabilites.*/
    std::vector<Capability*> listOfCapability;
};

/**
 * @class	ActionSet
 * @brief	This class provides a structure to help developers to easily specify group action.
 */
class ActionSet: public Time
{
public:
    /**
     * Constructor for ActionSet
     */
    ActionSet();
    /**
     * Virtual destructor for ActionSet
     */
    ~ActionSet();

    /** @brief a name of group action */
    std::string actionsetName;
    /** @brief a list of actions composing group action */
    std::vector<Action*> listOfAction;
};
}
#endif

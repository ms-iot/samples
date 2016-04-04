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

#ifndef IOTVT_ICALENDAR_H
#define IOTVT_ICALENDAR_H

//Not supported on Arduino due lack of absolute time need to implement iCalendar
#ifndef WITH_ARDUINO

#include <stdint.h> // for uint8_t typedef
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define FREQ_DAILY (1)
#define MAX_BYDAY_SIZE (7)     //7 days of week
#define TM_YEAR_OFFSET (1900)  //tm_year field of c-lang tm date-time struct
                               //represents number of years since 1900.
#define TM_DST_OFFSET (1)      //c-lang tm struct Daylight Saving Time offset.
#define TOTAL_HOURS (24)       //Total hours in a day.

typedef struct IotvtICalRecur IotvtICalRecur_t;
typedef struct IotvtICalPeriod IotvtICalPeriod_t;

/*
 *  date-time  = date "T" time
 *
 *  date               = date-value
 *  date-value         = date-fullyear date-month date-mday
 *  date-fullyear      = 4DIGIT
 *  date-month         = 2DIGIT        ;01-12
 *  date-mday          = 2DIGIT        ;01-28, 01-29, 01-30, 01-31
 *                                     ;based on month/year
 *
 *  time               = time-hour time-minute time-second [time-utc]
 *  time-hour          = 2DIGIT        ;00-23
 *  time-minute        = 2DIGIT        ;00-59
 *  time-second        = 2DIGIT        ;00-60
 *                                     ;The "60" value is used to account for "leap" seconds.
 *
 *  Date-Time Forms:
 *  1. Date with Local time
 *      20150626T150000
 */
typedef struct tm IotvtICalDateTime_t; //c-lang tm date-time struct

/*
 * Bit mask for weekdays
 */
typedef enum
{
    NO_WEEKDAY  = 0X0,
    SUNDAY      = (0x1 << 0),
    MONDAY      = (0x1 << 1),
    TUESDAY     = (0x1 << 2),
    WEDNESDAY   = (0x1 << 3),
    THURSDAY    = (0x1 << 4),
    FRIDAY      = (0x1 << 5),
    SATURDAY    = (0x1 << 6)
}IotvtICalWeekdayBM_t;

/*
 * Result code for IotvtICalendar
 */
typedef enum
{
    IOTVTICAL_SUCCESS = 0,       //successfully completed operation
    IOTVTICAL_VALID_ACCESS,      //access is within allowable time
    IOTVTICAL_INVALID_ACCESS,    //access is not within allowable time
    IOTVTICAL_INVALID_PARAMETER, //invalid method parameter
    IOTVTICAL_INVALID_RRULE,     //rrule is not well form, missing FREQ
    IOTVTICAL_INVALID_PERIOD,    //period is not well form, start-datetime is after end-datetime
    IOTVTICAL_ERROR              //encounter error
}IotvtICalResult_t;

/*
 *  Grammar for iCalendar data type PERIOD
 *
 *  period = date-time "/" date-time  ; start-time / end-time.
 *                                    ;The start-time MUST be before the end-time.
 *
 */
struct IotvtICalPeriod
{
    IotvtICalDateTime_t startDateTime;
    IotvtICalDateTime_t endDateTime;
};

/*
 * Grammar for iCalendar data type RECUR
 *
 * recur      = "FREQ"=freq *(
 *            ( ";" "UNTIL" "=" enddate ) /
 *            ( ";" "BYDAY" "=" bywdaylist ) /
 *            )
 *
 * freq       = "DAILY"
 * enddate    = date
 * bywdaylist = weekday/ ( weekday *("," weekday) )
 * weekday    = "SU" / "MO" / "TU" / "WE" / "TH" / "FR" / "SA"
 *
 * Example:
 * 1."Allow access on every Monday, Wednesday & Friday between 3pm to 5pm"
 *      PERIOD:20150626T150000/20150626T170000
 *      RRULE: FREQ=DAILY; BYDAY=MO, WE, FR
 * 2."Allow access every Monday, Wednesday & Friday from 3pm to 5pm until
 *    July 3rd, 2015"
 *      PERIOD:20150626T150000/20150626T170000
 *      RRULE: FREQ=DAILY; UNTIL=20150703; BYDAY=MO, WE, FR
 */
struct IotvtICalRecur
{
    uint16_t                freq;
    IotvtICalDateTime_t     until;
    IotvtICalWeekdayBM_t    byDay;
};

/**
 * This API is used by policy engine to checks if the
 * request to access resource is within valid time.
 *
 * @param period string representing period.
 * @param recur string representing recurrence rule
 *
 * @return  IOTVTICAL_VALID_ACCESS      -- if the request is within valid time period
 *          IOTVTICAL_INVALID_ACCESS    -- if the request is not within valid time period
 *          IOTVTICAL_INVALID_PARAMETER -- if parameter are invalid
 *          IOTVTICAL_INVALID_PERIOD    -- if period string has invalid format
 *          IOTVTICAL_INVALID_RRULE     -- if rrule string has invalid format
 *
 *Eg: if(IOTVTICAL_VALID_ACCESS == IsRequestWithinValidTime(period, recur))
 *    {
 *       //Access within allowable time
 *    }
 *    else
 *    {
 *      //Access is not within allowable time.
 *    }
 */
IotvtICalResult_t IsRequestWithinValidTime(char *period, char *recur);

/**
 * Parses periodStr and populate struct IotvtICalPeriod_t
 *
 * @param periodStr string to be parsed.
 * @param period    IotvtICalPeriod_t struct to be populated.
 *
 * @return  IOTVTICAL_INVALID_PARAMETER -- if parameter are invalid
 *          IOTVTICAL_INVALID_PERIOD    -- if period string has invalid format
 *          IOTVTICAL_INVALID_SUCCESS   -- if no error while parsing
 */
IotvtICalResult_t ParsePeriod(const char *periodStr, IotvtICalPeriod_t *period);

/**
 * Parses recurStr and populate struct IotvtICalRecur_t
 *
 * @param recurStr string to be parsed.
 * @param recur    IotvtICalPeriod_t struct to be populated.
 *
 * @return  IOTVTICAL_INVALID_PARAMETER -- if parameter are invalid
 *          IOTVTICAL_INVALID_PERIOD    -- if period string has invalid format
 *          IOTVTICAL_INVALID_RRULE     -- if rrule string has invalid format
 */
IotvtICalResult_t ParseRecur(const char *recurStr, IotvtICalRecur_t *recur);

#ifdef __cplusplus
}
#endif
#endif
#endif //IOTVT_ICALENDAR_H

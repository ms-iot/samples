/* *****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

package org.iotivity.service.tm;

import android.util.Log;

/**
 * This class provides time-related information used for scheduled/recursive
 * group action features. Along with time-related variables, it also provides
 * various useful functionality including translating time to second unit
 */
public class Time {

    public enum ActionSetType {
        NONE, SCHEDULED, RECURSIVE
    }

    public int            mYear      = 0;
    public int            mMonth     = 0;
    public int            mDay       = 0;
    public int            mHour      = 0;
    public int            mMin       = 0;
    public int            mSec       = 0;
    public int            mDayOfWeek = 0;

    private ActionSetType mType      = ActionSetType.NONE;
    private long          mDelay     = 0;

    private final String  LOG_TAG    = this.getClass().getSimpleName();

    /**
     * Set the time for executing ActionSet.
     *
     * @param year
     *            Year to be set
     * @param month
     *            Month of the year to be set
     * @param day
     *            Day of the month to be set
     * @param hour
     *            Hour to be set
     * @param min
     *            Minute to be set
     * @param sec
     *            Second to be set
     * @param dayOfTheWeek
     *            Day of the week to be set
     */
    public void setTime(int year, int month, int day, int hour, int min,
            int sec, int dayOfTheWeek) {
        if (year < 0 || month < 0 || day < 0 || hour < 0 || min < 0 || sec < 0
                || dayOfTheWeek < 0) {
            Log.e(LOG_TAG, "Input time is invalid");
            return;
        }

        year -= 1900;
        month -= 1;

        mDelay = 0;
        mYear = year;
        mMonth = month;
        mDay = day;
        mHour = hour;
        mMin = month;
        mSec = sec;
        mDayOfWeek = dayOfTheWeek;
        mType = ActionSetType.NONE;
    }

    /**
     * Set type of ActionSet.
     *
     * @param type
     *            Type of ActionSet
     */
    public void setType(ActionSetType type) {
        mType = type;
    }

    /**
     * Set day of the week for recursively executing ActionSet.
     *
     * @param day
     *            day of the week
     */
    public void setDayOfWeekForRecursive(int day) {
        if (day != -1) {
            mType = ActionSetType.RECURSIVE;
            setTime(0, 0, 0, 0, 0, 0, day);
        }
    }

    /**
     * Set the time delay in seconds for executing ActionSet.
     *
     * @param seconds
     *            time delay in seconds
     */
    public void setDelay(long seconds) {
        if (mType != ActionSetType.NONE) {
            mDelay = seconds;
        }
    }

    /**
     * Get absolute time in seconds.
     *
     * @return long - Absolute time in seconds.
     */
    public long getSecAbsTime() {
        long interval;

        interval = (mHour * 60 * 60);
        interval += (mMin * 60);
        interval += (mSec * 1);

        return interval;
    }

    /**
     * Get the type of ActionSet.
     *
     * @return ActionSetType - Type of ActionSet.
     */
    public ActionSetType getType() {
        return mType;
    }

    /**
     * Get the time delay in seconds set in the ActionSet.
     *
     * @return long - Delay in seconds.
     */
    public long getDelay() {
        return mDelay;
    }
}

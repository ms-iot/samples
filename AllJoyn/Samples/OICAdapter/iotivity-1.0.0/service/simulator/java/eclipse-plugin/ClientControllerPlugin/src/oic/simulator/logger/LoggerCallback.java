/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
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
 */

package oic.simulator.logger;

import java.util.Calendar;
import java.util.Date;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.utils.Constants;

import org.oic.simulator.ILogger;

/**
 * Class which provides a callback method to receive log from native layer.
 */
public class LoggerCallback implements ILogger {

    @Override
    public void write(String time, int level, String message) {
        if (null == time || level < 0 || null == message) {
            return;
        }
        // Parse the time
        Date date = parseTime(time);
        if (null == date) {
            return;
        }
        Activator activator = Activator.getDefault();
        if (null == activator) {
            return;
        }
        activator.getLogManager().log(level, date, message);
    }

    private Date parseTime(String time) {
        Date date;
        String[] token = time.split("\\.");
        int h, m, s;
        try {
            if (token.length == Constants.PROPER_LOG_TIME_TOKEN_LENGTH) {
                h = Integer.parseInt(token[0]);
                m = Integer.parseInt(token[1]);
                s = Integer.parseInt(token[2]);

                Calendar calendar;
                calendar = Calendar.getInstance();
                calendar.set(Calendar.HOUR, h);
                calendar.set(Calendar.MINUTE, m);
                calendar.set(Calendar.SECOND, s);

                date = calendar.getTime();
            } else {
                date = null;
            }
        } catch (NumberFormatException nfe) {
            date = null;
        }
        return date;
    }
}
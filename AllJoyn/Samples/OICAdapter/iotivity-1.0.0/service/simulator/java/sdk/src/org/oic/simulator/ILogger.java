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

package org.oic.simulator;

/**
 * Interface for receiving log messages.
 */
public interface ILogger {
    /**
     * This enumeration contains different levels of log.
     */
    public enum Level {
        INFO, DEBUG, WARNING, ERROR
    }

    /**
     * This callback method will be called to notify the application about the
     * status or problems along with their severity.
     *
     * @param time
     *            Local time information when action/event logged.
     * @param level
     *            Level or Severity of the log.
     * @param message
     *            The log message describing the issue.
     */
    public void write(String time, int level, String message);
}
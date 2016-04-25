/******************************************************************
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

/**
 * @file   simulator_logger.h
 *
 * @brief   This file provides the interface for logging messages to different targets (console/file).
 */

#ifndef SIMULATOR_LOGGER_H_
#define SIMULATOR_LOGGER_H_

#include <iostream>
#include <memory>

class ILogger
{
    public:
        enum Level
        {
            INFO = 0,
            DEBUG,
            WARNING,
            ERROR
        };

        static const char *getString(Level level)
        {
            switch (level)
            {
                case Level::INFO: return "INFO";
                case Level::DEBUG: return "DEBUG";
                case Level::WARNING: return "WARNING";
                case Level::ERROR: return "ERROR";
                default: return "UNKNOWN";
            }
        }

        virtual void write(std::string, Level, std::string) = 0;
};

class Logger
{
    public:
        bool setDefaultConsoleTarget();
        bool setDefaultFileTarget(const std::string &path);
        void setCustomTarget(const std::shared_ptr<ILogger> &target);
        void write(ILogger::Level level, std::ostringstream &str);

    private:
        std::shared_ptr<ILogger> m_target;
};

auto simLogger() -> Logger &;

#ifndef SIM_LOG
#define SIM_LOG(LEVEL, MSG) { \
        simLogger().write(LEVEL, static_cast<std::ostringstream&>(std::ostringstream()<<MSG)); \
    }
#endif

#endif
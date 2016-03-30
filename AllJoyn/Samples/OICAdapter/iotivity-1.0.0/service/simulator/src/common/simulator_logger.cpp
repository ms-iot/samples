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

#include "simulator_logger.h"
#include <sstream>
#include <fstream>
#include <time.h>

class ConsoleLogger : public ILogger
{
    public:
        void write(std::string time, ILogger::Level level, std::string message)
        {
            std::ostringstream out;
            out << time << " " << ILogger::getString(level) << " " << message;
            std::cout << out.str() << std::endl;
        }
};

class FileLogger : public ILogger
{
    public:
        FileLogger(std::string filePath) : m_filePath(filePath) {}

        bool open()
        {
            m_out.open(m_filePath, std::ofstream::out);
            return m_out.is_open();
        }

        void close()
        {
            if (m_out.is_open())
                m_out.close();
        }

        void write(std::string time, ILogger::Level level, std::string message)
        {
            m_out << time << " " << ILogger::getString(level) << " " << message;
        }

    private:
        std::ofstream m_out;
        std::string m_filePath;
};

bool Logger::setDefaultConsoleTarget()
{
    if (nullptr != m_target)
        return false;

    m_target = std::make_shared<ConsoleLogger>();
    return true;
}

bool Logger::setDefaultFileTarget(const std::string &path)
{
    if (nullptr != m_target || path.empty())
        return false;

    time_t timeInfo = time(NULL);
    struct tm *localTime = localtime(&timeInfo);
    if (nullptr == localTime)
        return false;
    std::ostringstream newFileName;
    newFileName << path << "/Simulator_";
    newFileName << localTime->tm_year << localTime->tm_mon << localTime->tm_mday << localTime->tm_hour
                << localTime->tm_min << localTime->tm_sec;
    newFileName << ".log";

    std::shared_ptr<FileLogger> fileLogger = std::make_shared<FileLogger>(newFileName.str());
    if (fileLogger->open())
    {
        m_target = fileLogger;
        return true;
    }

    return false;
}

void Logger::setCustomTarget(const std::shared_ptr<ILogger> &target)
{
    m_target = target;
}

void Logger::write(ILogger::Level level, std::ostringstream &str)
{
    if (nullptr != m_target)
    {
        time_t timeInfo = time(NULL);
        struct tm *localTime = localtime(&timeInfo);
        if (nullptr == localTime)
            return;
        std::ostringstream timeStr;
        timeStr << localTime->tm_hour << "." << localTime->tm_min << "." << localTime->tm_sec;
        m_target->write(timeStr.str(), level, str.str());
    }
}

auto simLogger() -> Logger &
{
    static Logger logger;
    return logger;
}
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

#ifndef COMMON_UTILS_SCOPELOGGER_H
#define COMMON_UTILS_SCOPELOGGER_H

#include "logger.h"

#ifdef TB_LOG
#include <exception>

namespace OIC
{
    namespace Service
    {
        namespace Logging
        {

            class ScopeLogger
            {
            public:
                ScopeLogger(LogLevel level, const char* tag, const char* scopeName) :
                    m_level{ level },
                    m_tag{ tag },
                    m_scopeName{ scopeName }
                {
                    static constexpr char DEFAULT_ENTER_STR[]{ "IN" };

                    OC_LOG_V(m_level, m_tag, "%s %s", m_scopeName, DEFAULT_ENTER_STR);
                }

                ~ScopeLogger()
                {
                    static constexpr char DEFAULT_EXIT_STR[]{ "OUT" };

                    if (std::uncaught_exception())
                    {
                        OC_LOG_V(m_level, m_tag, "%s %s by stack unwinding (uncaught exception)",
                                m_scopeName, DEFAULT_EXIT_STR);
                    }
                    else
                    {
                        OC_LOG_V(m_level, m_tag, "%s %s", m_scopeName, DEFAULT_EXIT_STR);
                    }
                }

                ScopeLogger(const ScopeLogger&) = delete;
                ScopeLogger(ScopeLogger&&) = delete;

                ScopeLogger& operator=(const ScopeLogger&) = delete;
                ScopeLogger& operator=(ScopeLogger&&) = delete;

            private:
                const LogLevel m_level;
                const char* m_tag;
                const char* m_scopeName;
            };
        }

    }
}

#define SCOPE_LOG(level, tag, scopeName) \
            Logging::ScopeLogger rcsScopeLogger__((level), (tag), (scopeName))

#define SCOPE_LOG_F(level, tag) SCOPE_LOG((level), (tag), __func__)

#else
#define SCOPE_LOG_F(level, tag)
#define SCOPE_LOG(level, tag, scopeName)
#endif


#endif // COMMON_UTILS_SCOPELOGGER_H

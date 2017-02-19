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

#include "ExpiryTimer.h"
#include "ExpiryTimerImpl.h"

namespace OIC
{
    namespace Service
    {

        namespace
        {
            constexpr size_t DEFAULT_SWEEP_SIZE{ 50 };
        }

        ExpiryTimer::ExpiryTimer() :
               m_nextSweep { DEFAULT_SWEEP_SIZE }
        {
        }

        ExpiryTimer::~ExpiryTimer()
        {
            cancelAll();
        }

        ExpiryTimer::Id ExpiryTimer::post(DelayInMilliSec milliSec, Callback cb)
        {
            auto task = ExpiryTimerImpl::getInstance()->post(milliSec, std::move(cb));
            m_tasks[task->getId()] = task;

            if (m_tasks.size() == m_nextSweep) sweep();

            return task->getId();
        }

        bool ExpiryTimer::cancel(Id id)
        {
            auto it = m_tasks.find(id);

            if (it == m_tasks.end()) return false;

            auto task = it->second;
            m_tasks.erase(it);

            if (task->isExecuted()) return false;

            return ExpiryTimerImpl::getInstance()->cancel(id);
        }

        void ExpiryTimer::cancelAll()
        {
            sweep();

            std::unordered_set< std::shared_ptr< TimerTask > > set;

            for(const auto& p : m_tasks)
            {
                set.insert(p.second);
            }

            ExpiryTimerImpl::getInstance()->cancelAll(set);
            m_tasks.clear();
        }

        size_t ExpiryTimer::getNumOfPending()
        {
            sweep();
            return m_tasks.size();
        }

        size_t ExpiryTimer::getNumOfPending() const
        {
            size_t ret{ 0 };

            for (const auto& p : m_tasks)
            {
                ret += p.second->isExecuted() ? 0U : 1U;
            }

            return ret;
        }

        void ExpiryTimer::sweep()
        {
            for (auto it = m_tasks.begin(); it != m_tasks.end();)
            {
                if (it->second->isExecuted())
                {
                    it = m_tasks.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            m_nextSweep = m_tasks.size() << 1;
        }

    }
}

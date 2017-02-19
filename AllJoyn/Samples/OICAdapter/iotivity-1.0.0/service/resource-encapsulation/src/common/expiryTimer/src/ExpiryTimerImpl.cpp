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

#include "ExpiryTimerImpl.h"

#include "RCSException.h"

namespace OIC
{
    namespace Service
    {

        namespace
        {
            constexpr ExpiryTimerImpl::Id INVALID_ID{ 0U };
        }

        ExpiryTimerImpl::ExpiryTimerImpl() :
                m_tasks{ },
                m_thread{ },
                m_mutex{ },
                m_cond{ },
                m_stop{ false },
                m_mt{ std::random_device{ }() },
                m_dist{ }
        {
            m_thread = std::thread(&ExpiryTimerImpl::run, this);
        }

        ExpiryTimerImpl::~ExpiryTimerImpl()
        {
            {
                std::lock_guard< std::mutex > lock{ m_mutex };
                m_tasks.clear();
                m_stop = true;
            }
            m_cond.notify_all();
            m_thread.join();
        }

        ExpiryTimerImpl* ExpiryTimerImpl::getInstance()
        {
            static ExpiryTimerImpl instance;
            return &instance;
        }

        std::shared_ptr< TimerTask > ExpiryTimerImpl::post(DelayInMillis delay, Callback cb)
        {
            if (delay < 0LL)
            {
                throw RCSInvalidParameterException{ "delay can't be negative." };
            }

            if (!cb)
            {
                throw RCSInvalidParameterException{ "callback is empty." };
            }

            return addTask(convertToTime(Milliseconds{ delay }), std::move(cb), generateId());
        }

        bool ExpiryTimerImpl::cancel(Id id)
        {
            if (id == INVALID_ID) return false;

            std::lock_guard< std::mutex > lock{ m_mutex };

            for(auto it = m_tasks.begin(); it != m_tasks.end(); ++it)
            {
                if(it->second->getId() == id)
                {
                    m_tasks.erase(it);
                    return true;
                }
            }
            return false;
        }

        size_t ExpiryTimerImpl::cancelAll(
                const std::unordered_set< std::shared_ptr<TimerTask > >& tasks)
        {
            std::lock_guard< std::mutex > lock{ m_mutex };
            size_t erased { 0 };

            for(auto it = m_tasks.begin(); it != m_tasks.end();)
            {
                if(tasks.count(it->second))
                {
                    it = m_tasks.erase(it);
                    ++erased;
                }
                else
                {
                    ++it;
                }
            }
            return erased;
        }

        ExpiryTimerImpl::Milliseconds ExpiryTimerImpl::convertToTime(Milliseconds delay)
        {
            const auto now = std::chrono::system_clock::now();
            return std::chrono::duration_cast< Milliseconds >(now.time_since_epoch()) + delay;
        }

        std::shared_ptr< TimerTask > ExpiryTimerImpl::addTask(
                Milliseconds delay, Callback cb, Id id)
        {
            std::lock_guard< std::mutex > lock{ m_mutex };

            auto newTask = std::make_shared< TimerTask >(id, std::move(cb));
            m_tasks.insert({ delay, newTask });
            m_cond.notify_all();

            return newTask;
        }

        bool ExpiryTimerImpl::containsId(Id id) const
        {
            for (const auto& info : m_tasks)
            {
                if (info.second->getId() == id) return true;
            }
            return false;
        }

        ExpiryTimerImpl::Id ExpiryTimerImpl::generateId()
        {
            Id newId = m_dist(m_mt);

            std::lock_guard< std::mutex > lock{ m_mutex };

            while (newId == INVALID_ID || containsId(newId))
            {
                newId = m_dist(m_mt);
            }
            return newId;
        }

        void ExpiryTimerImpl::executeExpired()
        {
            if (m_tasks.empty()) return;

            auto now = std::chrono::system_clock::now().time_since_epoch();

            auto it = m_tasks.begin();
            for (; it != m_tasks.end() && it->first <= now; ++it)
            {
                it->second->execute();
            }

            m_tasks.erase(m_tasks.begin(), it);
        }

        ExpiryTimerImpl::Milliseconds ExpiryTimerImpl::remainingTimeForNext() const
        {
            const Milliseconds& expiredTime = m_tasks.begin()->first;

            return std::chrono::duration_cast< Milliseconds >(expiredTime -
                    std::chrono::system_clock::now().time_since_epoch()) + Milliseconds{ 1 };
        }

        void ExpiryTimerImpl::run()
        {
            auto hasTaskOrStop = [this](){ return !m_tasks.empty() || m_stop; };

            std::unique_lock< std::mutex > lock{ m_mutex };

            while(!m_stop)
            {
                m_cond.wait(lock, hasTaskOrStop);

                if (m_stop) break;

                m_cond.wait_for(lock, remainingTimeForNext());

                executeExpired();
            }
        }


        TimerTask::TimerTask(ExpiryTimerImpl::Id id, ExpiryTimerImpl::Callback cb) :
            m_id{ id },
            m_callback{ std::move(cb) }
        {
        }

        void TimerTask::execute()
        {
            if (isExecuted()) return;

            ExpiryTimerImpl::Id id { m_id };
            m_id = INVALID_ID;

            std::thread(std::move(m_callback), id).detach();

            m_callback = ExpiryTimerImpl::Callback{ };
        }

        bool TimerTask::isExecuted() const
        {
            return m_id == INVALID_ID;
        }

        ExpiryTimerImpl::Id TimerTask::getId() const
        {
            return m_id;
        }

    }
}

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

#ifndef _EXPIRY_TIMER_IMPL_H_
#define _EXPIRY_TIMER_IMPL_H_

#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <random>
#include <unordered_set>
#include <atomic>

namespace OIC
{
    namespace Service
    {
        class TimerTask;

        class ExpiryTimerImpl
        {
        public:
            typedef unsigned int Id;
            typedef std::function< void(Id) > Callback;

            typedef long long DelayInMillis;

        private:
            typedef std::chrono::milliseconds Milliseconds;

        private:
            ExpiryTimerImpl();
            ~ExpiryTimerImpl();

            ExpiryTimerImpl(const ExpiryTimerImpl&) = delete;
            ExpiryTimerImpl& operator=(const ExpiryTimerImpl&) = delete;

        public:
            static ExpiryTimerImpl* getInstance();

            std::shared_ptr< TimerTask > post(DelayInMillis, Callback);

            bool cancel(Id);
            size_t cancelAll(const std::unordered_set< std::shared_ptr<TimerTask > >&);

        private:
            static Milliseconds convertToTime(Milliseconds);

            std::shared_ptr< TimerTask > addTask(Milliseconds, Callback, Id);

            /**
             * @pre The lock must be acquired with m_mutex.
             */
            bool containsId(Id) const;
            Id generateId();

            /**
             * @pre The lock must be acquired with m_mutex.
             */
            void executeExpired();

            /**
             * @pre The lock must be acquired with m_mutex.
             */
            Milliseconds remainingTimeForNext() const;

            void run();

        private:
            std::multimap< Milliseconds, std::shared_ptr< TimerTask > > m_tasks;

            std::thread m_thread;
            std::mutex m_mutex;
            std::condition_variable m_cond;
            bool m_stop;

            std::mt19937 m_mt;
            std::uniform_int_distribution< Id > m_dist;

        };

        class TimerTask
        {
        public:
            TimerTask(ExpiryTimerImpl::Id, ExpiryTimerImpl::Callback);

            TimerTask(const TimerTask&) = delete;
            TimerTask(TimerTask&&) = delete;

            TimerTask& operator=(const TimerTask&) = delete;
            TimerTask& operator=(TimerTask&&) = delete;

            bool isExecuted() const;
            ExpiryTimerImpl::Id getId() const;

        private:
            void execute();

        private:
            std::atomic< ExpiryTimerImpl::Id > m_id;
            ExpiryTimerImpl::Callback m_callback;

            friend class ExpiryTimerImpl;
        };

    }
}
#endif //_EXPIRY_TIMER_IMPL_H_

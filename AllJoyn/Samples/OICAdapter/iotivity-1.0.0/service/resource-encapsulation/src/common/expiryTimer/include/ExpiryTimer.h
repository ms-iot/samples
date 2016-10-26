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

#ifndef _EXPIRY_TIMER_H_
#define _EXPIRY_TIMER_H_

#include <functional>
#include <unordered_map>
#include <memory>

namespace OIC
{
    namespace Service
    {

        class TimerTask;

        class ExpiryTimer
        {
        public:
            typedef unsigned int Id;
            typedef std::function< void(Id) > Callback;
            typedef long long DelayInMilliSec;

        public:
            ExpiryTimer();
            ~ExpiryTimer();

            ExpiryTimer(ExpiryTimer&&) = default;
            ExpiryTimer& operator=(ExpiryTimer&&) = default;

            ExpiryTimer(const ExpiryTimer&) = delete;
            ExpiryTimer& operator=(const ExpiryTimer&) = delete;

            Id post(DelayInMilliSec, Callback);
            bool cancel(Id);
            void cancelAll();

            size_t getNumOfPending();
            size_t getNumOfPending() const;

        private:
            void sweep();

        private:
            size_t m_nextSweep;

            std::unordered_map< Id, std::shared_ptr< TimerTask > > m_tasks;
        };

    }
}
#endif //_EXPIRY_TIMER_H_

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

#ifndef IOTY_GTEST_HELPER_H
#define IOTY_GTEST_HELPER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

namespace iotivity
{
    namespace test
    {
        /**
         * Simple implementation of a deadman's timer that can be used to
         * terminate a test that hangs.
         *
         * Since there is no standard way to terminate an individual thread,
         * the entire process will be killed once time has been exceeded.
         *
         * @note provisions for watchdog thread cleanup are not currently added.
         * Testing has not yet shown any need for such complexity.
         */
        class DeadmanTimer
        {
        public:

            /**
             * Creates an instance of a timer set to kill the running process
             * after the specified timeout.
             *
             * If the destructor is invoked before time is up (aka this instance
             * goes out of scope) the timeout will not cause the program to be
             * terminated.
             *
             * @param time to wait before assuming the process is hung and must be
             * killed.
             * Examples of values that can be passed include
             * std::chrono::milliseconds(250), std::chrono::seconds(5),
             * std::chrono::minutes(3).
             */
            DeadmanTimer(std::chrono::milliseconds timeout) :
                m_ctx(new DeadmanCtx(timeout)),
                m_thread()
                {
                    m_thread = std::thread([this](){run(m_ctx);});
                    {
                        std::unique_lock<std::mutex> lock(m_ctx->m_mutex);
                        while (!m_ctx->m_isArmed)
                        {
                            m_ctx->m_cond.wait(lock);
                        }
                    }
                    // Now that the thread is live, we can stop tracking it.
                    m_thread.detach();
                }

            /**
             * Destructor that also will cancel the termination of the
             * running process.
             */
            ~DeadmanTimer()
            {
                std::unique_lock<std::mutex> lock(m_ctx->m_mutex);
                m_ctx->m_isArmed = false;
            }

        private:

            /**
             * Shared data that main and child thread might both need to
             * access.
             *
             * Avoids referencing data in class instances that have been
             * deleted.
             */
            class DeadmanCtx
            {
            public:

                DeadmanCtx(std::chrono::milliseconds timeout) :
                    m_mutex(),
                    m_cond(),
                    m_isArmed(false),
                    m_timeout(timeout)
                    {
                    }

                std::mutex m_mutex;
                std::condition_variable m_cond;
                bool m_isArmed;
                std::chrono::milliseconds m_timeout;
            };

            // Explicitly block assignment and copy ctor
            DeadmanTimer &operator=(const DeadmanTimer &rhs);
            DeadmanTimer(const iotivity::test::DeadmanTimer &rhs);

            std::shared_ptr<DeadmanCtx> m_ctx;
            std::thread m_thread;


            static void run(std::shared_ptr<DeadmanCtx> ctx)
            {
                // Let the calling thread know it can stop waiting:
                {
                    std::unique_lock<std::mutex> lock(ctx->m_mutex);
                    ctx->m_isArmed = true;
                    ctx->m_cond.notify_all();
                }

                std::this_thread::sleep_for(ctx->m_timeout);

                std::unique_lock<std::mutex> lock(ctx->m_mutex);
                if (ctx->m_isArmed)
                {
                    try {
                        throw std::runtime_error("deadman timer expired");
                    }
                    catch (std::exception&)
                    {
                        std::terminate();
                    }
                }
            }
        };
    } // namespace test
} // namespace iotivity

#endif // IOTY_GTEST_HELPER_H

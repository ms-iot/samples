//******************************************************************
//
// Copyright 2014 Intel Corporation.
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

#include "observer.h"

IoTObserver::IoTObserver(IoTObserverCb Cb) :
        m_callback(Cb), m_destroy(false), m_started(false)
{
    m_observerThread[0] = thread(&IoTObserver::observerThread, this);
}

IoTObserver::~IoTObserver()
{
    terminate();
}

void IoTObserver::start()
{
    if (!m_started)
    {
        {
            lock_guard<mutex> lock(m_mutex);
            m_started = true;
        }
        m_cond.notify_one();
    }
}

void IoTObserver::stop()
{
    m_started = false;
}

void IoTObserver::terminate()
{
    m_destroy = true;
    stop();
    m_cond.notify_one();
    m_observerThread[0].join();
}

void IoTObserver::observerThread()
{
    while (!m_destroy)
    {
        unique_lock<mutex> lock(m_mutex);
        if (!m_started)
            m_cond.wait(lock);
        while (m_started)
        {
            m_callback();
        }
    }
}

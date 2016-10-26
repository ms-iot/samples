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

#include <memory>
#include <cstdlib>
#include <functional>
#include <map>
#include <utility>
#include <ctime>

#include "DataCache.h"

#include "ResponseStatement.h"
#include "RCSResourceAttributes.h"
#include "ExpiryTimer.h"

namespace OIC
{
    namespace Service
    {

        namespace
        {
            void verifyObserveCB(
                const HeaderOptions &_hos, const ResponseStatement &_rep,
                int _result, int _seq, std::weak_ptr<DataCache> rpPtr)
            {
                std::shared_ptr<DataCache> Ptr = rpPtr.lock();
                if (Ptr)
                {
                    Ptr->onObserve(_hos, _rep, _result, _seq);
                }
            }

            ObserveCB verifiedObserveCB(std::weak_ptr<DataCache> rpPtr)
            {
                return std::bind(verifyObserveCB,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3, std::placeholders::_4, rpPtr);
            }

            void verifyGetCB(
                const HeaderOptions &_hos, const ResponseStatement &_rep,
                int _result, std::weak_ptr<DataCache> rpPtr)
            {
                std::shared_ptr<DataCache> Ptr = rpPtr.lock();
                if (Ptr)
                {
                    Ptr->onGet(_hos, _rep, _result);
                }
            }

            GetCB verifiedGetCB(std::weak_ptr<DataCache> rpPtr)
            {
                return std::bind(verifyGetCB,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3, rpPtr);
            }
        }

        DataCache::DataCache()
        {
            subscriberList = std::unique_ptr<SubscriberInfo>(new SubscriberInfo());

            sResource = nullptr;

            state = CACHE_STATE::READY_YET;
            mode = CACHE_MODE::FREQUENCY;

            networkTimeOutHandle = 0;
            pollingHandle = 0;
            lastSequenceNum = 0;
            isReady = false;
        }

        DataCache::~DataCache()
        {
            state = CACHE_STATE::DESTROYED;

            if (subscriberList != nullptr)
            {
                subscriberList->clear();
                subscriberList.release();
            }

            if (mode == CACHE_MODE::OBSERVE)
            {
                try
                {
                    sResource->cancelObserve();
                }
                catch (...)
                {
                    // ignore the exception because data cache was released.
                }
            }
        }

        void DataCache::initializeDataCache(PrimitiveResourcePtr pResource)
        {
            sResource = pResource;
            pObserveCB = verifiedObserveCB(std::weak_ptr<DataCache>(shared_from_this()));
            pGetCB = verifiedGetCB(std::weak_ptr<DataCache>(shared_from_this()));
            pTimerCB = (TimerCB)(std::bind(&DataCache::onTimeOut, this, std::placeholders::_1));
            pPollingCB = (TimerCB)(std::bind(&DataCache::onPollingOut, this, std::placeholders::_1));

            sResource->requestGet(pGetCB);
            if (sResource->isObservable())
            {
                sResource->requestObserve(pObserveCB);
            }
            networkTimeOutHandle = networkTimer.post(CACHE_DEFAULT_EXPIRED_MILLITIME, pTimerCB);
        }

        CacheID DataCache::addSubscriber(CacheCB func, REPORT_FREQUENCY rf, long repeatTime)
        {
            Report_Info newItem;
            newItem.rf = rf;
            newItem.repeatTime = repeatTime;
            newItem.timerID = 0;

            newItem.reportID = generateCacheID();

            std::lock_guard<std::mutex> lock(m_mutex);
            if (subscriberList != nullptr)
            {
                subscriberList->insert(
                    std::make_pair(newItem.reportID, std::make_pair(newItem, func)));
            }

            return newItem.reportID;
        }

        CacheID DataCache::deleteSubscriber(CacheID id)
        {
            CacheID ret = 0;

            SubscriberInfoPair pair = findSubscriber(id);

            std::lock_guard<std::mutex> lock(m_mutex);
            if (pair.first != 0)
            {
                ret = pair.first;
                subscriberList->erase(pair.first);
            }

            return ret;
        }

        SubscriberInfoPair DataCache::findSubscriber(CacheID id)
        {
            SubscriberInfoPair ret;

            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto &i : *subscriberList)
            {
                if (i.first == id)
                {
                    ret = std::make_pair(i.first, std::make_pair((Report_Info)i.second.first,
                                         (CacheCB)i.second.second));
                    break;
                }
            }

            return ret;
        }

        const PrimitiveResourcePtr DataCache::getPrimitiveResource() const
        {
            return (sResource != nullptr) ? sResource : nullptr;
        }

        const RCSResourceAttributes DataCache::getCachedData() const
        {
            std::lock_guard<std::mutex> lock(att_mutex);
            if (state != CACHE_STATE::READY)
            {
                return RCSResourceAttributes();
            }
            return attributes;
        }

        bool DataCache::isCachedData() const
        {
            return isReady;
        }

        void DataCache::onObserve(const HeaderOptions & /*_hos*/,
                                  const ResponseStatement &_rep, int _result, int _seq)
        {

            if (_result != OC_STACK_OK || _rep.getAttributes().empty() || lastSequenceNum > _seq)
            {
                return;
            }
            else
            {
                lastSequenceNum = _seq;
            }

            if (state != CACHE_STATE::READY)
            {
                state = CACHE_STATE::READY;
                isReady = true;
            }

            if (mode != CACHE_MODE::OBSERVE)
            {
                mode = CACHE_MODE::OBSERVE;
            }

            networkTimer.cancel(networkTimeOutHandle);
            networkTimeOutHandle = networkTimer.post(CACHE_DEFAULT_EXPIRED_MILLITIME, pTimerCB);

            notifyObservers(_rep.getAttributes());
        }

        void DataCache::onGet(const HeaderOptions & /*_hos*/,
                              const ResponseStatement &_rep, int _result)
        {
            if (_result != OC_STACK_OK || _rep.getAttributes().empty())
            {
                return;
            }

            if (state != CACHE_STATE::READY)
            {
                state = CACHE_STATE::READY;
                isReady = true;
            }

            if (mode != CACHE_MODE::OBSERVE)
            {
                networkTimer.cancel(networkTimeOutHandle);
                networkTimeOutHandle = networkTimer.post(
                                           CACHE_DEFAULT_EXPIRED_MILLITIME, pTimerCB);

                pollingHandle = pollingTimer.post(CACHE_DEFAULT_REPORT_MILLITIME, pPollingCB);
            }

            notifyObservers(_rep.getAttributes());
        }

        void DataCache::notifyObservers(const RCSResourceAttributes Att)
        {
            {
                std::lock_guard<std::mutex> lock(att_mutex);
                if (attributes == Att)
                {
                    return;
                }
                attributes = Att;
            }

            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto &i : * subscriberList)
            {
                if (i.second.first.rf == REPORT_FREQUENCY::UPTODATE)
                {
                    i.second.second(this->sResource, Att);
                }
            }
        }

        CACHE_STATE DataCache::getCacheState() const
        {
            return state;
        }

        void DataCache::onTimeOut(unsigned int /*timerID*/)
        {
            if (mode == CACHE_MODE::OBSERVE)
            {
                sResource->cancelObserve();
                mode = CACHE_MODE::FREQUENCY;

                networkTimer.cancel(networkTimeOutHandle);
                networkTimeOutHandle = networkTimer.post(
                                           CACHE_DEFAULT_EXPIRED_MILLITIME, pTimerCB);

                pollingHandle = pollingTimer.post(CACHE_DEFAULT_REPORT_MILLITIME, pPollingCB);
                return;
            }

            state = CACHE_STATE::LOST_SIGNAL;
        }
        void DataCache::onPollingOut(const unsigned int /*timerID*/)
        {
            if (sResource != nullptr)
            {
                mode = CACHE_MODE::FREQUENCY;
                sResource->requestGet(pGetCB);
            }
            return;
        }

        CacheID DataCache::generateCacheID()
        {
            CacheID retID = 0;
            srand(time(NULL));

            while (1)
            {
                if (findSubscriber(retID).first == 0 && retID != 0)
                {
                    break;
                }
                retID = rand();
            }

            return retID;
        }

        void DataCache::requestGet()
        {
            state = CACHE_STATE::UPDATING;
            if (sResource != nullptr)
            {
                sResource->requestGet(pGetCB);
            }
        }

        bool DataCache::isEmptySubscriber() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return (subscriberList != nullptr) ? subscriberList->empty() : true;
        }
    } // namespace Service
} // namespace OIC

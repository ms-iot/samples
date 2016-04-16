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

#include "ResourceCacheManager.h"

namespace OIC
{
    namespace Service
    {
        ResourceCacheManager *ResourceCacheManager::s_instance = NULL;
        std::mutex ResourceCacheManager::s_mutexForCreation;
        std::mutex ResourceCacheManager::s_mutex;
        std::unique_ptr<std::list<DataCachePtr>> ResourceCacheManager::s_cacheDataList(nullptr);

        ResourceCacheManager::~ResourceCacheManager()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_cacheDataList != nullptr)
            {
                s_cacheDataList->clear();
            }
        }

        ResourceCacheManager *ResourceCacheManager::getInstance()
        {
            if (s_instance == nullptr)
            {
                s_mutexForCreation.lock();
                if (s_instance == nullptr)
                {
                    s_instance = new ResourceCacheManager();
                    s_instance->initializeResourceCacheManager();
                }
                s_mutexForCreation.unlock();
            }
            return s_instance;
        }

        CacheID ResourceCacheManager::requestResourceCache(
            PrimitiveResourcePtr pResource, CacheCB func,
            REPORT_FREQUENCY rf, long reportTime)
        {
            if (pResource == nullptr)
            {
                throw InvalidParameterException {"[requestResourceCache] Primitive Resource is invaild"};
            }

            CacheID retID = 0;

            if (rf != REPORT_FREQUENCY::NONE)
            {
                if (func == NULL || func == nullptr)
                {
                    throw InvalidParameterException {"[requestResourceCache] CacheCB is invaild"};
                }
                if (!reportTime)
                {
                    // default setting
                    reportTime = CACHE_DEFAULT_REPORT_MILLITIME;
                }
            }

            DataCachePtr newHandler = findDataCache(pResource);
            if (newHandler == nullptr)
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                newHandler.reset(new DataCache());
                newHandler->initializeDataCache(pResource);
                s_cacheDataList->push_back(newHandler);
            }
            retID = newHandler->addSubscriber(func, rf, reportTime);

            cacheIDmap.insert(std::make_pair(retID, newHandler));

            return retID;
        }

        void ResourceCacheManager::cancelResourceCache(CacheID id)
        {
            if (id == 0 || cacheIDmap.find(id) == cacheIDmap.end())
            {
                throw InvalidParameterException {"[cancelResourceCache] CacheID is invaild"};
            }

            DataCachePtr foundCacheHandler = findDataCache(id);
            if (foundCacheHandler != nullptr)
            {
                CacheID retID = foundCacheHandler->deleteSubscriber(id);
                if (retID == id)
                {
                    cacheIDmap.erase(id);
                }
                std::lock_guard<std::mutex> lock(s_mutex);
                if (foundCacheHandler->isEmptySubscriber())
                {
                    s_cacheDataList->remove(foundCacheHandler);
                }
            }
        }

        void ResourceCacheManager::updateResourceCache(PrimitiveResourcePtr pResource) const
        {
            if (pResource == nullptr)
            {
                throw InvalidParameterException
                {"[updateResourceCache] Primitive Resource is invaild"};
            }

            DataCachePtr foundCache = findDataCache(pResource);
            if (foundCache == nullptr)
            {
                throw InvalidParameterException
                {"[updateResourceCache] Primitive Resource is invaild"};
            }
            foundCache->requestGet();
        }

        void ResourceCacheManager::updateResourceCache(CacheID updateId) const
        {
            if (updateId == 0)
            {
                throw InvalidParameterException {"[getCachedData] CacheID is NULL"};
            }

            DataCachePtr foundCache = findDataCache(updateId);
            if (foundCache == nullptr)
            {
                throw InvalidParameterException {"[getCachedData] CacheID is invaild"};
            }
            foundCache->requestGet();
        }

        const RCSResourceAttributes ResourceCacheManager::getCachedData(
            PrimitiveResourcePtr pResource) const
        {
            if (pResource == nullptr)
            {
                throw InvalidParameterException {"[getCachedData] Primitive Resource is nullptr"};
            }

            DataCachePtr handler = findDataCache(pResource);
            if (handler == nullptr)
            {
                throw InvalidParameterException {"[getCachedData] Primitive Resource is invaild"};
            }

            if (handler->isCachedData() == false)
            {
                throw HasNoCachedDataException {"[getCachedData] Cached Data is not stored"};
            }

            return handler->getCachedData();
        }

        const RCSResourceAttributes ResourceCacheManager::getCachedData(CacheID id) const
        {
            if (id == 0)
            {
                throw InvalidParameterException {"[getCachedData] CacheID is NULL"};
            }

            DataCachePtr handler = findDataCache(id);
            if (handler == nullptr)
            {
                throw InvalidParameterException {"[getCachedData] CacheID is invaild"};
            }

            if (handler->isCachedData() == false)
            {
                throw HasNoCachedDataException {"[getCachedData] Cached Data is not stored"};
            }

            return handler->getCachedData();
        }

        CACHE_STATE ResourceCacheManager::getResourceCacheState(
            PrimitiveResourcePtr pResource) const
        {
            if (pResource == nullptr)
            {
                throw InvalidParameterException {"[getResourceCacheState] Primitive Resource is nullptr"};
            }

            DataCachePtr handler = findDataCache(pResource);
            if (handler == nullptr)
            {
                return CACHE_STATE::NONE;
            }
            return handler->getCacheState();
        }

        CACHE_STATE ResourceCacheManager::getResourceCacheState(CacheID id) const
        {
            if (id == 0)
            {
                throw InvalidParameterException {"[getResourceCacheState] CacheID is NULL"};
            }

            DataCachePtr handler = findDataCache(id);
            if (handler == nullptr)
            {
                return CACHE_STATE::NONE;
            }
            return handler->getCacheState();
        }

        bool ResourceCacheManager::isCachedData(PrimitiveResourcePtr pResource) const
        {
            if (pResource == nullptr)
            {
                throw InvalidParameterException {"[isCachedData] Primitive Resource is nullptr"};
            }

            DataCachePtr handler = findDataCache(pResource);
            if (handler == nullptr)
            {
                throw InvalidParameterException {"[isCachedData] Primitive Resource is invaild"};
            }
            return handler->isCachedData();
        }

        bool ResourceCacheManager::isCachedData(CacheID id) const
        {
            if (id == 0)
            {
                throw InvalidParameterException {"[isCachedData] CacheID is NULL"};
            }

            DataCachePtr handler = findDataCache(id);
            if (handler == nullptr)
            {
                throw InvalidParameterException {"[isCachedData] CacheID is invaild"};
            }
            return handler->isCachedData();
        }

        void ResourceCacheManager::initializeResourceCacheManager()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_cacheDataList == nullptr)
            {
                s_cacheDataList
                    = std::unique_ptr<std::list<DataCachePtr>>(new std::list<DataCachePtr>);
            }
        }

        DataCachePtr ResourceCacheManager::findDataCache(PrimitiveResourcePtr pResource) const
        {
            DataCachePtr retHandler = nullptr;
            std::lock_guard<std::mutex> lock(s_mutex);
            for (auto &i : * s_cacheDataList)
            {
                if (i->getPrimitiveResource()->getUri() == pResource->getUri() &&
                    i->getPrimitiveResource()->getHost() == pResource->getHost())
                {
                    retHandler = i;
                    break;
                }
            }
            return retHandler;
        }

        DataCachePtr ResourceCacheManager::findDataCache(CacheID id) const
        {
            DataCachePtr retHandler = nullptr;
            for (auto it : cacheIDmap)
            {
                if (it.first == id)
                {
                    retHandler = it.second;
                    break;
                }
            }

            return retHandler;
        }
    } // namespace Service
} // namespace OIC

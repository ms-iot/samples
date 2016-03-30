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

#ifndef RCM_RESOURCECACHEMANAGER_H_
#define RCM_RESOURCECACHEMANAGER_H_

#include <list>
#include <string>
#include <mutex>
#include <map>

#include "CacheTypes.h"
#include "DataCache.h"

namespace OIC
{
    namespace Service
    {
        class ResourceCacheManager
        {
            public:
                class InvalidParameterException: public RCSException
                {
                    public:
                        InvalidParameterException(std::string &&what)
                            : RCSException { std::move(what) } {}
                };
                class HasNoCachedDataException: public RCSException
                {
                    public:
                        HasNoCachedDataException(std::string &&what)
                            : RCSException { std::move(what) } {}
                };

                static ResourceCacheManager *getInstance();

                // throw InvalidParameterException;
                CacheID requestResourceCache(
                    PrimitiveResourcePtr pResource, CacheCB func = NULL,
                    REPORT_FREQUENCY rf = REPORT_FREQUENCY::NONE, long time = 0l);

                // throw InvalidParameterException;
                void cancelResourceCache(CacheID id);

                // throw InvalidParameterException;
                void updateResourceCache(PrimitiveResourcePtr pResource) const;
                void updateResourceCache(CacheID id) const;

                // throw InvalidParameterException;
                // throw HasNoCachedDataException;
                const RCSResourceAttributes getCachedData(PrimitiveResourcePtr pResource) const;
                const RCSResourceAttributes getCachedData(CacheID id) const;

                // throw InvalidParameterException;
                CACHE_STATE getResourceCacheState(PrimitiveResourcePtr pResource) const;
                CACHE_STATE getResourceCacheState(CacheID id) const;

                // throw InvalidParameterException;
                bool isCachedData(PrimitiveResourcePtr pResource) const;
                bool isCachedData(CacheID id) const;

            private:
                static ResourceCacheManager *s_instance;
                static std::mutex s_mutex;
                static std::mutex s_mutexForCreation;
                static std::unique_ptr<std::list<DataCachePtr>> s_cacheDataList;
                std::map<CacheID, DataCachePtr> cacheIDmap;

                ResourceCacheManager() = default;
                ~ResourceCacheManager();
                ResourceCacheManager(const ResourceCacheManager &) = delete;
                ResourceCacheManager(ResourceCacheManager &&) = delete;

                ResourceCacheManager &operator=(const ResourceCacheManager &) const = delete;
                ResourceCacheManager &operator=(ResourceCacheManager && ) const = delete;

                static void initializeResourceCacheManager();
                DataCachePtr findDataCache(PrimitiveResourcePtr pResource) const;
                DataCachePtr findDataCache(CacheID id) const;
        };
    } // namespace Service
} // namespace OIC

#endif /* RCM_RESOURCECACHEMANAGER_H_ */

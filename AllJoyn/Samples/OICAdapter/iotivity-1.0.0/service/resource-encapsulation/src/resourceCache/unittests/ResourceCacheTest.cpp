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

#include <iostream>
#include <gtest/gtest.h>
#include <HippoMocks/hippomocks.h>

#include "ResourceCacheManager.h"
#include "UnitTestHelper.h"

using namespace OIC::Service;

class ResourceCacheManagerTest : public TestWithMock
{
    public:
        ResourceCacheManager *cacheInstance;
        PrimitiveResource::Ptr pResource;
        CacheCB cb;
        CacheID id;

    protected:
        virtual void SetUp()
        {
            TestWithMock::SetUp();
            cacheInstance = ResourceCacheManager::getInstance();
            pResource = PrimitiveResource::Ptr(mocks.Mock< PrimitiveResource >(), [](PrimitiveResource *) {});
            cb = ([](std::shared_ptr<PrimitiveResource >, const RCSResourceAttributes &)->OCStackResult {return OC_STACK_OK;});
        }

        virtual void TearDown()
        {
            pResource.reset();
            TestWithMock::TearDown();
        }
};

TEST_F(ResourceCacheManagerTest, requestResourceCache_resourceIsNULL)
{

    CacheCB func = cb;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;
    long reportTime = 20l;

    ASSERT_THROW(cacheInstance->requestResourceCache(NULL, func, rf, reportTime),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, requestResourceCache_cacheCBIsNULL)
{

    CacheCB func = NULL;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;
    long reportTime = 20l;

    ASSERT_THROW(cacheInstance->requestResourceCache(pResource, func, rf, reportTime),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, requestResourceCache_reportTimeIsNULL)
{

    mocks.ExpectCall(pResource.get(), PrimitiveResource::requestGet);
    mocks.ExpectCall(pResource.get(), PrimitiveResource::isObservable).Return(true);
    mocks.ExpectCall(pResource.get(), PrimitiveResource::requestObserve);
    mocks.OnCall(pResource.get(), PrimitiveResource::cancelObserve);

    CacheCB func = cb;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;

    id = cacheInstance->requestResourceCache(pResource, func, rf);
    cacheInstance->cancelResourceCache(id);

    ASSERT_NE(id, 0);

}

TEST_F(ResourceCacheManagerTest, requestResourceCache_normalCase)
{

    mocks.ExpectCall(pResource.get(), PrimitiveResource::requestGet);
    mocks.ExpectCall(pResource.get(), PrimitiveResource::isObservable).Return(true);
    mocks.ExpectCall(pResource.get(), PrimitiveResource::requestObserve);
    mocks.OnCall(pResource.get(), PrimitiveResource::cancelObserve);

    CacheCB func = cb;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;
    long reportTime = 20l;

    id = cacheInstance->requestResourceCache(pResource, func, rf, reportTime);
    cacheInstance->cancelResourceCache(id);

    ASSERT_NE(id, 0);
}

TEST_F(ResourceCacheManagerTest, cancelResourceCache_cacheIDIsZero)
{

    ASSERT_THROW(cacheInstance->cancelResourceCache(0),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, cancelResourceCache_normalCase)
{

    mocks.ExpectCall(pResource.get(), PrimitiveResource::requestGet);
    mocks.ExpectCall(pResource.get(), PrimitiveResource::isObservable).Return(true);
    mocks.ExpectCall(pResource.get(), PrimitiveResource::requestObserve);
    mocks.OnCall(pResource.get(), PrimitiveResource::cancelObserve);

    CacheCB func = cb;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;
    long reportTime = 20l;

    id = cacheInstance->requestResourceCache(pResource, func, rf, reportTime);

    cacheInstance->cancelResourceCache(id);
}

TEST_F(ResourceCacheManagerTest, updateResourceCachePrimitiveResource_resourceIsNULL)
{

    pResource = NULL;

    ASSERT_THROW(cacheInstance->updateResourceCache(pResource),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, updateResourceCachePrimitiveResource_cacheIsNULL)
{

    ASSERT_THROW(cacheInstance->updateResourceCache(pResource),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, updateResourceCachePrimitiveResource_normalCase)
{

    mocks.OnCall(pResource.get(), PrimitiveResource::requestGet);
    mocks.OnCall(pResource.get(), PrimitiveResource::isObservable).Return(true);
    mocks.OnCall(pResource.get(), PrimitiveResource::requestObserve);
    mocks.OnCall(pResource.get(), PrimitiveResource::getUri).Return("testUri");
    mocks.OnCall(pResource.get(), PrimitiveResource::getHost).Return("testHost");
    mocks.OnCall(pResource.get(), PrimitiveResource::cancelObserve);

    CacheCB func = cb;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;
    long reportTime = 20l;

    id = cacheInstance->requestResourceCache(pResource, func, rf, reportTime);

    cacheInstance->updateResourceCache(pResource);

    cacheInstance->cancelResourceCache(id);
}

TEST_F(ResourceCacheManagerTest, updateResourceCacheCacheID_cacheIDIsZero)
{

    ASSERT_THROW(cacheInstance->updateResourceCache(0),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, updateResourceCacheCacheID_cacheIsNULL)
{

    ASSERT_THROW(cacheInstance->updateResourceCache(id),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, updateResourceCacheCacheID_normalCase)
{

    mocks.OnCall(pResource.get(), PrimitiveResource::requestGet);
    mocks.OnCall(pResource.get(), PrimitiveResource::isObservable).Return(true);
    mocks.OnCall(pResource.get(), PrimitiveResource::requestObserve);
    mocks.OnCall(pResource.get(), PrimitiveResource::getUri).Return("testUri");
    mocks.OnCall(pResource.get(), PrimitiveResource::getHost).Return("testHost");
    mocks.OnCall(pResource.get(), PrimitiveResource::cancelObserve);

    CacheCB func = cb;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;
    long reportTime = 20l;

    id = cacheInstance->requestResourceCache(pResource, func, rf, reportTime);

    cacheInstance->updateResourceCache(id);

    cacheInstance->cancelResourceCache(id);
}

TEST_F(ResourceCacheManagerTest, getCachedDataPrimitiveResource_resourceIsNULL)
{

    pResource = NULL;

    ASSERT_THROW(cacheInstance->getCachedData(pResource),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, getCachedDataPrimitiveResource_handlerIsNULL)
{

    ASSERT_THROW(cacheInstance->getCachedData(pResource),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, getCachedDataCachID_resourceIsNULL)
{

    ASSERT_THROW(cacheInstance->getCachedData(0), ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, getCachedDataCachID_handlerIsNULL)
{

    ASSERT_THROW(cacheInstance->getCachedData(id), ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, getResourceCacheStatePrimitiveResource_resourceIsNULL)
{

    pResource = NULL;

    ASSERT_THROW(cacheInstance->getResourceCacheState(pResource),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, getResourceCacheStatePrimitiveResource_handlerIsNULL)
{

    ASSERT_EQ(cacheInstance->getResourceCacheState(pResource), CACHE_STATE::NONE);
}

TEST_F(ResourceCacheManagerTest, getResourceCacheStatePrimitiveResource_normalCase)
{

    mocks.OnCall(pResource.get(), PrimitiveResource::requestGet);
    mocks.OnCall(pResource.get(), PrimitiveResource::isObservable).Return(true);
    mocks.OnCall(pResource.get(), PrimitiveResource::requestObserve);
    mocks.OnCall(pResource.get(), PrimitiveResource::getUri).Return("testUri");
    mocks.OnCall(pResource.get(), PrimitiveResource::getHost).Return("testHost");
    mocks.OnCall(pResource.get(), PrimitiveResource::cancelObserve);

    CacheCB func = cb;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;
    long reportTime = 20l;

    id = cacheInstance->requestResourceCache(pResource, func, rf, reportTime);
    CACHE_STATE state = cacheInstance->getResourceCacheState(pResource);

    cacheInstance->cancelResourceCache(id);

    ASSERT_EQ(state, CACHE_STATE::READY_YET);
}

TEST_F(ResourceCacheManagerTest, getResourceCacheStateCacheID_cacheIDIsZero)
{

    ASSERT_THROW(cacheInstance->getResourceCacheState(0),
                 ResourceCacheManager::InvalidParameterException);
}

TEST_F(ResourceCacheManagerTest, getResourceCacheStateCacheID_handlerIsNULL)
{

    id = 1;
    ASSERT_EQ(cacheInstance->getResourceCacheState(id), CACHE_STATE::NONE);
}

TEST_F(ResourceCacheManagerTest, getResourceCacheStateCacheID_normalCase)
{

    mocks.OnCall(pResource.get(), PrimitiveResource::requestGet);
    mocks.OnCall(pResource.get(), PrimitiveResource::isObservable).Return(true);
    mocks.OnCall(pResource.get(), PrimitiveResource::requestObserve);
    mocks.OnCall(pResource.get(), PrimitiveResource::getUri).Return("testUri");
    mocks.OnCall(pResource.get(), PrimitiveResource::getHost).Return("testHost");
    mocks.OnCall(pResource.get(), PrimitiveResource::cancelObserve);

    CacheCB func = cb;
    REPORT_FREQUENCY rf = REPORT_FREQUENCY::UPTODATE;
    long reportTime = 20l;

    id = cacheInstance->requestResourceCache(pResource, func, rf, reportTime);
    CACHE_STATE state = cacheInstance->getResourceCacheState(id);

    cacheInstance->cancelResourceCache(id);

    ASSERT_EQ(state, CACHE_STATE::READY_YET);
}

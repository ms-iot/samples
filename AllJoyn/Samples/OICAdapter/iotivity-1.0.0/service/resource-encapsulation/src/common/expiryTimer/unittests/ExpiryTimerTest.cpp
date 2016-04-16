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

#include <UnitTestHelper.h>

#include <mutex>
#include <atomic>

#include "RCSException.h"
#include "ExpiryTimer.h"
#include "ExpiryTimerImpl.h"

using namespace OIC::Service;

constexpr int TOLERANCE_IN_MILLIS{ 50 };

class FunctionObject
{
public:
    virtual ~FunctionObject() { }

    virtual void execute(ExpiryTimerImpl::Id) { }
};

class ExpiryTimerImplTest: public TestWithMock
{
public:
    void Proceed()
    {
        cond.notify_all();
    }

    void Wait(int waitingTime = TOLERANCE_IN_MILLIS)
    {
        std::unique_lock< std::mutex > lock{ mutex };
        cond.wait_for(lock, std::chrono::milliseconds{ waitingTime });
    }

private:
    std::condition_variable cond;
    std::mutex mutex;
};


TEST_F(ExpiryTimerImplTest, PostThrowsIfDelayIsNegative)
{
    ASSERT_THROW(ExpiryTimerImpl::getInstance()->post(-1, [](ExpiryTimerImpl::Id){}), RCSException);
}

TEST_F(ExpiryTimerImplTest, PostThrowsIfCallbackIsEmpty)
{
    ASSERT_THROW(ExpiryTimerImpl::getInstance()->post(1, { }), RCSException);
}

TEST_F(ExpiryTimerImplTest, CallbackBeInvokedWithinTolerance)
{
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    mocks.ExpectCall(functor, FunctionObject::execute).Do(
            [this](ExpiryTimerImpl::Id){
                Proceed();
            }
    );

    ExpiryTimerImpl::getInstance()->post(10,
            std::bind(&FunctionObject::execute, functor, std::placeholders::_1));

    Wait();
}

TEST_F(ExpiryTimerImplTest, CallbackBeInvokedWithTimerId)
{
    ExpiryTimerImpl::Id returnedId;
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    mocks.ExpectCall(functor, FunctionObject::execute).Match(
            [this, &returnedId](ExpiryTimerImpl::Id id){
                return returnedId == id;
            }
    ).Do(
            [this](ExpiryTimerImpl::Id){
                Proceed();
            }
    );

    returnedId = ExpiryTimerImpl::getInstance()->post(1,
            std::bind(&FunctionObject::execute, functor, std::placeholders::_1))->getId();

    Wait();
}

TEST_F(ExpiryTimerImplTest, CanceledTaskBeNotCalled)
{
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    mocks.NeverCall(functor, FunctionObject::execute);

    ExpiryTimerImpl::Id id = ExpiryTimerImpl::getInstance()->post(10,
            std::bind(&FunctionObject::execute, functor, std::placeholders::_1))->getId();
    ExpiryTimerImpl::getInstance()->cancel(id);
    Wait(100);
}

TEST_F(ExpiryTimerImplTest, CancelReturnsTrueIfCanceledCorrectly)
{
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    ExpiryTimerImpl::Id id = ExpiryTimerImpl::getInstance()->post(10,
            std::bind(&FunctionObject::execute, functor, std::placeholders::_1))->getId();

    ASSERT_TRUE(ExpiryTimerImpl::getInstance()->cancel(id));
}

TEST_F(ExpiryTimerImplTest, CancelReturnsFalseIfAlreadyExecuted)
{
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    mocks.ExpectCall(functor, FunctionObject::execute).Do(
        [this](ExpiryTimerImpl::Id){
            Proceed();
        }
    );

    ExpiryTimerImpl::Id id = ExpiryTimerImpl::getInstance()->post(1,
            std::bind(&FunctionObject::execute, functor, std::placeholders::_1))->getId();
    Wait();

    ASSERT_FALSE(ExpiryTimerImpl::getInstance()->cancel(id));
}

TEST_F(ExpiryTimerImplTest, CallbackBeInvokedWithinToleranceWithMultiplePost)
{
    constexpr int NUM_OF_POST{ 10000 };
    std::atomic_int called{ 0 };

    for (int i=0; i<NUM_OF_POST; ++i)
    {
        FunctionObject* functor = mocks.Mock< FunctionObject >();
        mocks.OnCall(functor, FunctionObject::execute).Do(
                [&called](ExpiryTimerImpl::Id)
                {
                    ++called;
                }
        );

        ExpiryTimerImpl::getInstance()->post(rand() % 20 + 5,
                std::bind(&FunctionObject::execute, functor, std::placeholders::_1));
    }

    Wait(TOLERANCE_IN_MILLIS + 25);

    ASSERT_EQ(NUM_OF_POST, called);
}

class ExpiryTimerTest: public TestWithMock
{
public:
    ExpiryTimer timer;

public:
    void Proceed()
    {
        cond.notify_all();
    }

    void Wait(int waitingTime = TOLERANCE_IN_MILLIS)
    {
        std::unique_lock< std::mutex > lock{ mutex };
        cond.wait_for(lock, std::chrono::milliseconds{ waitingTime });
    }

private:
    std::condition_variable cond;
    std::mutex mutex;
};

TEST_F(ExpiryTimerTest, PostThrowsIfDelayIsNegative)
{
    ASSERT_THROW(timer.post(-1, [](ExpiryTimer::Id){}), RCSException);
}

TEST_F(ExpiryTimerTest, PostThrowsIfCallbackIsEmpty)
{
    ASSERT_THROW(timer.post(1, { }), RCSException);
}

TEST_F(ExpiryTimerTest, CallbackBeInvokedWithinTolerance)
{
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    mocks.ExpectCall(functor, FunctionObject::execute).Do(
            [this](ExpiryTimer::Id){
                Proceed();
            }
    );

    timer.post(10,
            std::bind(&FunctionObject::execute, functor, std::placeholders::_1));

    Wait();
}

TEST_F(ExpiryTimerTest, CallbackBeInvokedWithTimerId)
{
    ExpiryTimer::Id returnedId;
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    mocks.ExpectCall(functor, FunctionObject::execute).Match(
            [this, &returnedId](ExpiryTimer::Id id){
                return returnedId == id;
            }
    ).Do(
            [this](ExpiryTimer::Id){
                Proceed();
            }
    );

    returnedId = timer.post(1, std::bind(&FunctionObject::execute, functor, std::placeholders::_1));

    Wait();
}

TEST_F(ExpiryTimerTest, CanceledTaskBeNotCalled)
{
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    mocks.NeverCall(functor, FunctionObject::execute);

    auto id = timer.post(10, std::bind(&FunctionObject::execute, functor, std::placeholders::_1));
    timer.cancel(id);
    Wait(100);
}

TEST_F(ExpiryTimerTest, CancelReturnsTrueIfCanceledCorrectly)
{
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    auto id = timer.post(10, std::bind(&FunctionObject::execute, functor, std::placeholders::_1));

    ASSERT_TRUE(timer.cancel(id));
}

TEST_F(ExpiryTimerTest, CancelReturnsFalseIfAlreadyExecuted)
{
    FunctionObject* functor = mocks.Mock< FunctionObject >();

    mocks.ExpectCall(functor, FunctionObject::execute).Do(
        [this](ExpiryTimer::Id){
            Proceed();
        }
    );

    auto id = timer.post(1, std::bind(&FunctionObject::execute, functor, std::placeholders::_1));
    Wait();

    ASSERT_FALSE(timer.cancel(id));
}

TEST_F(ExpiryTimerTest, NumOfPendingReturnsNumberOfNotExecuted)
{
    constexpr size_t numOfFutureTask{ 100 };
    constexpr size_t numOfShortDelayTask{ 100 };

    for (size_t i=0; i<numOfFutureTask; ++i)
    {
        FunctionObject* functor = mocks.Mock< FunctionObject >();
        mocks.OnCall(functor, FunctionObject::execute);

        timer.post(1000, std::bind(&FunctionObject::execute, functor, std::placeholders::_1));
    }

    for (size_t i=0; i<numOfShortDelayTask; ++i)
     {
         FunctionObject* functor = mocks.Mock< FunctionObject >();
         mocks.OnCall(functor, FunctionObject::execute);

         timer.post(i, std::bind(&FunctionObject::execute, functor, std::placeholders::_1));
     }

    Wait(numOfShortDelayTask + TOLERANCE_IN_MILLIS);

    ASSERT_EQ(timer.getNumOfPending(), numOfFutureTask);
}

TEST_F(ExpiryTimerTest, CancelAllCancelsAllTasks)
{
    constexpr size_t numOfTask{ 100 };

    for (size_t i=0; i<numOfTask; ++i)
    {
        FunctionObject* functor = mocks.Mock< FunctionObject >();
        mocks.NeverCall(functor, FunctionObject::execute);

        timer.post(50 + i, std::bind(&FunctionObject::execute, functor, std::placeholders::_1));
    }

    timer.cancelAll();

    Wait(200);
}

TEST_F(ExpiryTimerTest, AllTasksAreCancelledAfterTimerDestroyed)
{
    {
        ExpiryTimer localTimer;
        FunctionObject* functor = mocks.Mock< FunctionObject >();

        mocks.NeverCall(functor, FunctionObject::execute);

        localTimer.post(50,
                std::bind(&FunctionObject::execute, functor, std::placeholders::_1));
    }

    Wait(200);
}

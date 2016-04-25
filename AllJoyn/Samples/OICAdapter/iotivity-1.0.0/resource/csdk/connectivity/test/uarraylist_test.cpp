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

#include "gtest/gtest.h"

#include "uarraylist.h"

class UArrayListF : public testing::Test {
public:
  UArrayListF() :
      testing::Test(),
      list(NULL)
  {
  }

protected:
    virtual void SetUp()
    {
        list = u_arraylist_create();
        ASSERT_TRUE(list != NULL);
    }

    virtual void TearDown()
    {
        u_arraylist_free(&list);
        ASSERT_EQ(NULL, list);
    }

    u_arraylist_t *list;
};

TEST(UArrayList, Base)
{
    u_arraylist_t *list = u_arraylist_create();
    ASSERT_TRUE(list != NULL);

    u_arraylist_free(&list);
    ASSERT_EQ(NULL, list);
}

TEST(UArrayList, CreateMany)
{
    for (int i = 0; i < 100; ++i)
    {
        u_arraylist_t *list = u_arraylist_create();
        ASSERT_TRUE(list != NULL);

        u_arraylist_free(&list);
        ASSERT_EQ(NULL, list);
    }
}

TEST(UArrayList, FreeNull)
{
    u_arraylist_free(NULL);
}

TEST_F(UArrayListF, Length)
{
    ASSERT_EQ(static_cast<uint32_t>(0), u_arraylist_length(list));

    int dummy = 0;
    bool rc = u_arraylist_add(list, &dummy);
    ASSERT_TRUE(rc);

    ASSERT_EQ(static_cast<uint32_t>(1), u_arraylist_length(list));

    // Add a few times without checking, just in case checking has side-effects
    rc = u_arraylist_add(list, &dummy);
    ASSERT_TRUE(rc);
    rc = u_arraylist_add(list, &dummy);
    ASSERT_TRUE(rc);
    rc = u_arraylist_add(list, &dummy);
    ASSERT_TRUE(rc);

    ASSERT_EQ(static_cast<uint32_t>(4), u_arraylist_length(list));
}

TEST_F(UArrayListF, LengthMulti)
{
    ASSERT_EQ(static_cast<uint32_t>(0), u_arraylist_length(list));

    int dummy = 0;
    for (int i = 0; i < 1000; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy);
        ASSERT_TRUE(rc);
    }

    ASSERT_EQ(static_cast<uint32_t>(1000), u_arraylist_length(list));
}

TEST_F(UArrayListF, NoReserve)
{
    static const int PAD_SIZE = 10000;

    int dummy = 0;

    //u_arraylist_reserve(list, PAD_SIZE);

    for (int i = 0; i < PAD_SIZE; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy);
        ASSERT_TRUE(rc);
    }
}

TEST_F(UArrayListF, Reserve)
{
    static const int PAD_SIZE = 10000;

    int dummy = 0;

    u_arraylist_reserve(list, PAD_SIZE);

    for (int i = 0; i < PAD_SIZE; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy);
        ASSERT_TRUE(rc);
    }
}


TEST_F(UArrayListF, ShrinkToFit)
{
    static const int PAD_SIZE = 100;

    int dummy = 0;

    u_arraylist_reserve(list, PAD_SIZE);

    for (int i = 0; i < PAD_SIZE; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy);
        ASSERT_TRUE(rc);
    }

    for (int i = PAD_SIZE; i > 0; --i)
    {
        u_arraylist_remove(list, i);
    }

    EXPECT_GT(list->capacity, list->length);
    u_arraylist_shrink_to_fit(list);
    EXPECT_EQ(list->capacity, list->length);
}

TEST_F(UArrayListF, Get)
{
    ASSERT_EQ(static_cast<uint32_t>(0), u_arraylist_length(list));

    int dummy[1000] = {0};
    size_t cap = sizeof(dummy) / sizeof(dummy[0]);

    for (size_t i = 0; i < cap; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy[i]);
        ASSERT_TRUE(rc);
    }
    ASSERT_EQ(static_cast<uint32_t>(1000), u_arraylist_length(list));

    for (size_t i = 0; i < cap; ++i)
    {
        void *value = u_arraylist_get(list, i);
        ASSERT_TRUE(value != NULL);
        ASSERT_EQ(&dummy[i], value);
    }
}

TEST_F(UArrayListF, Remove)
{
    ASSERT_EQ(static_cast<uint32_t>(0), u_arraylist_length(list));

    int dummy[1000] = {0};
    size_t cap = sizeof(dummy) / sizeof(dummy[0]);

    for (size_t i = 0; i < cap; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy[i]);
        ASSERT_TRUE(rc);
    }
    ASSERT_EQ(static_cast<uint32_t>(1000), u_arraylist_length(list));

    // Remove walking forward so as to have a non-trivial case.
    for (uint32_t idx = 0, old = 0;
         idx < u_arraylist_length(list);
         ++idx, old += 2)
    {
        void *value = u_arraylist_remove(list, idx);
        ASSERT_TRUE(value != NULL);
        ASSERT_EQ(value, &dummy[old]);
    }
    ASSERT_EQ(static_cast<uint32_t>(500), u_arraylist_length(list));
}

TEST_F(UArrayListF, Contains)
{
    ASSERT_EQ(static_cast<uint32_t>(0), u_arraylist_length(list));

    int dummy[1000] = {0};
    size_t cap = sizeof(dummy) / sizeof(dummy[0]);

    for (size_t i = 0; i < cap; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy[i]);
        ASSERT_TRUE(rc);
    }
    ASSERT_EQ(static_cast<uint32_t>(1000), u_arraylist_length(list));

    // Remove walking forward so as to have a non-trivial case.
    for (uint32_t idx = 0, old = 0;
         idx < u_arraylist_length(list);
         ++idx, old += 2)
    {
        void *value = u_arraylist_remove(list, idx);
        ASSERT_TRUE(value != NULL);
        ASSERT_EQ(value, &dummy[old]);
    }
    ASSERT_EQ(static_cast<uint32_t>(500), u_arraylist_length(list));

    // Finally, check that the ones we expect are present, and others are not.
    for (size_t i = 0; i < cap; ++i)
    {
        bool shouldBe = (i & 1) != 0;
        bool found = u_arraylist_contains(list, &dummy[i]);
        ASSERT_EQ(shouldBe, found) << " for entry " << i;
    }
}

// Test to repeatedly add and remove with some contents present so that
// a poor implmentation will thrash memory.
TEST_F(UArrayListF, Thrash)
{
    static const int PAD_SIZE = 1000;
    static const int THRASH_COUNT = 1500;
    ASSERT_EQ(static_cast<uint32_t>(0), u_arraylist_length(list));

    int dummy2 = 0;
    int dummy[PAD_SIZE] = {0};
    size_t cap = sizeof(dummy) / sizeof(dummy[0]);

    for (size_t i = 0; i < cap; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy[i]);
        ASSERT_TRUE(rc);
    }
    ASSERT_EQ(static_cast<uint32_t>(PAD_SIZE), u_arraylist_length(list));

    // Finally add and remove a lot.
    for (int i = 0; i < THRASH_COUNT; ++i)
    {
        bool rc = u_arraylist_add(list, &dummy2);
        ASSERT_TRUE(rc);
        ASSERT_EQ(static_cast<uint32_t>(PAD_SIZE + 1),
                  u_arraylist_length(list));

        ASSERT_EQ(&dummy2,
                  u_arraylist_remove(list, u_arraylist_length(list) - 1));

        ASSERT_EQ(static_cast<uint32_t>(PAD_SIZE), u_arraylist_length(list));
    }
}

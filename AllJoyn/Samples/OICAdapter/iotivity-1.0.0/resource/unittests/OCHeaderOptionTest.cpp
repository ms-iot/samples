//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
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

#include <algorithm>
#include <gtest/gtest.h>
#include <OCHeaderOption.h>
#include <OCException.h>
#include <StringConstants.h>

namespace OC
{
    namespace test
    {
        namespace OCHeaderOptionTests
        {
            using namespace OC;

            TEST(OCHeaderOptionTest, ConstructorValidRangeTest)
            {
                // Note: this test just assures that none of these
                // constructors throw an exception
                for(uint16_t i = HeaderOption::MIN_HEADER_OPTIONID;
                        i < HeaderOption::MAX_HEADER_OPTIONID;
                        ++i)
                {
                    HeaderOption::OCHeaderOption{i, ""};
                }
            }

            TEST(OCHeaderOptionTest, ConstructorTooLowOptionIdTest)
            {
                for(uint16_t i = 0; i < HeaderOption::MIN_HEADER_OPTIONID; ++i)
                {
                    ASSERT_THROW(
                            HeaderOption::OCHeaderOption(i,""),
                            OCException);
                }
            }

            TEST(OCHeaderOptionTest, ConstructorTooHighOptionIdTest)
            {
                for(uint16_t i = HeaderOption::MAX_HEADER_OPTIONID +1 ; i <UINT16_MAX;++i)
                {
                    ASSERT_THROW(
                            HeaderOption::OCHeaderOption(i,""),
                            OCException);
                }
            }

            TEST(OCHeaderOptionTest, OptionIDTest)
            {
                HeaderOption::OCHeaderOption opt {HeaderOption::MIN_HEADER_OPTIONID + 5, ""};
                EXPECT_EQ(HeaderOption::MIN_HEADER_OPTIONID + 5, opt.getOptionID());
            }

            TEST(OCHeaderOptionTest, OptionDataTest)
            {
                std::string optionData {"134kl5jt iopdfgj;lwe45 puiondj;vlk345t89o sdkl;ag"};
                HeaderOption::OCHeaderOption opt {HeaderOption::MIN_HEADER_OPTIONID, optionData};
                EXPECT_EQ(optionData, opt.getOptionData());
            }

        } //namespace OCHeaderOptionTests
    } //namespace test
} //namespace OC

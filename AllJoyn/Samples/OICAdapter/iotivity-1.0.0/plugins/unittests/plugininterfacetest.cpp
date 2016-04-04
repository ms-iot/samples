//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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


#include "plugininterface.h"
#include "gtest/gtest.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <stdint.h>

#include "gtest_helper.h"

namespace itst = iotivity::test;

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------

std::chrono::seconds const SHORT_TEST_TIMEOUT = std::chrono::seconds(5);

//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------

// Plugin Interface API PIStartPlugin()
TEST(PITests, StartPluginTest)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_INVALID_PARAM, PIStartPlugin(NULL, PLUGIN_UNKNOWN, NULL));
    EXPECT_EQ(OC_STACK_INVALID_PARAM, PIStartPlugin("", PLUGIN_UNKNOWN, NULL));
    EXPECT_EQ(OC_STACK_INVALID_PARAM, PIStartPlugin("", PLUGIN_UNKNOWN, NULL));
// Note: The following test is invalid for unit tests. Please do not enable tests which
//       actually enable hardware radios.
//    EXPECT_EQ(OC_STACK_INVALID_PARAM, PIStartPlugin("/dev/ttyUSB0", PLUGIN_ZIGBEE, NULL));
}

// Plugin Interface API PIStopPlugin()
TEST(PITests, StopPluginTest)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_INVALID_PARAM, PIStopPlugin(NULL));
}

// Plugin Interface API PIProcess()
TEST(PITests, ProcessTest)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_INVALID_PARAM, PIProcess(NULL));
}




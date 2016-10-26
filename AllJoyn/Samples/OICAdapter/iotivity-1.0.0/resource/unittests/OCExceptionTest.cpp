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
#include <OCException.h>
#include <StringConstants.h>

namespace OC
{
    namespace test
    {
        namespace OCExceptionTests
        {
            using namespace OC;

            OCStackResult resultCodes[] =
            {
                OC_STACK_OK,
                OC_STACK_RESOURCE_CREATED,
                OC_STACK_RESOURCE_DELETED,
                OC_STACK_CONTINUE,
                OC_STACK_INVALID_URI,
                OC_STACK_INVALID_QUERY,
                OC_STACK_INVALID_IP,
                OC_STACK_INVALID_PORT,
                OC_STACK_INVALID_CALLBACK,
                OC_STACK_INVALID_METHOD,
                OC_STACK_INVALID_PARAM,
                OC_STACK_INVALID_OBSERVE_PARAM,
                OC_STACK_NO_MEMORY,
                OC_STACK_COMM_ERROR,
                OC_STACK_TIMEOUT,
                OC_STACK_ADAPTER_NOT_ENABLED,
                OC_STACK_NOTIMPL,
                OC_STACK_NO_RESOURCE,
                OC_STACK_RESOURCE_ERROR,
                OC_STACK_SLOW_RESOURCE,
                OC_STACK_DUPLICATE_REQUEST,
                OC_STACK_NO_OBSERVERS,
                OC_STACK_OBSERVER_NOT_FOUND,
                OC_STACK_VIRTUAL_DO_NOT_HANDLE,
                OC_STACK_INVALID_OPTION,
                OC_STACK_MALFORMED_RESPONSE,
                OC_STACK_PERSISTENT_BUFFER_REQUIRED,
                OC_STACK_INVALID_REQUEST_HANDLE,
                OC_STACK_INVALID_DEVICE_INFO,
                OC_STACK_INVALID_JSON,
                OC_STACK_UNAUTHORIZED_REQ,
                OC_STACK_PRESENCE_STOPPED,
                OC_STACK_PRESENCE_TIMEOUT,
                OC_STACK_PRESENCE_DO_NOT_HANDLE,
                OC_STACK_ERROR,
                OC_STACK_PDM_IS_NOT_INITIALIZED,
                OC_STACK_DUPLICATE_UUID,
                OC_STACK_INCONSISTENT_DB
            };

            std::string resultMessages[]=
            {
                OC::Exception::NO_ERROR,
                OC::Exception::RESOURCE_CREATED,
                OC::Exception::RESOURCE_DELETED,
                OC::Exception::STACK_CONTINUE,
                OC::Exception::INVALID_URI,
                OC::Exception::INVALID_QUERY,
                OC::Exception::INVALID_IP,
                OC::Exception::INVALID_PORT,
                OC::Exception::INVALID_CB,
                OC::Exception::INVALID_METHOD,
                OC::Exception::INVALID_PARAM,
                OC::Exception::INVALID_OBESERVE,
                OC::Exception::NO_MEMORY,
                OC::Exception::COMM_ERROR,
                OC::Exception::TIMEOUT,
                OC::Exception::ADAPTER_NOT_ENABLED,
                OC::Exception::NOT_IMPL,
                OC::Exception::NOT_FOUND,
                OC::Exception::RESOURCE_ERROR,
                OC::Exception::SLOW_RESOURCE,
                OC::Exception::DUPLICATE_REQUEST,
                OC::Exception::NO_OBSERVERS,
                OC::Exception::OBSV_NO_FOUND,
                OC::Exception::VIRTUAL_DO_NOT_HANDLE,
                OC::Exception::INVALID_OPTION,
                OC::Exception::MALFORMED_STACK_RESPONSE,
                OC::Exception::PERSISTENT_BUFFER_REQUIRED,
                OC::Exception::INVALID_REQUEST_HANDLE,
                OC::Exception::INVALID_DEVICE_INFO,
                OC::Exception::INVALID_REPRESENTATION,
                OC::Exception::UNAUTHORIZED_REQUEST,
                OC::Exception::PRESENCE_STOPPED,
                OC::Exception::PRESENCE_TIMEOUT,
                OC::Exception::PRESENCE_NOT_HANDLED,
                OC::Exception::GENERAL_FAULT,
                OC::Exception::PDM_DB_NOT_INITIALIZED,
                OC::Exception::DUPLICATE_UUID,
                OC::Exception::INCONSISTENT_DB
            };
            TEST(OCExceptionTest, ReasonCodeMatches)
            {
                for(OCStackResult res : resultCodes)
                {
                    OCException ex{"", res};
                    EXPECT_EQ(res, ex.code());
                }
            }

            TEST(OCExceptionTest, MessageCodeMatches)
            {
                std::string exceptionMessage = "This is the exception message!";
                OCException ex {exceptionMessage, OC_STACK_OK};

                EXPECT_EQ(exceptionMessage, ex.what());
            }

            TEST(OCExceptionTest, ReasonMapping)
            {
                int i=0;
                for(OCStackResult res : resultCodes)
                {
                    OCException ex{"", res};
                    EXPECT_EQ(resultMessages[i], ex.reason());
                    ++i;
                }
            }

            TEST(OCExceptionTest, UnknownReasonMappings)
            {
                for(int i = 0; i < OC_STACK_ERROR; ++i)
                {
                    if(std::find(
                                std::begin(resultCodes),
                                std::end(resultCodes),
                                static_cast<OCStackResult>(i))
                            == std::end(resultCodes))
                    {
                        OCException ex {"", static_cast<OCStackResult>(i)};
                        EXPECT_EQ(OC::Exception::UNKNOWN_ERROR, ex.reason());
                    }
                }
            }
        } //namespace OCExceptionTests
    } //namespace test
} //namespace OC

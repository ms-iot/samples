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

#include "gtest/gtest.h"
#include <pwd.h>
#include <grp.h>
#include <linux/limits.h>
#include "ocstack.h"
#include "cainterface.h"
#include "srmresourcestrings.h"

using namespace std;

#define PE_UT_TAG "\tPE-UT-message: "

#ifdef __cplusplus
extern "C" {
#endif

#include "policyengine.h"
#include "doxmresource.h"

// test parameters
PEContext_t g_peContext;

#ifdef __cplusplus
}
#endif

OicUuid_t g_subjectIdA = {"SubjectA"};
OicUuid_t g_subjectIdB = {"SubjectB"};
OicUuid_t g_devOwner;
char g_resource1[] = "Resource1";
char g_resource2[] = "Resource2";

//Policy Engine Core Tests
TEST(PolicyEngineCore, InitPolicyEngine)
{
    EXPECT_EQ(OC_STACK_OK, InitPolicyEngine(&g_peContext));
}

TEST(PolicyEngineCore, CheckPermissionNoAcls)
{
    EXPECT_EQ(ACCESS_DENIED_SUBJECT_NOT_FOUND,
        CheckPermission(&g_peContext,
                        &g_subjectIdA,
                        g_resource1,
                        PERMISSION_READ));
}

//TODO This won't work until we figure out how to OcInit() or equivalent.
TEST(PolicyEngineCore, CheckDevOwnerRequest)
{
    if(OC_STACK_OK == InitDoxmResource())
    {
        if(OC_STACK_OK == GetDoxmDevOwnerId(&g_devOwner))
        {
            printf("%s", PE_UT_TAG);
            for(int i = 0; i < UUID_LENGTH; i++)
            {
                printf("%d", g_devOwner.id[i]);
            }
            printf("\n");
                EXPECT_EQ(ACCESS_GRANTED,
                    CheckPermission(&g_peContext,
                        &g_devOwner,
                        g_resource1,
                        PERMISSION_FULL_CONTROL));
        }
        else
        {
            printf("%s WARNING: InitDoxmResource() returned ERROR!\n", \
                PE_UT_TAG);
        }
    }
    else
    {
        printf("%s WARNING: GetDoxmDevOwnerId() returned ERROR!\n", PE_UT_TAG);
    }


}

TEST(PolicyEngineCore, DeInitPolicyEngine)
{
    DeInitPolicyEngine(&g_peContext);
    EXPECT_EQ(STOPPED, g_peContext.state);
    EXPECT_EQ((uint16_t)0, g_peContext.permission);
    EXPECT_FALSE(g_peContext.matchingAclFound);
    EXPECT_EQ(ACCESS_DENIED_POLICY_ENGINE_ERROR, g_peContext.retVal);
}

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
#include <sys/stat.h>
#include "ocstack.h"
#include "oic_malloc.h"
#include "cJSON.h"
#include "cainterface.h"
#include "secureresourcemanager.h"
#include "securevirtualresourcetypes.h"
#include "srmresourcestrings.h"
#include "svcresource.h"
#include "srmtestcommon.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
extern char * BinToSvcJSON(const OicSecSvc_t * svc);
extern OicSecSvc_t * JSONToSvcBin(const char * jsonStr);
extern void DeleteSVCList(OicSecSvc_t* svc);
#ifdef __cplusplus
}
#endif

static const char* JSON_FILE_NAME = "oic_unittest.json";

#define NUM_SVC_IN_JSON_DB (2)


// JSON Marshalling Tests
TEST(SVCResourceTest, JSONMarshallingTests)
{
    char *jsonStr1 = ReadFile(JSON_FILE_NAME);
    if (jsonStr1)
    {
        OicSecSvc_t * svc = JSONToSvcBin(jsonStr1);
        EXPECT_TRUE(NULL != svc);

        int cnt = 0;
        OicSecSvc_t * tempSvc = svc;
        while(tempSvc)
        {

            EXPECT_EQ(tempSvc->svct, ACCESS_MGMT_SERVICE);
            cnt++;
            tempSvc = tempSvc->next;
        }
        EXPECT_EQ(cnt, NUM_SVC_IN_JSON_DB);

        char * jsonStr2 = BinToSvcJSON(svc);
        EXPECT_TRUE(NULL != jsonStr2);

        OICFree(jsonStr1);
        OICFree(jsonStr2);
        DeleteSVCList(svc);
    }
}


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
#include "secureresourcemanager.h"

using namespace std;

// Helper Methods
void UTRequestHandler(const CAEndpoint_t * /*endPoint*/,
                      const CARequestInfo_t * /*requestInfo*/)
{
    EXPECT_TRUE(true) << "UTRequestHandler\n";
}

void UTResponseHandler(const CAEndpoint_t * /*endPoint*/,
                       const CAResponseInfo_t * /*responseInfo*/)
{
     EXPECT_TRUE(true) << "UTResponseHandler\n";
}

void UTErrorHandler(const CAEndpoint_t * /*endPoint*/,
                    const CAErrorInfo_t * /*errorInfo*/)
{
     EXPECT_TRUE(true) << "UTErrorHandler\n";
}

FILE *utopen(const char *path, const char *mode)
{
    EXPECT_TRUE((path != NULL)) << "utopen\n";
    FILE *stream = fopen(path, mode);
    return stream;

}

size_t utread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread(ptr, size, nmemb, stream);
}

size_t utwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

int utclose(FILE *fp)
{
    EXPECT_TRUE((fp != NULL)) << "utclose\n";
    return fclose(fp);
}
int utunlink(const char *path)
{
    EXPECT_TRUE((path != NULL)) << "utunlink\n";
    return unlink(path);
}
static OCPersistentStorage gpsi;

//RegisterHandler Tests
TEST(RegisterHandlerTest, RegisterNullRequestHandler)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRMRegisterHandler(NULL, UTResponseHandler, NULL));
}

TEST(RegisterHandlerTest, RegisterNullResponseHandler)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRMRegisterHandler(UTRequestHandler, NULL, NULL));
}

TEST(RegisterHandlerTest, RegisterNullHandler)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRMRegisterHandler(NULL, NULL, NULL));
}

TEST(RegisterHandlerTest, RegisterValidHandler)
{
    EXPECT_EQ(OC_STACK_OK, SRMRegisterHandler(UTRequestHandler, UTResponseHandler, UTErrorHandler));
}

// PersistentStorageHandler Tests
TEST(PersistentStorageHandlerTest, RegisterNullHandler)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM,
            SRMRegisterPersistentStorageHandler(NULL));
}

TEST(PersistentStorageHandlerTest, RegisterValidHandler)
{
    gpsi.open = utopen;
    gpsi.read = utread;
    gpsi.write = utwrite;
    gpsi.close = utclose;
    gpsi.unlink = utunlink;

    EXPECT_EQ(OC_STACK_OK,
            SRMRegisterPersistentStorageHandler(&gpsi));
    OCPersistentStorage *ps = SRMGetPersistentStorageHandler();
    EXPECT_TRUE(&gpsi == ps);
}

TEST(PersistentStorageHandlerTest, PersistentStorageValidHandlers)
{
    OCPersistentStorage *psi = SRMGetPersistentStorageHandler();
    EXPECT_TRUE(psi != NULL);

    unsigned char buf[PATH_MAX];
    FILE* streamIn = NULL;
    FILE* streamOut = NULL;
    struct passwd *pw = getpwuid(getuid());
    const char *homeDir = pw->pw_dir;
    char inFilePath [PATH_MAX];
    char outFilePath [PATH_MAX];
    snprintf(inFilePath, PATH_MAX, "%s/iotivity/Readme.scons.txt", homeDir );
    snprintf(outFilePath, PATH_MAX, "%s/Downloads/Readme.scons.out.txt", homeDir );

    streamIn = psi->open(inFilePath, "r");
    streamOut = psi->open(outFilePath, "w");

    if (streamIn && streamOut)
    {
        size_t value = 1;
        while (value)
        {
            value = psi->read(buf, 1, sizeof(buf), streamIn);
            psi->write(buf, 1, value, streamOut);
        }
    }

    if (streamIn)
    {
        psi->close(streamIn);
    }
    if (streamOut)
    {
        psi->close(streamOut);
    }
    psi->unlink(outFilePath);
}

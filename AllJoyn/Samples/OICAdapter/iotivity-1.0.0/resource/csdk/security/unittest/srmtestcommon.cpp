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
#include "oic_malloc.h"
#include "ocstack.h"

char* ReadFile(const char* filename)
{

    FILE *fp = NULL;
    char *data = NULL;
    struct stat st;
    // TODO: Find the location of the executable and concatenate the SVR file name
    // before opening it.
    fp = fopen(filename, "r");
    if (fp)
    {
        if (stat(filename, &st) == 0)
        {
            data = (char*)OICMalloc(st.st_size);
            if (data)
            {
                if (fread(data, 1, st.st_size, fp) != (size_t)st.st_size)
                {
                    printf("Error in reading file %s", filename);
                }
            }
        }
        fclose(fp);
    }
    else
    {
        printf("Unable to open %s file", filename);
    }

    return data;
}

void SetPersistentHandler(OCPersistentStorage *ps, bool set)
{
    if (set)
    {
        ps->open = fopen;
        ps->read = fread;
        ps->write = fwrite;
        ps->close = fclose;
        ps->unlink = unlink;
    }
    else
    {
        memset(ps, 0, sizeof(OCPersistentStorage));
    }
    EXPECT_EQ(OC_STACK_OK,
            OCRegisterPersistentStorageHandler(ps));
}

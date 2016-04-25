// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "gtest/gtest.h"
#include "ocstack.h"
#include "srmutility.h"
#include "oic_string.h"

//ParseRestQuery Tests
TEST(ParseRestQueryTest, ParseRestQueryEmpty)
{
    unsigned char query[] = "";
    OicParseQueryIter_t parseIter =  OicParseQueryIter_t();
    ParseQueryIterInit(query, &parseIter);
    EXPECT_EQ(NULL,  GetNextQuery(&parseIter));
}


TEST(ParseRestQueryTest, ParseSingleRestQuery)
{
    char attr[10], val[10];
    unsigned char query[] = "owned=false";

    OicParseQueryIter_t parseIter =  OicParseQueryIter_t();
    ParseQueryIterInit(query, &parseIter);
    EXPECT_NE((OicParseQueryIter_t *)NULL,  GetNextQuery(&parseIter));

    OICStrcpyPartial(attr, sizeof(attr), (char *)parseIter.attrPos, parseIter.attrLen);
    OICStrcpyPartial(val, sizeof(val), (char *)parseIter.valPos, parseIter.valLen);
    printf("\nAttribute: %s  value: %s\n\n", attr, val);
}

TEST(ParseRestQueryTest, ParseRestMultipleQuery)
{
    char attr[10], val[10];
    unsigned char query[] = "oxm=0;owned=true;owner=owner1";

    OicParseQueryIter_t parseIter =  OicParseQueryIter_t();
    ParseQueryIterInit(query, &parseIter);
    printf("\n");
    while(GetNextQuery(&parseIter))
    {
        EXPECT_NE(static_cast<size_t>(0),  parseIter.pi.segment_length);

        OICStrcpyPartial(attr, sizeof(attr), (char *)parseIter.attrPos, parseIter.attrLen);
        OICStrcpyPartial(val, sizeof(val), (char *)parseIter.valPos, parseIter.valLen);
        printf("Attribute: %s  value: %s\n", attr, val);

    }
    printf("\n");
}

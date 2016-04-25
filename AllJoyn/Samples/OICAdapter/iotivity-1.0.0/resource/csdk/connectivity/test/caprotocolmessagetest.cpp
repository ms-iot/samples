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

#include <stdio.h>

#include "gtest/gtest.h"

#include "caprotocolmessage.h"

namespace {

class CoAPOptionCase
{
public:
    unsigned short key;
    unsigned int length;
    std::string dataStr; // data could be binary... for testing we'll use str
};



/**
 * Helper to validate the state of CoAP URI parsing.
 *
 * @param cases array of expected parse results.
 * @param numCases number of expected parse results.
 * @param optlist parsed option list to verify.
 */
void verifyParsedOptions(CoAPOptionCase const *cases,
			 size_t numCases,
			 coap_list_t *optlist)
{
    size_t index = 0;
    for (coap_list_t *opt = optlist; opt; opt = opt->next)
    {
        coap_option *option = (coap_option *) opt->data;
        EXPECT_TRUE(option != NULL);
        EXPECT_LT(index, numCases);
        if (option && (index < numCases))
        {
            unsigned short key = COAP_OPTION_KEY(*option);
            unsigned int length = COAP_OPTION_LENGTH(*option);
            std::string dataStr((const char*)COAP_OPTION_DATA(*option), length);
            // First validate the test case:
            EXPECT_EQ(cases[index].length, cases[index].dataStr.length());

            // Ensure data matches expected parsing
            EXPECT_EQ(cases[index].key, key);
            EXPECT_EQ(cases[index].length, length);
            EXPECT_EQ(cases[index].dataStr, dataStr);
        }

        index++;
    }
    // Ensure we saw the proper number of parts:
    EXPECT_EQ(numCases, index);
}

} // namespace

TEST(CAProtocolMessage, CAParseURIBase)
{
    char sampleURI[] = "coap://[::]/oic/res?rt=core.sensor;if=core.mi.ll";
    CoAPOptionCase cases[] = {
        {COAP_OPTION_URI_PATH, 3, "oic"},
        {COAP_OPTION_URI_PATH, 3, "res"},
        {COAP_OPTION_URI_QUERY, 14, "rt=core.sensor"},
        {COAP_OPTION_URI_QUERY, 13, "if=core.mi.ll"},
    };
    size_t numCases = sizeof(cases) / sizeof(cases[0]);


    coap_list_t *optlist = NULL;
    CAParseURI(sampleURI, &optlist);


    verifyParsedOptions(cases, numCases, optlist);
    coap_delete_list(optlist);
}

// Try for multiple URI path components that still total less than 128
TEST(CAProtocolMessage, CAParseURIManyPath)
{
    char sampleURI[] = "coap://[::]"
        "/medium/a/b/c/d/e/f/g/h/i/j/"
        "?rt=core.sensor;if=core.mi.ll";

    CoAPOptionCase cases[] = {
        {COAP_OPTION_URI_PATH, 6, "medium"},
        {COAP_OPTION_URI_PATH, 1, "a"},
        {COAP_OPTION_URI_PATH, 1, "b"},
        {COAP_OPTION_URI_PATH, 1, "c"},
        {COAP_OPTION_URI_PATH, 1, "d"},
        {COAP_OPTION_URI_PATH, 1, "e"},
        {COAP_OPTION_URI_PATH, 1, "f"},
        {COAP_OPTION_URI_PATH, 1, "g"},
        {COAP_OPTION_URI_PATH, 1, "h"},
        {COAP_OPTION_URI_PATH, 1, "i"},
        {COAP_OPTION_URI_PATH, 1, "j"},
        {COAP_OPTION_URI_QUERY, 14, "rt=core.sensor"},
        {COAP_OPTION_URI_QUERY, 13, "if=core.mi.ll"},
    };
    size_t numCases = sizeof(cases) / sizeof(cases[0]);


    coap_list_t *optlist = NULL;
    CAParseURI(sampleURI, &optlist);


    verifyParsedOptions(cases, numCases, optlist);
    coap_delete_list(optlist);
}

// Try for multiple URI parameters that still total less than 128
TEST(CAProtocolMessage, CAParseURIManyParams)
{
    char sampleURI[] = "coap://[::]/oic/res/"
        "?rt=core.sensor;a=0;b=1;c=2;d=3;e=4;f=5;g=6;h=7;i=8;j=9";

    CoAPOptionCase cases[] = {
        {COAP_OPTION_URI_PATH, 3, "oic"},
        {COAP_OPTION_URI_PATH, 3, "res"},
        {COAP_OPTION_URI_QUERY, 14, "rt=core.sensor"},
        {COAP_OPTION_URI_QUERY, 3, "a=0"},
        {COAP_OPTION_URI_QUERY, 3, "b=1"},
        {COAP_OPTION_URI_QUERY, 3, "c=2"},
        {COAP_OPTION_URI_QUERY, 3, "d=3"},
        {COAP_OPTION_URI_QUERY, 3, "e=4"},
        {COAP_OPTION_URI_QUERY, 3, "f=5"},
        {COAP_OPTION_URI_QUERY, 3, "g=6"},
        {COAP_OPTION_URI_QUERY, 3, "h=7"},
        {COAP_OPTION_URI_QUERY, 3, "i=8"},
        {COAP_OPTION_URI_QUERY, 3, "j=9"},
    };
    size_t numCases = sizeof(cases) / sizeof(cases[0]);


    coap_list_t *optlist = NULL;
    CAParseURI(sampleURI, &optlist);


    verifyParsedOptions(cases, numCases, optlist);
    coap_delete_list(optlist);
}

// Test that an initial long path component won't hide latter ones.
TEST(CAProtocolMessage, CAParseURILongPath)
{
    char sampleURI[] = "coap://[::]/oic"
        "123456789012345678901234567890123456789012345678901234567890"
        "12345678901234567890123456789012345678901234567890"
        "/res?rt=core.sensor;if=core.mi.ll";

    CoAPOptionCase cases[] = {
        {COAP_OPTION_URI_PATH, 113, "oic"
	 "123456789012345678901234567890123456789012345678901234567890"
	 "12345678901234567890123456789012345678901234567890"},
        {COAP_OPTION_URI_PATH, 3, "res"},
        {COAP_OPTION_URI_QUERY, 14, "rt=core.sensor"},
        {COAP_OPTION_URI_QUERY, 13, "if=core.mi.ll"},
    };
    size_t numCases = sizeof(cases) / sizeof(cases[0]);


    coap_list_t *optlist = NULL;
    CAParseURI(sampleURI, &optlist);


    verifyParsedOptions(cases, numCases, optlist);
    coap_delete_list(optlist);
}

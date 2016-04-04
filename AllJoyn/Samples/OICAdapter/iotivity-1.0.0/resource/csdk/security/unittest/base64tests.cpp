 /******************************************************************
  *
  * Copyright 2015 Samsung Electronics All Rights Reserved.
  *
  *
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************/

#include "gtest/gtest.h"
#include "base64.h"
#include <stdlib.h>
#include <stdint.h>

// Tests for base64 encode function
TEST(B64EncodeTest, ValidInputForEncoding)
{
    char buf[128];
    uint32_t outputLength;
    uint32_t expectedLength;
    uint32_t i=0;
    B64Result res = B64_OK;

    const char* input = "IoTivity base64~!@#$%^&*()-=0123456789<>?;:'[]{},.\"\\|";

    /**< expected output is generated from
             "http://www.convertstring.com/EncodeDecode/Base64Encode" */
    const char* expectedOutput[53] = {
        "SQ==",
        "SW8=",
        "SW9U",
        "SW9UaQ==",
        "SW9UaXY=",
        "SW9UaXZp",
        "SW9UaXZpdA==",
        "SW9UaXZpdHk=",
        "SW9UaXZpdHkg",
        "SW9UaXZpdHkgYg==",
        "SW9UaXZpdHkgYmE=",
        "SW9UaXZpdHkgYmFz",
        "SW9UaXZpdHkgYmFzZQ==",
        "SW9UaXZpdHkgYmFzZTY=",
        "SW9UaXZpdHkgYmFzZTY0",
        "SW9UaXZpdHkgYmFzZTY0fg==",
        "SW9UaXZpdHkgYmFzZTY0fiE=",
        "SW9UaXZpdHkgYmFzZTY0fiFA",
        "SW9UaXZpdHkgYmFzZTY0fiFAIw==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQ=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQl",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXg==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiY=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYq",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCk=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCkt",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPQ==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTA=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAx",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMg==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NQ==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3OA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pg==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj8=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87Og==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87Oic=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87Oidb",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXQ==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXs=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4i",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4iXA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4iXHw="
    };

    for(i=0; i< strlen(input); i++)
    {
        memset(buf, 0, sizeof(buf));

        expectedLength = strlen(expectedOutput[i]);

        res = b64Encode((uint8_t*)input, i + 1, buf, 128, &outputLength);

        EXPECT_EQ(B64_OK, res);
        EXPECT_EQ(expectedLength, outputLength);
        EXPECT_EQ(0, strcmp(expectedOutput[i], buf));
    }
}

// Tests for base64 decode function
TEST(B64DeodeTest, ValidInputForDecoding)
{
    uint8_t buf[128];
    uint32_t outputLength;
    uint32_t i=0;
    B64Result res = B64_OK;

    const char* input[53] = {
        "SQ==",
        "SW8=",
        "SW9U",
        "SW9UaQ==",
        "SW9UaXY=",
        "SW9UaXZp",
        "SW9UaXZpdA==",
        "SW9UaXZpdHk=",
        "SW9UaXZpdHkg",
        "SW9UaXZpdHkgYg==",
        "SW9UaXZpdHkgYmE=",
        "SW9UaXZpdHkgYmFz",
        "SW9UaXZpdHkgYmFzZQ==",
        "SW9UaXZpdHkgYmFzZTY=",
        "SW9UaXZpdHkgYmFzZTY0",
        "SW9UaXZpdHkgYmFzZTY0fg==",
        "SW9UaXZpdHkgYmFzZTY0fiE=",
        "SW9UaXZpdHkgYmFzZTY0fiFA",
        "SW9UaXZpdHkgYmFzZTY0fiFAIw==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQ=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQl",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXg==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiY=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYq",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCk=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCkt",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPQ==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTA=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAx",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMg==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NQ==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3OA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pg==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj8=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87Og==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87Oic=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87Oidb",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXQ==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXs=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4i",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4iXA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4iXHw="
    };
    const char* expectedOutput = "IoTivity base64~!@#$%^&*()-=0123456789<>?;:'[]{},.\"\\|";

    for(i=0; i< (sizeof(input)/sizeof(char*)); i++)
    {
        memset(buf, 0, sizeof(buf));

        res = b64Decode(input[i], strlen(input[i]), buf, 128, &outputLength);

        EXPECT_EQ(B64_OK, res);
        EXPECT_EQ(i + 1, outputLength);
        EXPECT_EQ(0, memcmp(expectedOutput, buf, i + 1));
    }
}

// Tests for base64 decode function
TEST(B64DeodeTest, InvalidInputForDecoding)
{
    uint8_t buf[128];
    uint32_t outputLength;
    uint32_t i=0;

    const char* input[53] = {
        "SQ=",
        "Sw8=",
        "SW1U",
        "SW9Uaq==",
        "SW9uaXY=",
        "SW91UaXZp",
        "SW9UaXZpdA.==",
        "Sw9UAXZpdHk=",
        "SW9UAXZpdHkg",
        "SW9UaXZpdhkgYg==",
        "SW9UaXZpd5kgYmE=",
        "SW9UaXZ1dHkgYmFz",
        "SW9UaXZpdHkgymFzZQ==",
        "SW9UaXZpdHkgYmFzZTY==",
        "SW9UaXZpdHkgYmFzZTY0=",
        "SW9UaXZpdHkgYmFzZTY0fg=",
        "SW9UaXZpdHkgYmFzZTY0fiE==",
        "SW8UaXZpdHkgYmFzZTY0fiFA",
        "SW9UaxzPDHkgYmFzZTY0fiFAIw==",
        "SW9UaXZpdHKGYmFzZTY0fiFAIyQ=",
        "SW9UaXZpdHkgYmFZztY0fiFAIyQl=",
        "SW8UaXZpdHkgYmFzZTY0fiFAIyQlXg=",
        "#SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiY=",
        "SSW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYq",
        "SW9UaXZpdHkgYmFzZTY0fiFAiyQlXiYqKA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCk===",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKckt",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKcktpQ==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQiXiYqKCktPTA=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAx=",
        "SW9UaXZpdHkgYmFzZTY0fifAIyQlXiYqKCktPTAxmg=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM#1=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM1",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0Nq==",
        "sw9uaxzpdhkgymfzzty0fifaiyqlxiyqKcktptaxmjm0nty=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0nTY3",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3Ok==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3OA==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pg",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pg=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjm0NTY3ODk8Pj9=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXqKCktPTAxMjM0NTY3ODk8Pj87=",
        "SW9UaXZpdHkgYmFzZTY0fiFaIyiYqKCktPTAxMjM0NTY3ODk8Pj87Og==",
        "W9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87Oic1=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCkTAxMjM0NTY3ODk8Pj87Oidb==",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYCktPTAxMjM0NTY3ODk8Pj87Oidbxq==",
        "SW9UaXZpdHkgYmzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87Oidbxxs=",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCkPTAxMjM0NTY3ODk8Pj87OidbXXT9",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9la==",
        "SW9UaXZpdHkgYzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9Lc4=",
        "SW9UaXZpdHkgYmFzZ0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9lC4i",
        "SW9UaXZpdHkgYmFzZTY0fiFAIyQlXiYqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9lc4iXA==",
        "SW9UaXZpdHkgYmFzZTY0fiFqKCktPTAxMjM0NTY3ODk8Pj87OidbXXt9LC4ixHw="
    };
    const char* expectedOutput = "IoTivity base64~!@#$%^&*()-=0123456789<>?;:'[]{},.\"\\|";

    for(i=0; i< (sizeof(input)/sizeof(char*)); i++)
    {
        memset(buf, 0, sizeof(buf));

        if( B64_OK == b64Decode(input[i], strlen(input[i]), buf, 128, &outputLength) )
        {
            EXPECT_NE(0, memcmp(expectedOutput, buf, i + 1));
        }
    }
}


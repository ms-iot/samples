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
 *      LICENSE-2.0" target="_blank">http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *

 ******************************************************************/

#include "der_dec.h"

/**
 * Computes length.
 */
PKIError DecodeLength(ByteArray *code, size_t *length)
{
    FUNCTION_INIT(
        CHECK_NULL_BYTE_ARRAY_PTR(code, PKI_NULL_PASSED);
    );
    CHECK_INC_BYTE_ARRAY_PTR(code, 1);

    if ((*(code->data)) < LEN_LONG)
    {
        *length = *(code->data);
        CHECK_INC_BYTE_ARRAY_PTR(code, 1);
    }
    else
    {
        uint8_t i = 0;
        uint8_t blocksNum = *(code->data) - LEN_LONG;
        CHECK_INC_BYTE_ARRAY_PTR(code, 1);
        CHECK_LESS(blocksNum, 5, PKI_WRONG_OCTET_LEN);
        *length = 0;

        for (i = 0; i < blocksNum; ++i)
        {
            *length |= *(code->data) << ((blocksNum - i - 1) * SIZE_OF_BYTE);
            CHECK_INC_BYTE_ARRAY_PTR(code, 1);
        }
    }

    //should be: length  <=  array size
    CHECK_LESS_EQUAL(*length, code->len, PKI_WRONG_OCTET_LEN);
    FUNCTION_CLEAR();
}

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

#include "sn_store.h"
#include <string.h>
#include <stdlib.h>

//Size of initial memory
#define CRL_BLOCK_LEN 20


/**
 * @struct SNStore
 *
 * General structure for storing serial numbers.
 *
 * Contains pointer to array of bytes and it's length.
 */
typedef struct
{
    ByteArray array;     /**< Byte array with data*/
    size_t blockNumber;  /**< Number of used memory blocks */
} SNStore;


/**
 * Internal storage for serial numbers.
 */
static SNStore  Store = {{NULL, 0}, 1};

// Frees memory occupied by SN storage.
void FreeSNStore(void)
{
    free(Store.array.data);
    INIT_BYTE_ARRAY(Store.array);
    Store.blockNumber = 1;
}

// Stores serial number to SN storage.
PKIError StoreSerialNumber(const ByteArray sn)
{
    FUNCTION_INIT(
        CHECK_NULL(sn.data, PKI_NULL_PASSED);
        uint8_t *temp = NULL;
    );
    if (Store.array.len == 0 || Store.array.len + sn.len + 1 > CRL_BLOCK_LEN * Store.blockNumber)
    {
        temp = (uint8_t *) realloc(Store.array.data,
                                    sizeof(uint8_t) * CRL_BLOCK_LEN * Store.blockNumber * 2);
        CHECK_NULL(temp, PKI_MEMORY_ALLOC_FAILED);
        Store.array.data = temp;
        Store.blockNumber *= 2;
    }
    Store.array.data[Store.array.len] = sn.len;
    memcpy(&Store.array.data[Store.array.len + 1], sn.data, sn.len);
    Store.array.len += sn.len + 1;
    FUNCTION_CLEAR(
        if (error_value != PKI_SUCCESS)  free(temp);
    );
}


// Checks whether there is serial number in SN storage
PKIError CheckSerialNumber(const ByteArray sn)
{
    FUNCTION_INIT(
        int i, res;
        CHECK_NULL(sn.data, PKI_NULL_PASSED);
    );
    CHECK_NULL(Store.array.data, PKI_SUCCESS);
    for ( i = 0; i < Store.array.len; i += Store.array.data[i] + 1)
    {
        if (sn.len == Store.array.data[i])
        {
            res  = memcmp(&Store.array.data[i + 1], sn.data, sn.len);
            CHECK_NOT_EQUAL(res, 0, PKI_CERT_REVOKED);
        }
    }
    FUNCTION_CLEAR();
}

#ifdef X509_DEBUG
//Prints store content
void PrintSNStore(void)
{
    ByteArray curr;
    int i, count = 0;
    if (Store.array.data != NULL)
    {
        for ( i = 0; i < Store.array.len; i += Store.array.data[i] + 1)
        {
            curr.len = Store.array.data[i];
            curr.data = &Store.array.data[i + 1];
            PRINT_BYTE_ARRAY("", curr);
            count++;
        }
    }
    printf("\nSN STORE CONTAINS %d ELEMENTS\n", count);
}
#endif //DEBUG

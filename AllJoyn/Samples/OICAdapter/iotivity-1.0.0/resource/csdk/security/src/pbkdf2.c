/* *****************************************************************
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
 * *****************************************************************/
#include <string.h>
#include <math.h>
#include "pbkdf2.h"
#include "hmac.h"
#include "debug.h"
#include "logger.h"

#define TAG "PBDKF2"
#define XOR_BUF(in, out, bufSize)\
do{\
    size_t i=0;\
    for(i=0; i< (bufSize); i++)\
    {\
        (out)[i] = (in)[i] ^ (out)[i];\
    }\
}while(0)\


static int isLittle()
{
    static int a = 1;
    static int flag = -1;
    if (flag == -1)
    {
        if (  ((uint8_t *)&a)[0]  == 0x1) // little
            flag = 1;
        else
            flag = 0;
    }
    return flag;
}

static void GetBigEndianBuf(uint8_t *buf, int num)
{
    uint8_t *nBuf = (uint8_t *)&num;
    if ( isLittle() == 1 )
    {
        size_t i = 0;
        for (i = 0; i < sizeof(int); i++)
        {
            buf[i] = nBuf[ sizeof(int) - i - 1];
        }
    }
    else
    {
        memcpy(buf, nBuf, sizeof(int));
    }
}

// TODO: Add comments to explain implementation.
int DeriveCryptoKeyFromPassword(const unsigned char *passwd, size_t pLen,
                                const uint8_t *salt, const size_t saltLen,
                                const size_t iterations,
                                const size_t keyLen, uint8_t *derivedKey)
{
    int res = 0;
    uint8_t buf[DTLS_HMAC_DIGEST_SIZE];
    uint8_t uBuf[DTLS_HMAC_DIGEST_SIZE];

    size_t nBlocks = 0;
    size_t nOctetInLastBlock = 0;

    nBlocks = (size_t)ceil ((double)keyLen / (double)DTLS_HMAC_DIGEST_SIZE);
    nOctetInLastBlock = keyLen - (nBlocks - 1) * DTLS_HMAC_DIGEST_SIZE;

    dtls_hmac_context_t *ctx = NULL;
    ctx = dtls_hmac_new( (const unsigned char *)passwd, pLen);
    if (NULL == ctx)
    {
        OC_LOG(ERROR, TAG, "DTLS HMAC Context is NULL");
        goto bail;
    }

    size_t i = 1;
    size_t idx = 0; //index for buffer
    size_t counter = 0;
    while (i != nBlocks + 1)
    {
        counter = 0 ;
        dtls_hmac_init(ctx, (const unsigned char *)passwd, pLen);
        while (counter != iterations)
        {
            if (counter == 0)
            {
                uint8_t intBuf[4] = {0x00, 0x00, 0x00, 0x00};
                dtls_hmac_update(ctx, salt, saltLen);
                GetBigEndianBuf(intBuf, i);
                dtls_hmac_update(ctx, intBuf, 4);

                int len = dtls_hmac_finalize(ctx, buf);
                if (DTLS_HMAC_DIGEST_SIZE != len)
                {
                    OC_LOG(ERROR, TAG, "DTLS HMAC is failed");
                    res = -1;
                }
                memcpy(uBuf, buf, DTLS_HMAC_DIGEST_SIZE);
            }
            else
            {
                dtls_hmac_init(ctx, (const unsigned char *)passwd, pLen);
                dtls_hmac_update(ctx, buf, DTLS_HMAC_DIGEST_SIZE);
                int len = dtls_hmac_finalize(ctx, buf);
                if (DTLS_HMAC_DIGEST_SIZE != len)
                {
                    OC_LOG(ERROR, TAG, "DTLS HMAC is failed");
                    res = -1;
                }
                XOR_BUF(buf, uBuf, DTLS_HMAC_DIGEST_SIZE);
            }
            counter++;
        }


        if (i == nBlocks)
        {
            memcpy(derivedKey + idx, uBuf, nOctetInLastBlock);
        }
        else
        {
            memcpy(derivedKey + idx, uBuf, DTLS_HMAC_DIGEST_SIZE);
            idx += DTLS_HMAC_DIGEST_SIZE;
        }
        i++;
    }

bail:
    dtls_hmac_free(ctx);
    return res;
}


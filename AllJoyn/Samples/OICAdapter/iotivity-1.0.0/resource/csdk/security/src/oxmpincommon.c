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
#include "ocstack.h"
#include "ocrandom.h"
#include "logger.h"
#include "pinoxmcommon.h"

#define TAG "PIN_OXM_COMMON"

static GeneratePinCallback gGenPinCallback = NULL;
static InputPinCallback gInputPinCallback = NULL;

void SetInputPinCB(InputPinCallback pinCB)
{
    if(NULL == pinCB)
    {
        OC_LOG(ERROR, TAG, "Failed to set callback for input pin.");
        return;
    }

    gInputPinCallback = pinCB;
}

void SetGeneratePinCB(GeneratePinCallback pinCB)
{
    if(NULL == pinCB)
    {
        OC_LOG(ERROR, TAG, "Failed to set callback for generate pin.");
        return;
    }

    gGenPinCallback = pinCB;
}

OCStackResult GeneratePin(char* pinBuffer, size_t bufferSize)
{
    if(!pinBuffer)
    {
        OC_LOG(ERROR, TAG, "PIN buffer is NULL");
        return OC_STACK_INVALID_PARAM;
    }
    if(OXM_RANDOM_PIN_SIZE + 1 > bufferSize)
    {
        OC_LOG(ERROR, TAG, "PIN buffer size is too small");
        return OC_STACK_INVALID_PARAM;
    }
    for(size_t i = 0; i < OXM_RANDOM_PIN_SIZE; i++)
    {
        pinBuffer[i] = OCGetRandomRange((uint32_t)'0', (uint32_t)'9');
    }
    pinBuffer[OXM_RANDOM_PIN_SIZE] = '\0';

    if(gGenPinCallback)
    {
        gGenPinCallback(pinBuffer, OXM_RANDOM_PIN_SIZE);
    }
    else
    {
        OC_LOG(ERROR, TAG, "Invoke PIN callback failed!");
        OC_LOG(ERROR, TAG, "Callback for genrate PIN should be registered to use PIN based OxM.");
        return OC_STACK_ERROR;
    }

    return OC_STACK_OK;
}


OCStackResult InputPin(char* pinBuffer, size_t bufferSize)
{
    if(!pinBuffer)
    {
        OC_LOG(ERROR, TAG, "PIN buffer is NULL");
        return OC_STACK_INVALID_PARAM;
    }
    if(OXM_RANDOM_PIN_SIZE + 1 > bufferSize)
    {
        OC_LOG(ERROR, TAG, "PIN buffer size is too small");
        return OC_STACK_INVALID_PARAM;
    }

    if(gInputPinCallback)
    {
        gInputPinCallback(pinBuffer, OXM_RANDOM_PIN_SIZE + 1);
    }
    else
    {
        OC_LOG(ERROR, TAG, "Invoke PIN callback failed!");
        OC_LOG(ERROR, TAG, "Callback for input PIN should be registered to use PIN based OxM.");
        return OC_STACK_ERROR;
    }

    return OC_STACK_OK;
}

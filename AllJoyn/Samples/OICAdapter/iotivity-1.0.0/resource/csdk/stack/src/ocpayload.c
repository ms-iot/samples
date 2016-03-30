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


#include "ocpayload.h"
#include "octypes.h"
#include <string.h>
#include "oic_malloc.h"
#include "oic_string.h"
#include "ocstackinternal.h"
#include "ocresource.h"
#include "logger.h"
#include "rdpayload.h"

#define TAG "OCPayload"
static void OCFreeRepPayloadValueContents(OCRepPayloadValue* val);
static void FreeOCDiscoveryResource(OCResourcePayload* payload);

void OCPayloadDestroy(OCPayload* payload)
{
    if(!payload)
    {
        return;
    }

    switch(payload->type)
    {
        case PAYLOAD_TYPE_REPRESENTATION:
            OCRepPayloadDestroy((OCRepPayload*)payload);
            break;
        case PAYLOAD_TYPE_DISCOVERY:
            OCDiscoveryPayloadDestroy((OCDiscoveryPayload*)payload);
            break;
        case PAYLOAD_TYPE_DEVICE:
            OCDevicePayloadDestroy((OCDevicePayload*)payload);
            break;
        case PAYLOAD_TYPE_PLATFORM:
            OCPlatformPayloadDestroy((OCPlatformPayload*)payload);
            break;
        case PAYLOAD_TYPE_PRESENCE:
            OCPresencePayloadDestroy((OCPresencePayload*)payload);
            break;
        case PAYLOAD_TYPE_SECURITY:
            OCSecurityPayloadDestroy((OCSecurityPayload*)payload);
            break;
        case PAYLOAD_TYPE_RD:
           OCRDPayloadDestroy((OCRDPayload*)payload);
           break;
        default:
            OC_LOG_V(ERROR, TAG, "Unsupported payload type in destroy: %d", payload->type);
            OICFree(payload);
            break;
    }
}
OCRepPayload* OCRepPayloadCreate()
{
    OCRepPayload* payload = (OCRepPayload*)OICCalloc(1, sizeof(OCRepPayload));

    if(!payload)
    {
        return NULL;
    }

    payload->base.type = PAYLOAD_TYPE_REPRESENTATION;

    return payload;
}

void OCRepPayloadAppend(OCRepPayload* parent, OCRepPayload* child)
{
    if(!parent)
    {
        return;
    }

    while(parent->next)
    {
        parent = parent->next;
    }

    parent->next= child;
    child->next = NULL;
}

static OCRepPayloadValue* OCRepPayloadFindValue(const OCRepPayload* payload, const char* name)
{
    if(!payload || !name)
    {
        return NULL;
    }

    OCRepPayloadValue* val = payload->values;
    while(val)
    {
        if(0 == strcmp(val->name, name))
        {
            return val;
        }
        val = val->next;
    }

    return NULL;

}

static void OCCopyPropertyValueArray(OCRepPayloadValue* dest, OCRepPayloadValue* source)
{
    if(!dest || !source)
    {
        return;
    }

    size_t dimTotal = calcDimTotal(source->arr.dimensions);
    switch(source->arr.type)
    {
        case OCREP_PROP_INT:
            dest->arr.iArray = (int64_t*)OICMalloc(dimTotal * sizeof(int64_t));
            memcpy(dest->arr.iArray, source->arr.iArray, dimTotal * sizeof(int64_t));
            break;
        case OCREP_PROP_DOUBLE:
            dest->arr.dArray = (double*)OICMalloc(dimTotal * sizeof(double));
            memcpy(dest->arr.dArray, source->arr.dArray, dimTotal * sizeof(double));
            break;
        case OCREP_PROP_BOOL:
            dest->arr.bArray = (bool*)OICMalloc(dimTotal * sizeof(bool));
            memcpy(dest->arr.bArray, source->arr.bArray, dimTotal * sizeof(bool));
            break;
        case OCREP_PROP_STRING:
            dest->arr.strArray = (char**)OICMalloc(dimTotal * sizeof(char*));
            for(size_t i = 0; i < dimTotal; ++i)
            {
                dest->arr.strArray[i] = OICStrdup(source->arr.strArray[i]);
            }
            break;
        case OCREP_PROP_ARRAY:
            dest->arr.objArray = (OCRepPayload**)OICMalloc(dimTotal * sizeof(OCRepPayload*));
            for(size_t i = 0; i < dimTotal; ++i)
            {
                dest->arr.objArray[i] = OCRepPayloadClone(source->arr.objArray[i]);
            }
            break;
        default:
            OC_LOG(ERROR, TAG, "CopyPropertyValueArray invalid type");
            break;
    }
}

static void OCCopyPropertyValue (OCRepPayloadValue *dest, OCRepPayloadValue *source)
{
    if (!source || !dest)
    {
        return;
    }

    switch(source->type)
    {
        case OCREP_PROP_STRING:
            dest->str = OICStrdup(source->str);
            break;
        case OCREP_PROP_OBJECT:
            dest->obj = OCRepPayloadClone(source->obj);
            break;
        case OCREP_PROP_ARRAY:
            OCCopyPropertyValueArray(dest, source);
            break;
        default:
            // Nothing to do for the trivially copyable types.
            break;
    }
}

static void OCFreeRepPayloadValueContents(OCRepPayloadValue* val)
{
    if(!val)
    {
        return;
    }

    if(val->type == OCREP_PROP_STRING)
    {
        OICFree(val->str);
    }
    else if (val->type == OCREP_PROP_OBJECT)
    {
        OCRepPayloadDestroy(val->obj);
    }
    else if (val->type == OCREP_PROP_ARRAY)
    {
        size_t dimTotal = calcDimTotal(val->arr.dimensions);
        switch(val->arr.type)
        {
            case OCREP_PROP_INT:
            case OCREP_PROP_DOUBLE:
            case OCREP_PROP_BOOL:
                // Since this is a union, iArray will
                // point to all of the above
                OICFree(val->arr.iArray);
                break;
            case OCREP_PROP_STRING:
                for(size_t i = 0; i< dimTotal;++i)
                {
                    OICFree(val->arr.strArray[i]);
                }
                OICFree(val->arr.strArray);
                break;
            case OCREP_PROP_OBJECT:
                for(size_t i = 0; i< dimTotal;++i)
                {
                    OCRepPayloadDestroy(val->arr.objArray[i]);
                }
                OICFree(val->arr.objArray);
                break;
            case OCREP_PROP_NULL:
            case OCREP_PROP_ARRAY:
                OC_LOG_V(ERROR, TAG, "FreeRepPayloadValueContents: Illegal type\
                        inside an array: %d", val->arr.type);
                break;
        }
    }
}

static void OCFreeRepPayloadValue(OCRepPayloadValue* val)
{
    if(!val)
    {
        return;
    }

    OICFree(val->name);
    OCFreeRepPayloadValueContents(val);
    OCFreeRepPayloadValue(val->next);
    OICFree(val);
}
static OCRepPayloadValue* OCRepPayloadValueClone (OCRepPayloadValue* source)
{
    if (!source)
    {
        return NULL;
    }

    OCRepPayloadValue *sourceIter = source;
    OCRepPayloadValue *destIter = (OCRepPayloadValue*) OICCalloc(1, sizeof(OCRepPayloadValue));
    if (!destIter)
    {
        return NULL;
    }

    OCRepPayloadValue *headOfClone = destIter;

    // Copy payload type and non pointer types in union.
    *destIter = *sourceIter;
    destIter->name = OICStrdup (sourceIter->name);
    OCCopyPropertyValue (destIter, sourceIter);

    sourceIter = sourceIter->next;

    while (sourceIter)
    {
        destIter->next = (OCRepPayloadValue*) OICCalloc(1, sizeof(OCRepPayloadValue));
        if (!destIter->next)
        {
            OCFreeRepPayloadValue (headOfClone);
            return NULL;
        }

        *(destIter->next) = *sourceIter;
        destIter->next->name = OICStrdup (sourceIter->name);
        OCCopyPropertyValue (destIter->next, sourceIter);

        sourceIter = sourceIter->next;
        destIter = destIter->next;
    }
    return headOfClone;
}

static OCRepPayloadValue* OCRepPayloadFindAndSetValue(OCRepPayload* payload, const char* name,
        OCRepPayloadPropType type)
{
    if(!payload || !name)
    {
        return NULL;
    }

    OCRepPayloadValue* val = payload->values;
    if(val == NULL)
    {
        payload->values = (OCRepPayloadValue*)OICCalloc(1, sizeof(OCRepPayloadValue));
        if(!payload->values)
        {
            return NULL;
        }
        payload->values->name = OICStrdup(name);
        if(!payload->values->name)
        {
            OICFree(payload->values);
            payload->values = NULL;
            return NULL;
        }
        payload->values->type =type;
        return payload->values;
    }

    while(val)
    {
        if(0 == strcmp(val->name, name))
        {
            OCFreeRepPayloadValueContents(val);
            val->type = type;
            return val;
        }
        else if(val->next == NULL)
        {
            val->next = (OCRepPayloadValue*)OICCalloc(1, sizeof(OCRepPayloadValue));
            if(!val->next)
            {
                return NULL;
            }
            val->next->name = OICStrdup(name);
            if(!val->next->name)
            {
                OICFree(val->next);
                val->next = NULL;
                return NULL;
            }
            val->next->type =type;
            return val->next;
        }

        val = val->next;
    }

    OC_LOG(ERROR, TAG, "FindAndSetValue reached point after while loop, pointer corruption?");
    return NULL;
}

bool OCRepPayloadAddResourceType(OCRepPayload* payload, const char* resourceType)
{
    return OCRepPayloadAddResourceTypeAsOwner(payload, OICStrdup(resourceType));
}

bool OCRepPayloadAddResourceTypeAsOwner(OCRepPayload* payload, char* resourceType)
{
    if(!payload || !resourceType)
    {
        return false;
    }

    if(payload->types)
    {
        OCStringLL* cur = payload->types;
        while(cur->next)
        {
            cur = cur->next;
        }
        cur->next = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));

        if(!cur->next)
        {
            return false;
        }

        cur->next->value = resourceType;
        return true;
    }
    else
    {
        payload->types = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));
        if(!payload->types)
        {
            return false;
        }
        payload->types->value = resourceType;
        return true;
    }
}

bool OCRepPayloadAddInterface(OCRepPayload* payload, const char* interface)
{
    return OCRepPayloadAddInterfaceAsOwner(payload, OICStrdup(interface));
}

bool OCRepPayloadAddInterfaceAsOwner(OCRepPayload* payload, char* interface)
{
    if(!payload || !interface)
    {
        return false;
    }

    if(payload->interfaces)
    {
        OCStringLL* cur = payload->interfaces;
        while(cur->next)
        {
            cur = cur->next;
        }
        cur->next = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));

        if(!cur->next)
        {
            return false;
        }
        cur->next->value = interface;
        return true;
    }
    else
    {
        payload->interfaces = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));
        if(!payload->interfaces)
        {
            return false;
        }
        payload->interfaces->value = interface;
        return true;
    }
}

bool OCRepPayloadSetUri(OCRepPayload* payload, const char*  uri)
{
    if(!payload)
    {
        return false;
    }

    payload->uri = OICStrdup(uri);
    return payload->uri != NULL;
}

bool OCRepPayloadIsNull(const OCRepPayload* payload, const char* name)
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val)
    {
        return false;
    }

    return val->type == OCREP_PROP_NULL;
}

static bool OCRepPayloadSetProp(OCRepPayload* payload, const char* name,
        void* value, OCRepPayloadPropType type)
{
    OCRepPayloadValue* val = OCRepPayloadFindAndSetValue(payload, name, type);
    if(!val)
    {
        return false;
    }
    switch(type)
    {
        case OCREP_PROP_INT:
               val->i = *(int64_t*)value;
               break;
        case OCREP_PROP_DOUBLE:
               val->d = *(double*)value;
               break;
        case OCREP_PROP_BOOL:
               val->b = *(bool*)value;
               break;
        case OCREP_PROP_OBJECT:
               val->obj = (OCRepPayload*)value;
               break;
        case OCREP_PROP_STRING:
               val->str = (char*)value;
               return val->str != NULL;
        case OCREP_PROP_NULL:
               return val != NULL;
        case OCREP_PROP_ARRAY:
        default:
               return false;
    }

    return true;
}

bool OCRepPayloadSetNull(OCRepPayload* payload, const char* name)
{
    return OCRepPayloadSetProp(payload, name, NULL, OCREP_PROP_NULL);
}

bool OCRepPayloadSetPropInt(OCRepPayload* payload,
        const char* name, int64_t value)
{
    return OCRepPayloadSetProp(payload, name, &value, OCREP_PROP_INT);
}

bool OCRepPayloadGetPropInt(const OCRepPayload* payload, const char* name, int64_t* value)
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_INT)
    {
        return false;
    }

    *value = val->i;
    return true;
}

bool OCRepPayloadSetPropDouble(OCRepPayload* payload,
        const char* name, double value)
{
    return OCRepPayloadSetProp(payload, name, &value, OCREP_PROP_DOUBLE);
}

bool OCRepPayloadGetPropDouble(const OCRepPayload* payload, const char* name, double* value)
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_DOUBLE)
    {
        return false;
    }

    *value = val->d;
    return true;
}

bool OCRepPayloadSetPropString(OCRepPayload* payload, const char* name, const char* value)
{
    char* temp = OICStrdup(value);
    bool b = OCRepPayloadSetPropStringAsOwner(payload, name, temp);

    if(!b)
    {
        OICFree(temp);
    }
    return b;
}

bool OCRepPayloadSetPropStringAsOwner(OCRepPayload* payload, const char* name, char* value)
{
    return OCRepPayloadSetProp(payload, name, value, OCREP_PROP_STRING);
}

bool OCRepPayloadGetPropString(const OCRepPayload* payload, const char* name, char** value)
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_STRING)
    {
        return false;
    }

    *value = OICStrdup(val->str);
    return *value != NULL;
}

bool OCRepPayloadSetPropBool(OCRepPayload* payload,
        const char* name, bool value)
{
    return OCRepPayloadSetProp(payload, name, &value, OCREP_PROP_BOOL);
}

bool OCRepPayloadGetPropBool(const OCRepPayload* payload, const char* name, bool* value)
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_BOOL)
    {
        return false;
    }

    *value = val->b;
    return true;
}

bool OCRepPayloadSetPropObject(OCRepPayload* payload, const char* name, const OCRepPayload* value)
{
    OCRepPayload* temp = OCRepPayloadClone(value);
    bool b = OCRepPayloadSetPropObjectAsOwner(payload, name, temp);

    if(!b)
    {
        OCRepPayloadDestroy(temp);
    }
    return b;
}

bool OCRepPayloadSetPropObjectAsOwner(OCRepPayload* payload, const char* name, OCRepPayload* value)
{
    return OCRepPayloadSetProp(payload, name, value, OCREP_PROP_OBJECT);
}

bool OCRepPayloadGetPropObject(const OCRepPayload* payload, const char* name, OCRepPayload** value)
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_OBJECT)
    {
        return false;
    }

    *value = OCRepPayloadClone(val->obj);
    return *value != NULL;
}

size_t calcDimTotal(const size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    if(dimensions[0] == 0)
    {
        return 0;
    }

    size_t total = 1;
    for(uint8_t i = 0; i < MAX_REP_ARRAY_DEPTH && dimensions[i] != 0; ++i)
    {
        total *= dimensions[i];
    }
    return total;
}

bool OCRepPayloadSetIntArrayAsOwner(OCRepPayload* payload, const char* name,
        int64_t* array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindAndSetValue(payload, name, OCREP_PROP_ARRAY);

    if(!val)
    {
        return false;
    }

    val->arr.type = OCREP_PROP_INT;
    memcpy(val->arr.dimensions, dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));
    val->arr.iArray = array;

    return true;
}

bool OCRepPayloadSetIntArray(OCRepPayload* payload, const char* name,
        const int64_t* array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    size_t dimTotal = calcDimTotal(dimensions);
    if(dimTotal == 0)
    {
        return false;
    }

    int64_t* newArray = (int64_t*)OICMalloc(dimTotal * sizeof(int64_t));

    if(!newArray)
    {
        return false;
    }

    memcpy(newArray, array, dimTotal * sizeof(int64_t));


    bool b = OCRepPayloadSetIntArrayAsOwner(payload, name, newArray, dimensions);
    if(!b)
    {
        OICFree(newArray);
    }
    return b;
}

bool OCRepPayloadGetIntArray(const OCRepPayload* payload, const char* name,
        int64_t** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_ARRAY || val->arr.type != OCREP_PROP_INT
            || !val->arr.iArray)
    {
        return false;
    }

    size_t dimTotal = calcDimTotal(val->arr.dimensions);
    if(dimTotal == 0)
    {
        return false;
    }
    *array = (int64_t*)OICMalloc(dimTotal * sizeof(int64_t));
    if(!*array)
    {
        return false;
    }

    memcpy(*array, val->arr.iArray, dimTotal * sizeof(int64_t));
    memcpy(dimensions, val->arr.dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));
    return true;
}

bool OCRepPayloadSetDoubleArrayAsOwner(OCRepPayload* payload, const char* name,
        double* array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindAndSetValue(payload, name, OCREP_PROP_ARRAY);

    if(!val)
    {
        return false;
    }

    val->arr.type = OCREP_PROP_DOUBLE;
    memcpy(val->arr.dimensions, dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));
    val->arr.dArray = array;

    return true;
}
bool OCRepPayloadSetDoubleArray(OCRepPayload* payload, const char* name,
        const double* array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    size_t dimTotal = calcDimTotal(dimensions);
    if(dimTotal == 0)
    {
        return false;
    }

    double* newArray = (double*)OICMalloc(dimTotal * sizeof(double));

    if(!newArray)
    {
        return false;
    }

    memcpy(newArray, array, dimTotal * sizeof(double));

    bool b = OCRepPayloadSetDoubleArrayAsOwner(payload, name, newArray, dimensions);
    if(!b)
    {
        OICFree(newArray);
    }
    return b;
}

bool OCRepPayloadGetDoubleArray(const OCRepPayload* payload, const char* name,
        double** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_ARRAY || val->arr.type != OCREP_PROP_DOUBLE
            || !val->arr.dArray)
    {
        return false;
    }

    size_t dimTotal = calcDimTotal(val->arr.dimensions);
    if(dimTotal == 0)
    {
        return false;
    }
    *array = (double*)OICMalloc(dimTotal * sizeof(double));
    if(!*array)
    {
        return false;
    }

    memcpy(*array, val->arr.dArray, dimTotal * sizeof(double));
    memcpy(dimensions, val->arr.dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));
    return true;
}

bool OCRepPayloadSetStringArrayAsOwner(OCRepPayload* payload, const char* name,
        char** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindAndSetValue(payload, name, OCREP_PROP_ARRAY);

    if(!val)
    {
        return false;
    }

    val->arr.type = OCREP_PROP_STRING;
    memcpy(val->arr.dimensions, dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));
    val->arr.strArray = array;

    return true;
}
bool OCRepPayloadSetStringArray(OCRepPayload* payload, const char* name,
        const char** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    size_t dimTotal = calcDimTotal(dimensions);
    if(dimTotal == 0)
    {
        return false;
    }

    char** newArray = (char**)OICMalloc(dimTotal * sizeof(char*));

    if(!newArray)
    {
        return false;
    }

    for(size_t i = 0; i < dimTotal; ++i)
    {
        newArray[i] = OICStrdup(array[i]);
    }

    bool b = OCRepPayloadSetStringArrayAsOwner(payload, name, newArray, dimensions);

    if(!b)
    {
        for(size_t i = 0; i < dimTotal; ++i)
        {
            OICFree(newArray[i]);
        }
        OICFree(newArray);
    }
    return b;
}

bool OCRepPayloadGetStringArray(const OCRepPayload* payload, const char* name,
        char*** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_ARRAY || val->arr.type != OCREP_PROP_STRING
            || !val->arr.strArray)
    {
        return false;
    }

    size_t dimTotal = calcDimTotal(val->arr.dimensions);
    if(dimTotal == 0)
    {
        return false;
    }
    *array = (char**)OICMalloc(dimTotal * sizeof(char*));
    if(!*array)
    {
        return false;
    }

    memcpy(dimensions, val->arr.dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));

    for(size_t i = 0; i < dimTotal; ++i)
    {
        (*array)[i] = OICStrdup(val->arr.strArray[i]);
    }

    return true;

}

bool OCRepPayloadSetBoolArrayAsOwner(OCRepPayload* payload, const char* name,
        bool* array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{

    OCRepPayloadValue* val = OCRepPayloadFindAndSetValue(payload, name, OCREP_PROP_ARRAY);

    if(!val)
    {
        return false;
    }

    val->arr.type = OCREP_PROP_BOOL;
    memcpy(val->arr.dimensions, dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));
    val->arr.bArray = array;

    return true;
}
bool OCRepPayloadSetBoolArray(OCRepPayload* payload, const char* name,
        const bool* array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    size_t dimTotal = calcDimTotal(dimensions);
    if(dimTotal == 0)
    {
        return false;
    }

    bool* newArray = (bool*)OICMalloc(dimTotal * sizeof(bool));

    if(!newArray)
    {
        return false;
    }

    memcpy(newArray, array, dimTotal * sizeof(bool));


    bool b = OCRepPayloadSetBoolArrayAsOwner(payload, name, newArray, dimensions);
    if(!b)
    {
        OICFree(newArray);
    }
    return b;
}

bool OCRepPayloadGetBoolArray(const OCRepPayload* payload, const char* name,
        bool** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_ARRAY || val->arr.type != OCREP_PROP_BOOL
            || !val->arr.bArray)
    {
        return false;
    }

    size_t dimTotal = calcDimTotal(val->arr.dimensions);
    if(dimTotal == 0)
    {
        return false;
    }
    *array = (bool*)OICMalloc(dimTotal * sizeof(bool));
    if(!*array)
    {
        return false;
    }

    memcpy(*array, val->arr.bArray, dimTotal * sizeof(bool));
    memcpy(dimensions, val->arr.dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));
    return true;
}

bool OCRepPayloadSetPropObjectArrayAsOwner(OCRepPayload* payload, const char* name,
        OCRepPayload** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindAndSetValue(payload, name, OCREP_PROP_ARRAY);

    if(!val)
    {
        return false;
    }

    val->arr.type = OCREP_PROP_OBJECT;
    memcpy(val->arr.dimensions, dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));
    val->arr.objArray = array;

    return true;
}

bool OCRepPayloadSetPropObjectArray(OCRepPayload* payload, const char* name,
        const OCRepPayload** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    size_t dimTotal = calcDimTotal(dimensions);
    if(dimTotal == 0)
    {
        return false;
    }

    OCRepPayload** newArray = (OCRepPayload**)OICMalloc(dimTotal * sizeof(OCRepPayload*));

    if(!newArray)
    {
        return false;
    }

    for(size_t i = 0; i < dimTotal; ++i)
    {
        newArray[i] = OCRepPayloadClone(array[i]);
    }

    bool b = OCRepPayloadSetPropObjectArrayAsOwner(payload, name, newArray, dimensions);

    if(!b)
    {
        for(size_t i = 0; i < dimTotal; ++i)
        {
           OCRepPayloadDestroy(newArray[i]);
        }
        OICFree(newArray);
    }
    return b;
}

bool OCRepPayloadGetPropObjectArray(const OCRepPayload* payload, const char* name,
        OCRepPayload*** array, size_t dimensions[MAX_REP_ARRAY_DEPTH])
{
    OCRepPayloadValue* val = OCRepPayloadFindValue(payload, name);

    if(!val || val->type != OCREP_PROP_ARRAY || val->arr.type != OCREP_PROP_OBJECT
            || !val->arr.objArray)
    {
        return false;
    }

    size_t dimTotal = calcDimTotal(val->arr.dimensions);
    if(dimTotal == 0)
    {
        return false;
    }
    *array = (OCRepPayload**)OICMalloc(dimTotal * sizeof(OCRepPayload*));
    if(!*array)
    {
        return false;
    }

    memcpy(dimensions, val->arr.dimensions, MAX_REP_ARRAY_DEPTH * sizeof(size_t));

    for(size_t i = 0; i < dimTotal; ++i)
    {
        (*array)[i] = OCRepPayloadClone(val->arr.objArray[i]);
    }

    return true;
}

void OCFreeOCStringLL(OCStringLL* ll)
{
    if(!ll)
    {
        return;
    }

    OCFreeOCStringLL(ll->next);
    OICFree(ll->value);
    OICFree(ll);
}

OCStringLL* CloneOCStringLL (OCStringLL* ll)
{
    if (!ll)
    {
        return NULL;
    }

    OCStringLL *sourceIter = ll;

    OCStringLL *destIter = (OCStringLL*)OICCalloc (1, sizeof (OCStringLL));
    if (!destIter)
    {
        return NULL;
    }
    destIter->value = OICStrdup (sourceIter->value);

    OCStringLL *headOfClone = destIter;

    sourceIter = sourceIter->next;

    while (sourceIter)
    {
        destIter->next  = (OCStringLL*)OICCalloc (1, sizeof (OCStringLL));
        if (!destIter->next)
        {
            OCFreeOCStringLL (headOfClone);
            return NULL;
        }
        destIter->next->value = OICStrdup (sourceIter->value);

        destIter = destIter->next;
        sourceIter = sourceIter->next;
    }
    return headOfClone;
}

OCRepPayload* OCRepPayloadClone (const OCRepPayload* payload)
{
    if (!payload)
    {
        return NULL;
    }

    OCRepPayload *clone = OCRepPayloadCreate();

    if (!clone)
    {
        return NULL;
    }

    clone->uri = OICStrdup (payload->uri);
    clone->types = CloneOCStringLL (payload->types);
    clone->interfaces = CloneOCStringLL (payload->interfaces);
    clone->values = OCRepPayloadValueClone (payload->values);

    return clone;
}


void OCRepPayloadDestroy(OCRepPayload* payload)
{
    if(!payload)
    {
        return;
    }

    OICFree(payload->uri);
    OCFreeOCStringLL(payload->types);
    OCFreeOCStringLL(payload->interfaces);
    OCFreeRepPayloadValue(payload->values);
    OCRepPayloadDestroy(payload->next);
    OICFree(payload);
}

OCDiscoveryPayload* OCDiscoveryPayloadCreate()
{
    OCDiscoveryPayload* payload = (OCDiscoveryPayload*)OICCalloc(1, sizeof(OCDiscoveryPayload));

    if(!payload)
    {
        return NULL;
    }

    payload->base.type = PAYLOAD_TYPE_DISCOVERY;

    return payload;
}

OCSecurityPayload* OCSecurityPayloadCreate(const char* securityData)
{
    OCSecurityPayload* payload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));

    if(!payload)
    {
        return NULL;
    }

    payload->base.type = PAYLOAD_TYPE_SECURITY;
    payload->securityData = OICStrdup(securityData);

    return payload;
}

void OCSecurityPayloadDestroy(OCSecurityPayload* payload)
{
    if(!payload)
    {
        return;
    }

    OICFree(payload->securityData);
    OICFree(payload);
}

size_t OCDiscoveryPayloadGetResourceCount(OCDiscoveryPayload* payload)
{
    size_t i = 0;
    OCResourcePayload* p = payload->resources;
    while(p)
    {
        ++i;
        p = p->next;
    }
    return i;
}

OCResourcePayload* OCDiscoveryPayloadGetResource(OCDiscoveryPayload* payload, size_t index)
{
    size_t i = 0;
    OCResourcePayload* p = payload->resources;
    while(p)
    {
        if(i == index)
        {
            return p;
        }
        ++i;
        p = p->next;
    }
    return NULL;
}

static OCResourcePayload* OCCopyResource(const OCResource* res, uint16_t port)
{
    OCResourcePayload* pl = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
    if(!pl)
    {
        return NULL;
    }

    pl->uri = OICStrdup(res->uri);
    pl->sid = (uint8_t*)OICCalloc(1, UUID_SIZE);
    if(!pl->uri || ! pl->sid)
    {
        FreeOCDiscoveryResource(pl);
        return NULL;
    }
    memcpy(pl->sid, OCGetServerInstanceID(), UUID_SIZE);

    // types
    OCResourceType* typePtr = res->rsrcType;

    if(typePtr != NULL)
    {
        pl->types = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));
        if(!pl->types)
        {
            FreeOCDiscoveryResource(pl);
            return NULL;
        }
        pl->types->value = OICStrdup(typePtr->resourcetypename);
        if(!pl->types->value)
        {
            FreeOCDiscoveryResource(pl);
            return NULL;
        }

        OCStringLL* cur = pl->types;
        typePtr = typePtr->next;
        while(typePtr)
        {
            cur->next = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));
            if(!cur->next)
            {
                FreeOCDiscoveryResource(pl);
                return NULL;
            }
            cur->next->value = OICStrdup(typePtr->resourcetypename);
            if(!cur->next->value)
            {
                FreeOCDiscoveryResource(pl);
                return NULL;
            }
            cur = cur->next;
            typePtr = typePtr->next;
        }
    }

    // interfaces
    OCResourceInterface* ifPtr = res->rsrcInterface;
    if(ifPtr != NULL)
    {
        pl->interfaces = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));
        if(!pl->interfaces)
        {
            FreeOCDiscoveryResource(pl);
            return NULL;
        }
        pl->interfaces->value = OICStrdup(ifPtr->name);
        if(!pl->interfaces->value)
        {
            FreeOCDiscoveryResource(pl);
            return NULL;
        }

        OCStringLL* cur = pl->interfaces;
        ifPtr = ifPtr->next;
        while(ifPtr && cur)
        {
            cur->next = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));
            if(!cur->next)
            {
                FreeOCDiscoveryResource(pl);
                return NULL;
            }
            cur->next->value = OICStrdup(ifPtr->name);
            if(!cur->next->value)
            {
                FreeOCDiscoveryResource(pl);
                return NULL;
            }
            cur = cur->next;
            ifPtr = ifPtr->next;
        }
    }

    pl->bitmap = res->resourceProperties & (OC_OBSERVABLE | OC_DISCOVERABLE);
    pl->secure = (res->resourceProperties & OC_SECURE) != 0;
    pl->port = port;

    return pl;
}

void OCDiscoveryPayloadAddResource(OCDiscoveryPayload* payload, const OCResource* res,
        uint16_t port)
{
    OCDiscoveryPayloadAddNewResource(payload, OCCopyResource(res, port));
}

bool OCResourcePayloadAddResourceType(OCResourcePayload* payload, const char* resourceType)
{
    if (!resourceType)
    {
        return false;
    }

    char* dup = OICStrdup(resourceType);

    if (!dup)
    {
        return false;
    }

    if (!payload->types)
    {
        payload->types = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));

        if (!payload->types)
        {
            OICFree(dup);
            return false;
        }

        payload->types->value = dup;
        return true;
    }

    else
    {
        OCStringLL* temp = payload->types;

        while(temp->next)
        {
            temp = temp->next;
        }

        temp->next = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));
        if (!temp->next)
        {
            OICFree(dup);
            return false;
        }

        temp->next->value = dup;
        return true;
    }
}

bool OCResourcePayloadAddInterface(OCResourcePayload* payload, const char* interface)
{
    if (!interface)
    {
        return false;
    }

    char* dup = OICStrdup(interface);

    if (!dup)
    {
        return false;
    }

    if (!payload->interfaces)
    {
        payload->interfaces = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));

        if (!payload->interfaces)
        {
            OICFree(dup);
            return false;
        }

        payload->interfaces->value = dup;
        return true;
    }

    else
    {
        OCStringLL* temp = payload->interfaces;

        while(temp->next)
        {
            temp = temp->next;
        }

        temp->next = (OCStringLL*)OICCalloc(1, sizeof(OCStringLL));
        if (!temp->next)
        {
            OICFree(dup);
            return false;
        }

        temp->next->value = dup;
        return true;
    }
}

void OCDiscoveryPayloadAddNewResource(OCDiscoveryPayload* payload, OCResourcePayload* res)
{
    if(!payload->resources)
    {
        payload->resources = res;
    }
    else
    {
        OCResourcePayload* p = payload->resources;
        while(p->next)
        {
            p = p->next;
        }
        p->next = res;
    }
}

static void FreeOCDiscoveryResource(OCResourcePayload* payload)
{
    if(!payload)
    {
        return;
    }

    OICFree(payload->uri);
    OICFree(payload->sid);
    OCFreeOCStringLL(payload->types);
    OCFreeOCStringLL(payload->interfaces);
    FreeOCDiscoveryResource(payload->next);
    OICFree(payload);

}
void OCDiscoveryPayloadDestroy(OCDiscoveryPayload* payload)
{
    if(!payload)
    {
        return;
    }

    FreeOCDiscoveryResource(payload->resources);
    OICFree(payload);
}

OCDevicePayload* OCDevicePayloadCreate(const char* uri, const uint8_t* sid, const char* dname,
        const char* specVer, const char* dmVer)
{

    OCDevicePayload* payload = (OCDevicePayload*)OICCalloc(1, sizeof(OCDevicePayload));

    if(!payload)
    {
        return NULL;
    }

    payload->base.type = PAYLOAD_TYPE_DEVICE;

    payload->uri = OICStrdup(uri);
    if(uri && !payload->uri)
    {
        goto exit;
    }

    if(sid)
    {
        payload->sid = (uint8_t*)OICMalloc(UUID_SIZE);
        if(!payload->sid)
        {
            goto exit;
        }
        memcpy(payload->sid, sid, UUID_SIZE);
    }

    payload->deviceName = OICStrdup(dname);
    if(dname && !payload->deviceName)
    {
        goto exit;
    }

    payload->specVersion = OICStrdup(specVer);
    if(specVer && !payload->specVersion)
    {
        goto exit;
    }

    payload->dataModelVersion = OICStrdup(dmVer);
    if(dmVer && !payload->dataModelVersion)
    {
        goto exit;
    }

    return payload;

exit:
    OCDevicePayloadDestroy((OCDevicePayload*)payload);
    return NULL;
}

void OCDevicePayloadDestroy(OCDevicePayload* payload)
{
    if(!payload)
    {
        return;
    }

    OICFree(payload->uri);
    OICFree(payload->sid);
    OICFree(payload->deviceName);
    OICFree(payload->specVersion);
    OICFree(payload->dataModelVersion);
    OICFree(payload);
}

static void OCCopyPlatformInfo(const OCPlatformInfo* platformInfo, OCPlatformPayload* target)
{
    if(!platformInfo || !target)
    {
        return;
    }

    target->info.platformID = OICStrdup(platformInfo->platformID);
    target->info.manufacturerName = OICStrdup(platformInfo->manufacturerName);
    target->info.manufacturerUrl = OICStrdup(platformInfo->manufacturerUrl);
    target->info.modelNumber = OICStrdup(platformInfo->modelNumber);
    target->info.dateOfManufacture = OICStrdup(platformInfo->dateOfManufacture);
    target->info.platformVersion = OICStrdup(platformInfo->platformVersion);
    target->info.operatingSystemVersion = OICStrdup(platformInfo->operatingSystemVersion);
    target->info.hardwareVersion = OICStrdup(platformInfo->hardwareVersion);
    target->info.firmwareVersion = OICStrdup(platformInfo->firmwareVersion);
    target->info.supportUrl = OICStrdup(platformInfo->supportUrl);
    target->info.systemTime = OICStrdup(platformInfo->systemTime);
}

OCPlatformPayload* OCPlatformPayloadCreateAsOwner(char* uri, OCPlatformInfo* platformInfo)
{
    OCPlatformPayload* payload = (OCPlatformPayload*)OICCalloc(1, sizeof(OCPlatformPayload));
    if(!payload)
    {
        return NULL;
    }

    payload->base.type = PAYLOAD_TYPE_PLATFORM;
    payload->uri = uri;
    payload->info = *platformInfo;

    return payload;
}

OCPlatformPayload* OCPlatformPayloadCreate(const char* uri, const OCPlatformInfo* platformInfo)
{
    OCPlatformPayload* payload = (OCPlatformPayload*)OICCalloc(1, sizeof(OCPlatformPayload));

    if(!payload)
    {
        return NULL;
    }

    payload->base.type = PAYLOAD_TYPE_PLATFORM;
    payload->uri = OICStrdup(uri);
    OCCopyPlatformInfo(platformInfo, payload);

    return payload;
}

void OCPlatformPayloadDestroy(OCPlatformPayload* payload)
{
    if(!payload)
    {
        return;
    }
    OICFree(payload->uri);
    OICFree(payload->info.platformID);
    OICFree(payload->info.manufacturerName);
    OICFree(payload->info.manufacturerUrl);
    OICFree(payload->info.modelNumber);
    OICFree(payload->info.dateOfManufacture);
    OICFree(payload->info.platformVersion);
    OICFree(payload->info.operatingSystemVersion);
    OICFree(payload->info.hardwareVersion);
    OICFree(payload->info.firmwareVersion);
    OICFree(payload->info.supportUrl);
    OICFree(payload->info.systemTime);
    OICFree(payload);
}

OCPresencePayload* OCPresencePayloadCreate(uint32_t seqNum, uint32_t maxAge,
        OCPresenceTrigger trigger, const char* resourceType)
{
    OCPresencePayload* payload = (OCPresencePayload*)OICCalloc(1, sizeof(OCPresencePayload));
    if(!payload)
    {
        return NULL;
    }

    payload->base.type = PAYLOAD_TYPE_PRESENCE;
    payload->sequenceNumber = seqNum;
    payload->maxAge = maxAge;
    payload->trigger = trigger;
    payload->resourceType = OICStrdup(resourceType);
    return payload;
}

void OCPresencePayloadDestroy(OCPresencePayload* payload)
{
    if(!payload)
    {
        return;
    }
    OICFree(payload->resourceType);
    OICFree(payload);
}

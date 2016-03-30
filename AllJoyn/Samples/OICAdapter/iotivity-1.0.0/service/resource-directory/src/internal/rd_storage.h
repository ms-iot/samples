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
#ifndef _RESOURCE_DIRECTORY_SERVER_STORAGE_H_
#define _RESOURCE_DIRECTORY_SERVER_STORAGE_H_

#include "octypes.h"

/** Stucture holding Published Resources on the Resource Directory. */
typedef struct OCRDStorePublishResources
{
    /** Publish resource. */
    OCResourceCollectionPayload *publishedResource;
    /** Linked list pointing to next published resource. */
    struct OCRDStorePublishResources *next;
} OCRDStorePublishResources;

/**
 * Stores the publish resources.
 *
 * @param payload RDPublish payload sent from the remote device.
 *
 * @return ::OC_STACK_OK upon success, ::OC_STACK_ERROR in case of error.
 */
OCStackResult OCRDStorePublishedResources(const OCResourceCollectionPayload *payload);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_RESOURCE_DIRECTORY_SERVER_STORAGE_H_

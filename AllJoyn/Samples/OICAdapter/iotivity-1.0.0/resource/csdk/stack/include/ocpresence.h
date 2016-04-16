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

#ifndef OCPRESENCE_H_
#define OCPRESENCE_H_

#ifdef WITH_PRESENCE

/**
 * The OCPresenceTrigger enum delineates the three spec-compliant modes for
 * "Trigger." These enum values are then mapped to  strings
 * "create", "change", "delete", respectively, before getting encoded into
 * the payload.
 */
typedef enum
{
    /** The creation of a resource is associated with this invocation. */
    OC_PRESENCE_TRIGGER_CREATE = 0,

    /** The change/update of a resource is associated this invocation. */
    OC_PRESENCE_TRIGGER_CHANGE = 1,

    /** The deletion of a resource is associated with this invocation.*/
    OC_PRESENCE_TRIGGER_DELETE = 2
} OCPresenceTrigger;
#endif

#endif

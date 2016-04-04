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

#ifndef IOTVT_SRM_RSRC_STRINGS_H
#define IOTVT_SRM_RSRC_STRINGS_H

#include "securevirtualresourcetypes.h"

extern const char * SVR_DB_FILE_NAME;
extern const char * OIC_MI_DEF;

//AMACL
extern const char * OIC_RSRC_TYPE_SEC_AMACL;
extern const char * OIC_RSRC_AMACL_URI;
extern const char * OIC_JSON_AMACL_NAME;

//ACL
extern const char * OIC_RSRC_TYPE_SEC_ACL;
extern const char * OIC_RSRC_ACL_URI;
extern const char * OIC_JSON_ACL_NAME;

//PSTAT
extern const char * OIC_RSRC_TYPE_SEC_PSTAT;
extern const char * OIC_RSRC_PSTAT_URI;
extern const char * OIC_JSON_PSTAT_NAME;


//DOXM
extern const char * OIC_RSRC_TYPE_SEC_DOXM;
extern const char * OIC_RSRC_DOXM_URI;
extern const char * OIC_JSON_DOXM_NAME;

//cred
extern const char * OIC_RSRC_TYPE_SEC_CRED;
extern const char * OIC_RSRC_CRED_URI;
extern const char * OIC_JSON_CRED_NAME;

//CRL
extern const char * OIC_RSRC_TYPE_SEC_CRL;
extern const char * OIC_RSRC_CRL_URI;
extern const char * OIC_JSON_CRL_NAME;

//SVC
extern const char * OIC_RSRC_TYPE_SEC_SVC;
extern const char * OIC_RSRC_SVC_URI;
extern const char * OIC_JSON_SVC_NAME;

extern const char * OIC_JSON_SUBJECT_NAME;
extern const char * OIC_JSON_RESOURCES_NAME;
extern const char * OIC_JSON_AMSS_NAME;
extern const char * OIC_JSON_PERMISSION_NAME;
extern const char * OIC_JSON_OWNERS_NAME;
extern const char * OIC_JSON_OWNER_NAME;
extern const char * OIC_JSON_OWNED_NAME;
extern const char * OIC_JSON_OXM_NAME;
extern const char * OIC_JSON_OXM_TYPE_NAME;
extern const char * OIC_JSON_OXM_SEL_NAME;
extern const char * OIC_JSON_DEVICE_ID_FORMAT_NAME;
extern const char * OIC_JSON_CREDID_NAME;
extern const char * OIC_JSON_ROLEIDS_NAME;
extern const char * OIC_JSON_CREDTYPE_NAME;
extern const char * OIC_JSON_PUBLICDATA_NAME;
extern const char * OIC_JSON_PRIVATEDATA_NAME;
extern const char * OIC_JSON_PERIOD_NAME;
extern const char * OIC_JSON_PERIODS_NAME;
extern const char * OIC_JSON_RECURRENCES_NAME;
extern const char * OIC_JSON_ISOP_NAME;
extern const char * OIC_JSON_COMMIT_HASH_NAME;
extern const char * OIC_JSON_DEVICE_ID_NAME;
extern const char * OIC_JSON_CM_NAME;
extern const char * OIC_JSON_TM_NAME;
extern const char * OIC_JSON_OM_NAME;
extern const char * OIC_JSON_SM_NAME;
extern const char * OIC_JSON_SERVICE_DEVICE_ID;
extern const char * OIC_JSON_SERVICE_TYPE;
extern const char * OIC_JSON_SUPPORTED_CRED_TYPE_NAME;

extern OicUuid_t WILDCARD_SUBJECT_ID;
extern size_t WILDCARD_SUBJECT_ID_LEN;
extern const char * WILDCARD_RESOURCE_URI;

//Ownership Transfer Methods
extern const char * OXM_JUST_WORKS;
extern const char * OXM_RANDOM_DEVICE_PIN;
extern const char * OXM_MANUFACTURER_CERTIFICATE;

extern const char * OIC_SEC_TRUE;
extern const char * OIC_SEC_FALSE;

extern const char * OIC_SEC_REST_QUERY_SEPARATOR;
extern char OIC_SEC_REST_QUERY_DELIMETER;

#endif //IOTVT_SRM_RSRC_STRINGS_H


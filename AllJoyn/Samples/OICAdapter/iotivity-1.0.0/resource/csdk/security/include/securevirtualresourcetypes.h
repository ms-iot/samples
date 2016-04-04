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

/**
 * Data type definitions for all oic.sec.* types defined in the
 * OIC Security Specification.
 *
 * Note that throughout, ptrs are used rather than arrays.  There
 * are two primary reasons for this:
 * 1) The Spec defines many structures with optional fields, so pre-
 *    allocating these would be wasteful.
 * 2) There are in many cases arrays of Strings or arrays of Structs,
 *    which could not be defined as variable length arrays (e.g. array[])
 *    without breaking from the structure order and definition in the Spec.
 *
 * The primary drawback to this decision is that marshalling functions
 * will have to be written by hand to marshal these structures (e.g. to/from
 * Persistent Storage, or across memory boundaries).
 *
 * TODO reconcile against latest OIC Security Spec to ensure all fields correct.
 * (Last checked against v0.95)
 */

#ifndef OC_SECURITY_RESOURCE_TYPES_H
#define OC_SECURITY_RESOURCE_TYPES_H

#include <stdint.h> // for uint8_t typedef
#include <stdbool.h>
#ifdef __WITH_X509__
#include "byte_array.h"
#endif /* __WITH_X509__ */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Values used to create bit-maskable enums for single-value
 *          response with embedded code.
 */
#define ACCESS_GRANTED_DEF            (1 << 0)
#define ACCESS_DENIED_DEF             (1 << 1)
#define INSUFFICIENT_PERMISSION_DEF   (1 << 2)
#define SUBJECT_NOT_FOUND_DEF         (1 << 3)
#define RESOURCE_NOT_FOUND_DEF        (1 << 4)
#define POLICY_ENGINE_ERROR_DEF       (1 << 5)
#define INVALID_PERIOD_DEF            (1 << 6)
#define ACCESS_WAITING_DEF            (1 << 7)
#define AMS_SERVICE_DEF               (1 << 8)
#define REASON_MASK_DEF               (INSUFFICIENT_PERMISSION_DEF | \
                                       INVALID_PERIOD_DEF | \
                                       SUBJECT_NOT_FOUND_DEF | \
                                       RESOURCE_NOT_FOUND_DEF | \
                                       POLICY_ENGINE_ERROR_DEF)


/**
 * Access policy in least significant bits (from Spec):
 * 1st lsb:  C (Create)
 * 2nd lsb:  R (Read, Observe, Discover)
 * 3rd lsb:  U (Write, Update)
 * 4th lsb:  D (Delete)
 * 5th lsb:  N (Notify)
 */
#define PERMISSION_CREATE       (1 << 0)
#define PERMISSION_READ         (1 << 1)
#define PERMISSION_WRITE        (1 << 2)
#define PERMISSION_DELETE       (1 << 3)
#define PERMISSION_NOTIFY       (1 << 4)
#define PERMISSION_FULL_CONTROL (PERMISSION_CREATE | \
                                 PERMISSION_READ | \
                                 PERMISSION_WRITE | \
                                 PERMISSION_DELETE | \
                                 PERMISSION_NOTIFY)

/**
 * @brief   Response type for all Action requests from CA layer;
 *          may include a reason code.
 *
 * To extract codes use GetReasonCode function on SRMAccessResponse:
 *
 * SRMAccessResponse_t response = SRMRequestHandler(obj, info);
 * if(SRM_TRUE == IsAccessGranted(response)) {
 *     SRMAccessResponseReasonCode_t reason = GetReasonCode(response);
 *     switch(reason) {
 *         case INSUFFICIENT_PERMISSION:
 *         ...etc.
 *     }
 * }
 */
typedef enum
{
    ACCESS_GRANTED = ACCESS_GRANTED_DEF,
    ACCESS_DENIED = ACCESS_DENIED_DEF,
    ACCESS_DENIED_INVALID_PERIOD = ACCESS_DENIED_DEF
        | INVALID_PERIOD_DEF,
    ACCESS_DENIED_INSUFFICIENT_PERMISSION = ACCESS_DENIED_DEF
        | INSUFFICIENT_PERMISSION_DEF,
    ACCESS_DENIED_SUBJECT_NOT_FOUND = ACCESS_DENIED_DEF
        | SUBJECT_NOT_FOUND_DEF,
    ACCESS_DENIED_RESOURCE_NOT_FOUND = ACCESS_DENIED_DEF
        | RESOURCE_NOT_FOUND_DEF,
    ACCESS_DENIED_POLICY_ENGINE_ERROR = ACCESS_DENIED_DEF
        | POLICY_ENGINE_ERROR_DEF,
    ACCESS_WAITING_FOR_AMS = ACCESS_WAITING_DEF
        | AMS_SERVICE_DEF,
    ACCESS_DENIED_AMS_SERVICE_ERROR = ACCESS_DENIED
        | AMS_SERVICE_DEF
} SRMAccessResponse_t;

/**
 * Reason code for SRMAccessResponse.
 */
typedef enum
{
    NO_REASON_GIVEN = 0,
    INSUFFICIENT_PERMISSION = INSUFFICIENT_PERMISSION_DEF,
    SUBJECT_NOT_FOUND = SUBJECT_NOT_FOUND_DEF,
    RESOURCE_NOT_FOUND = RESOURCE_NOT_FOUND_DEF,
} SRMAccessResponseReasonCode_t;

/**
 * Extract Reason Code from Access Response.
 */
static inline SRMAccessResponseReasonCode_t GetReasonCode(
    SRMAccessResponse_t response)
{
    SRMAccessResponseReasonCode_t reason =
        (SRMAccessResponseReasonCode_t)(response & REASON_MASK_DEF);
    return reason;
}

/**
 * Returns 'true' iff request should be passed on to RI layer.
 */
static inline bool IsAccessGranted(SRMAccessResponse_t response)
{
    if(ACCESS_GRANTED == (response & ACCESS_GRANTED))
    {
        return true;
    }
    else
    {
        return false;
    }
}

typedef struct OicSecAcl OicSecAcl_t;

typedef struct OicSecAmacl OicSecAmacl_t;

typedef struct OicSecCred OicSecCred_t;

/**
 * Aid for assigning/testing vals with OicSecCredType_t.
 * Example:
 *  OicSecCredType_t ct = PIN_PASSWORD | ASYMMETRIC_KEY;
 *  if((ct & PIN_PASSWORD) == PIN_PASSWORD)
 *  {
 *      // ct contains PIN_PASSWORD flag.
 *  }
 */
typedef enum OSCTBitmask
{
    NO_SECURITY_MODE                = 0x0,
    SYMMETRIC_PAIR_WISE_KEY         = (0x1 << 0),
    SYMMETRIC_GROUP_KEY             = (0x1 << 1),
    ASYMMETRIC_KEY                  = (0x1 << 2),
    SIGNED_ASYMMETRIC_KEY           = (0x1 << 3),
    PIN_PASSWORD                    = (0x1 << 4),
    ASYMMETRIC_ENCRYPTION_KEY       = (0x1 << 5),
} OSCTBitmask_t;

/**
 * @brief   /oic/sec/credtype (Credential Type) data type.
 *          Derived from OIC Security Spec /oic/sec/cred; see Spec for details.
 *              0:  no security mode
 *              1:  symmetric pair-wise key
 *              2:  symmetric group key
 *              4:  asymmetric key
 *              8:  signed asymmetric key (aka certificate)
 *              16: PIN /password
 */
typedef OSCTBitmask_t OicSecCredType_t;

typedef struct OicSecDoxm OicSecDoxm_t;

typedef enum OicSecDpm
{
    NORMAL                          = 0x0,
    RESET                           = (0x1 << 0),
    TAKE_OWNER                      = (0x1 << 1),
    BOOTSTRAP_SERVICE               = (0x1 << 2),
    SECURITY_MANAGEMENT_SERVICES    = (0x1 << 3),
    PROVISION_CREDENTIALS           = (0x1 << 4),
    PROVISION_ACLS                  = (0x1 << 5),
    // << 6 THROUGH 15 RESERVED
} OicSecDpm_t;

typedef enum OicSecDpom
{
    MULTIPLE_SERVICE_SERVER_DRIVEN  = 0x0,
    SINGLE_SERVICE_SERVER_DRIVEN    = 0x1,
    MULTIPLE_SERVICE_CLIENT_DRIVEN  = 0x2,
    SINGLE_SERVICE_CLIENT_DRIVEN    = 0x3,
} OicSecDpom_t;

typedef enum OicSecSvcType
{
    SERVICE_UNKNOWN                 = 0x0,
    ACCESS_MGMT_SERVICE             = 0x1,  //urn:oic.sec.ams
} OicSecSvcType_t;


//TODO: Need more clarification on deviceIDFormat field type.
#if 0
typedef enum
{
    URN = 0x0
}OicSecDvcIdFrmt_t;
#endif

typedef enum
{
    OIC_JUST_WORKS                          = 0x0,
    OIC_RANDOM_DEVICE_PIN                   = 0x1,
    OIC_MANUFACTURER_CERTIFICATE           = 0x2,
    OIC_OXM_COUNT
}OicSecOxm_t;

typedef struct OicSecJwk OicSecJwk_t;

typedef struct OicSecPstat OicSecPstat_t;

typedef struct OicSecRole OicSecRole_t;

typedef struct OicSecSacl OicSecSacl_t;

typedef struct OicSecSvc OicSecSvc_t;

typedef char *OicUrn_t; //TODO is URN type defined elsewhere?

typedef struct OicUuid OicUuid_t; //TODO is UUID type defined elsewhere?


#ifdef __WITH_X509__
typedef struct OicSecCrl OicSecCrl_t;
#endif /* __WITH_X509__ */

/**
 * @brief   /oic/uuid (Universal Unique Identifier) data type.
 */
#define UUID_LENGTH 128/8 // 128-bit GUID length
//TODO: Confirm the length and type of ROLEID.
#define ROLEID_LENGTH 128/8 // 128-bit ROLEID length
#define OWNER_PSK_LENGTH_128 128/8 //byte size of 128-bit key size
#define OWNER_PSK_LENGTH_256 256/8 //byte size of 256-bit key size

struct OicUuid
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    //TODO fill in unless this is defined elsewhere?
    uint8_t             id[UUID_LENGTH];
};

/**
 * @brief   /oic/sec/jwk (JSON Web Key) data type.
 *          See JSON Web Key (JWK)  draft-ietf-jose-json-web-key-41
 */
#define JWK_LENGTH 256/8 // 256 bit key length
struct OicSecJwk
{
    char                *data;
};

/**
 * @brief   /oic/sec/acl (Access Control List) data type.
 *          Derived from OIC Security Spec; see Spec for details.
 */
struct OicSecAcl
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    OicUuid_t           subject;        // 0:R:S:Y:uuid TODO: this deviates
                                        // from spec and needs to be updated
                                        // in spec (where it's a String).
    size_t              resourcesLen;   // the number of elts in Resources
    char                **resources;    // 1:R:M:Y:String
    uint16_t            permission;     // 2:R:S:Y:UINT16
    size_t              prdRecrLen;     // the number of elts in Periods
    char                **periods;       // 3:R:M*:N:String (<--M*; see Spec)
    char                **recurrences;   // 5:R:M:N:String
    size_t              ownersLen;      // the number of elts in Owners
    OicUuid_t           *owners;        // 8:R:M:Y:oic.uuid
    // NOTE: we are using UUID for Owners instead of Svc type for mid-April
    // SRM version only; this will change to Svc type for full implementation.
    //TODO change Owners type to oic.sec.svc
    //OicSecSvc_t         *Owners;        // 6:R:M:Y:oic.sec.svc
    OicSecAcl_t         *next;
};

/**
 * @brief   /oic/sec/amacl (Access Manager Service Accesss Control List)
 *          data type.
 *          Derived from OIC Security Spec; see Spec for details.
 */
struct OicSecAmacl
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    size_t              resourcesLen;   // the number of elts in Resources
    char                **resources;    // 0:R:M:Y:String
    size_t              amssLen;        // the number of elts in Amss
    OicUuid_t           *amss;          // 1:R:M:Y:acl
    size_t              ownersLen;      // the number of elts in Owners
    OicUuid_t           *owners;        // 2:R:M:Y:oic.uuid
    // NOTE: we are using UUID for Owners instead of Svc type for mid-April
    // SRM version only; this will change to Svc type for full implementation.
    //TODO change Owners type to oic.sec.svc
    //OicSecSvc_t         *Owners;        // 2:R:M:Y:oic.sec.svc
    OicSecAmacl_t         *next;
};

/**
 * @brief   /oic/sec/cred (Credential) data type.
 *          Derived from OIC Security Spec; see Spec for details.
 */
struct OicSecCred
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    uint16_t            credId;         // 0:R:S:Y:UINT16
    OicUuid_t           subject;        // 1:R:S:Y:oic.uuid
    //Note: Need further clarification on roleID data type
    //NOTE: Need further clarification on roleId datatype.
    //size_t              roleIdsLen;     // the number of elts in RoleIds
    //OicSecRole_t        *roleIds;       // 2:R:M:N:oic.sec.role
    OicSecCredType_t    credType;       // 3:R:S:Y:oic.sec.credtype
    OicSecJwk_t         publicData;     // 5:R:S:N:oic.sec.jwk
    OicSecJwk_t         privateData;    // 6:R:S:N:oic.sec.jwk
    char                *period;        // 7:R:S:N:String
    size_t              ownersLen;      // the number of elts in Owners
    OicUuid_t           *owners;        // 8:R:M:Y:oic.uuid
    // NOTE: we are using UUID for Owners instead of Svc type for mid-April
    // SRM version only; this will change to Svc type for full implementation.
    //OicSecSvc_t         *Owners;        // 8:R:M:Y:oic.sec.svc
    //TODO change Owners type to oic.sec.svc
    OicSecCred_t        *next;
};

/**
 * @brief   /oic/sec/doxm (Device Owner Transfer Methods) data type
 *          Derived from OIC Security Spec; see Spec for details.
 */
struct OicSecDoxm
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    OicUrn_t            *oxmType;       // 0:R:M:N:URN
    size_t              oxmTypeLen;     // the number of elts in OxmType
    OicSecOxm_t         *oxm;           // 1:R:M:N:UINT16
    size_t              oxmLen;         // the number of elts in Oxm
    OicSecOxm_t         oxmSel;         // 2:R/W:S:Y:UINT16
    OicSecCredType_t    sct;            // 3:R:S:Y:oic.sec.credtype
    bool                owned;          // 4:R:S:Y:Boolean
    //TODO: Need more clarification on deviceIDFormat field type.
    //OicSecDvcIdFrmt_t   deviceIDFormat; // 5:R:S:Y:UINT8
    OicUuid_t           deviceID;       // 6:R:S:Y:oic.uuid
    OicUuid_t           owner;         // 7:R:S:Y:oic.uuid
    // NOTE: we are using UUID for Owner instead of Svc type for mid-April
    // SRM version only; this will change to Svc type for full implementation.
    //OicSecSvc_t       devOwner;        // 7:R:S:Y:oic.sec.svc
    //OicSecSvc_t       rOwner;        // 8:R:S:Y:oic.sec.svc
    //TODO change Owner type to oic.sec.svc
};

/**
 * @brief   /oic/sec/pstat (Provisioning Status) data type.
 * NOTE: this struct is ahead of Spec v0.95 in definition to include Sm.
 * TODO: change comment when reconciled to Spec v0.96.
 */
struct OicSecPstat
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    bool                isOp;           // 0:R:S:Y:Boolean
    OicSecDpm_t         cm;             // 1:R:S:Y:oic.sec.dpm
    OicSecDpm_t         tm;             // 2:RW:S:Y:oic.sec.dpm
    OicUuid_t           deviceID;       // 3:R:S:Y:oic.uuid
    OicSecDpom_t        om;             // 4:RW:M:Y:oic.sec.dpom
    size_t              smLen;          // the number of elts in Sm
    OicSecDpom_t        *sm;            // 5:R:M:Y:oic.sec.dpom
    uint16_t            commitHash;     // 6:R:S:Y:oic.sec.sha256
    //TODO: this is supposed to be a 256-bit uint; temporarily use uint16_t
    //TODO: need to decide which 256 bit and 128 bit types to use... boost?
};

/**
 * @brief   /oic/sec/role (Role) data type.
 *          Derived from OIC Security Spec; see Spec for details.
 */
struct OicSecRole
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    //TODO fill in with Role definition
    uint8_t             id[ROLEID_LENGTH];
};

/**
 * @brief   /oic/sec/sacl (Signed Access Control List) data type.
 *          Derived from OIC Security Spec; see Spec for details.
 */
struct OicSecSacl
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    //TODO fill in from OIC Security Spec
    char temp;
};

/**
 * @brief   /oic/sec/svc (Service requiring a secure connection) data type.
 *          Derived from OIC Security Spec; see Spec for details.
 */
struct OicSecSvc
{
    // <Attribute ID>:<Read/Write>:<Multiple/Single>:<Mandatory?>:<Type>
    OicUuid_t               svcdid;                 //0:R:S:Y:oic.uuid
    OicSecSvcType_t         svct;                   //1:R:M:Y:OIC Service Type
    size_t                  ownersLen;              //2:the number of elts in Owners
    OicUuid_t               *owners;                //3:R:M:Y:oic.uuid
    OicSecSvc_t             *next;
};

#ifdef __WITH_X509__
struct OicSecCrl
{
    uint16_t CrlId;
    ByteArray ThisUpdate;
    ByteArray CrlData;
};
#endif /* __WITH_X509__ */

#ifdef __cplusplus
}
#endif

#endif //OC_SECURITY_RESOURCE_TYPES_H

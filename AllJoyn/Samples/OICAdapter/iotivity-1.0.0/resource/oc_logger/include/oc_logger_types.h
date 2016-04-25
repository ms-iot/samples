//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

#ifndef __OC_LOGGER_TYPES_H_2024_09_5
 #define __OC_LOGGER_TYPES_H_2024_09_5

#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif

 typedef enum
 {
     OC_LOG_MIN_VAL__   = -1,
     OC_LOG_ALL         = 0,
     OC_LOG_FATAL,
     OC_LOG_ERROR,
     OC_LOG_WARNING,
     OC_LOG_INFO,
     OC_LOG_DEBUG,
     OC_LOG_DISABLED,
     OC_LOG_MAX_VAL__
 } oc_log_level;

typedef struct _oc_log_ctx
{
 void*                  ctx;

 oc_log_level           log_level;

 char*                  module_name;

 /* Required interface: */
 int  (*init)           (struct _oc_log_ctx *, void *);
 void (*destroy)        (struct _oc_log_ctx *);
 void (*flush)          (struct _oc_log_ctx *);
 void (*set_level)      (struct _oc_log_ctx *, const int);
 size_t (*write_level)  (struct _oc_log_ctx *, const int, const char *);
 int  (*set_module)     (struct _oc_log_ctx *, const char *);

 /* Optional interface (if one is implemented, all must be implemented): */
 int (*lock)            (struct _oc_log_ctx *);
 int (*unlock)          (struct _oc_log_ctx *);
 int (*try_lock)        (struct _oc_log_ctx *);
 int (*locked_destroy)  (struct _oc_log_ctx *);

} oc_log_ctx_t;

/* Notice that these are all passed the /top level/ ctx-- it's "public" with respect to
these functions, they have full access to fiddle with the structure all they want (but,
generally should avoid doing that); I could certainly be convinced to go the other direction,
and have most functions only take the inner context: */
typedef int    (*oc_log_init_t)          (oc_log_ctx_t *, void *);
typedef void   (*oc_log_destroy_t)       (oc_log_ctx_t *);
typedef void   (*oc_log_flush_t)         (oc_log_ctx_t *);
typedef void   (*oc_log_set_level_t)     (oc_log_ctx_t *, const int);
typedef size_t (*oc_log_write_level_t)   (oc_log_ctx_t *, const int, const char *);
typedef int    (*oc_log_set_module_t)    (oc_log_ctx_t *, const char *);
typedef int    (*oc_log_lock_t)          (oc_log_ctx_t *);
typedef int    (*oc_log_unlock_t)        (oc_log_ctx_t *);
typedef int    (*oc_log_try_lock_t)      (oc_log_ctx_t *);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif


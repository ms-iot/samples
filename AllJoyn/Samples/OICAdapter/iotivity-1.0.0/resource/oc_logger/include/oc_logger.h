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

#ifndef __OC_LOGGER_H_2014_09_5
 #define __OC_LOGGER_H_2014_09_5

#include "oc_logger_types.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* Basic interface: */
oc_log_ctx_t *oc_log_make_ctx(
                void*                 world,
                const oc_log_level    level,
                oc_log_init_t         init,
                oc_log_destroy_t      destroy,
                oc_log_flush_t        flush,
                oc_log_set_level_t    set_level,
                oc_log_write_level_t  write_level,
                oc_log_set_module_t   set_module
               );

void oc_log_destroy(oc_log_ctx_t *ctx);

void oc_log_flush(oc_log_ctx_t *ctx);
void oc_log_set_level(oc_log_ctx_t *ctx, const oc_log_level ll);
size_t oc_log_write(oc_log_ctx_t *ctx, const char *msg);
size_t oc_log_write_level(oc_log_ctx_t *ctx, const oc_log_level ll, const char *msg);
int oc_log_set_module(oc_log_ctx_t *ctx, const char *module_name);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif

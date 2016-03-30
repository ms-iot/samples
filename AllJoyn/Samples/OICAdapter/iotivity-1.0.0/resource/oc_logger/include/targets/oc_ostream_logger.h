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

#ifndef __OC_OSTREAM_LOGGER_H_2014_09_5
 #define __OC_OSTREAM_LOGGER_H_2014_09_5

#include "oc_logger_types.h"

/* Example of a C-callable C++ logger: */
#ifdef __cplusplus
 extern "C" {
#endif

oc_log_ctx_t *oc_make_ostream_logger();

int  oc_ostream_log_init(oc_log_ctx_t *ctx, void *world);
void oc_ostream_log_destroy(oc_log_ctx_t *ctx);
void oc_ostream_log_flush(oc_log_ctx_t *ctx);
void oc_ostream_log_set_level(oc_log_ctx_t *ctx, const int level);
size_t oc_ostream_log_write(oc_log_ctx_t *ctx, const int level, const char *msg);
int  oc_ostream_log_set_module(oc_log_ctx_t *ctx, const char *module_name);

int oc_ostream_log_lock(oc_log_ctx_t *ctx);
int oc_ostream_log_unlock(oc_log_ctx_t *ctx);
int oc_ostream_log_try_lock(oc_log_ctx_t *ctx);     // non-blocking
int oc_ostream_log_locked_destroy(oc_log_ctx_t *ctx);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif

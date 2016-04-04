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

#ifndef __OC_CONSOLE_LOGGER_H_2014_09_5
 #define __OC_CONSOLE_LOGGER_H_2014_09_5

#include "oc_logger_types.h"

#ifdef __cplusplus
 extern "C" {
#endif

oc_log_ctx_t *oc_make_console_logger();

int oc_console_logger_init(oc_log_ctx_t *ctx, void *world);
void oc_console_logger_destroy(oc_log_ctx_t *ctx);
void oc_console_logger_flush(oc_log_ctx_t *ctx);
void oc_console_logger_set_level(oc_log_ctx_t *ctx, const int level);
size_t oc_console_logger_write(oc_log_ctx_t *ctx, const int level, const char *msg);
int oc_console_logger_set_module(oc_log_ctx_t *ctx, const char *module_name);

#ifdef __cplusplus
 } // extern "C"
#endif

#endif

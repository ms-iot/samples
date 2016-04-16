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

#ifndef OIC_CONSOLE_LOGGER_
#define OIC_CONSOLE_LOGGER_

#include "oic_logger_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

oic_log_ctx_t *oic_make_console_logger();

int oic_console_logger_init(oic_log_ctx_t *ctx, void *world);
void oic_console_logger_destroy(oic_log_ctx_t *ctx);
void oic_console_logger_flush(oic_log_ctx_t *ctx);
void oic_console_logger_set_level(oic_log_ctx_t *ctx, const int level);
size_t oic_console_logger_write(oic_log_ctx_t *ctx, const int level, const char *msg);
int oic_console_logger_set_module(oic_log_ctx_t *ctx, const char *module_name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OIC_CONSOLE_LOGGER_ */


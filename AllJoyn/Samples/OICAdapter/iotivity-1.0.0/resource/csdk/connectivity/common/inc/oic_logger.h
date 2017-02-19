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

#ifndef OIC_LOGGER_H_
#define OIC_LOGGER_H_

#include "oic_logger_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Basic interface: */
oic_log_ctx_t *oic_log_make_ctx(void *world, const oic_log_level level, oic_log_init_t init,
                                    oic_log_destroy_t destroy, oic_log_flush_t flush,
                                    oic_log_set_level_t set_level,oic_log_write_level_t write_level,
                                    oic_log_set_module_t set_module);

void oic_log_destroy(oic_log_ctx_t *ctx);

void oic_log_flush(oic_log_ctx_t *ctx);
void oic_log_set_level(oic_log_ctx_t *ctx, const oic_log_level ll);
size_t oic_log_write(oic_log_ctx_t *ctx, const char *msg);
size_t oic_log_write_level(oic_log_ctx_t *ctx, const oic_log_level ll, const char *msg);
int oic_log_set_module(oic_log_ctx_t *ctx, const char *module_name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OIC_LOGGER_H_ */


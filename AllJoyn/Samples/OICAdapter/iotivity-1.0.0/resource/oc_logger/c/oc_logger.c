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

#include "oc_logger.h"
#include "oic_string.h"

#include <string.h>
#include <stdlib.h>

oc_log_ctx_t *oc_log_make_ctx(
                            void*                 world,
                            const oc_log_level    level,
                            oc_log_init_t         init,
                            oc_log_destroy_t      destroy,
                            oc_log_flush_t        flush,
                            oc_log_set_level_t    set_level,
                            oc_log_write_level_t  write_level,
                            oc_log_set_module_t   set_module
                           )
{
 oc_log_ctx_t *log_ctx;

 if(0 == init ||
    0 == destroy ||
    0 == flush ||
    0 == set_level ||
    0 == write_level ||
    0 == set_module)
 {
  return 0;
 }

 if(OC_LOG_MIN_VAL__ >= level || OC_LOG_MAX_VAL__ <= level)
 {
     return 0;
 }

 log_ctx = (oc_log_ctx_t *)malloc(sizeof(oc_log_ctx_t));

 if(!log_ctx)
 {
     return 0;
 }

 log_ctx->ctx           = 0; /* we'll get to this in a sec... */
 log_ctx->log_level     = level;
 log_ctx->module_name   = 0;
 log_ctx->init          = init;
 log_ctx->destroy       = destroy;
 log_ctx->flush         = flush;
 log_ctx->set_level     = set_level;
 log_ctx->set_module    = set_module;

 log_ctx->write_level   = write_level;

 if(!log_ctx->init(log_ctx, world))
  {
    free(log_ctx);
    return 0;
  }

 return log_ctx;
}

void oc_log_destroy(oc_log_ctx_t *ctx)
{
 if(!ctx)
 {
     return;
 }

 ctx->destroy(ctx);

 if(0 != ctx->module_name)
 {
     free(ctx->module_name);
 }

 free(ctx);
}

int oc_log_init(oc_log_ctx_t *ctx, void *world)
{
 if(!ctx)
 {
     return 0;
 }

 return ctx->init(ctx, world);
}

void oc_log_flush(oc_log_ctx_t *ctx)
{
    if(!ctx)
    {
        return;
    }
    ctx->flush(ctx);
}

void oc_log_set_level(oc_log_ctx_t *ctx, const oc_log_level loglevel)
{
    if(!ctx)
    {
        return;
    }
    ctx->set_level(ctx, loglevel);
}

size_t oc_log_write(oc_log_ctx_t *ctx, const char *msg)
{
    if(!ctx)
    {
        return 0;
    }

 return oc_log_write_level(ctx, ctx->log_level, msg);
}

size_t oc_log_write_level(oc_log_ctx_t *ctx, const oc_log_level loglevel, const char *msg)
{
 if(!ctx)
 {
     return 0;
 }

 ctx->log_level = loglevel;

 /* Notify: */
 return ctx->write_level(ctx, loglevel, msg);
}

int oc_log_set_module(oc_log_ctx_t *ctx, const char *module_name)
{
 char *mn = NULL;

 if(!ctx || !module_name)
 {
     return 0;
 }

 /* Swap pointers so that module data's not erased in the event of failure: */
 mn = OICStrdup(module_name);

 if(!mn)
 {
     return 0;
 }

 if(!ctx->module_name)
 {
     free(ctx->module_name);
 }

 ctx->module_name = mn;

 /* Notify: */
 return ctx->set_module(ctx, ctx->module_name);
}



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

#include "oic_logger.h"
#include "oic_string.h"

#include <string.h>
#include <stdlib.h>

oic_log_ctx_t *oic_log_make_ctx(void *world, const oic_log_level level, oic_log_init_t init,
                                oic_log_destroy_t destroy, oic_log_flush_t flush,
                                oic_log_set_level_t set_level,oic_log_write_level_t write_level,
                                oic_log_set_module_t set_module)
{
    oic_log_ctx_t *log_ctx;

    if (0 == init || 0 == destroy || 0 == flush || 0 == set_level || 0 == write_level
        || 0 == set_module)
        return 0;

    if (__OIC_LOG_MIN__ > level || __OIC_LOG_MAX__ < level)
        return 0;

    log_ctx = (oic_log_ctx_t *) malloc(sizeof(oic_log_ctx_t));

    if (0 == log_ctx)
        return 0;

    log_ctx->ctx = 0; /* we'll get to this in a sec... */
    log_ctx->log_level = level;
    log_ctx->module_name = 0;
    log_ctx->init = init;
    log_ctx->destroy = destroy;
    log_ctx->flush = flush;
    log_ctx->set_level = set_level;
    log_ctx->set_module = set_module;

    log_ctx->write_level = write_level;

    if (0 == log_ctx->init(log_ctx, world))
    {
        free(log_ctx);
        return 0;
    }

    return log_ctx;
}

void oic_log_destroy(oic_log_ctx_t *ctx)
{
    if (0 == ctx)
        return;

    ctx->destroy(ctx);

    if (0 != ctx->module_name)
        free(ctx->module_name);

    free(ctx);
}

int oic_log_init(oic_log_ctx_t *ctx, void *world)
{
    if (0 == ctx)
        return 0;

    return ctx->init(ctx, world);
}

void oic_log_flush(oic_log_ctx_t *ctx)
{
    if (0 == ctx)
    {
        return;
    }
    ctx->flush(ctx);
}

void oic_log_set_level(oic_log_ctx_t *ctx, const oic_log_level ll)
{
    if (0 == ctx)
    {
        return;
    }
    ctx->set_level(ctx, ll);
}

size_t oic_log_write(oic_log_ctx_t *ctx, const char *msg)
{
    if (0 == ctx)
        return 0;

    return oic_log_write_level(ctx, ctx->log_level, msg);
}

size_t oic_log_write_level(oic_log_ctx_t *ctx, const oic_log_level ll, const char *msg)
{
    if (0 == ctx)
        return 0;

    ctx->log_level = ll;

    /* Notify: */
    return ctx->write_level(ctx, ll, msg);
}

int oic_log_set_module(oic_log_ctx_t *ctx, const char *module_name)
{
    char *mn;

    if (0 == ctx)
        return 0;

    /* Swap pointers so that module data's not erased in the event of failure: */
    mn = OICStrdup(module_name);
    if (0 == mn)
    {
        if (0 != ctx->module_name)
            free(ctx->module_name);
        return 0;
    }

    if (0 != ctx->module_name)
        free(ctx->module_name);

    ctx->module_name = mn;

    /* Notify: */
    return ctx->set_module(ctx, ctx->module_name);
}



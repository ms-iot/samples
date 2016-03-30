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
#include "oic_console_logger.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    FILE *out;
} oic_console_logger_ctx;

oic_log_ctx_t *oic_make_console_logger()
{
    return oic_log_make_ctx(NULL, OIC_LOG_ALL, oic_console_logger_init, oic_console_logger_destroy,
                            oic_console_logger_flush, oic_console_logger_set_level,
                            oic_console_logger_write,oic_console_logger_set_module);
}

int oic_console_logger_init(oic_log_ctx_t *ctx, void *world)
{
    (void)world;
    oic_console_logger_ctx *my_ctx =
        (oic_console_logger_ctx *) malloc(sizeof(oic_console_logger_ctx));

    if (0 == my_ctx)
        return 0;

    my_ctx->out = stderr;

    ctx->ctx = (void *) my_ctx;

    return 1;
}

void oic_console_logger_destroy(oic_log_ctx_t *ctx)
{
    oic_console_logger_ctx *lctx = (oic_console_logger_ctx *) ctx->ctx;

    fflush(lctx->out);

    free(lctx);
}

void oic_console_logger_flush(oic_log_ctx_t *ctx)
{
    oic_console_logger_ctx *lctx = (oic_console_logger_ctx *) ctx->ctx;

    fflush(lctx->out);
}

void oic_console_logger_set_level(oic_log_ctx_t *ctx, const int level)
{
    (void)ctx;
    (void)level;
    /* We don't have any special thing we need to do when a log level changes. */
    return;
}

size_t oic_console_logger_write(oic_log_ctx_t *ctx, const int level, const char *msg)
{
    oic_console_logger_ctx *lctx = (oic_console_logger_ctx *) ctx->ctx;

    /* A "real" implementation might want to replace the loglevel with a mnemonic: */

    if (0 == ctx->module_name)
        return 1 + fprintf(lctx->out, "%d: %s\n", level, msg);

    return 1 + fprintf(lctx->out, "%d: [%s]: %s\n", level, ctx->module_name, msg);
}

int oic_console_logger_set_module(oic_log_ctx_t *ctx, const char *module_name)
{
    (void)ctx;
    (void)module_name;
    /* We don't do anything special when the module name changes: */
    return 1;
}

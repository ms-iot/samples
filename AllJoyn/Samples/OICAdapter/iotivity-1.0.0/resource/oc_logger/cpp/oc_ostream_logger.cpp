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

#include "oc_logger.hpp"
#include "targets/oc_ostream_logger.h"

#include <cstdio>
#include <cstdlib>

#include <mutex>
#include <memory>
#include <sstream>
#include <iostream>

namespace {

struct oc_ostream_logger_ctx
{
 std::ostream*  os_ptr;
 std::ostream&  os;

 std::mutex     mutex;

 oc_ostream_logger_ctx(std::ostream *here)
  : os_ptr(here),
    os(*os_ptr)
 {}
};

} // namespace

/* Courtesy-function: */
oc_log_ctx_t *oc_make_ostream_logger()
{
 return oc_log_make_ctx(
            nullptr,
            OC_LOG_ALL,
            oc_ostream_log_init,
            oc_ostream_log_destroy,
            oc_ostream_log_flush,
            oc_ostream_log_set_level,
            oc_ostream_log_write,
            oc_ostream_log_set_module
        );
}

int oc_ostream_log_init(oc_log_ctx_t *ctx, void *world)
try
{
 auto *target = reinterpret_cast<std::ostream *>(world);

 if(nullptr == world)
  target = &std::cout;

 oc_ostream_logger_ctx *my_ctx = new oc_ostream_logger_ctx(target);

 ctx->ctx = static_cast<void *>(my_ctx);

 return 1;
}
catch(...)
{
 return 0;
}

void oc_ostream_log_destroy(oc_log_ctx_t *ctx)
try
{
 static std::mutex dtor_mtx;

 oc_ostream_logger_ctx *lctx = static_cast<oc_ostream_logger_ctx *>(ctx->ctx);

 {
 std::unique_lock<std::mutex> ul(dtor_mtx);

 lctx->os << std::flush;

 delete lctx;
 }
}
catch(...)
{
}

void oc_ostream_log_flush(oc_log_ctx_t *ctx)
try
{
 oc_ostream_logger_ctx *lctx = static_cast<oc_ostream_logger_ctx *>(ctx->ctx);

 std::lock_guard<std::mutex> lg(lctx->mutex);

 lctx->os << std::flush;
}
catch(...)
{
}

void oc_ostream_log_set_level(oc_log_ctx_t * /*ctx*/, const int /*level*/)
try
{
 /* We don't have any special thing we need to do when a log level changes. */
 return;
}
catch(...)
{
}

size_t oc_ostream_log_write(oc_log_ctx_t *ctx, const int level, const char *msg)
try
{
 oc_ostream_logger_ctx *lctx = static_cast<oc_ostream_logger_ctx *>(ctx->ctx);

 std::lock_guard<std::mutex> lg(lctx->mutex);

 std::ostringstream os;

 os << level << ": ";

 if(nullptr != ctx->module_name)
  os << '[' << ctx->module_name << "] ";

 os << msg << '\n';

 lctx->os << os.str().c_str();

 return 1 + os.str().length();
}
catch(...)
{
 return 0;
}

int oc_ostream_log_set_module(oc_log_ctx_t * /*ctx*/,
                              const char * /*module_name*/)
try
{
 // Nothing special needs to happen for a module name change:
 return 1;
}
catch(...)
{
 return 0;
}

int oc_ostream_log_lock(oc_log_ctx_t *ctx)
try
{
 oc_ostream_logger_ctx *lctx = static_cast<oc_ostream_logger_ctx *>(ctx->ctx);

 lctx->mutex.lock();

 return 1;
}
catch(...)
{
 return 0;
}

int oc_ostream_log_unlock(oc_log_ctx_t *ctx)
try
{
 oc_ostream_logger_ctx *lctx = static_cast<oc_ostream_logger_ctx *>(ctx->ctx);

 lctx->mutex.unlock();

 return 1;
}
catch(...)
{
 return 0;
}

int oc_ostream_log_try_lock(oc_log_ctx_t *ctx)
try
{
 oc_ostream_logger_ctx *lctx = static_cast<oc_ostream_logger_ctx *>(ctx->ctx);

 return lctx->mutex.try_lock();
}
catch(...)
{
 return 0;
}

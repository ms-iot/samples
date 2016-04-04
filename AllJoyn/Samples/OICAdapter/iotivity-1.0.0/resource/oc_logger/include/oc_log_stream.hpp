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

#ifndef __OC_LOG_STREAM_HPP_20140910
 #define __OC_LOG_STREAM_HPP_20140910

#include <iosfwd>
#include <memory>
#include <cassert>
#include <iostream>

#include <boost/config.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/ios.hpp>

#include "oc_logger.h"

namespace OC {

class oc_log_stream : boost::iostreams::sink
{
 std::shared_ptr<oc_log_ctx_t> m_log;

 public:
 typedef char                            char_type;
 typedef boost::iostreams::sink_tag      category;

 public:
 template <class ContextCtor>
 oc_log_stream(ContextCtor& c)
  : m_log { c(), oc_log_destroy }
 {}

 template <class ContextCtor>
 oc_log_stream(ContextCtor& c, void *world)
  : m_log { c(world), oc_log_destroy }
 {}

 public:
 inline void flush()                                    noexcept { return oc_log_flush(m_log.get()); }
 inline void set_level(const oc_log_level new_level)    noexcept { return oc_log_set_level(m_log.get(), new_level); }
 inline int  set_module(const std::string& module_name) noexcept { return oc_log_set_module(m_log.get(), module_name.c_str()); }

 public:
 std::streamsize write(const char_type *s, std::streamsize n)
 {
    /* It may seem strange to do this here, but it's a consequence of the
    underlying library not supporting ptr+len style buffers at this time: */
    std::string s2(s, n + s);

    oc_log_write(m_log.get(), s2.c_str());

    return n;
 }

 private:
 oc_log_stream operator=(const oc_log_stream&)  = delete;
};

} // namespace OC

#endif

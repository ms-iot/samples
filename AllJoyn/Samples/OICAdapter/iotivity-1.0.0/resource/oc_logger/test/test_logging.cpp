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

#include "oc_log.hpp"
#include "oc_console_logger.h"
#include "oc_ostream_logger.hpp"

#include <iosfwd>
#include <memory>
#include <cassert>
#include <sstream>
#include <iostream>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>

#include <boost/config.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/ios.hpp>

int main()
{
 using OC::oc_log_stream;

 oc_log_stream ols(oc_make_console_logger);

 boost::iostreams::stream<oc_log_stream> os(ols);

 os << "Greetings from the nifty world of loggin'!" << std::flush;

 (*os).set_module("TheHappiestModuleEver");

 os << "Whee!" << std::flush;

 return 0;
}


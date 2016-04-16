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

void basic_demo()
{
 using OC::oc_log_stream;

 oc_log_stream ols(oc_make_ostream_logger);

 boost::iostreams::stream<oc_log_stream> os(ols);

 os << "Greetings from the nifty world of logging!" << std::flush;

 ols.set_level(OC_LOG_ALL);
 ols.set_module("TheHappyModule");
 ols.set_module("TheModule");
 os << "Whee!" << std::flush;

 // Setting the module name by getting the device from the stream itself:
 (*os).set_module("TheHappiestModuleEver");
 os << "Whee! Again!" << std::flush;
}

/* Show that we can use a C logger from C++: */
void c_demo()
{
 using OC::oc_log_stream;

 oc_log_stream ols(oc_make_console_logger);

 boost::iostreams::stream<oc_log_stream> os(ols);

 os << "Greetings from the nifty world of logging!" << std::flush;

 ols.set_level(OC_LOG_ALL);
 ols.set_module("TheHappyModule");
 os << "Whee!" << std::flush;

 (*os).set_module("TheHappiestModuleEver");
 os << "Whee!" << std::flush;
}

void alternative_demo()
{
 /* Annother way to create a context: */
 auto logger = []() -> boost::iostreams::stream<OC::oc_log_stream>&
  {
    static OC::oc_log_stream ols(oc_make_ostream_logger);
    static boost::iostreams::stream<OC::oc_log_stream> os(ols);

    return os;
  };

 logger()->set_module("FantasticModule");
 logger() << "Hello, logging world!" << std::flush;
}

int main()
{
 basic_demo();
 c_demo();
 alternative_demo();
 return 0;
}


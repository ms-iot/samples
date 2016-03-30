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
#include "targets/oc_console_logger.h"
#include "targets/oc_ostream_logger.h"

#include <stdio.h>

/* Example of basic usage of the C library: */
void basic_demo(void)
{
 oc_log_ctx_t *log;

 log = oc_make_console_logger();

 if(0 == log)
  {
	fprintf(stderr, "Unable to initialize logging subsystem.\n");
	return;
  }

 oc_log_write(log, "Hello, World!");

 oc_log_set_module(log, "FabulousModule");

 oc_log_set_level(log, 50);

 oc_log_write(log, "Hello again, World!");

 oc_log_destroy(log);
}

/* Example of calling a C++ log implementation from C: */
void cpp_demo()
{
 oc_log_ctx_t *log;

 log = oc_make_ostream_logger();

 if(0 == log)
  {
	fprintf(stderr, "Unable to initialize logging subsystem.\n");
	return;
  }

 oc_log_write(log, "Hello from C++, World!");

 oc_log_set_module(log, "BestModuleEver");

 oc_log_set_level(log, 50);

 oc_log_write(log, "Hello again from C++, World!");
 oc_log_write(log, "Hello once more from C++, World!");

 oc_log_destroy(log);
}

int main()
{
 basic_demo();
 cpp_demo();

 return 0;
}

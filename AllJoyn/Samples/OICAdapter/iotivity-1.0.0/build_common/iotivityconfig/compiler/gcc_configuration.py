# ------------------------------------------------------------------------
# Copyright 2015 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------

from configuration import Configuration

# GCC compiler-specific configuration
class GccConfiguration(Configuration):
    def __init__(self, context):
        Configuration.__init__(self, context)

    # ------------------------------------------------------------
    # Return test program to be used when checking for basic C99
    # support in GCC.
    # ------------------------------------------------------------
    def _c99_test_program(self):
        # Use the default C99 test program but enable pedantic
        # diagnostics specific to GCC to force errors to occur if a
        # flag is required to compile C99 code without warning or
        # error.

        from default_configuration import DefaultConfiguration
        def_config = DefaultConfiguration(self._context)

        return """
#ifndef __clang__
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-pedantic"
#endif
""" + def_config._c99_test_program()

    # -------------------------------------------------------------
    # Get flags known to enable C99 support for GCC C compiler.
    # -------------------------------------------------------------
    def _c99_flags(self):
        # Favor flags that do not enable GNU extensions by default,
        # e.g. '-std=c99'.
        return [ '-std=c99',
                 '-std=iso9899:1999',
                 '-std=gnu99',
                 '-std=c9x',
                 '-std=iso9899:199x',
                 '-std=gnu9x' ]

    # ------------------------------------------------------------
    # Return test program to be used when checking for basic C++11
    # support in GCC.
    # ------------------------------------------------------------
    def _cxx11_test_program(self):
        # Besides checking for a basic C++11 feature, this contrived
        # test program is designed to trigger an issue in some older
        # (pre-C++11) C library headers that causes functions like
        # snprintf() to remain undeclared when -std=c++0x or -ansi,
        # for example, is added to the g++ command line flags, and
        # despite the fact the appropriate feature test macro to make
        # the prototypes visible is defined.
        #
        # The recommended workaround is to instead of -std=c++0x use
        # -std=gnu++0x to explicitly enable GNU extensions.  Failure
        # will force the configuration test to attempt the build with
        # -std=gnu++0x, for example.
        #
        # For a more complete description of this issue, see:
        #     https://gcc.gnu.org/bugzilla/show_bug.cgi?id=34032
        #
        # Other compilers should be able to compile this code without
        # issue.
        return """
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>

int main()
{
    int x = 3210;
    auto f = [x](){
        char buf[15] = { 0 };
        snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%d", x);
        return buf[0];
    };

    return f() != '3';
}
"""

    # -------------------------------------------------------------
    # Get flags known to enable C++11 support for GCC C++ compiler.
    # -------------------------------------------------------------
    def _cxx11_flags(self):
        # Favor command line flags from more recent versions of the
        # compiler over older/deprecated flags.  Also, favor flags
        # that do not enable GNU extensions by default,
        # e.g. '-std=c++..'.
        return [ '-std=c++11',
                 '-std=gnu++11',
                 '-std=c++0x',
                 '-std=gnu++0x' ]

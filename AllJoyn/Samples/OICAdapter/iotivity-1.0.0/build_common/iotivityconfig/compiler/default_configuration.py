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

# Default (very simple) compiler configuration
class DefaultConfiguration(Configuration):
    def __init__(self, context):
        Configuration.__init__(self, context)

    # ------------------------------------------------------------
    # Return test program to be used when checking for basic C++11
    # support.
    # ------------------------------------------------------------
    def _c99_test_program(self):
        return """
// Some headers found in C99.
#include <stdbool.h>
#include <stdint.h>

int main()
{
    struct foo
    {
        bool b;      // C99 type
        int i;
        uint64_t q;  // C99 type
    };

    // Designated initializer.
    struct foo bar = { .b = false, .q = UINT64_MAX };

    // Implicitly initialized field.
    return bar.i != 0;
}
"""

    # --------------------------------------------------------------
    # Get list of flags that could potentially enable C99 support.
    #
    # The default configuration assumes that no flag is needed to
    # enable C99 support.
    # --------------------------------------------------------------
    def _c99_flags(self):
        return []

    # ------------------------------------------------------------
    # Return test program to be used when checking for basic C++11
    # support.
    # ------------------------------------------------------------
    def _cxx11_test_program(self):
        return """
int main()
{
    int x = 3210;
    auto f = [x](){
        return x;
    };

    return f() != x;
}
"""

    # --------------------------------------------------------------
    # Get list of flags that could potentially enable C++11 support.
    #
    # The default configuration assumes that no flag is needed to
    # enable C++11 support.
    # --------------------------------------------------------------
    def _cxx11_flags(self):
        return []

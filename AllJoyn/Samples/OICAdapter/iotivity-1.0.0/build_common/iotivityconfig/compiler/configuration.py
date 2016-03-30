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

class Configuration:
    """Compiler-specific configuration abstract base class"""

    def __init__(self, context):
        """
        Initialize the Configuration object

        Arguments:
        context -- the scons configure context
        """

        if type(self) is Configuration:
            raise TypeError('abstract class cannot be instantiated')

        self._context = context      # scons configure context
        self._env     = context.env  # scons environment

    def check_c99_flags(self):
        """
        Check if command line flag is required to enable C99
        support.

        Returns 1 if no flag is required, 0 if no flag was
        found, and the actual flag if one was found.

        CFLAGS will be updated with appropriate C99 flag,
        accordingly.
        """

        return self._check_flags(self._c99_flags(),
                                 self._c99_test_program(),
                                 '.c',
                                 'CFLAGS')

    def check_cxx11_flags(self):
        """
        Check if command line flag is required to enable C++11
        support.

        Returns 1 if no flag is required, 0 if no flag was
        found, and the actual flag if one was found.

        CXXFLAGS will be updated with appropriate C++11 flag,
        accordingly.
        """

        return self._check_flags(self._cxx11_flags(),
                                 self._cxx11_test_program(),
                                 '.cpp',
                                 'CXXFLAGS')

    def has_pthreads_support(self):
        """
        Check if PThreads are supported by this system

        Returns 1 if this system DOES support pthreads, 0
        otherwise
        """

        return self._context.TryCompile(self._pthreads_test_program(), '.c')

    # --------------------------------------------------------------
    # Check if flag is required to build the given test program.
    #
    # Arguments:
    # test_flags     -- list of flags that may be needed to build
    #                   test_program
    # test_program   -- program used used to determine if one of the
    #                   given flags is required to for a successful
    #                   build
    # test_extension -- file extension associated with the test
    #                   program, e.g. '.cpp' for C++ and '.c' for C
    # flags_key      -- key used to retrieve compiler flags that may
    #                   be updated by this check from the SCons
    #                   environment
    # --------------------------------------------------------------
    def _check_flags(self,
                     test_flags,
                     test_program,
                     test_extension,
                     flags_key):
        # Check if no additional flags are required.
        ret = self._context.TryCompile(test_program,
                                       test_extension)

        if ret is 0:
            # Try flags known to enable compiler features needed by
            # the test program.

            last_flags = self._env[flags_key]
            for flag in test_flags:
                self._env.Append(**{flags_key : flag})
                ret = self._context.TryCompile(test_program,
                                               test_extension)

                if ret:
                    # Found a flag!
                    return flag
                else:
                    # Restore original compiler flags for next flag
                    # test.
                    self._env.Replace(**{flags_key : last_flags})

        return ret

    # ------------------------------------------------------------
    # Return test program to be used when checking for basic C99
    # support.
    #
    # Subclasses should implement this template method or use the
    # default test program found in the DefaultConfiguration class
    # through composition.
    # ------------------------------------------------------------
    def _c99_test_program(self):
        raise NotImplementedError('unimplemented method')

    # --------------------------------------------------------------
    # Get list of flags that could potentially enable C99 support.
    #
    # Subclasses should implement this template method if flags are
    # needed to enable C99 support.
    # --------------------------------------------------------------
    def _c99_flags(self):
        raise NotImplementedError('unimplemented method')

    # ------------------------------------------------------------
    # Return test program to be used when checking for basic C++11
    # support.
    #
    # Subclasses should implement this template method or use the
    # default test program found in the DefaultConfiguration class
    # through composition.
    # ------------------------------------------------------------
    def _cxx11_test_program(self):
        raise NotImplementedError('unimplemented method')

    # --------------------------------------------------------------
    # Get list of flags that could potentially enable C++11 support.
    #
    # Subclasses should implement this template method if flags are
    # needed to enable C++11 support.
    # --------------------------------------------------------------
    def _cxx11_flags(self):
        raise NotImplementedError('unimplemented method')

    # --------------------------------------------------------------
    # Return a test program to be used when checking for PThreads
    # support
    #
    # --------------------------------------------------------------
    def _pthreads_test_program(self):
        return """
#include <unistd.h>
#include <pthread.h>
int main()
{
    #ifndef _POSIX_THREADS
    # error POSIX Threads support not available
    #endif
    return 0;
}
"""

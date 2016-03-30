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

# This file contains compiler tests for use in scons 'Configure'
# tests.

from compiler import factory

def _check_for_broken_gcc_headers(context, flag):
    # Check for issue in some older (pre-C++11) C library headers that
    # causes functions like snprintf() to remain undeclared when
    # -std=c++0x or -ansi, for example, is added to the g++ command
    # line flags, and despite the fact the appropriate feature test
    # macro to make the prototypes visible is defined.
    #
    # Returns 1 if the broken headers were detected, 0 otherwise.
    #
    # This should only be called if the compiler is g++ (which it
    # should be if we are here) and a flag was automatically appended
    # to CXXFLAGS.

    context.Message('Checking for broken GCC C headers when C++11 is enabled... ')
    ret = '-std=gnu++' in flag
    context.Result(ret)

    if ret:
        print('Warning: detected pre-C++11 GCC C header bugs.  See:')
        print('           https://gcc.gnu.org/bugzilla/show_bug.cgi?id=34032')
        print('         for related details.')

def _inform_user_of_broken_gcc_headers(context, flag):
    # Informative tests used to inform the user of broken GCC headers.
    # They are unnecessary for actual builds.
    if flag is not 1 and flag is not 0:
        # The flag is neither 1 nor 0, meaning it contains the
        # automatically detected C++11 flag.

        # Now verify that the compiler is actually GCC.
        is_gcc = factory.check_for_gcc_cxx(context)
        if is_gcc:
            # This should only be called if the compiler is g++ and a
            # flag was automatically appended to CXXFLAGS.
            #
            # We do not care if the user added a flag that triggers
            # the header bug.  It's the user's responsibility to
            # handle the issue in that case.
            _check_for_broken_gcc_headers(context, flag)

def check_c99_flags(context):
    """
    Check if command line flag is required to enable C99 support.

    Returns 1 if no flag is required, 0 if no flag was found, or the
    actual flag if one was found.
    """

    cc = context.env['CC']
    context.Message('Checking for C99 flag for ' + cc + '... ')
    config = factory.make_c_compiler_config(context)
    ret = config.check_c99_flags()
    context.Result(ret)

    return ret

def check_cxx11_flags(context):
    """
    Check if command line flag is required to enable C++11 support.

    Returns 1 if no flag is required, 0 if no flag was found, or the
    actual flag if one was found.
    """

    cxx = context.env['CXX']
    context.Message('Checking for C++11 flag for ' + cxx + '... ')
    config = factory.make_cxx_compiler_config(context)
    ret = config.check_cxx11_flags()
    context.Result(ret)

    # Let the user know if a workaround was enabled for broken GCC C
    # headers when C++11 is enabled.
    _inform_user_of_broken_gcc_headers(context, ret)

    return ret

def check_pthreads(context):
    """
    Check if pthreads are supported for this platform.

    Sets POSIX_SUPPORTED based on the result.
    """
    context.Message('Checking for POSIX Thread Support...')
    config = factory.make_c_compiler_config(context)

    ret = config.has_pthreads_support()
    context.env['POSIX_SUPPORTED'] = ret
    context.Result(ret)
    return ret

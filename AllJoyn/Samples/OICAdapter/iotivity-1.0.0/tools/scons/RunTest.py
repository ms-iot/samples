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

import os

def run_test(env, xml_file, test):
    """
    Run test with the given SCons Environment, dumping Valgrind
    results to the given XML file.  If no Valgrind run is desired
    simply pass in an empty string or None for the xml_file
    parameter.

    Note that the test path should not include the build directory
    where binaries are placed.  The build directory will be prepended
    to the test path automatically.
    """

    build_dir = env.get('BUILD_DIR')
    result_dir = os.path.join(build_dir, 'test_out/')
    if not os.path.isdir(result_dir):
        os.makedirs(result_dir)

    # Dump test report in XML format to the results directory.
    env.AppendENVPath('GTEST_OUTPUT', ['xml:' + result_dir])

    # Make sure the Google Test libraries are in the dynamic
    # linker/loader path.
    env.AppendENVPath('LD_LIBRARY_PATH', [build_dir])
    env.AppendENVPath('LD_LIBRARY_PATH', ['./extlibs/gtest/gtest-1.7.0/lib/.libs'])

    test_cmd = os.path.join(build_dir, test)

    if xml_file:
        # Environment variables to be made available during the
        # Valgrind run.
        valgrind_environment = ''

        # GLib uses a custom memory allocation scheme that can
        # sometimes confuse Valgrind.  Configure GLib to be Valgrind
        # friendly.
        valgrind_environment += 'G_DEBUG=gc-friendly G_SLICE=always-malloc'

        # Valgrind suppressions file.
        suppression_file = env.File('#tools/valgrind/iotivity.supp').srcnode().path

        # Set up to run the test under Valgrind.
        test_cmd = '%s valgrind --leak-check=full --suppressions=%s --xml=yes --xml-file=%s %s' % (valgrind_environment, suppression_file, xml_file, test_cmd)

    ut = env.Command('ut', None, test_cmd)
    env.AlwaysBuild('ut')

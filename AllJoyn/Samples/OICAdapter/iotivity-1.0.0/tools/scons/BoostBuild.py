# -*- coding: utf-8 -*-

# *********************************************************************
#
# Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
#
# *********************************************************************
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# *********************************************************************

# This builder executes the boost builder ('b2') for the toolchain
# defined currently in the SCONS environment. This builder was created
# to create cross-compiled version of boost. In particular, it has
# created to create boost binaries for Android's various architectures.

import os, subprocess
import SCons.Builder, SCons.Node, SCons.Errors

# Creates the building message
#
# @param s original message
# @param target target name
# @param source source name
# @param env environment object
def __message( s, target, source, env ) :
    print "building boost from [%s] for ..." % (source[0])


# Create the builder action which constructs a user-config.jam based
# on the current toolchain and executes the boost build system ('b2')
#
# @param target target file on the local drive
# @param source URL for download
# @@param env environment object
def __action( target, source, env ) :
    cmd = None

    # Windows...
    if env["PLATFORM"] in ["win32"] :
        if env.WhereIs("cmd") :
            # TODO: Add Windows Support
            cmd = None

    # read the tools on *nix systems and sets the default parameters
    elif env["PLATFORM"] in ["darwin", "linux", "posix"] :
        if env.WhereIs("sh") :
            cmd = ['./b2']

    if not cmd :
        raise SCons.Errors.StopError("Boost build system not supported on this platform [%s]" % (env["PLATFORM"]))

    # We need to be in the target's directory
    cwd = os.path.dirname(os.path.realpath(source[0].path))

    # Gather all of the path, bin and flags
    version     = env.get('VERSION','')
    target_os   = env['TARGET_OS']
    target_arch = env['TARGET_ARCH']
    tool_path   = os.path.dirname(env['CXX'])
    cxx_bin     = os.path.basename(env['CXX'])
    ar_bin      = os.path.basename(env['AR'])
    ranlib_bin  = os.path.basename(env['RANLIB'])
    ccflags     = list(env['CFLAGS'])
    cxxflags    = list(env['CXXFLAGS'])

    try:
        cxxflags.remove('-fno-rtti')
    except ValueError:
        pass
    try:
        cxxflags.remove('-fno-exceptions')
    except ValueError:
        pass

    # Write a user-config for this variant
    user_config_name = cwd+os.sep+'tools'+os.sep+'build'+os.sep+'src'+os.sep+'user-config.jam'
    user_config_file = open(user_config_name, 'w')
    user_config_file.write('import os ;\n')
    user_config_file.write('using gcc :')
    user_config_file.write(' '+version+' :')
    #user_config_file.write(' :')
    #user_config_file.write(' '+os.path.basename(toolchain['CXX']['BIN'])+' :\n')
    user_config_file.write(' '+cxx_bin+' :\n')
    user_config_file.write('    <archiver>'+ar_bin+'\n')
    user_config_file.write('    <ranlib>'+ranlib_bin+'\n')
    for value in env['CPPDEFINES'] :
        if len(value) > 1 :
            user_config_file.write('    <compileflags>-D'+value[0]+'='+value[1]+'\n')
        else :
            user_config_file.write('    <compileflags>-D'+value[0]+'\n')
    for value in env['CPPPATH'] :
        user_config_file.write('    <compileflags>-I'+value+'\n')
    for flag in ccflags :
        user_config_file.write('    <compileflags>'+flag+'\n')
    for flag in cxxflags :
        user_config_file.write('    <cxxflags>'+flag+'\n')
    user_config_file.write('    ;\n')
    user_config_file.close();

    # Ensure that the toolchain is in the PATH
    penv = os.environ.copy()
    penv["PATH"] = tool_path+":" + penv["PATH"]

    build_path = 'build' + os.sep + target_os + os.sep + target_arch

    cmd.append('-q')
    cmd.append('target-os=linux')
    cmd.append('link=static')
    cmd.append('threading=multi')
    cmd.append('--layout=system')
    cmd.append('--build-type=minimal')
    cmd.append('--prefix='+env['PREFIX'])
    cmd.append('--build-dir='+build_path)
    for module in env.get('MODULES',[]) :
        cmd.append('--with-'+module)
    cmd.append('headers')
    cmd.append('install')

    # build it now (we need the shell, because some programs need it)
    devnull = open(os.devnull, "wb")
    handle  = subprocess.Popen( cmd, env=penv, cwd=cwd ) #, stdout=devnull )

    if handle.wait() <> 0 :
        raise SCons.Errors.BuildError( "Building boost [%s] on the source [%s]" % (cmd, source[0])  )

# Define the emitter of the builder
#
# @param target target file on the local drive
# @param source
# @param env environment object
def __emitter( target, source, env ) :
    return target, source

# Generate function which adds the builder to the environment
#
# @param env environment object
def generate( env ) :
    env["BUILDERS"]["BoostBuild"] = SCons.Builder.Builder( action = __action,  emitter = __emitter,  target_factory = SCons.Node.FS.Entry,  source_factory = SCons.Node.FS.File,  single_source = True,  PRINT_CMD_LINE_FUNC = __message )

# Exist function of the builder
# @param env environment object
# @return true
def exists( env ) :
    return 1

# -*- coding: utf-8 -*-

############################################################################
# GPL License                                                              #
#                                                                          #
# This file is a SCons (http://www.scons.org/) builder                     #
# Copyright (c) 2012-14, Philipp Kraus, <philipp.kraus@flashpixx.de>       #
# Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.     #
# This program is free software: you can redistribute it and/or modify     #
# it under the terms of the GNU General Public License as                  #
# published by the Free Software Foundation, either version 3 of the       #
# License, or (at your option) any later version.                          #
#                                                                          #
# This program is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with this program. If not, see <http://www.gnu.org/licenses/>.     #
############################################################################

# the URLDownload-Builder can download any data from an URL into a target
# file. The target name is used are the file name of the downloaded file.

# This builder originated from work by Philipp Kraus and flashpixx project
# (see https://github.com/flashpixx). It has been modified to leverage
# the HTTP ETag header to be used as the csig. This allows the download
# builder to determine if the file should be downloaded again when the
# ETag header is supported

import os, time
import urllib2, urlparse
import SCons.Builder, SCons.Node, SCons.Errors

# Define a source node to represent the remote file. The construction of the
# node will query the hosting site to get the ETag, size and last-modified
# date.  This node also defines the method by which we will determine if
# the file should be downloaded again.
#
# This node derives from the Python.Value node
#
class URLNode(SCons.Node.Python.Value) :
    def make_ready(self) :
        try :
            stream = urllib2.urlopen( str(self.value) )
            info = stream.info()

            self.url_etag = None
            self.url_last_modified = None
            self.url_content_length = None

            if 'ETag' in info :
                self.url_etag = info['ETag']
            if 'Last-Modified' in info :
                self.url_last_modified = time.mktime(time.strptime(info['Last-Modified'], '%a, %d %b %Y %H:%M:%S GMT'))
            if 'Content-Length' in info :
                self.url_content_legth = info['Content-Length']
        except Exception, e :
            raise SCons.Errors.StopError( '%s [%s]' % (e, self.value) )

    def visited(self) :
        ninfo = self.get_ninfo()

        if self.url_etag :
            ninfo.csig = self.url_etag
        if self.url_last_modified :
            ninfo.timestamp = self.url_last_modified
        if self.url_content_length :
            ninfo.size = self.url_content_length
        SCons.Node.Node.visited(self);

    def changed_since_last_build(self, target, prev_ni):
        if prev_ni :
            if self.url_etag :
                if prev_ni.csig == self.url_etag :
                    # print 'Matched on ETag:'+prev_ni.csig
                    return False

            if not self.url_last_modified :
                # print 'Last modified date is not available'
                return True
            if not self.url_content_length :
                # print 'Content length is not available'
                return True
            if prev_ni.timestamp != self.url_last_modified :
                # print 'Modified since last build'
                return True
            if prev_ni.size != self.url_content_length :
                # print 'Content length has changed'
                return True

            return False

        # print 'Not previous built'
        return True

# Creates the output message
# @param s original message
# @param target target name
# @param source source name
# @param env environment object
def __message( s, target, source, env ) :
    print 'downloading [%s] from [%s] ...' % (target[0], source[0])

# Creates the action ie. the download function.
# This reads the data from the URL and writes it down to the file
# @param target target file on the local drive
# @param source URL for download
# @@param env environment object
def __action( target, source, env ) :
    try :
        source_name = str(source[0])
        target_name = str(target[0])
        stream = urllib2.urlopen(source_name)
        file = open( target_name, 'wb' )
        file.write(stream.read())
        file.close()

        # Change the access/modified time to match
        # the date on the downloaded file, if available
        ninfo = source[0].get_ninfo()
        if hasattr(ninfo, 'timestamp') :
            mtime = ninfo.timestamp
            if mtime :
                os.utime(target_name, (mtime, mtime))
    except Exception, e :
        raise SCons.Errors.StopError( '%s [%s]' % (e, source[0]) )


# Defines the emitter of the builder
# @param target target file on the local drive
# @param source URL for download
# @param env environment object
def __emitter( target, source, env ) :
    return target, source

# generate function, that adds the builder to the environment,
# @param env environment object
def generate( env ) :
    env['BUILDERS']['URLDownload'] = SCons.Builder.Builder( action = __action,  emitter = __emitter,  target_factory = SCons.Node.FS.File,  source_factory = URLNode,  single_source = True,  PRINT_CMD_LINE_FUNC = __message )

# existing function of the builder
# @param env environment object
# @return true
def exists(env) :
    return 1

/******************************************************************
*
* Copyright 2014 Samsung Electronics All Rights Reserved.
*
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************/


===============================================================================
==                How to Build Things Manager                                ==
===============================================================================

Once the source code is downloaded in your local specific folder, you may follow
the steps to build and execute Things Manager and its applications.
In this context, we assume that the code was downloaded into 'oic' folder.

=======================================
1. Download source code
=======================================

From the url, you can download Things Manager source code;
https://www.iotivity.org/downloads

Once you download the codes, and Make sure that the downloaded code structure is as follows;
Four directories for oic; extlib, resource, service, and tools.

oic/extlib
oic/resource
oic/service
oic/tools

The path for Things Manager is as following;

oic/service/things-manager

The things-manager directory includes following sub directories;

Directories                                         Description
oic/service/things-manager/sdk                   : The SDK APIs for applications is located.
                                                   The main functionality of this SDK is to provide
                                                   developer-friendly APIs of Things manager component
                                                   to application developers.
oic/service/things-manager/sampleapp             : It is the sample application on Ubuntu.
                                                   Basically, the input and output of application
                                                   on Ubuntu are displayed in the console.
oic/service/things-manager/build                 : Whole library files and binary files would be made
                                                   in this folder



=======================================
2. Build
=======================================
Simply, type "make" to build things manager as follows;

/oic/service/things-manager/build/linux$ make

=======================================
3. Build the API reference documentation
=======================================
To build the API reference documentation:
a.    Navigate to oic-resource/docs folder using the terminal window.
b.    Run the following command:

    $ doxygen

This command builds the API reference documentation in the output directory.

The output directory for this command is oic-resource/docs/html/index.html.


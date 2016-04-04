#ifndef OICMIDDLE_H
#define OICMIDDLE_H

//******************************************************************
//
// Copyright 2014 Intel Corporation.
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

#include <string>
#include <cstdlib>
#include "OCPlatform.h"
#include "OCApi.h"

class MiddleClient;
class MiddleServer;
class LineInput;
class RestInput;
class WrapResource;
class HueResources;

using namespace OC;
using namespace std;

enum AppType {
    AT_None = 0,
    AT_Server = 1,
    AT_Client = 2,
    AT_Both = 3
};

enum LineResult {
    LR_OK,
    LR_NoCommand,
    LR_NoClient,
    LR_NoResource,
    LR_Timeout,
    LR_Param,
    LR_Unrecognized,
    LR_Quit,
    LR_Syntax,
    LR_Error
};

class HueResource;

typedef int token_t;
typedef map<string, string> stringmap_t;

class Middle
{
public:
    Middle();
    ~Middle();
    void init();
    void run(int argc, char* argv[]);

protected:
    friend class MiddleClient;
    friend class MiddleServer;
    friend class RestInput;
    friend class HueResources;

    AppType m_appType;
    bool m_useLineInput;
    bool m_useRestInput;
    string m_hueAddr;
    MiddleClient *m_client;
    MiddleServer *m_server;
    LineInput *m_lineInput;
    RestInput *m_restInput;

protected:
    void startPlatform();
    bool parseCommandLineOptions(int argc, char *argv[]);
    void provideHelp();
};

extern Middle middle;

#endif // OICMIDDLE_H

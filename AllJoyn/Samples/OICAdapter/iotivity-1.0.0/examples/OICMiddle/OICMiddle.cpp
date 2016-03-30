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

//
// OICMiddle.cpp : OIC demo application for Minnowboard
//

#include <unistd.h>

#include "OICMiddle.h"
#include "WrapResource.h"
#include "Client.h"
#include "Server.h"
#include "LineInput.h"
#include "RestInput.h"

class Middle middle;    // one and only

Middle::Middle() :
    m_appType(AT_None),
    m_useLineInput(false),
    m_useRestInput(false),
    m_client(nullptr),
    m_server(nullptr),
    m_lineInput(nullptr),
    m_restInput(nullptr)
{
}

Middle::~Middle()
{
    delete m_client;
    delete m_server;
    delete m_lineInput;
    delete m_restInput;
}

void Middle::init()
{

}

void Middle::run(int argc, char* argv[])
{
    parseCommandLineOptions(argc, argv);

    startPlatform();

    if (m_appType & AT_Client) {
        m_client = new MiddleClient();
        m_client->init();
    }

    m_lineInput = new LineInput(m_client);

    if (m_appType & AT_Server) {
        m_server = new MiddleServer();
        m_server->init();
    }
    if (m_useRestInput) {
        if (!m_server) {
            m_server = new MiddleServer();
            m_server->init();
        }
        m_restInput = new RestInput(m_lineInput);
        m_restInput->init();
    }
    if (m_useLineInput) {
        if (m_server) {
            m_lineInput->setServer(m_server);
        }
        m_lineInput->run();
    } else {
        while (true)
            sleep(1);
    }
}

void Middle::startPlatform()
{
    uint16_t port = 0;
    //std::string ipaddr = INADDR_ANY;
    std::string ipaddr = "0.0.0.0";

    PlatformConfig cfg { ServiceType::InProc, ModeType::Both,
                  ipaddr, port, QualityOfService::LowQos};

    OC::OCPlatform::Configure(cfg);
}

void Middle::provideHelp()
{
    static const char usage[] = "\nUsage:  IOCMiddle args\n"
                "    where args may include any of these:\n"
                "\t-client      Run OIC client\n"
                "\t-server      Run OIC server\n"
                "\t-both        Run OIC client and server\n"
                "\t-console     Run console line interpreter\n"
                "\t-rest        Run ReST server\n"
                "\t-hue addr    Enable Hue resources on bridge at addr\n"
                "\t-help        Show Usage again\n"
                "Any combination of the above is okay.\n\n";
    cout << usage;
}

bool Middle::parseCommandLineOptions(int argc, char *argv[])
{
    bool any = false;

    for (int i = 1; i < argc; i++) {
        if (argv[i] == string("-server")) {
            middle.m_appType = AT_Server; any = true;
        } else if (argv[i] == string("-client")) {
            middle.m_appType = AT_Client; any = true;
        } else if (argv[i] == string("-both")) {
            middle.m_appType = AT_Both; any = true;
        } else if (argv[i] == string("-console")) {
            middle.m_useLineInput = true; any = true;
        } else if (argv[i] == string("-rest")) {
            middle.m_useRestInput = true; any = true;
        } else if (argv[i] == string("-hue")) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                m_hueAddr = argv[++i];
                any = true;
            }
        } else if (argv[i] == string("-help")) {
                any = false;
            break;
        } else {
            std::cerr << "Not enough or invalid arguments, please try again.\n";
            exit(1);
        }
    }
    if (!any)
            provideHelp();
    return true;
}

int main(int argc, char* argv[])
{
    middle.run(argc, argv);
    return 0;
}

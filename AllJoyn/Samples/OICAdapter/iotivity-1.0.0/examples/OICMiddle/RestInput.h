#ifndef RESTINPUT_H
#define RESTINPUT_H

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

#include <netinet/in.h>

class LineInput;
class Connection;

class RestInput
{
    static const int MAX_CONNS = 5;
public:
    RestInput(LineInput *lineInput);
    ~RestInput();
    bool init();
    void startAccept(int &sockfd);
    void startThread();
    void processClient(void);
    void handleRead(std::string & restCmd);
    void handleShow(stringstream &ss, observecb_t &cb);
    void parseString(std::string &toParse);

protected:
    LineInput *m_lineInput;
    int m_sockfd, m_port, m_threadCount;
    struct sockaddr_in m_serverAddr;
    char *m_data;
    std::thread m_thread[MAX_CONNS];
};

#endif // RESTINPUT_H


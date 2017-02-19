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

#include "WrapResource.h"
#include "RestInput.h"
#include "LineInput.h"
#include "OICMiddle.h"

using namespace std;

#define BUFLEN 10000

static bool enableDebug = false; // set to true to print debug messages

void printDebugMessage(std::string message)
{
    if (enableDebug) {
        cout << "RestInput: " << message  << endl;
    }
}

RestInput::RestInput(LineInput *lineInput) : m_lineInput(lineInput)
{
    m_data = (char*)malloc(BUFLEN);
    m_threadCount = 0;
}

RestInput::~RestInput()
{
    free(m_data);
    close(m_sockfd);
}

bool RestInput::init()
{
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0) {
        cerr << "Failed to open socket. Exiting" << endl;
        return false;
    }
    m_port = 1441; //listening on port 1441

    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_serverAddr.sin_port = htons(m_port);

    if (::bind(m_sockfd, (struct sockaddr*)&m_serverAddr, sizeof m_serverAddr) < 0) {
        cerr << "Failed to bind. Exiting " << endl;
        return false;
    }

    listen(m_sockfd, MAX_CONNS);
    startAccept(m_sockfd);
    return true;
}

// accept incoming connection(s)
void RestInput::startAccept(int &sockfd)
{
    if (m_threadCount >= MAX_CONNS) {
        cerr << " Max # of connections reached. Skipping " << endl;
        return;
    } else {
        while (true) {
            int connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);
            if (connfd < 0) {
                cerr << " Failed to accept incoming connection " << endl;
                return;
            }
            int n = read(connfd, m_data, BUFLEN);
            close(connfd);
            if (n < 0) {
                cerr << "Failed to read from socket" << endl;
                return;
            }
            startThread();
        }
    }
}

// start client thread
void RestInput::startThread()
{
    std::thread t(&RestInput::processClient, this);
    m_thread[m_threadCount] = std::move(t);
    m_thread[m_threadCount++].join();
}

// process read commands for the client
void RestInput::processClient(void)
{
    std::string restCmd = m_data;
    std::size_t found = restCmd.find('\n');
    if (found != std::string::npos) {
        restCmd = restCmd.substr(0, found-1);
    }
    handleRead(restCmd);
}

void RestInput::handleRead(std::string& restCmd)
{
    parseString(restCmd);
    if (restCmd.find("exit") == 0) {
        std::thread::id id = std::this_thread::get_id();
        for(int i = 0; i < m_threadCount; ++i) {
            if (id == m_thread[i].get_id()) {
                m_thread[i].detach();
                --m_threadCount;
                cout << "Exiting thread " << id << endl;
            }
        }
        return;
    }
    stringstream ss;
    observecb_t cb;
    std::string msg = "command sent to LineInput is: " + restCmd;
    printDebugMessage(msg);
    m_lineInput->processLine(restCmd, ss, cb);
    if (restCmd.find("show") != string::npos) {
        // if command is show, we want to list out the details of each resource
        handleShow(ss, cb);
    }
}

void RestInput::handleShow(stringstream &ss, observecb_t &cb) {
    std::string temp = ss.str();
    size_t n = std::count(temp.begin(), temp.end(), '\n'); // number of resources found
    std::stringstream sstm;
    std::string lineInputData;

    for (size_t i = 0; i < n; ++i) {
        sstm.str("");
        sstm << "details " << i;
        lineInputData = sstm.str();
        std::string msg = "Details: " + lineInputData;
        printDebugMessage(msg);
        m_lineInput->processLine(lineInputData, ss, cb);
        sstm.str("");
        sstm << "get " << i;
        lineInputData = sstm.str();
        msg = "Get: " + lineInputData;
        printDebugMessage(msg);
        m_lineInput->processLine(lineInputData, ss, cb);
    }
}

void RestInput::parseString(std::string &toParse)
{
    std::size_t pos = toParse.find("HTTP"); // split on HTTP
    toParse = toParse.substr(0, pos);
    pos = toParse.find("/"); // find 1st occurance of /
    toParse = toParse.substr(pos + 1, toParse.size() - 1);
    std::replace(toParse.begin(), toParse.end(), '/', ' '); // replace all '/' with ' '
}

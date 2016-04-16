#ifndef LINEINPUT_H
#define LINEINPUT_H

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

#include "OICMiddle.h"
#include "Client.h"
#include "Server.h"

typedef vector<string> elements_t;

enum ParseState {
    PS_None,
    PS_Between,
    PS_Infirst,
    PS_Startsecond,
    PS_Insecond,
    PS_Endsecond,
};

class LineInput
{
public:
    LineInput(MiddleClient *client);
    void setServer(MiddleServer *server);
    int run();
    LineResult processLine(string command, stringstream& result, observecb_t cb);

protected:
    MiddleClient *m_client;
    MiddleServer *m_server;
    vector<string> m_resourceList;
    observecb_t m_obsCB;
    WrapResource *m_observer;
    char m_elem[1000];

    LineResult processHelp(elements_t& elems, stringstream& ss);
    LineResult processUnrecognized(elements_t& elems, stringstream& ss);
    LineResult processFind(elements_t& elems, stringstream& ss);
    LineResult processShow(elements_t& elems, stringstream& ss);
    LineResult processDetails(elements_t& elems, stringstream& ss);
    LineResult processGet(elements_t& elems, stringstream& ss);
    LineResult processPut(elements_t& elems, stringstream& ss);
    LineResult processObserve(elements_t& elems, stringstream& ss, observecb_t cb);
    LineResult processCancel(elements_t& elems, stringstream& ss);
    WrapResource *resolveResource(string resID, stringstream& ss);
    LineResult parseLine(string lineIn, elements_t& elems);
    ParseState finishElem(char*& e, elements_t& elems);
    ParseState putCharInElem(char c, char *& e, ParseState newState);
    void obsCB(const token_t token, const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode, const int sequenceNumber);
    void registerResourceWithServer(std::string &url);
};

#endif // LINEINPUT_H

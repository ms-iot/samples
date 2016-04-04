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

#include <map>
#include <iostream>

#include <stdio.h>

#include "WrapResource.h"
#include "LineInput.h"

#define NEED_CLIENT { if (!m_client) return LR_NoClient; }

LineInput::LineInput(MiddleClient *client)
    : m_client(client), m_server(nullptr),
      m_obsCB(nullptr), m_observer(nullptr)
{
    m_obsCB = std::bind(&LineInput::obsCB, this,
                        placeholders::_1,
                        placeholders::_2,
                        placeholders::_3,
                        placeholders::_4,
                        placeholders::_5);
}

void LineInput::setServer(MiddleServer *server) {
    m_server = server;
}

int LineInput::run()
{
    size_t len;
    char *line = nullptr;

    while (true) {
        fputs(">", stdout);
        len = 0;
        const ssize_t n = getline(&line, &len, stdin);
        if (n <= 0)
            continue;
        if (m_observer) {
            m_observer->cancelObserve();
            m_observer = nullptr;
        }
        if (line[n - 1] == '\n') {
            if (n == 1)
                continue;
            line[n - 1] = '\0';
        }
        stringstream result;
        LineResult lr = processLine(line, result, m_obsCB);
        if (lr == LR_Quit)
            break;
        cout << result.str();
    }
    free(line);
    return true;
}

LineResult LineInput::processLine(string command, stringstream& result, observecb_t cb)
{
    elements_t elems;

    if (parseLine(command, elems) != LR_OK) {
        cerr << "syntax error" << endl;
        return LR_Syntax;
    }
    if (!elems.size())
        return LR_NoCommand;

    if (elems[0] == "quit" || elems[0] == "exit")
        return LR_Quit;

    if (elems.size() == 1) {
        if (elems[0] == "help") {
            return processHelp(elems, result);
        } else if (elems[0] == "find") {
            NEED_CLIENT return processFind(elems, result);
        } else if (elems[0] == "show") {
            NEED_CLIENT return processShow(elems, result);
        }
    } else if (elems.size() == 2) {
        if (elems[0] == "details") {
            NEED_CLIENT return processDetails(elems, result);
        } else if (elems[0] == "get") {
            NEED_CLIENT return processGet(elems, result);
        } else if (elems[0] == "observe") {
            NEED_CLIENT return processObserve(elems, result, cb);
        } else if (elems[0] == "cancel") {
            NEED_CLIENT return processCancel(elems, result);
        }
    } else {
        if (elems[0] == "put") {
            NEED_CLIENT return processPut(elems, result);
        }
    }

    return processUnrecognized(elems, result);
}

LineResult LineInput::processHelp(elements_t& /*elems*/, stringstream& ss)
{
    ss << "\nUsage:\n"
                "\tfind\t\tFind resources\n"
                "\tshow\t\tShow resources\n"
                "\tdetails n\tShow details of resource n\n"
                "\tget n\t\tGet value(s) of resource n\n"
                "\tput n v\t\tPut value(s) to resource n\n"
                "\tobserve n\tObserve value(s) of resource n\n"
                "\thelp\t\tThis usage message\n"
                "\nResource can be identified by Resource ID or Show index\n"
                "\nValue in 'put' can be key=value or key:value\n\n"
                ;
    return LR_OK;
}

LineResult LineInput::processUnrecognized(elements_t& elems, stringstream& ss)
{
    ss << "Command not recognized\n";
    processHelp(elems, ss);
    return LR_Unrecognized;
}

LineResult LineInput::processFind(elements_t& /*elems*/, stringstream& /*ss*/)
{
    m_client->findResources();
    return LR_OK;
}

void LineInput::registerResourceWithServer(std::string & url) {
    string type;
    std::size_t index = url.rfind("/");
    if (index != std::string::npos) {
        type = url.substr(index+1);
    }
    const std::string resType = type;
    const std::string iface = "MB_INTERFACE";
    m_server->registerResource(url, resType, iface);
}

LineResult LineInput::processShow(elements_t& /*elems*/, stringstream& ss)
{
    int index = 0;
    m_resourceList.clear();
    resourcemap_t& pmap = m_client->m_resourceMap;

    for (resourcemap_t::iterator it = pmap.begin(); it != pmap.end(); it++) {
        string resID = it->first;
        ss << index++ << '\t' << resID << '\n';
        m_resourceList.push_back(resID);
        if (m_server) {
            registerResourceWithServer(resID);
        }
    }

    return LR_OK;
}

LineResult LineInput::processDetails(elements_t& elems, stringstream& ss)
{
    WrapResource *wres = resolveResource(elems[1], ss);
    if (!wres)
        return LR_NoResource;

    ss << wres->getResourceID() + " [ ";
    for (auto &types : wres->getResourceTypes()) {
        ss << types + ' ';
    }
    ss << "] ";
    for (auto &ifs : wres->getResourceInterfaces()) {
        ss << ifs << " ";
    }
    ss << '\n';
    return LR_OK;
}

void printJSONAsTable(std::string &jsonString) {
    std::string str = jsonString;
    std::string key, value;
    size_t found = str.find("rep");
    if (found == std::string::npos) { // not found
        return;
    }
    str = str.substr(found+5);
    while (true) {
        found = str.find(":");
        if (found == std::string::npos) {
            return;
        }
        key = str.substr(1, found-1);
        str = str.substr(found);
        found = str.find(",");
        if (found != std::string::npos) {
            value = str.substr(1, found-1);
            str = str.substr(found);
        } else {
            found = str.find("}");
            if (found != std::string::npos) {
                value = str.substr(1, found-1);
                str = str.substr(found);
            }
        }
        cout << key << "\t:" << value << endl;
    }
}

LineResult LineInput::processGet(elements_t& elems, stringstream& ss)
{
    WrapResource *wres = resolveResource(elems[1], ss);
    if (!wres)
        return LR_NoResource;

    token_t token = wres->getResource();

    WrapRequest *wreq = wres->waitResource(token);
    if (!wreq) {
        ss << "Get timed out\n";
        return LR_Timeout;
    }

    std::string jsonRep ;//= wreq->m_rep.getJSONRepresentation();
    //ss << jsonRep << endl;
    printJSONAsTable(jsonRep);
        return LR_OK;
}

LineResult LineInput::processPut(elements_t& elems, stringstream& ss)
{
    WrapResource *wres = resolveResource(elems[1], ss);
    if (!wres)
        return LR_NoResource;

    string format;
    OCRepresentation rep;

    bool error = false;
    for (size_t i = 2; i < elems.size(); i++) {
        string elem = elems[i];
        char *s = (char *)elem.c_str();    // elem string is intentionally damaged
        char *key = strtok(s, "=:");
        char *value = strtok(nullptr, "");
        if (!value) {
            ss << "missing separator in element starting with " << key << '\n';
            error = true;
            continue;
        }
        char delim = value[0];
        size_t len = strlen(value);
        if (delim == '\'' || delim == '"') {
            if (len > 1 && delim == value[len - 1]) {
                value[len - 1] = '\0';
                value++;
            }
        }
        string v(value, len);
        stringmap_t formats = wres->getFormats();
        try {
            format = formats.at(key);
        } catch (...) {
            cerr << "element in arg " << i << " has no format\n";
            continue;
        }
        if (format == "bool") {
            bool b = v != "0" && v != "false";
            rep.setValue(key, b);
        } else if (format == "number") {
            char *end;
            int n = (int)strtol(value, &end, 10);
            if (size_t(end - value) != len) {
                double d = atof(value);
                rep.setValue(key, d);
            } else {
                rep.setValue(key, n);
            }
        } else {    // assume string
            rep.setValue(key, v);
        }
    }
    if (error)
        return LR_Param;

    token_t token = wres->putResource(rep);

    WrapRequest *wreq = wres->waitResource(token);
    if (!wreq) {
        ss << "Get timed out\n";
        return LR_Timeout;
    }

    return LR_OK;
}

LineResult LineInput::processObserve(elements_t& elems, stringstream& ss, observecb_t cb)
{
    WrapResource *wres = resolveResource(elems[1], ss);
    if (!wres)
        return LR_NoResource;
    m_observer = wres;
    wres->observeResource(cb);
    return LR_OK;
}

LineResult LineInput::processCancel(elements_t& elems, stringstream& ss)
{
    WrapResource *wres = resolveResource(elems[1], ss);
    if (!wres)
        return LR_NoResource;

    wres->cancelObserve();
    m_observer = nullptr;
    return LR_OK;
}

WrapResource *LineInput::resolveResource(string resID, stringstream& /*ss*/)
{
    size_t len;
    string useID = resID;
    int index = std::stoi(useID, &len);

    if (len == resID.size()) {            // it's an index, not a uri
        if (size_t(index) >= m_resourceList.size()) {
            cout << "Resource index out of range (use 'show')\n";
            return nullptr;
        }
        useID = m_resourceList[index];  // now it's a uri
    }

    resourcemap_t::iterator it = m_client->m_resourceMap.find(useID);
    if (it == m_client->m_resourceMap.end()) {
        cout << resID << " is currently not available\n";
        return nullptr;
    }

    return it->second;
}

void LineInput::obsCB(token_t /*token*/,
                      const HeaderOptions& /*headerOptions*/,
                      const OCRepresentation& /*rep*/,
                      const int eCode,
                      const int sequenceNumber)
{
    if (!m_observer)
        return;
    cout << "cb " << eCode << " " << sequenceNumber << '\n';
    //cout << rep.getJSONRepresentation() << "\n";
}

ParseState LineInput::finishElem(char*& e, elements_t& elems)
{
    *e = '\0';
    elems.push_back(m_elem);
    e = m_elem;
    return PS_Between;
}

ParseState LineInput::putCharInElem(char c, char *& e, ParseState newState)
{
    *e++ = c;
    if (size_t(e - m_elem) >= sizeof (m_elem))
        throw 20;    // hightly unlikely exception
    return newState;
}

/*
 *     See processHelp() above for line format
 */
LineResult LineInput::parseLine(string lineIn, elements_t& elems)
{
    const char *d;
    char c, *e, delim = 0;
    bool isSep1, isSep2;
    size_t len = lineIn.size();
    ParseState state = PS_Between;
    const char *line = lineIn.c_str();

    d = line;
    e = m_elem;
    while (true) {
        if (size_t(d - line) >= len) {
            if (e != m_elem) {
                if (state == PS_Infirst || state == PS_Endsecond || (state == PS_Insecond && !delim)) {
                    state = finishElem(e, elems);
                    return LR_OK;
                }
            }
            return LR_Syntax;
        }
        c = *d++;
        if (c == '\n')
            continue;
        isSep1 = c == ' ' || c == '\t';
        isSep2 = c == '=' || c == ':';

        switch (state) {
        case PS_Between:
            if (isSep1)
                continue;
            if (isSep2)
                return LR_Syntax;
            state = putCharInElem(c, e, PS_Infirst);
            break;
        case PS_Infirst:
            if (isSep1) {
                state = finishElem(e, elems);
                continue;
            }
            if (isSep2) {
                delim = 0;
                state = PS_Startsecond;
            }
            putCharInElem(c, e, state);
            break;
        case PS_Startsecond:
            if (isSep1 || isSep2)
                return LR_Syntax;
            if (c == '\'' || c == '"' || c == '|')
                delim = c;
            state = putCharInElem(c, e, PS_Insecond);
            break;
        case PS_Insecond:
            if (isSep1 && delim == 0) {
                state = finishElem(e, elems);
                continue;
            }
            if (c == delim) {
                state = PS_Endsecond;
            }
            *e++ = c;
            break;
        case PS_Endsecond:
            if (isSep1) {
                state = finishElem(e, elems);
                continue;
            }
            return LR_Syntax;
        case PS_None:
            return LR_Syntax;
        }
    }
    return LR_OK;
}



#ifndef WRAPRESOURCE_H
#define WRAPRESOURCE_H

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
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

#include "OICMiddle.h"

using namespace OC;
using namespace std;

class WrapRequest;
class MiddleClient;

enum RequestType {
    RT_Get,
    RT_Put,
    RT_Observe,
};

typedef std::shared_ptr<OCResource> ocresource_t;
typedef std::map<token_t, WrapRequest *> requestmap_t;
typedef std::function<void(const token_t token, const HeaderOptions&, const OCRepresentation&, const int, const int)> observecb_t;

class WrapResource
{
public:
    WrapResource(string resourceID, ocresource_t resource);

    token_t getResource();
    token_t putResource(OCRepresentation& rep);
    token_t observeResource(observecb_t& callback);
    string getResourceID();
    bool cancelObserve();
    std::vector<std::string> getResourceTypes();
    std::vector<std::string> getResourceInterfaces();
    WrapRequest *waitResource(token_t token);
    const stringmap_t& getFormats();

    friend class WrapRequest;
    friend class MiddleClient;

protected:
    WrapRequest *newRequest(RequestType type);
    void resourceCallback(WrapRequest *wreq);
    void parseJSON(WrapRequest *wreq);
    void findTypes();

    string m_resourceID;
    ocresource_t m_ocResource;
    int m_listIndex;
    int m_x;
    bool m_repGetReady;
    bool m_gettingRep;
    mutex m_mutexMap;
    mutex m_mutexGet;
    observecb_t m_observeCB;
    bool m_callbackRunning;
    int m_requestToken;
    requestmap_t m_requestMap;
    WrapRequest *m_observeRequest;        // can only be one
    stringmap_t m_typeMap;
    vector<WrapRequest *> m_typeResults;
    WrapRequest *m_typeRequest;
};

struct WrapRequest
{
    WrapRequest(WrapResource *wres, RequestType type, token_t token);

    friend class WrapResource;

    HeaderOptions m_headerOptions;
    OCRepresentation m_rep;
    int m_eCode;
    int m_sequenceNumber;
    stringmap_t m_valueMap;
    unsigned long m_touchTime;

protected:
    WrapResource *m_parent;
    RequestType m_type;
    token_t m_token;
    condition_variable m_cvGet;
    bool m_forTypeOnly;
    bool m_typeReady;

    void getCB(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode);
    void putCB(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode);
    void observeCB(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode, const int sequenceNumber);

    GetCallback m_getCB;
    PutCallback m_putCB;
    ObserveCallback m_obsCB;
};

#endif // WRAPRESOURCE_H

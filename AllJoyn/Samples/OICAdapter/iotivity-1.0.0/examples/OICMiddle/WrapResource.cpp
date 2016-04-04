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

#include <chrono>
#include <sys/time.h>

#include "WrapResource.h"

unsigned long GetTickCount()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
        return 0;
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

WrapResource::WrapResource(string resourceID, ocresource_t resource)
        : m_resourceID(resourceID), m_ocResource(resource),
         m_listIndex(-1), m_x(0), m_repGetReady(false), m_gettingRep(false),
         m_observeCB(nullptr), m_callbackRunning(false),
         m_requestToken(0), m_observeRequest(nullptr),
     m_typeRequest(nullptr)
{
}

string WrapResource::getResourceID() {
    return m_resourceID;
}

token_t WrapResource::getResource()
{
    WrapRequest *wreq;
    QueryParamsMap m;

    wreq = newRequest(RT_Get);
    m_ocResource->get(m, wreq->m_getCB, QualityOfService::HighQos);
    return wreq->m_token;
}

token_t WrapResource::putResource(OCRepresentation& rep)
{
    WrapRequest *wreq;
    QueryParamsMap m;

    wreq = newRequest(RT_Put);
    rep.setUri(m_ocResource->uri());
    m_ocResource->put(rep, m, wreq->m_putCB, QualityOfService::HighQos);
    return wreq->m_token;
}

token_t WrapResource::observeResource(observecb_t& cb)
{
    WrapRequest *wreq;
    QueryParamsMap m;
    ObserveType type;

    wreq = newRequest(RT_Observe);
    m_observeRequest = wreq;
    m_observeCB = cb;
    m_callbackRunning = true;
    type = ObserveType::Observe;
    m_ocResource->observe(type, m, wreq->m_obsCB);
    return wreq->m_token;
}

bool WrapResource::cancelObserve()
{
    m_callbackRunning = false;
    m_observeCB = nullptr;

    if (!m_observeRequest)
        return false;

    OCStackResult result = m_ocResource->cancelObserve();
    if (result != OC_STACK_OK)
        return false;

    m_observeRequest->m_touchTime = GetTickCount();
    return true;
}

WrapRequest *WrapResource::waitResource(token_t token)
{
    WrapRequest *wreq;
    cv_status st;

    try {
        m_mutexMap.lock();
        wreq = m_requestMap.at(token);
        m_mutexMap.unlock();
    } catch (const out_of_range& oor) {
        m_mutexMap.unlock();
        return nullptr;
    }

    std::unique_lock<std::mutex> lk(m_mutexGet);
    st = wreq->m_cvGet.wait_for(lk, chrono::seconds(5));
    return (st == cv_status::no_timeout) ? wreq : nullptr;
}

std::vector<std::string> WrapResource::getResourceTypes()
{
    return m_ocResource->getResourceTypes();
}

std::vector<std::string> WrapResource::getResourceInterfaces()
{
    return m_ocResource->getResourceInterfaces();
}

WrapRequest *WrapResource::newRequest(RequestType type)
{
    WrapRequest *wreq = new WrapRequest(this, type, ++m_requestToken);
    m_requestMap[m_requestToken] = wreq;
    return wreq;
}

void WrapResource::resourceCallback(WrapRequest *wreq)
{
    parseJSON(wreq);

    if (wreq->m_forTypeOnly) {
        wreq->m_typeReady = true;
        return;
    }

    if (wreq->m_type == RT_Observe) {
        if (!m_observeCB) {
            if (m_callbackRunning)
                cout << "callback missing " << m_resourceID << '\n';
            return;
        }
        m_observeCB(wreq->m_token, wreq->m_headerOptions, wreq->m_rep, wreq->m_eCode,
                                    wreq->m_sequenceNumber);
    } else {
        wreq->m_cvGet.notify_one();
    }

    wreq->m_touchTime = GetTickCount();
}

/*
 *  this parser infers types from json string since no other introspection
 *  is available.  It also parses the key-value pairs.
 */
void WrapResource::parseJSON(WrapRequest *wreq)
{
    string sep = "\":";
    string anchor = "\"rep\":{";
    string json;// = wreq->m_rep.getJSONRepresentation();
    string name, type, value, next;
    size_t r, e, e1, s, c;

    r = json.find(anchor);
    if (r == string::npos) {
        return;
    }
    c = r + anchor.length() - 1;
    do {
        c++;
        if (json[c] != '"') {
            if (json[c] == '}')
                break;
            return;
        }
        c++;
        e = json.find(sep, c);
        if (e == string::npos) {
            return;
        }
        name = json.substr(c, e - c);
        s = e + sep.length();
        char q = json[s];
        switch (q) {
        case 't':
        case 'f':
            type = "bool";
            e1 = json.find_first_of(",}", s + 1);
            if (e1 == string::npos) {
                return;
            }
            value = json.substr(s, e1 - s);
            break;
        case '"':
            type = "string";
            s++;
            e1 = json.find_first_of("\"", s);
            if (e1 == string::npos) {
                return;
            }
            value = json.substr(s, e1 - s);
            e1++;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
            type = "number";
            e1 = json.find_first_of(",}", s + 1);
            if (e1 == string::npos) {
                return;
            }
            value = json.substr(s, e1 - s);
            break;
        default:
            return;
        }
        wreq->m_valueMap[name] = value; // key-value map
        m_typeMap[name] = type;         // key-type map
        c = e1;
    } while (json[c] == ',');
}

void WrapResource::findTypes()
{
    delete m_typeRequest;
    m_typeRequest = new WrapRequest(this, RT_Get, ++m_requestToken);
    m_typeRequest->m_forTypeOnly = true;
    getResource();
}

const stringmap_t& WrapResource::getFormats()
{
    return m_typeMap;
}

/********** WrapRequest ***********/

WrapRequest::WrapRequest(WrapResource *wres, RequestType type, token_t token)
            : m_eCode(0), m_sequenceNumber(0), m_parent(wres), m_type(type),
              m_token(token), m_forTypeOnly(false), m_typeReady(false)
{
    m_getCB = std::bind(&WrapRequest::getCB, this,
        placeholders::_1, placeholders::_2, placeholders::_3);
    m_putCB = std::bind(&WrapRequest::putCB, this,
        placeholders::_1, placeholders::_2, placeholders::_3);
    m_obsCB = std::bind(&WrapRequest::observeCB, this,
        placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4);
    m_touchTime = GetTickCount();
}

void WrapRequest::getCB(const HeaderOptions& headerOptions, const OCRepresentation& rep, int eCode)
{
    m_headerOptions = headerOptions;
    m_rep = rep;
    m_eCode = eCode;
    m_parent->resourceCallback(this);
}

void WrapRequest::putCB(const HeaderOptions& headerOptions, const OCRepresentation& rep, int eCode)
{
    m_headerOptions = headerOptions;
    m_rep = rep;
    m_eCode = eCode;
    m_parent->resourceCallback(this);
}

void WrapRequest::observeCB(const HeaderOptions& headerOptions, const OCRepresentation& rep, int eCode, int sequenceNumber)
{
    m_headerOptions = headerOptions;
    m_rep = rep;
    m_eCode = eCode;
    m_sequenceNumber = sequenceNumber;
    m_parent->resourceCallback(this);
}

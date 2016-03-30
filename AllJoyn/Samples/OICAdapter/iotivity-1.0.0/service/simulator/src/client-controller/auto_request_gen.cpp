/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
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

#include "auto_request_gen.h"

AutoRequestGeneration::AutoRequestGeneration(RequestType type, int id,
        RequestSenderSP &requestSender, ProgressStateCallback callback)
    :   m_type(type),
        m_id(id),
        m_requestSender(requestSender),
        m_callback(callback),
        m_requestsSent(false),
        m_requestCnt(0),
        m_responseCnt(0) {}

void AutoRequestGeneration::start()
{
    startSending();
}

void AutoRequestGeneration::stop()
{
    stopSending();
}

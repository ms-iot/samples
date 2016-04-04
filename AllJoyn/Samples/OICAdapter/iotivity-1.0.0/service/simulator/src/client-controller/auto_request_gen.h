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

#ifndef AUTO_REQUEST_GEN_H_
#define AUTO_REQUEST_GEN_H_

#include "request_sender.h"

class AutoRequestGeneration
{
    public:
        typedef std::function<void (int, OperationState)> ProgressStateCallback;

        AutoRequestGeneration(RequestType type, int id,
                              RequestSenderSP &requestSender, ProgressStateCallback callback);
        RequestType type() const { return m_type;}
        int id() const {return m_id;}
        void start();
        void stop();

    protected:
        virtual void startSending() = 0;
        virtual void stopSending() = 0;

        RequestType m_type;
        int m_id;
        RequestSenderSP m_requestSender;
        ProgressStateCallback m_callback;
        bool m_requestsSent;
        int m_requestCnt;
        int m_responseCnt;
};

#endif

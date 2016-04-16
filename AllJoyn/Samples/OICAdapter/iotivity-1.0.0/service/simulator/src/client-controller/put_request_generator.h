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

#ifndef PUT_REQUEST_GEN_H_
#define PUT_REQUEST_GEN_H_

#include "auto_request_gen.h"
#include "query_param_generator.h"
#include "attribute_generator.h"

class PUTRequestGenerator : public AutoRequestGeneration
{
    public:
        PUTRequestGenerator(int id, RequestSenderSP &requestSender,
                            SimulatorResourceModelSP &representation,
                            ProgressStateCallback callback);

        PUTRequestGenerator(int id, RequestSenderSP &requestSender,
                            const std::map<std::string, std::vector<std::string>> &queryParams,
                            SimulatorResourceModelSP &representation,
                            ProgressStateCallback callback);

        void startSending();
        void stopSending();

    private:
        void SendAllRequests();
        void onResponseReceived(SimulatorResult result, SimulatorResourceModelSP repModel);
        void completed();

        QPGenerator m_queryParamGen;
        SimulatorResourceModelSP m_rep;
        std::mutex m_statusLock;
        bool m_status;
        bool m_stopRequested;
        std::shared_ptr<std::thread> m_thread;
};

typedef std::shared_ptr<PUTRequestGenerator> PUTRequestGeneratorSP;

#endif



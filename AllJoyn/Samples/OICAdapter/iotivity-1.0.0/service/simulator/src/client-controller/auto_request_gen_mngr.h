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

/**
 * @file auto_request_gen_mngr.h
 *
 * @brief This file provides class for managing auto request generation sessions.
 *
 */

#ifndef AUTO_REQUEST_GEN_MNGR_H_
#define AUTO_REQUEST_GEN_MNGR_H_

#include "auto_request_gen.h"

class AutoRequestGenMngr
{
    public:
        AutoRequestGenMngr() : m_id(0) {};

        int startOnGET(RequestSenderSP requestSender,
                       const std::map<std::string, std::vector<std::string>> &queryParams,
                       AutoRequestGeneration::ProgressStateCallback callback);

        int startOnPUT(RequestSenderSP requestSender,
                       const std::map<std::string, std::vector<std::string>> &queryParams,
                       SimulatorResourceModelSP resModel,
                       AutoRequestGeneration::ProgressStateCallback callback);

        int startOnPOST(RequestSenderSP requestSender,
                        const std::map<std::string, std::vector<std::string>> &queryParams,
                        SimulatorResourceModelSP resModel,
                        AutoRequestGeneration::ProgressStateCallback callback);

        void stop(int id);

    private:
        void onProgressChange(int sessionId, OperationState state,
                              AutoRequestGeneration::ProgressStateCallback clientCallback);
        bool isValid(int id);
        bool isInProgress(RequestType type);
        void remove(int id);

        std::mutex m_lock;
        std::map<int, std::shared_ptr<AutoRequestGeneration>> m_requestGenList;
        int m_id;
};

typedef std::shared_ptr<AutoRequestGenMngr> AutoRequestGenMngrSP;

#endif

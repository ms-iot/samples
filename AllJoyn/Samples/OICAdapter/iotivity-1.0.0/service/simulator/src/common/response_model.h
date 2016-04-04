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

#ifndef RESPONSE_MODEL_H_
#define RESPONSE_MODEL_H_

#include "simulator_client_types.h"
#include "simulator_resource_model.h"

class RequestModelBuilder;
class ResponseModel
{
    public:
        friend class RequestModelBuilder;

        SimulatorResult verifyResponse(const OC::OCRepresentation &rep);

    private:
        ResponseModel(int code);
        void setRepSchema(SimulatorResourceModelSP &repSchema);
        SimulatorResult validateAttributeInteger(SimulatorResourceModel::Attribute &attrSchema,
                const OC::OCRepresentation::AttributeItem &ocAttribute);
        SimulatorResult validateAttributeDouble(SimulatorResourceModel::Attribute &attrSchema,
                                                const OC::OCRepresentation::AttributeItem &ocAttribute);
        SimulatorResult validateAttributeString(SimulatorResourceModel::Attribute &attrSchema,
                                                const OC::OCRepresentation::AttributeItem &ocAttribute);

        int m_code;
        SimulatorResourceModelSP m_repSchema;
};

typedef std::shared_ptr<ResponseModel> ResponseModelSP;

#endif

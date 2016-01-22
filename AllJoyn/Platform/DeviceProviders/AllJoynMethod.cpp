//
// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#include "pch.h"
#include "AllJoynProvider.h"
#include "AllJoynService.h"
#include "AllJoynBusObject.h"
#include "AllJoynInterface.h"
#include "AllJoynMethod.h"
#include "AllJoynTypeDefinition.h"
#include "AllJoynStatus.h"
#include "AllJoynHelpers.h"
#include "TypeConversionHelpers.h"

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;
using namespace Platform::Collections;
using namespace Platform;
using namespace concurrency;
using namespace std;

namespace DeviceProviders
{
    AllJoynMethod::AllJoynMethod(AllJoynInterface ^ parent, const alljoyn_interfacedescription_member& methodDescription)
        : m_interface(parent)
        , m_name(methodDescription.name)
        , m_inSignatureString(methodDescription.signature)
        , m_outSignatureString(methodDescription.returnSignature)
        , m_active(true)
    {
        DEBUG_LIFETIME_IMPL(AllJoynMethod);

        auto argNamesVector = AllJoynHelpers::TokenizeArgNamesString(methodDescription.argNames);
        m_inSignature = AllJoynTypeDefinition::CreateParameterInfo(m_inSignatureString, argNamesVector);

        auto outArgNamesVector = vector<string>();
        if (argNamesVector.size() > m_inSignature->Size)
        {
            outArgNamesVector.insert(outArgNamesVector.begin(), argNamesVector.begin() + m_inSignature->Size, argNamesVector.end());
        }
        m_outSignature = AllJoynTypeDefinition::CreateParameterInfo(m_outSignatureString, outArgNamesVector);

        //annotations
        m_annotations = TypeConversionHelpers::GetAnnotationsView<alljoyn_interfacedescription_member>(
            methodDescription,
            alljoyn_interfacedescription_member_getannotationscount,
            alljoyn_interfacedescription_member_getannotationatindex);
    }

    AllJoynMethod::~AllJoynMethod()
    {
    }

    void AllJoynMethod::Shutdown()
    {
        m_active = false;
    }

    IAsyncOperation<InvokeMethodResult^>^ AllJoynMethod::InvokeAsync(IVector<Object ^>^ params)
    {
        return create_async([this, params]() -> InvokeMethodResult ^
        {
            InvokeMethodResult ^ result = ref new InvokeMethodResult();
            const char *errorName = nullptr;

            if (!m_active)
            {
                result->Status = ref new AllJoynStatus(ER_FAIL, "Service no longer available!");
                return result;
            }

            if (params->Size != m_inSignature->Size)
            {
                result->Status = ref new AllJoynStatus(ER_BAD_ARG_COUNT);
                return result;
            }

            alljoyn_message response = alljoyn_message_create(this->GetParent()->GetBusAttachment());
            alljoyn_msgarg inputs = alljoyn_msgarg_array_create(params->Size);

            vector<string> completeTypes;
            auto status = (QStatus)TypeConversionHelpers::CreateCompleteTypeSignatureArrayFromSignature(m_inSignatureString.c_str(), completeTypes);
            LEAVE_IF_QSTATUS_ERROR(status);

            for (uint32 i = 0; i < params->Size; ++i)
            {
                status = (QStatus)TypeConversionHelpers::SetAllJoynMessageArg(
                    alljoyn_msgarg_array_element(inputs, i),
                    completeTypes[i].c_str(),
                    params->GetAt(i));

                LEAVE_IF_QSTATUS_ERROR(status);
            }

            status = alljoyn_proxybusobject_methodcall(this->GetParent()->GetProxyBusObject(),
                this->GetParent()->GetName().c_str(),
                m_name.c_str(),
                inputs,
                params->Size,
                response,
                c_MessageTimeoutInMilliseconds,
                0);

            if (status == ER_BUS_REPLY_IS_ERROR_MESSAGE )
            {
                char * errorMessage = nullptr;
                size_t errorMessageSize;
                errorName = alljoyn_message_geterrorname(response, errorMessage, &errorMessageSize);
            }
            else if (status != ER_OK)
            {
                goto leave;
            }

            auto responseSignature = alljoyn_message_getsignature(response);
            if (status == ER_OK)
            {
                if (m_outSignatureString != responseSignature)
                {
                    status = ER_BUS_UNEXPECTED_SIGNATURE;
                    goto leave;
                }
            }

            result->Values = ref new Vector<Object ^>();
            TypeConversionHelpers::CreateCompleteTypeSignatureArrayFromSignature(responseSignature, completeTypes);

            for (size_t i = 0; i < completeTypes.size(); ++i)
            {
                Object ^ value;
                status = (QStatus)TypeConversionHelpers::GetAllJoynMessageArg(
                    alljoyn_message_getarg(response, i),
                    completeTypes[i].c_str(),
                    &value);

                LEAVE_IF_QSTATUS_ERROR(status);

                result->Values->Append(value);
            }

            leave:
            alljoyn_message_destroy(response);
            alljoyn_msgarg_destroy(inputs);

            result->Status = ref new AllJoynStatus(status, errorName == nullptr ? "" : errorName);
            return result;
        });
    }

    IVector<ParameterInfo ^>^ AllJoynMethod::InSignature::get()
    {
        return m_inSignature;
    }

    IVector<ParameterInfo ^>^ AllJoynMethod::OutSignature::get()
    {
        return m_outSignature;
    }

    String ^ AllJoynMethod::Name::get()
    {
        return AllJoynHelpers::MultibyteToPlatformString(m_name.c_str());
    }
}
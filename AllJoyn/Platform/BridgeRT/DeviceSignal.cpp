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

#include <sstream>
#include "BridgeUtils.h"
#include "DeviceMain.h"
#include "DeviceSignal.h"
#include "AllJoynHelper.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;

using namespace BridgeRT;
using namespace std;


DeviceSignal::DeviceSignal()
    : m_adapterSignal(nullptr)
{
}

DeviceSignal::~DeviceSignal()
{
}

QStatus DeviceSignal::Initialize(_In_ DeviceMain *parent, _In_ IAdapterSignal ^adapterSignal)
{
    QStatus status = ER_OK;

    // sanity check
    if (nullptr == parent)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    if (nullptr == adapterSignal)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

    m_parent = parent;
    m_adapterSignal = adapterSignal;

    // build signal name
    status = SetName(m_adapterSignal->Name);
    if (ER_OK != status)
    {
        goto leave;
    }

    // build signatures and parameter list
    status = BuildSignature();
    if (ER_OK != status)
    {
        goto leave;
    }

    // add signal to interface
    status = alljoyn_interfacedescription_addsignal(m_parent->GetInterfaceDescription(),
        m_exposedName.c_str(),
        m_signature.c_str(),
        m_parameterNames.c_str(),
        0,
        nullptr);
    if (ER_OK != status)
    {
        goto leave;
    }

leave:
    return status;
}

void DeviceSignal::SendSignal()
{
    QStatus status = ER_OK;
    size_t nbOfArgs = 0;
    alljoyn_msgarg args = NULL;
    alljoyn_interfacedescription_member signalDescription;
    QCC_BOOL signalFound = QCC_FALSE;

    // create out arguments if necessary
    if (m_adapterSignal->Params->Size != 0)
    {
        args = alljoyn_msgarg_array_create(m_adapterSignal->Params->Size);
        if (NULL == args)
        {
            goto leave;
        }
    }

    // set arguments
    for (auto signalParam : m_adapterSignal->Params)
    {
        //check if the param is of type IPropertyValue
        auto propertyValue = dynamic_cast<IPropertyValue^>(signalParam->Data);
        if (nullptr == propertyValue)
        {
            //Not a property value, see if its an Object
            if (TypeCode::Object == Type::GetTypeCode(signalParam->Data->GetType()))
            {
                status = AllJoynHelper::SetMsgArgFromAdapterObject(signalParam, alljoyn_msgarg_array_element(args, nbOfArgs), m_parent);
            }
            else
            {
                goto leave;
            }
        }
        else
        {
            status = AllJoynHelper::SetMsgArg(signalParam, alljoyn_msgarg_array_element(args, nbOfArgs));
        }

        if (ER_OK != status)
        {
            goto leave;
        }
        nbOfArgs++;
    }

    // send signal on AllJoyn
    signalFound = alljoyn_interfacedescription_getsignal(m_parent->GetInterfaceDescription(), m_exposedName.c_str(), &signalDescription);
    if (QCC_TRUE == signalFound)
    {
        alljoyn_busobject_signal(m_parent->GetBusObject(), NULL, ALLJOYN_SESSION_ID_ALL_HOSTED, signalDescription, args, nbOfArgs, 0, 0, NULL);
    }

leave:
    if (NULL != args)
    {
        alljoyn_msgarg_destroy(args);
    }
}

QStatus DeviceSignal::SetName(Platform::String ^name)
{
    QStatus status = ER_OK;

    m_exposedName.clear();

    if (name->IsEmpty())
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    AllJoynHelper::EncodePropertyOrMethodOrSignalName(name, m_exposedName);
    if (!m_parent->IsSignalNameUnique(m_exposedName))
    {
        // append unique id
        std::ostringstream tempString;
        m_exposedName += '_';
        tempString << m_parent->GetIndexForSignal();
        m_exposedName += tempString.str();
    }

leave:
    return status;
}

QStatus DeviceSignal::BuildSignature()
{
    QStatus status = ER_OK;

    m_signature.clear();
    m_parameterNames.clear();

    for (auto signalParam : m_adapterSignal->Params)
    {
        std::string tempSignature;
        std::string hint;
        if (nullptr == signalParam->Data)
        {
            // can't do anything with this param
            status = ER_BUS_BAD_SIGNATURE;
            goto leave;
        }

        //check if the value is of type IPropertyValue^
        auto propertyValue = dynamic_cast<IPropertyValue^>(signalParam->Data);
        if (nullptr == propertyValue)
        {
            if (TypeCode::Object == Type::GetTypeCode(signalParam->Data->GetType()))
            {
                IAdapterProperty ^tempProperty = dynamic_cast<IAdapterProperty ^> (signalParam->Data);
                if (nullptr == tempProperty)
                {
                    // wrong object type
                    status = ER_BUS_BAD_SIGNATURE;
                    goto leave;
                }

                // adapter object are exposed as string on AllJoyn
                tempSignature += "s";
                hint = " (bus object path)";
            }
            else
            {
                status = ER_BUS_BAD_SIGNATURE;
                goto leave;
            }
        }
        else
        {
            status = AllJoynHelper::GetSignature(propertyValue->Type, tempSignature);
            if (ER_OK != status)
            {
                goto leave;
            }
            m_signature += tempSignature;
        }

        // add parameter name to parameter list
        if (0 != m_parameterNames.length())
        {
            m_parameterNames += ",";
        }
        m_parameterNames += ConvertTo<std::string>(signalParam->Name->Data());
        m_parameterNames += hint;
    }

leave:
    return status;
}

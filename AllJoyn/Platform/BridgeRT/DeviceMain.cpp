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
#include "Bridge.h"
#include "DeviceMain.h"
#include "DeviceMethod.h"
#include "DeviceSignal.h"
#include "AllJoynHelper.h"

using namespace BridgeRT;

static const std::string INTERFACE_NAME_FOR_MAIN_DEVICE = ".MainInterface";

DeviceMain::DeviceMain()
    : m_AJBusObject(NULL),
    m_interfaceDescription(NULL),
    m_parent(nullptr),
    m_registeredOnAllJoyn(false),
    m_indexForMethod(1),
    m_indexForSignal(1)
{
}

DeviceMain::~DeviceMain()
{
    Shutdown();
}

QStatus DeviceMain::Initialize(BridgeDevice ^parent)
{
    QStatus status = ER_OK;
    std::string tempString;

    // sanity check
    if (nullptr == parent)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    m_parent = parent;

    // build bus object path
    AllJoynHelper::EncodeBusObjectName(parent->GetAdapterDevice()->Name, tempString);
    m_busObjectPath = "/" + tempString;

    // create alljoyn bus object and register it
    m_AJBusObject = alljoyn_busobject_create(m_busObjectPath.c_str(), QCC_FALSE, nullptr, nullptr);
    if (NULL == m_AJBusObject)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    // create interface
    status = CreateMethodsAndSignals();
    if (ER_OK != status)
    {
        return status;
    }

    // add interface to bus object and expose it
    status = alljoyn_busobject_addinterface(m_AJBusObject, m_interfaceDescription);
    if (ER_OK != status)
    {
        goto leave;
    }

    // add method handler
    for (auto val : m_deviceMethods)
    {
        alljoyn_interfacedescription_member member = { 0 };
        QCC_BOOL found = false;

        found = alljoyn_interfacedescription_getmember(m_interfaceDescription, val.second->GetName().c_str(), &member);
        if (!found)
        {
            status = ER_INVALID_DATA;
            goto leave;
        }

        status = alljoyn_busobject_addmethodhandler(m_AJBusObject, member, AJMethod, NULL);
        if (ER_OK != status)
        {
            goto leave;
        }
    }

    // expose bus object
    status = alljoyn_busattachment_registerbusobject(parent->GetBusAttachment(), m_AJBusObject);
    if (ER_OK != status)
    {
        goto leave;
    }
    m_registeredOnAllJoyn = true;

leave:
    return status;
}

void DeviceMain::Shutdown()
{
    // clear methods and signals
    for (auto val : m_deviceMethods)
    {
        delete val.second;
    }
    m_deviceMethods.clear();

    for (auto val : m_deviceSignals)
    {
        delete val.second;
    }
    m_deviceSignals.clear();

    // shutdown AllJoyn Bus object
    if (NULL != m_AJBusObject)
    {
        if (m_registeredOnAllJoyn)
        {
            // unregister bus object
            alljoyn_busattachment_unregisterbusobject(m_parent->GetBusAttachment(), m_AJBusObject);
            m_registeredOnAllJoyn = false;
        }
        alljoyn_busobject_destroy(m_AJBusObject);
        m_AJBusObject = NULL;
    }

    // final clean up
    m_interfaceName.clear();
    m_busObjectPath.clear();
    m_parent = nullptr;
    m_indexForMethod = 1;
    m_indexForSignal = 1;
}

QStatus DeviceMain::CreateMethodsAndSignals()
{
    QStatus status = ER_OK;
    DeviceMethod *method = nullptr;
    DeviceSignal *signal = nullptr;
    std::string tempString;

    // 1st create the interface
    m_interfaceName = m_parent->GetRootNameForInterface();

    //add device name
    AllJoynHelper::EncodeStringForServiceName(m_parent->GetAdapterDevice()->Name, tempString);
    if (!tempString.empty())
    {
        m_interfaceName += ".";
        m_interfaceName += tempString;
    }

    m_interfaceName += INTERFACE_NAME_FOR_MAIN_DEVICE;

    // create interface
    if (DsbBridge::SingleInstance()->GetConfigManager()->IsDeviceAccessSecured())
    {
        status = alljoyn_busattachment_createinterface_secure(m_parent->GetBusAttachment(), m_interfaceName.c_str(), &m_interfaceDescription, AJ_IFC_SECURITY_REQUIRED);
    }
    else
    {
        status = alljoyn_busattachment_createinterface(m_parent->GetBusAttachment(), m_interfaceName.c_str(), &m_interfaceDescription);
    }
    if (ER_OK != status)
    {
        return status;
    }

    // create the methods
    if (nullptr != m_parent->GetAdapterDevice()->Methods)
    {
        for (auto adapterMethod : m_parent->GetAdapterDevice()->Methods)
        {
            method = new(std::nothrow) DeviceMethod();
            if (nullptr == method)
            {
                status = ER_OUT_OF_MEMORY;
                goto leave;
            }

            status = method->Initialize(this, adapterMethod);
            if (ER_OK != status)
            {
                goto leave;
            }

            m_deviceMethods.insert(std::make_pair(method->GetName(), method));
            method = nullptr;
        }
    }

    // create signals
    if (nullptr != m_parent->GetAdapterDevice()->Signals)
    {
        for (auto adapterSignal : m_parent->GetAdapterDevice()->Signals)
        {
            if (adapterSignal->Name == Constants::CHANGE_OF_VALUE_SIGNAL)
            {
                // change of value signal only concerns IAdapterProperty hence not this class
                continue;
            }

            signal = new(std::nothrow) DeviceSignal();
            if (nullptr == signal)
            {
                status = ER_OUT_OF_MEMORY;
                goto leave;
            }

            status = signal->Initialize(this, adapterSignal);
            if (ER_OK != status)
            {
                goto leave;
            }

            m_deviceSignals.insert(std::make_pair(adapterSignal->GetHashCode(), signal));
            signal = nullptr;
        }
    }

    // activate the interface
    alljoyn_interfacedescription_activate(m_interfaceDescription);

leave:
    if (nullptr != method)
    {
        delete method;
    }
    if (nullptr != signal)
    {
        delete signal;
    }

    return status;
}

bool DeviceMain::IsMethodNameUnique(std::string name)
{
    // verify there is no method with same name
    auto methodIterator = m_deviceMethods.find(name);
    if (methodIterator == m_deviceMethods.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DeviceMain::IsSignalNameUnique(std::string name)
{
    bool retVal = true;

    // verify there is no signal with same name
    for (auto tempSignal : m_deviceSignals)
    {
        if (tempSignal.second->GetName() == name)
        {
            retVal = false;
            break;
        }
    }

    return retVal;
}

DeviceMain *DeviceMain::GetInstance(_In_ alljoyn_busobject busObject)
{
    // sanity check
    if (NULL == busObject)
    {
        return nullptr;
    }

    // find out the DeviceMain instance that correspond to the alljoyn bus object
    DeviceMain *objectPointer = nullptr;
    auto deviceList = DsbBridge::SingleInstance()->GetDeviceList();
    for (auto device : deviceList)
    {
        objectPointer = device.second->GetDeviceMainObject();
        if (objectPointer != nullptr &&
            objectPointer->GetBusObject() == busObject)
        {
            break;
        }
        else
        {
            objectPointer = nullptr;
        }
    }

    return objectPointer;
}

void DeviceMain::AJMethod(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg)
{
    QStatus status = ER_OK;
    uint32 adapterStatus = ERROR_SUCCESS;

    std::map<std::string, DeviceMethod *>::iterator methodIterator;
    DeviceMethod *deviceMehod = nullptr;
    alljoyn_msgarg outArgs = NULL;
    size_t nbOfArgs = 0;

    // get instance of device main from bus object
    DeviceMain *deviceMain = DeviceMain::GetInstance(busObject);
    if (nullptr == deviceMain)
    {
        status = ER_OS_ERROR;
        goto leave;
    }

    // get adapter method to invoke
    methodIterator = deviceMain->m_deviceMethods.find(member->name);
    if (methodIterator == deviceMain->m_deviceMethods.end())
    {
        status = ER_NOT_IMPLEMENTED;
        goto leave;
    }
    deviceMehod = methodIterator->second;

    // invoke method
    adapterStatus = methodIterator->second->InvokeMethod(msg, &outArgs, &nbOfArgs);
    if (ERROR_SUCCESS != adapterStatus)
    {
        status = ER_OS_ERROR;
        goto leave;
    }

leave:
    if (ER_OK != status)
    {
        alljoyn_busobject_methodreply_status(busObject, msg, status);
    }
    else if (0 == nbOfArgs)
    {
        alljoyn_busobject_methodreply_args(busObject, msg, NULL, 0);
    }
    else
    {
        alljoyn_busobject_methodreply_args(busObject, msg, outArgs, nbOfArgs);
    }
    if (NULL != outArgs)
    {
        alljoyn_msgarg_destroy(outArgs);
    }
}

void DeviceMain::HandleSignal(_In_ IAdapterSignal ^adapterSignal)
{
    // sanity check
    if (nullptr == adapterSignal)
    {
        return;
    }

    // get corresponding signal class instance
    auto signal = m_deviceSignals.find(adapterSignal->GetHashCode());
    if (m_deviceSignals.end() == signal)
    {
        // unknown IAdapterSignal
        return;
    }

    // send signal to alljoyn
    signal->second->SendSignal(adapterSignal);
}
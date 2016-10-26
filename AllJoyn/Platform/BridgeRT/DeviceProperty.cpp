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

#include "Bridge.h"
#include "BridgeDevice.h"
#include "DeviceProperty.h"
#include "PropertyInterface.h"
#include "AllJoynProperty.h"
#include "AllJoynHelper.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;

using namespace BridgeRT;
using namespace std;
using namespace Windows::Foundation;

DeviceProperty::DeviceProperty()
    : m_deviceProperty(nullptr),
    m_parent(nullptr),
    m_AJBusObject(NULL),
    m_registeredOnAllJoyn(false),
    m_propertyInterface(nullptr)
{
}

DeviceProperty::~DeviceProperty()
{
}

QStatus DeviceProperty::Initialize(IAdapterProperty ^deviceProperty, PropertyInterface *propertyInterface, BridgeDevice ^parent)
{
    QStatus status = ER_OK;
    string tempString;
    alljoyn_busobject_callbacks callbacks =
    {
        &DeviceProperty::GetProperty,
        &DeviceProperty::SetProperty,
        nullptr,
        nullptr
    };

    // sanity check
    if (nullptr == deviceProperty)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    if (nullptr == propertyInterface)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }
    if (nullptr == parent)
    {
        status = ER_BAD_ARG_3;
        goto leave;
    }

    m_deviceProperty = deviceProperty;
    m_propertyInterface = propertyInterface;
    m_parent = parent;

    // build bus object path
    AllJoynHelper::EncodeBusObjectName(m_deviceProperty->Name, tempString);
    m_AJBusObjectPath = "/" + tempString;

    if (!parent->IsBusObjectPathUnique(m_AJBusObjectPath))
    {
        DWORD id = 0;
        string tempPath;
        do
        {
            tempPath = m_AJBusObjectPath;
            std::ostringstream tmp;
            tmp << ++id;
            tempPath += '/';
            tempPath += tmp.str();
        } while (!parent->IsBusObjectPathUnique(tempPath));

        m_AJBusObjectPath = tempPath;
    }

    // create alljoyn bus object and register it
    m_AJBusObject = alljoyn_busobject_create(m_AJBusObjectPath.c_str(), QCC_FALSE, &callbacks, this);
    if (NULL == m_AJBusObject)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    status = alljoyn_busobject_addinterface(m_AJBusObject, propertyInterface->GetInterfaceDescription());
    if (ER_BUS_IFACE_ALREADY_EXISTS == status)
    {
        // this is OK
        status = ER_OK;
    }
    else if (ER_OK != status)
    {
        goto leave;
    }

    status = PairAjProperties();
    if (ER_OK != status)
    {
        goto leave;
    }

    status = alljoyn_busattachment_registerbusobject(parent->GetBusAttachment(), m_AJBusObject);
    if (ER_OK != status)
    {
        goto leave;
    }
    m_registeredOnAllJoyn = true;

leave:
    return status;
}

void DeviceProperty::Shutdown()
{
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
    m_propertyInterface = nullptr;
    m_parent = nullptr;
    m_AJBusObjectPath.clear();
    m_AJpropertyAdapterValuePairs.clear();
}

QStatus BridgeRT::DeviceProperty::PairAjProperties()
{
    QStatus status = ER_OK;

    vector <IAdapterAttribute ^> tempList;

    // create temporary list of IAdapterValue that have to match with one of the
    // AllJoyn properties
    for (auto adapterAttr : m_deviceProperty->Attributes)
    {
        tempList.push_back(adapterAttr);
    }

    // go through AllJoyn properties and find matching IAdapterValue
    for (auto ajProperty : m_propertyInterface->GetAJProperties())
    {
        bool paired = false;

        auto adapterAttr = tempList.end();
        for (adapterAttr = tempList.begin(); adapterAttr != tempList.end(); adapterAttr++)
        {
            if (ajProperty->IsSameType(*adapterAttr))
            {
                AJpropertyAdapterValuePair tempPair = { ajProperty, *adapterAttr };
                m_AJpropertyAdapterValuePairs.insert(std::make_pair(*ajProperty->GetName(), tempPair));
                paired = true;
                break;
            }
        }
        if (!paired)
        {
            // a matching IAdapterValue must exist for each AllJoyn property
            status = ER_INVALID_DATA;
            goto leave;
        }

        // remove adapterValue from temp list
        tempList.erase(adapterAttr);
    }

leave:
    return status;
}

QStatus AJ_CALL DeviceProperty::GetProperty(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;
    uint32 adapterStatus = ERROR_SUCCESS;
    DeviceProperty *deviceProperty = nullptr;
    IAdapterAttribute ^adapterAttr = nullptr;
    IAdapterValue ^adapterValue = nullptr;
    AllJoynProperty *ajProperty = nullptr;
    IAdapterIoRequest^ request;

    UNREFERENCED_PARAMETER(ifcName);

    deviceProperty = (DeviceProperty *)context;
    if (nullptr == deviceProperty)	// sanity test
    {
        return ER_BAD_ARG_1;
    }

    // identify alljoyn property and its corresponding adapter value
    auto index = deviceProperty->m_AJpropertyAdapterValuePairs.find(propName);
    if (deviceProperty->m_AJpropertyAdapterValuePairs.end() == index)
    {
        status = ER_BUS_NO_SUCH_PROPERTY;
        goto leave;
    }

    ajProperty = index->second.ajProperty;
    adapterAttr = index->second.adapterAttr;
    adapterValue = adapterAttr->Value;

    // get value of adapter value
    adapterStatus = DsbBridge::SingleInstance()->GetAdapter()->GetPropertyValue(deviceProperty->m_deviceProperty, adapterValue->Name, &adapterValue, &request);
    if (ERROR_IO_PENDING == adapterStatus &&
        nullptr != request)
    {
        // wait for completion
        adapterStatus = request->Wait(WAIT_TIMEOUT_FOR_ADAPTER_OPERATION);
    }
    if (ERROR_SUCCESS != adapterStatus)
    {
        status = ER_OS_ERROR;
        goto leave;
    }

    // build alljoyn response to get
    status = AllJoynHelper::SetMsgArg(adapterValue, val);

leave:
    return status;
}

QStatus AJ_CALL DeviceProperty::SetProperty(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _In_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;
    uint32 adapterStatus = ERROR_SUCCESS;
    DeviceProperty *deviceProperty = nullptr;
    IAdapterAttribute^ adapterAttr = nullptr;
    IAdapterValue ^adapterValue = nullptr;
    AllJoynProperty *ajProperty = nullptr;
    IAdapterIoRequest^ request;

    UNREFERENCED_PARAMETER(ifcName);

    deviceProperty = (DeviceProperty *)context;
    if (nullptr == deviceProperty)	// sanity test
    {
        return ER_BAD_ARG_1;
    }

    // identify alljoyn property and its corresponding adapter value
    auto index = deviceProperty->m_AJpropertyAdapterValuePairs.find(propName);
    if (deviceProperty->m_AJpropertyAdapterValuePairs.end() == index)
    {
        status = ER_BUS_NO_SUCH_PROPERTY;
        goto leave;
    }

    ajProperty = index->second.ajProperty;
    adapterAttr = index->second.adapterAttr;
    adapterValue = adapterAttr->Value;

    // update IAdapterValue from AllJoyn message
    status = AllJoynHelper::GetAdapterValue(adapterValue, val);
    if (ER_OK != status)
    {
        goto leave;
    }

    // set value in adapter
    adapterStatus = DsbBridge::SingleInstance()->GetAdapter()->SetPropertyValue(deviceProperty->m_deviceProperty, adapterValue, &request);
    if (ERROR_IO_PENDING == adapterStatus &&
        nullptr != request)
    {
        // wait for completion
        adapterStatus = request->Wait(WAIT_TIMEOUT_FOR_ADAPTER_OPERATION);
    }
    if (ERROR_ACCESS_DENIED == adapterStatus)
    {
        status = ER_BUS_PROPERTY_ACCESS_DENIED;
        goto leave;
    }
    if (ERROR_SUCCESS != adapterStatus)
    {
        status = ER_BUS_PROPERTY_VALUE_NOT_SET;
        goto leave;
    }

leave:
    return status;
}

void DeviceProperty::EmitSignalCOV(IAdapterValue ^newValue, const std::vector<alljoyn_sessionid>& sessionIds)
{
    QStatus status = ER_OK;
    alljoyn_msgarg msgArg = NULL;
    auto valuePair = m_AJpropertyAdapterValuePairs.end();

    // sanity check
    if (nullptr == newValue)
    {
        goto leave;
    }

    // get AllJoyn property that match with IAdapterValue that has changed
    for (valuePair = m_AJpropertyAdapterValuePairs.begin(); valuePair != m_AJpropertyAdapterValuePairs.end(); valuePair++)
    {
        if (valuePair->second.adapterAttr->Value->Name == newValue->Name)
        {
            break;
        }
    }
    if (valuePair == m_AJpropertyAdapterValuePairs.end())
    {
        // can't find any Alljoyn property that correspond to IAdapterValue
        goto leave;
    }

    // prepare signal arguments
    msgArg = alljoyn_msgarg_create();
    if (NULL == msgArg)
    {
        goto leave;
    }

    // build alljoyn message from IAdapterValue
    status = AllJoynHelper::SetMsgArg(newValue, msgArg);
    if (status != ER_OK)
    {
        goto leave;
    }

	for (auto sessionId : sessionIds)
	{
		// emit property change
		alljoyn_busobject_emitpropertychanged(m_AJBusObject,
			m_propertyInterface->GetInterfaceName()->c_str(),
			valuePair->second.ajProperty->GetName()->c_str(),
			msgArg, sessionId);
	}

leave:
    if (NULL != msgArg)
    {
        alljoyn_msgarg_destroy(msgArg);
    }

    return;
}
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

#pragma once

namespace BridgeRT
{
    ref class BridgeDevice;
    class PropertyInterface;
    class AllJoynProperty;

    typedef struct
    {
        AllJoynProperty *ajProperty;
        IAdapterAttribute ^adapterAttr;
    } AJpropertyAdapterValuePair;

    class DeviceProperty
    {
    public:
        DeviceProperty();
        virtual ~DeviceProperty();

        QStatus Initialize(_In_ IAdapterProperty ^deviceProperty, _In_ PropertyInterface *propertyInterface, _In_ BridgeDevice ^parent);
        void Shutdown();
        void EmitSignalCOV(_In_ IAdapterValue ^newValue, const std::vector<alljoyn_sessionid>& sessionIds);

        inline std::string *GetPathName()
        {
            return &m_AJBusObjectPath;
        }
        inline alljoyn_busobject GetBusObject()
        {
            return m_AJBusObject;
        }
        inline PropertyInterface *GetPropertyInterface()
        {
            return m_propertyInterface;
        }
        inline IAdapterProperty ^GetAdapterProperty()
        {
            return m_deviceProperty;
        }

    private:
        QStatus PairAjProperties();

        static QStatus AJ_CALL GetProperty(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val);
        static QStatus AJ_CALL SetProperty(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _In_ alljoyn_msgarg val);

        // parent class
        BridgeDevice ^m_parent;

        // device property
        IAdapterProperty ^m_deviceProperty;

        // AllJoyn related
        alljoyn_busobject m_AJBusObject;
        PropertyInterface *m_propertyInterface;
        bool m_registeredOnAllJoyn;

        // store association of each bus interface property exposed on AllJoyn by this bus object
        // with its corresponding device instance (adapter value)
        std::map<std::string, AJpropertyAdapterValuePair> m_AJpropertyAdapterValuePairs;

        std::string m_AJBusObjectPath;
    };
}


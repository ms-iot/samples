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

#include "AllJoynProperty.h"
#include "PropertyInterface.h"
#include "AllJoynHelper.h"

using namespace BridgeRT;
using namespace std;
using namespace Windows::Foundation;

AllJoynProperty::AllJoynProperty() 
    : m_parent(nullptr),
    m_originalName(nullptr),
    m_dsbType(Platform::TypeCode::Empty),
    m_dsbSubType(PropertyType::Empty),
    m_annotations(nullptr),
    m_access(E_ACCESS_TYPE::ACCESS_READWRITE)
{
}

AllJoynProperty::~AllJoynProperty()
{
}

QStatus AllJoynProperty::Create(IAdapterAttribute ^adapterAttribute, PropertyInterface *parent)
{
    QStatus status = ER_OK;

    // sanity check
    if (nullptr == adapterAttribute || nullptr == adapterAttribute->Value)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    if (nullptr == parent)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

    m_parent = parent;
    m_originalName = adapterAttribute->Value->Name;
    m_annotations = adapterAttribute->Annotations;
    m_access = adapterAttribute->Access;
    m_covBehavior = adapterAttribute->COVBehavior;

    m_dsbType = Platform::Type::GetTypeCode(adapterAttribute->Value->Data->GetType());

    status = SetName(m_originalName);
    if (ER_OK != status)
    {
        goto leave;
    }

    m_signature.clear();

    {
        //check  if the dsbValue is of type IPropertyValue
        auto propertyValue = dynamic_cast<IPropertyValue^>(adapterAttribute->Value->Data);
        if (nullptr == propertyValue)
        {
            status = ER_BAD_ARG_1;
            goto leave;
        }

        m_dsbSubType = propertyValue->Type;
        status = AllJoynHelper::GetSignature(propertyValue->Type, m_signature);
        if (ER_OK != status)
        {
            goto leave;
        }
    }

leave:
    if (ER_OK != status)
    {
        m_parent = nullptr;
        m_signature.clear();
        m_exposedName.clear();
        m_originalName = nullptr;
        m_dsbType = Platform::TypeCode::Empty;
        m_dsbSubType = PropertyType::Empty;
    }

    return status;
}

QStatus AllJoynProperty::SetName(Platform::String ^name)
{
    QStatus status = ER_OK;

    m_exposedName.clear();

    if (name->IsEmpty())
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    AllJoynHelper::EncodePropertyOrMethodOrSignalName(name, m_exposedName);
    if (!m_parent->IsAJPropertyNameUnique(m_exposedName))
    {
        // append unique id
        std::ostringstream tempString;
        m_exposedName += '_';
        tempString << m_parent->GetIndexForAJProperty();
        m_exposedName += tempString.str();
    }

leave:
    return status;
}

bool AllJoynProperty::IsSameType(IAdapterAttribute ^adapterAttribute)
{
    bool retval = false;

    if (adapterAttribute != nullptr && adapterAttribute->Value != nullptr &&
        adapterAttribute->Value->Data != nullptr &&
        m_originalName == adapterAttribute->Value->Name &&
        m_dsbType == Platform::Type::GetTypeCode(adapterAttribute->Value->Data->GetType()) &&
        m_access == adapterAttribute->Access &&
        m_covBehavior == adapterAttribute->COVBehavior &&
        AreAnnotationsSame(adapterAttribute->Annotations))
    {
        retval = true;
        
        auto propertyValue = dynamic_cast<IPropertyValue^>(adapterAttribute->Value->Data);
        if (propertyValue != nullptr && propertyValue->Type != m_dsbSubType)
        {
            retval = false;
        }
    }

    return retval;
}

bool AllJoynProperty::AreAnnotationsSame(_In_ IAnnotationMap ^annotations)
{

    if (m_annotations && annotations)
    {
        if (m_annotations->Size != annotations->Size)
        {
            return false;
        }
        auto iter1 = m_annotations->First();
        while (iter1->HasCurrent)
        {
            auto key = iter1->Current->Key;
            auto value = iter1->Current->Value;
            if (value != annotations->Lookup(key))
            {
                return false;
            }
            iter1->MoveNext();
        }
    }
    else if(m_annotations || annotations)
    {
        //return false if only one of them have annotations and not both
        return false;
    }

    return true;
}

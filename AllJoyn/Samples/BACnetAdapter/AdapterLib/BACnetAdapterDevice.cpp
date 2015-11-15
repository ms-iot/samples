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
#include "BACnetAdapter.h"
#include "BACnetAdapterDevice.h"
#include "BACnetInterface.h"
#include "BfiDefinitions.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

using namespace BridgeRT;
using namespace DsbCommon;

namespace AdapterLib
{
    //
    // BACnetAdapterValue.
    // Description:
    //  The class that implements BridgeRT::IAdapterValue.
    //
    BACnetAdapterValue::BACnetAdapterValue(
        String^ ObjectName, 
        Object^ ParentObject,
        Object^ DefaultData // = nullptr
        )
        : name(ObjectName)
        , parent(ParentObject)
        , data(DefaultData)
        , isModified(false)
    {
    }

    BACnetAdapterValue::BACnetAdapterValue(const BACnetAdapterValue^ Other)
        : name(Other->name)
        , parent(Other->parent)
        , data(Other->data)
        , isModified(false)
    {
    }


    uint32 
    BACnetAdapterValue::Set(IAdapterValue^ Other)
    {
        this->name = Other->Name;
        this->data = Other->Data;

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32 
    BACnetAdapterValue::Set(const BACNET_APPLICATION_DATA_VALUE& BACnetValue)
    {
        return this->FromBACnet(BACnetValue);
    }

    
    _Use_decl_annotations_
    uint32 
    BACnetAdapterValue::FromBACnet(const BACNET_APPLICATION_DATA_VALUE& BACnetValue)
    {
        uint32 status = ERROR_SUCCESS;

        try
        {
            this->data= PropertyValue::CreateEmpty();

            switch (BACnetValue.tag)
            {
            case BACNET_APPLICATION_TAG_BOOLEAN:
                this->data = PropertyValue::CreateBoolean(BACnetValue.type.Boolean);
                break;

            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                this->data = PropertyValue::CreateUInt32(BACnetValue.type.Unsigned_Int);
                break;

            case BACNET_APPLICATION_TAG_SIGNED_INT:
                this->data = PropertyValue::CreateUInt32(BACnetValue.type.Signed_Int);
                break;

            case BACNET_APPLICATION_TAG_REAL:
                __fallthrough;
            case BACNET_APPLICATION_TAG_DOUBLE:
                this->data = PropertyValue::CreateDouble(double(BACnetValue.type.Real));
                break;

            case BACNET_APPLICATION_TAG_OCTET_STRING:
            {
                const Platform::Array<uint8_t>^ octetArray = ref new Platform::Array<uint8_t>(
                    (unsigned char*)&BACnetValue.type.Octet_String.value[0],
                    int(BACnetValue.type.Octet_String.length)
                    );
                this->data = PropertyValue::CreateUInt8Array(octetArray);
                break;
            }

            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
            {
                // Convert to wide character
                std::string asciiStr(
                    &BACnetValue.type.Character_String.value[0],
                    size_t(BACnetValue.type.Character_String.length)
                    );
                std::wstring wideStr(asciiStr.begin(), asciiStr.end());

                this->data = PropertyValue::CreateString(ref new String(wideStr.c_str()));
                break;
            }

            case BACNET_APPLICATION_TAG_OBJECT_ID:
            {
                BACNET_ADAPTER_OBJECT_ID objectId(
                    ULONG(BACnetValue.type.Object_Id.type),
                    ULONG(BACnetValue.type.Object_Id.instance)
                    );

                this->data = PropertyValue::CreateUInt32(objectId.Ulong);
                break;
            }

            case BACNET_APPLICATION_TAG_ENUMERATED:
                this->data = PropertyValue::CreateUInt32(BACnetValue.type.Enumerated);
                break;

            default:
                status = ERROR_NOT_SUPPORTED;
                break;
            }
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }

        return status;
    }


    _Use_decl_annotations_
    uint32 
    BACnetAdapterValue::ToBACnet(
        BACNET_ADAPTER_OBJECT_ID& BACnetObjectId,
        BACNET_PROPERTY_ID BACnetPropertyId,
        BACNET_APPLICATION_DATA_VALUE& BACnetValue
        )
    {
        DWORD status = ERROR_SUCCESS;
        IPropertyValue^ ipv = dynamic_cast<IPropertyValue^>(this->data);

        RtlZeroMemory(&BACnetValue, sizeof(BACNET_APPLICATION_DATA_VALUE));

        if (ipv == nullptr)
        {
            DSB_ASSERT(FALSE);
            return ERROR_NOT_SUPPORTED;
        }

        const BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR* attrDescPtr = GetObjectAttributeDescriptor(
                                                                BACNET_OBJECT_TYPE(BACnetObjectId.Bits.Type),
                                                                BACnetPropertyId
                                                                );
        if (attrDescPtr == nullptr)
        {
            DSB_ASSERT(FALSE);
            return ERROR_NOT_SUPPORTED;
        }

        try
        {
            switch (ipv->Type)
            {
            case PropertyType::Boolean:
                BACnetValue.type.Boolean = ipv->GetBoolean();
                break;

            case PropertyType::UInt8:
                BACnetValue.type.Unsigned_Int = (uint32_t)(ipv->GetUInt8());
                break;

            case PropertyType::UInt16:
                BACnetValue.type.Unsigned_Int = (uint32_t)(ipv->GetUInt16());
                break;

            case PropertyType::UInt32:
                BACnetValue.type.Unsigned_Int = (uint32_t)(ipv->GetUInt32());
                break;

            case PropertyType::Int16:
                BACnetValue.type.Signed_Int = (int32_t)(ipv->GetInt16());
                break;

            case PropertyType::Int32:
                BACnetValue.type.Signed_Int = (int32_t)(ipv->GetInt32());
                break;

            case PropertyType::Double:
                if (attrDescPtr->BACnetSignature == BACNET_APPLICATION_TAG_REAL)
                {
                    BACnetValue.type.Real = float(ipv->GetDouble());
                }
                else
                {
                    DSB_ASSERT(attrDescPtr->BACnetSignature == BACNET_APPLICATION_TAG_DOUBLE);

                    BACnetValue.type.Double = ipv->GetDouble();
                }
                break;

            case PropertyType::String:
            {
                //
                // We may have translated a BACnet attribute to a string, in that
                // case we need to translate back.
                //
                status = BACnetAdapterValue::TranslateAttributeValue(this, BACnetPropertyId, false);
                if (status != ERROR_NOT_SUPPORTED)
                {
                    // Either translation was successful, or something went wrong...

                    if (status == ERROR_SUCCESS)
                    {
                        // Translation was successful, now we are ready to convert to BACnet...
                        status = this->ToBACnet(BACnetObjectId, BACnetPropertyId, BACnetValue);
                    }
                    break;
                }
                status = ERROR_SUCCESS;

                //
                // No translation was used for this attribute, convert the string...
                //

                std::string asciiStr = To_Ascii_String(ipv->GetString()->Data());

                if (asciiStr.length() > ARRAYSIZE(BACnetValue.type.Character_String.value))
                {
                    status = ERROR_BUFFER_OVERFLOW;
                    goto done;
                }

                BACnetValue.type.Character_String.length = asciiStr.length();
                RtlCopyMemory(
                    &BACnetValue.type.Character_String.value[0],
                    asciiStr.data(),
                    asciiStr.length()
                    );
                break;
            }

            default:
                status = ERROR_NOT_SUPPORTED;
                goto done;

            } // switch 
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        BACnetValue.tag = UINT8(attrDescPtr->BACnetSignature);

    done:

        return status;
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapterValue::TranslateAttributeValue(
        BACnetAdapterValue^ Attribute,
        ULONG BACnetPropertyId,
        bool IsFromBACnet
        )
    {
        IPropertyValue^ ipv = dynamic_cast<IPropertyValue^>(Attribute->Data);
        const wchar_t* translatedAttrWsz = nullptr;
        UINT32 translatedAttr = UINT32(-1);

        DSB_ASSERT(ipv != nullptr);

        switch (BACnetPropertyId)
        {
        case PROP_UNITS:
        {
            if (IsFromBACnet)
            {
                BACNET_ENGINEERING_UNITS units = BACNET_ENGINEERING_UNITS(ipv->GetUInt32());
                translatedAttrWsz = AdapterLib::ToString(units);
            }
            else
            {
                DSB_ASSERT(ipv->GetString() != nullptr);

                AdapterLib::FromString(
                    ipv->GetString()->Data(),
                    reinterpret_cast<BACNET_ENGINEERING_UNITS*>(&translatedAttr)
                    );
            }

            break;

        } // PROP_UNITS

        case PROP_POLARITY:
        {
            if (IsFromBACnet)
            {
                BACNET_POLARITY polarity = BACNET_POLARITY(ipv->GetUInt32());
                translatedAttrWsz = AdapterLib::ToString(polarity);
            }
            else
            {
                DSB_ASSERT(ipv->GetString() != nullptr);

                AdapterLib::FromString(
                    ipv->GetString()->Data(),
                    reinterpret_cast<BACNET_POLARITY*>(&translatedAttr)
                    );
            }

            break;

        } // PROP_POLARITY

        default:
            return ERROR_NOT_FOUND;

        } // switch 

        if (translatedAttrWsz != nullptr)
        {
            DSB_ASSERT(translatedAttr == UINT32(-1));

            Attribute->Data = ref new String(translatedAttrWsz);
        }
        else if (translatedAttr != UINT32(-1))
        {
            Attribute->Data = PropertyValue::CreateUInt32(translatedAttr);
        }

        return ERROR_SUCCESS;
    }


    //
    // BACnetAdapterProperty.
    // Description:
    //  The class that implements BridgeRT::IAdapterProperty.
    //
    BACnetAdapterProperty::BACnetAdapterProperty(String^ Name, Object^ ParentObject, String^ ifHint)
        : name(Name)
        , parent(ParentObject)
    {
        std::wstring wszIfName = cAdapterPrefix + L"." + cAdapterName + L"." + ifHint->Data();
        interfaceHint = ref new String(wszIfName.c_str());
    }


    BACnetAdapterProperty::BACnetAdapterProperty(ULONG BACnetObjectId, Object^ ParentObject, String^ ifHint)
        : parent(ParentObject)
        , objectId(BACnetObjectId)
    {
        std::wstring wszIfName = cAdapterPrefix + L"." + cAdapterName + L"." + ifHint->Data();
        interfaceHint = ref new String(wszIfName.c_str());
    }


    BACnetAdapterProperty::BACnetAdapterProperty(const BACnetAdapterProperty^ Other)
        : name(Other->name)
        , parent(Other->parent)
        , attributes(Other->attributes)
        , objectId(Other->objectId)
        , interfaceHint(Other->interfaceHint)
    {
    }


    void 
    BACnetAdapterProperty::SetName(String^ PropertyName)
    {
        this->name = PropertyName;
    }


    BACnetAdapterAttribute^
    BACnetAdapterProperty::GetAttributeByName(String^ AttributeName)
    {
        for (auto attr : this->attributes)
        {
            if (_wcsicmp(attr->Value->Name->Data(), AttributeName->Data()) == 0)
            {
                return dynamic_cast<BACnetAdapterAttribute^>(attr);
            }
        }

        return nullptr;
    }


    BACnetAdapterAttribute^ 
    BACnetAdapterProperty::GetAttributeByPropertyId(BACNET_PROPERTY_ID PropertyId)
    {
        const wchar_t* propertyNameWsz = AdapterLib::ToString(PropertyId);
        if (propertyNameWsz == nullptr)
        {
            return nullptr;
        }

        return this->GetAttributeByName(ref new String(propertyNameWsz));
    }


    _Use_decl_annotations_
    uint32 
    BACnetAdapterProperty::SetAttributeByPropertyId(BACNET_PROPERTY_ID PropertyId, const BACNET_APPLICATION_DATA_VALUE& Value)
    {
        BACnetAdapterAttribute^ attribute;

        try
        {
            attribute = this->GetAttributeByPropertyId(PropertyId);
            if (attribute == nullptr)
            {
                return ERROR_NOT_FOUND;
            }
        } 
        catch (OutOfMemoryException^)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        uint32 status = dynamic_cast<BACnetAdapterValue^>(attribute->Value)->Set(Value);
        if (status == ERROR_SUCCESS)
        {
            dynamic_cast<BACnetAdapterValue^>(attribute->Value)->SetModified(true);
        }

        return status;
    }


    uint32
    BACnetAdapterProperty::Set(IAdapterProperty^ Other)
    {
        this->name = Other->Name;

        IAdapterAttributeVector^ otherAttributes = Other->Attributes;

        if (this->attributes.size() != otherAttributes->Size)
        {
            throw ref new InvalidArgumentException(L"Incompatible DSB Properties");
        }

        for (uint32 attrInx = 0; attrInx < otherAttributes->Size; ++attrInx)
        {
            BACnetAdapterValue^ attr = static_cast<BACnetAdapterValue^>(this->attributes[attrInx]->Value);

            attr->Set(otherAttributes->GetAt(attrInx)->Value);
        }

        return ERROR_SUCCESS;
    }


    ULONG
    BACnetAdapterProperty::GetBACnetObjectId()
    {
        return this->objectId;
    }


    IAdapterValue^ 
    BACnetAdapterProperty::GetPresentValue()
    {
        return this->GetAttributeByPropertyId(PROP_PRESENT_VALUE)->Value;
    }


    uint32 
    BACnetAdapterProperty::SetPresentValue(const BACNET_APPLICATION_DATA_VALUE& PresetValue)
    {
        IAdapterValue^ presentValue = this->GetPresentValue();
        if (presentValue == nullptr)
        {
            DSB_ASSERT(FALSE);
            return ERROR_INTERNAL_ERROR;
        }

        BACnetAdapterValue^ currentValue = dynamic_cast<BACnetAdapterValue^>(presentValue);
        if (currentValue == nullptr)
        {
            DSB_ASSERT(FALSE);
            return ERROR_INTERNAL_ERROR;
        }

        return currentValue->Set(PresetValue);
    }


    void 
    BACnetAdapterProperty::NotifyCovSignal()
    {
        BACnetAdapterDevice^ device = dynamic_cast<BACnetAdapterDevice^>(this->parent);
        DSB_ASSERT(device != nullptr);

        IAdapterValue^ presentValue = this->GetPresentValue();
        DSB_ASSERT(presentValue != nullptr);

        device->SendSignal(Constants::CHANGE_OF_VALUE_SIGNAL, this, presentValue);
    }


    void
    BACnetAdapterProperty::Relinquish(bool IsOnlyModified)
    {
        BACnetAdapterDevice^ device = dynamic_cast<BACnetAdapterDevice^>(this->parent);
        DSB_ASSERT(device != nullptr);

        if (IsOnlyModified)
        {
            // Relinquish all modified attributes...
            for (auto adapterAttr : this->attributes)
            {
                BACnetAdapterValue^ attribute = dynamic_cast<BACnetAdapterValue^>(adapterAttr->Value);
                DSB_ASSERT(attribute != nullptr);

                if (attribute->IsModified())
                {
                    // Relinquish the modified attribute (BACnet property)
                    (void)device->WritePropertyAttribute(this, attribute, true, nullptr);
                }

            } // More attributes
        }
        else
        {
            // Based on the spec 'Present_Value' is the only commendable property
            BACnetAdapterValue^ presentValue = dynamic_cast<BACnetAdapterValue^>(this->GetPresentValue());
            if (presentValue != nullptr)
            {
                // Relinquish the commendable property
                (void)device->WritePropertyAttribute(this, presentValue, true, nullptr);
            }
        }
    }

    //
    // BACnetAdapterAttribute.
    // Description:
    //  The class that implements BridgeRT::IAdapterAttribute.
    //
    BACnetAdapterAttribute::BACnetAdapterAttribute(String^ ObjectName, Object^ ParentObject, Object^ DefaultData)
        : value(ref new BACnetAdapterValue(ObjectName, ParentObject, DefaultData))
    {
    }

    BACnetAdapterAttribute::BACnetAdapterAttribute(const BACnetAdapterAttribute^ other)
        : value(other->value)
        , annotations(other->annotations)
        , covBehavior(other->covBehavior)
        , access(other->access)
    {

    }


    //
    // BACnetAdapterDevice.
    // Description:
    //  The class that implements BridgeRT::IAdapterDevice.
    //
    BACnetAdapterDevice::BACnetAdapterDevice(String^ Name, Object^ ParentObject)
        : name(Name)
        , parent(ParentObject)
    {
        // Used only for signature spec
    }


    BACnetAdapterDevice::BACnetAdapterDevice(Object^ ParentObject)
        : parent(ParentObject)
    {
        this->bacnetAdapter = dynamic_cast<BACnetAdapter^>(ParentObject);
        DSB_ASSERT(this->bacnetAdapter != nullptr);
    }


    uint32
    BACnetAdapterDevice::Init(
        UINT32 DeviceId,
        UINT16 VendorId,
        BACnetInterface^ StackInterface
        )
    {
        DWORD status = ERROR_SUCCESS;

        this->deviceId = DeviceId;
        this->vendorId = VendorId;
        this->stackInterface = StackInterface;

        try
        {
            //
            // 1) We start with the device information
            //
            status = this->readDeviceInformation();
            if (status != ERROR_SUCCESS)
            {
                goto done;
            }

            //
            // 2) Now read associated objects
            //
            status = this->readObjects();
            if (status != ERROR_SUCCESS)
            {
                goto done;
            }

            //
            // 3) Subscribe for device related signals
            //
            status = this->subscribeForSignals();
            if (status != ERROR_SUCCESS)
            {
                goto done;
            }
        }
        catch (OutOfMemoryException^)
        {
            DSB_ASSERT(FALSE);

            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

    done:

        return status;
    }


    void 
    BACnetAdapterDevice::Shutdown()
    {
        // Relinquish all (modified) properties
        for (IAdapterProperty^ adapterProperty : this->properties)
        {
            BACnetAdapterProperty^ bacnetAdapterProperty = dynamic_cast<BACnetAdapterProperty^>(adapterProperty);
            DSB_ASSERT(bacnetAdapterProperty != nullptr);

            // Relinquish only modified properties
            bacnetAdapterProperty->Relinquish(true);
        }

        this->UnsubscribeAll();
    }


    BACnetAdapterProperty^
    BACnetAdapterDevice::GetPropertyByObjectId(
        ULONG PropertyObjectId
        )
    {
        auto propIter = this->propertyMap.find(PropertyObjectId);
        if (propIter == this->propertyMap.end())
        {
            return nullptr;
        }

        return propIter->second;
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapterDevice::ReadProperty(
        ULONG PropertyObjectId,
        IAdapterProperty^ adapterProperty,
        IAdapterIoRequest^* RequestPtr
        )
    {
        uint32 status = ERROR_SUCCESS;
        BACNET_ADAPTER_OBJECT_ID bacnetObjectId(PropertyObjectId);
        const BACNET_ADAPTER_OBJECT_DESCRIPTOR* objectDescPtr = GetObjectDescriptor(BACNET_OBJECT_TYPE(bacnetObjectId.Bits.Type));
        BACnetAdapterProperty^ bacnetAdapterProperty = dynamic_cast<BACnetAdapterProperty^>(adapterProperty);
        bool isNewProperty;

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        if (objectDescPtr == nullptr)
        {
            // An unsupported object. move on...
            return ERROR_NOT_SUPPORTED;
        }

        if (this->stackInterface == nullptr)
        {
            return ERROR_NOT_READY;
        }

        if (bacnetAdapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        //
        // If we are creating an new property (Attributes.size() == 0),
        // or updating an existing property ((Attributes.size() != 0).
        //
        isNewProperty = adapterProperty->Attributes->Size == 0;

        try
        {
            //
            // Add common attributes, if needed...
            //

            // PROP_OBJECT_TYPE string
            if (isNewProperty)
            {
                auto attribute = ref new BACnetAdapterAttribute(
                                                        ref new String(AdapterLib::ToString(PROP_OBJECT_TYPE)),
                                                        bacnetAdapterProperty,
                                                        ref new String(AdapterLib::ToString(BACNET_OBJECT_TYPE(bacnetObjectId.Bits.Type)))
                                                        );

                bacnetAdapterProperty += attribute;
            }

            //
            // PROP_OBJECT_IDENTIFIER
            // We can add it to the list of attributes and read it from 
            // the device, but since we have it we can just manually add it.
            //
            if (isNewProperty)
            {
                auto attribute = ref new BACnetAdapterAttribute(
                                                        ref new String(AdapterLib::ToString(PROP_OBJECT_IDENTIFIER)),
                                                        bacnetAdapterProperty,
                                                        PropertyValue::CreateUInt32(PropertyObjectId)
                                                        );

                bacnetAdapterProperty += attribute;
            }

            //
            // Read and add all required attributes for the this DSB property (BACnet object)
            //

            for (UINT attrInx = 0; attrInx < ARRAYSIZE(objectDescPtr->AttributeDescriptors); ++attrInx)
            {
                const BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR* bacnetAttrDescDsec =
                    &objectDescPtr->AttributeDescriptors[attrInx];

                if (bacnetAttrDescDsec->Id == BACNET_ADAPTER_LAST_ATTRIBUTE)
                {
                    break;
                }

                if (!bacnetAttrDescDsec->IsReadFromDevice)
                {
                    continue;
                }

                //
                // Check if we are creating a new property or updating 
                // an existing one.
                //
                BACnetAdapterAttribute^ attribute;

                if (isNewProperty)
                {
                    // Creating a new property...
                    attribute = ref new BACnetAdapterAttribute(
                                            ref new String(AdapterLib::ToString(BACNET_PROPERTY_ID(bacnetAttrDescDsec->Id))),
                                            bacnetAdapterProperty
                                            );
                    attribute->Access = bacnetAttrDescDsec->Access;
                    attribute->COVBehavior = bacnetAttrDescDsec->COVBehavior;

                    DSB_ASSERT(!attribute->Value->Name->IsEmpty());
                }
                else
                {
                    // We are updating an existing property...
                    attribute = dynamic_cast<BACnetAdapterAttribute^>(bacnetAdapterProperty->Attributes->GetAt(attrInx));
                }

                BACNET_OBJECT_PROPERTY_DESCRIPTOR propAttrDesc;
                propAttrDesc.DeviceId = this->deviceId;
                propAttrDesc.ObjectInstance = bacnetObjectId.Bits.Instance;
                propAttrDesc.ObjectType = BACNET_OBJECT_TYPE(bacnetObjectId.Bits.Type);
                propAttrDesc.PropertyId = bacnetAttrDescDsec->Id;
                propAttrDesc.Params.AssociatedAdapterValue = dynamic_cast<BACnetAdapterValue^>(attribute->Value);
                propAttrDesc.ValueIndex = BACNET_ARRAY_ALL;

                status = this->stackInterface->ReadObjectProperty(
                                                &propAttrDesc,
                                                nullptr, nullptr, nullptr
                                                );
                if (status != ERROR_SUCCESS)
                {
                    if (status != ERROR_NOT_FOUND)
                    {
                        goto done;
                    }

                    //
                    // ERROR_NOT_FOUND is not an error, it means that
                    // the specific attribute is not supported by this object.
                    //
                    status = ERROR_SUCCESS;
                    continue;
                }

                if (bacnetAttrDescDsec->Id == PROP_OBJECT_NAME)
                {
                    IPropertyValue^ ipv = dynamic_cast<IPropertyValue^>(attribute->Value->Data);
                    DSB_ASSERT(ipv != nullptr);

                    bacnetAdapterProperty->SetName(ipv->GetString());
                }

                status = BACnetAdapterValue::TranslateAttributeValue(
                                            dynamic_cast<BACnetAdapterValue^>(attribute->Value),
                                            bacnetAttrDescDsec->Id,
                                            true // From BACnet
                                            );
                if (status == ERROR_NOT_FOUND)
                {
                    // Translation not needed
                    status = ERROR_SUCCESS;
                }

                if (status != ERROR_SUCCESS)
                {
                    goto done;
                }

                // Add the read attribute to the DSB property
                if (isNewProperty)
                {
                    bacnetAdapterProperty += attribute;
                }

            } // More attributes to read
        }
        catch (std::bad_alloc)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

    done:

        return status;
    }
    

    _Use_decl_annotations_
    uint32 
    BACnetAdapterDevice::ReadPropertyAttribute(
        IAdapterProperty^ adapterProperty,
        IAdapterValue^* adapterAttributePtr,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        if (adapterAttributePtr == nullptr)
        {
            return ERROR_INVALID_PARAMETER;
        }

        if (this->stackInterface == nullptr)
        {
            return ERROR_NOT_READY;
        }

        BACnetAdapterProperty^ bacnetProperty = dynamic_cast<BACnetAdapterProperty^>(adapterProperty);
        if (bacnetProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACnetAdapterValue^ bacnetAttribute = dynamic_cast<BACnetAdapterValue^>(*adapterAttributePtr);
        if (bacnetAttribute == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACNET_ADAPTER_OBJECT_ID bacnetObjectId(bacnetProperty->GetBACnetObjectId());
        BACNET_PROPERTY_ID propId;
        DWORD status = AdapterLib::FromString(bacnetAttribute->Name->Data(), &propId);
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        BACNET_OBJECT_PROPERTY_DESCRIPTOR propAttrDesc;
        propAttrDesc.DeviceId = this->deviceId;
        propAttrDesc.ObjectInstance = bacnetObjectId.Bits.Instance;
        propAttrDesc.ObjectType = BACNET_OBJECT_TYPE(bacnetObjectId.Bits.Type);
        propAttrDesc.PropertyId = propId;
        propAttrDesc.Params.AssociatedAdapterValue = bacnetAttribute;
        propAttrDesc.ValueIndex = BACNET_ARRAY_ALL;

        status = this->stackInterface->ReadObjectProperty(
                                        &propAttrDesc,
                                        nullptr, nullptr, nullptr
                                        );
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        if (bacnetAttribute->Data != nullptr)
        {
            status = BACnetAdapterValue::TranslateAttributeValue(bacnetAttribute, propId, true);
            if (status == ERROR_NOT_FOUND)
            {
                // Translation not needed
                status = ERROR_SUCCESS;
            }
        }

        return status;
    }


    _Use_decl_annotations_
    uint32 
    BACnetAdapterDevice::WritePropertyAttribute(
        IAdapterProperty^ adapterProperty,
        IAdapterValue^ adapterAttribute,
        bool IsRelinquish,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            RequestPtr = nullptr;
        }

        if (this->stackInterface == nullptr)
        {
            return ERROR_NOT_READY;
        }

        BACnetAdapterProperty^ bacnetProperty = dynamic_cast<BACnetAdapterProperty^>(adapterProperty);
        if (bacnetProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACnetAdapterValue^ bacnetAttribute = dynamic_cast<BACnetAdapterValue^>(adapterAttribute);
        if (bacnetAttribute == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACNET_ADAPTER_OBJECT_ID bacnetObjectId(bacnetProperty->GetBACnetObjectId());
        BACNET_PROPERTY_ID propId;
        DWORD status = AdapterLib::FromString(bacnetAttribute->Name->Data(), &propId);
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        BACNET_OBJECT_PROPERTY_DESCRIPTOR propAttrDesc;
        propAttrDesc.DeviceId = this->deviceId;
        propAttrDesc.ObjectInstance = bacnetObjectId.Bits.Instance;
        propAttrDesc.ObjectType = BACNET_OBJECT_TYPE(bacnetObjectId.Bits.Type);
        propAttrDesc.PropertyId = propId;

        if (!IsRelinquish)
        {
            status = bacnetAttribute->ToBACnet(
                                        bacnetObjectId,
                                        propId,
                                        propAttrDesc.Params.AssociatedBACnetValue
                                        );
            if (status != ERROR_SUCCESS)
            {
                return status;
            }
        }
        else
        {
            //
            // We relinquish a commendable property by writing a NULL
            // value to it.
            //
            RtlZeroMemory(
                &propAttrDesc.Params.AssociatedBACnetValue,
                sizeof(propAttrDesc.Params.AssociatedBACnetValue)
                );
            propAttrDesc.Params.AssociatedBACnetValue.tag = BACNET_APPLICATION_TAG_NULL;
        }

        return this->stackInterface->WriteObjectProperty(
                                        &propAttrDesc,
                                        nullptr, nullptr, nullptr
                                        );
    }


    uint32
    BACnetAdapterDevice::Subscribe(IAdapterSignal^ adapterSignal, bool IsSubscribe)
    {
        return this->doSubscribe(adapterSignal->GetHashCode(), IsSubscribe);
    }


    void
    BACnetAdapterDevice::UnsubscribeAll()
    {
        for (auto iter = this->signalSubscriptions.begin();
             iter != this->signalSubscriptions.end();
             iter++)
        {
            if (iter->second.IsActive)
            {
                (void)this->doSubscribe(iter->first, false);
            }
        }
    }


    uint32
    BACnetAdapterDevice::readDeviceInformation()
    {
        uint32 status = ERROR_SUCCESS;

        //
        // Get the BACnet object descriptor that
        // tells us how to assemble the device.
        //
        const BACNET_ADAPTER_OBJECT_DESCRIPTOR* bacnetObjectDescPtr = GetObjectDescriptor(OBJECT_DEVICE);

        DSB_ASSERT(bacnetObjectDescPtr != nullptr);

        _Analysis_assume_(bacnetObjectDescPtr != nullptr);

        //
        // We use the serial number as the device ID,
        // since it is unique.
        //
        this->serialNumber = this->deviceId.ToString();

        //
        // 1) We start with the device information
        //
        for (int attrInx = 0; attrInx < ARRAYSIZE(bacnetObjectDescPtr->AttributeDescriptors); ++attrInx)
        {
            const BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR* bacnetAttrDescDsec =
                &bacnetObjectDescPtr->AttributeDescriptors[attrInx];

            if (bacnetAttrDescDsec->Id == BACNET_ADAPTER_LAST_ATTRIBUTE)
            {
                break;
            }

            if (!bacnetAttrDescDsec->IsReadFromDevice)
            {
                continue;
            }

            //
            // Read the next attributes
            //
            BACnetAdapterValue^ attribute = ref new BACnetAdapterValue(
                                                    ref new String (AdapterLib::ToString(bacnetAttrDescDsec->Id)), 
                                                    this
                                                    );

            BACNET_OBJECT_PROPERTY_DESCRIPTOR deviceObjPropDesc;
            deviceObjPropDesc.DeviceId = this->deviceId;
            deviceObjPropDesc.ObjectInstance = this->deviceId;
            deviceObjPropDesc.ObjectType = OBJECT_DEVICE;
            deviceObjPropDesc.PropertyId = bacnetAttrDescDsec->Id;
            deviceObjPropDesc.Params.AssociatedAdapterValue = attribute;
            deviceObjPropDesc.ValueIndex = BACNET_ARRAY_ALL;

            status = this->stackInterface->ReadObjectProperty(
                                            &deviceObjPropDesc,
                                            nullptr, nullptr, nullptr
                                            );
            if (status != ERROR_SUCCESS)
            {
                goto done;
            }

            status = this->storeAttribute(attribute, bacnetAttrDescDsec->Id);
            if (status != ERROR_SUCCESS)
            {
                goto done;
            }

        } // More device attributes to read

        //
        // We do PROP_OBJECT_LIST as a special case,
        // since the stack does not support segmentation,
        // and we need to read each list member separately.
        //
        status = this->readObjectList();

    done:

        return status;
    }


    uint32
    BACnetAdapterDevice::readObjectList()
    {
        uint32 status = ERROR_SUCCESS;
        BACNET_OBJECT_PROPERTY_DESCRIPTOR deviceObjPropDesc;

        this->objectIdentifiers.clear();

        try
        {
            //
            // Read object list size
            //
            {
                BACnetAdapterValue^ listSizeValue = ref new BACnetAdapterValue(nullptr, this);

                deviceObjPropDesc.DeviceId = this->deviceId;
                deviceObjPropDesc.ObjectInstance = this->deviceId;
                deviceObjPropDesc.ObjectType = OBJECT_DEVICE;
                deviceObjPropDesc.PropertyId = PROP_OBJECT_LIST;
                deviceObjPropDesc.Params.AssociatedAdapterValue = listSizeValue;
                deviceObjPropDesc.ValueIndex = 0; // Get array size

                status = this->stackInterface->ReadObjectProperty(
                                                    &deviceObjPropDesc,
                                                    nullptr, nullptr, nullptr
                                                    );
                if (status != ERROR_SUCCESS)
                {
                    goto done;
                }

                IPropertyValue^ ipv = dynamic_cast<IPropertyValue^>(listSizeValue->Data);
                if (ipv == nullptr)
                {
                    DSB_ASSERT(FALSE);
                    
                    status = ERROR_INVALID_DATA;
                    goto done;
                }

                size_t listSize = size_t(ipv->GetUInt32());

                this->objectIdentifiers.resize(listSize);

            } // Read object list size

            //
            // Read object identifiers
            //
            for (UINT32 objIdInx = 0; objIdInx < UINT32(this->objectIdentifiers.size()); ++objIdInx)
            {
                BACnetAdapterValue^ objectIdValue = ref new BACnetAdapterValue(nullptr, this);

                deviceObjPropDesc.DeviceId = this->deviceId;
                deviceObjPropDesc.ObjectInstance = this->deviceId;
                deviceObjPropDesc.ObjectType = OBJECT_DEVICE;
                deviceObjPropDesc.PropertyId = PROP_OBJECT_LIST;
                deviceObjPropDesc.Params.AssociatedAdapterValue = objectIdValue;
                deviceObjPropDesc.ValueIndex = objIdInx + 1; // Get the n<th> array element

                status = this->stackInterface->ReadObjectProperty(
                                                    &deviceObjPropDesc,
                                                    nullptr, nullptr, nullptr
                                                    );
                if (status != ERROR_SUCCESS)
                {
                    goto done;
                }

                IPropertyValue^ ipv = dynamic_cast<IPropertyValue^>(objectIdValue->Data);
                if (ipv == nullptr)
                {
                    DSB_ASSERT(FALSE);

                    status = ERROR_INVALID_DATA;
                    goto done;
                }

                this->objectIdentifiers[objIdInx] = ULONG(ipv->GetUInt32());

            } // More object identifiers
        }
        catch (std::bad_alloc)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

    done:

        if (status != ERROR_SUCCESS)
        {
            this->objectIdentifiers.clear();
        }

        return status;
    }

    
    uint32
    BACnetAdapterDevice::readObjects()
    {
        uint32 status = ERROR_SUCCESS;

        for (UINT objInx = 0; objInx < UINT(this->objectIdentifiers.size()); ++objInx)
        {
            BACNET_ADAPTER_OBJECT_ID objectId(this->objectIdentifiers[objInx]);

            if (objectId.Bits.Type == OBJECT_DEVICE)
            {
                continue;
            }

            const wchar_t* wszIfHint = AdapterLib::ToString(BACNET_OBJECT_TYPE(objectId.Bits.Type));
            if (wszIfHint == nullptr)
            {
                continue;
            }
            BACnetAdapterProperty^ newProperty = ref new BACnetAdapterProperty(objectId.Ulong, this, StringReference(wszIfHint) + objectId.Bits.Instance.ToString());

            status = this->ReadProperty(objectId.Ulong, newProperty, nullptr);
            if (status != ERROR_SUCCESS)
            {
                if (status != ERROR_NOT_SUPPORTED)
                {
                    goto done;
                }

                // An unsupported object. move on...
                status = ERROR_SUCCESS;
                continue;
            }

            try
            {
                this->propertyMap.insert(std::pair<ULONG, BACnetAdapterProperty^>(objectId.Ulong, newProperty));

                // Finally add the property to the device
                this->properties.push_back(std::move(newProperty));
            }
            catch (std::bad_alloc)
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }
        }

    done:

        return status;
    }


    uint32
    BACnetAdapterDevice::subscribeForSignals()
    {
        uint32 status = ERROR_SUCCESS;

        if (this->stackInterface == nullptr)
        {
            status = ERROR_NOT_READY;
            goto done;
        }

        for (UINT objInx = 0; objInx < UINT(this->objectIdentifiers.size()); ++objInx)
        {
            BACNET_ADAPTER_OBJECT_ID objectId(this->objectIdentifiers[objInx]);

            if (objectId.Bits.Type == OBJECT_DEVICE)
            {
                continue;
            }

            try
            {
                const BACNET_ADAPTER_OBJECT_DESCRIPTOR* objectDescPtr = GetObjectDescriptor(BACNET_OBJECT_TYPE(objectId.Bits.Type));
                if (objectDescPtr == nullptr)
                {
                    // If we do not support this object, just ignore
                    continue;
                }

                for (int signalIdx = 0; signalIdx < ARRAYSIZE(objectDescPtr->SignalDescriptors); ++signalIdx)
                {
                    const BACNET_ADAPTER_SIGNAL_DESCRIPTOR& signalDescripror = objectDescPtr->SignalDescriptors[signalIdx];

                    BACNET_ADAPTER_SIGNAL_TYPE sigType = signalDescripror.Type;
                    if (sigType == BACNET_ADAPTER_LAST_SIGNAL)
                    {
                        break;
                    }

                    // Subscribe to the BACnet for notification
                    status = this->addSignalSubscription(signalDescripror, objectId);
                    if (status != ERROR_SUCCESS)
                    {
                        goto done;
                    }

                } // More signals for this object
            }
            catch (std::bad_alloc)
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }

        } // More objects

    done:

        return status;
    }


    uint32
    BACnetAdapterDevice::doSubscribe(int SignalHashCode, bool IsSubscribe)
    {
        uint32 status = ERROR_SUCCESS;

        auto subscriptions = this->signalSubscriptions.equal_range(SignalHashCode);

        for (auto iter = subscriptions.first; iter != subscriptions.second; ++iter)
        {
            SIGNAL_SUBSCRIPTION& subscription = iter->second;

            if ((!IsSubscribe && !subscription.IsActive) ||
                (IsSubscribe && subscription.IsActive))
            {
                // Already done...
                goto done;
            }

            switch (subscription.SignalDescriptor.Type)
            {
            case BACnetAdapterSignalTypeValueChanged:
            {
                BACNET_ADAPTER_OBJECT_ID objectId(subscription.SignalDescriptor.Cov.ObjectId);

                BACNET_OBJECT_PROPERTY_DESCRIPTOR propAttrDesc;
                propAttrDesc.DeviceId = this->deviceId;
                propAttrDesc.ObjectInstance = objectId.Bits.Instance;
                propAttrDesc.ObjectType = BACNET_OBJECT_TYPE(objectId.Bits.Type);
                propAttrDesc.PropertyId = subscription.SignalDescriptor.Cov.PropId;
                propAttrDesc.Params.AssociatedAdapterSignal = SignalHashCode;

                if (IsSubscribe)
                {
                    status = this->stackInterface->SubscribeCOVProperty(
                                                    &propAttrDesc,
                                                    nullptr, nullptr, nullptr
                                                    );
                }
                else
                {
                    status = this->stackInterface->UnsubscribeCOVProperty(
                                                    &propAttrDesc,
                                                    nullptr, nullptr, nullptr
                                                    );
                }
                if (status != ERROR_SUCCESS)
                {
                    goto done;
                }

                subscription.IsActive = IsSubscribe;
                break;

            } // BACnetAdapterSignalTypeValueChanged

            default:
                status = ERROR_NOT_SUPPORTED;
                goto done;
            }

        } // More subscriptions for this signals

    done:

        return status;
    }


    uint32
    BACnetAdapterDevice::storeAttribute(
        BACnetAdapterValue^ DeviceAttribute,
        ULONG BACnetPropertyId
        )
    {
        IPropertyValue^ ipv = dynamic_cast<IPropertyValue^>(DeviceAttribute->Data);
        if (ipv == nullptr)
        {
            DSB_ASSERT(FALSE);
            return ERROR_INVALID_PARAMETER;
        }

        switch (BACnetPropertyId)
        {
        case PROP_OBJECT_NAME:
            this->name = ipv->GetString();
            break;

        case PROP_VENDOR_NAME:
            this->vendor = ipv->GetString();
            break;

        case PROP_DESCRIPTION:
            this->description = ipv->GetString();
            break;

        case PROP_MODEL_NAME:
            this->model = ipv->GetString();

            if (!this->bacnetAdapter->IsAllowedDevice(this->model))
            {
                return ERROR_MOD_NOT_FOUND;
            }
            break;

        case PROP_FIRMWARE_REVISION:
            this->firmwareVersion = ipv->GetString();
            break;

        default:
            DSB_ASSERT(FALSE);
            break;
        }

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    bool 
    BACnetAdapterDevice::IsCovSupported(SIGNAL_SUBSCRIPTION &newSubscription, BACnetAdapterSignal^ signalOfType)
    {
        bool retVal = false;
        DWORD status = ERROR_SUCCESS;
        BACNET_ADAPTER_OBJECT_ID objectId(newSubscription.SignalDescriptor.Cov.ObjectId);
        BACNET_OBJECT_PROPERTY_DESCRIPTOR propAttrDesc;

        propAttrDesc.DeviceId = this->deviceId;
        propAttrDesc.ObjectInstance = objectId.Bits.Instance;
        propAttrDesc.ObjectType = BACNET_OBJECT_TYPE(objectId.Bits.Type);
        propAttrDesc.PropertyId = newSubscription.SignalDescriptor.Cov.PropId;
        propAttrDesc.Params.AssociatedAdapterSignal = signalOfType->GetHashCode();

        status = this->stackInterface->SubscribeCOVProperty(&propAttrDesc, nullptr, nullptr, nullptr);
        if (ERROR_SUCCESS == status)
        {
            // COV signal is supported
            this->stackInterface->UnsubscribeCOVProperty(&propAttrDesc, nullptr, nullptr, nullptr);
            retVal = true;
        }

        return retVal;
    }

    _Use_decl_annotations_
    uint32
    BACnetAdapterDevice::buildCovSignal(BACnetAdapterSignal^* CovSignalPtr)
    {
        uint32 status = ERROR_SUCCESS;

        DSB_ASSERT(CovSignalPtr != nullptr);

        *CovSignalPtr = nullptr;

        try
        {
            BACnetAdapterSignal^ adapterCovSignal = ref new BACnetAdapterSignal(Constants::CHANGE_OF_VALUE_SIGNAL, this);

            //
            // Signal parameters
            // 
            {
                // Template objects for signal parameter signatures
                BACnetAdapterProperty^ tmpltProperty = ref new BACnetAdapterProperty(L"DsbProperty", this);
                BACnetAdapterValue^ tmpltValue = ref new BACnetAdapterValue(L"DsbValue", tmpltProperty);

                adapterCovSignal += ref new BACnetAdapterValue(Constants::COV__PROPERTY_HANDLE, adapterCovSignal, tmpltProperty);
                adapterCovSignal += ref new BACnetAdapterValue(Constants::COV__ATTRIBUTE_HANDLE, adapterCovSignal, tmpltValue);
            }

            *CovSignalPtr = adapterCovSignal;

            this->signalMap.insert(std::pair<BACNET_ADAPTER_SIGNAL_TYPE, BACnetAdapterSignal^>(BACnetAdapterSignalTypeValueChanged, adapterCovSignal));

            this->signals.push_back(std::move(adapterCovSignal));
        }
        catch (std::bad_alloc)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

    done:

        if (status != ERROR_SUCCESS)
        {
            *CovSignalPtr = nullptr;
        }

        return status;
    }


    _Use_decl_annotations_
    uint32 
    BACnetAdapterDevice::addSignalSubscription(
        const BACNET_ADAPTER_SIGNAL_DESCRIPTOR& SignalDescriptor,
        const BACNET_ADAPTER_OBJECT_ID& ObjectId
        )
    {
        DWORD status = ERROR_SUCCESS;

        SIGNAL_SUBSCRIPTION newSubscription;
        newSubscription.SignalDescriptor = SignalDescriptor;

        //
        // Check if we already have a signal for this type
        //
        BACnetAdapterSignal^ signalOfType;
        auto sigMapIter = this->signalMap.find(SignalDescriptor.Type);
        if (sigMapIter != this->signalMap.end())
        {
            signalOfType = sigMapIter->second;
        }

        switch (SignalDescriptor.Type)
        {
        case BACnetAdapterSignalTypeValueChanged:
            newSubscription.SignalDescriptor.Cov.ObjectId = ObjectId.Ulong;

            // Create a new signal, if not already created
            if (signalOfType == nullptr)
            {
                status = this->buildCovSignal(&signalOfType);
                if (status != ERROR_SUCCESS)
                {
                    goto done;
                }
            }
            // verification of COV signal support needs to be done after 
            // signal creation because BacNet device access is required
            if (!this->IsCovSupported(newSubscription, signalOfType))
            {
                // this is not an error per say, it is just that the device doesn't support COV signal
                // so not adding signal in the list and returning ERROR_SUCCESS is OK
                goto done;
            }
            break;

        default:
            status = ERROR_NOT_SUPPORTED;
            goto done;
        }

        DSB_ASSERT(signalOfType != nullptr);

        this->signalSubscriptions.insert(
            std::pair<int, SIGNAL_SUBSCRIPTION>(
            signalOfType->GetHashCode(),
            newSubscription
            ));

    done:

        return status;
    }


    _Use_decl_annotations_
    void 
    BACnetAdapterDevice::SendSignal(
        String^ SignalName, 
        IAdapterProperty^ Property,
        IAdapterValue^ Attribute
        )
    {
        //
        // Search for target signal
        //

        BACnetAdapterSignal^ targetSignal = nullptr;
        for (auto signal : this->signals)
        {
            if (signal->Name == SignalName)
            {
                targetSignal = dynamic_cast<BACnetAdapterSignal^>(signal);
                break;
            }
        }
        if (targetSignal == nullptr)
        {
            DSB_ASSERT(false);
            return;
        }

        // 
        // Prepare signal...
        //

        AutoLock sync(&this->lock, true);

        if (targetSignal->Name == Constants::CHANGE_OF_VALUE_SIGNAL)
        {
            DSB_ASSERT(Property != nullptr);
            DSB_ASSERT(Attribute != nullptr);

            IAdapterValueVector^ sigParams = targetSignal->Params;
            DSB_ASSERT((sigParams != nullptr) && (sigParams->Size == 2));

            sigParams->GetAt(0)->Data = Property;
            sigParams->GetAt(1)->Data = Attribute;
        }

        //
        // Notify listeners
        //

        BACnetAdapter^ adapter = dynamic_cast<BACnetAdapter^>(this->parent);
        DSB_ASSERT(adapter != nullptr);

        (void)adapter->NotifySignalListener(targetSignal);
    }


} // namespace AdapterLib
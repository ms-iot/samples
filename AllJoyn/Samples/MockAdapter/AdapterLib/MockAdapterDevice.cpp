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
#include "MockAdapter.h"
#include "MockAdapterDevice.h"
#include "MockDevices.h"
#include "BfiDefinitions.h"
#include "BridgeUtils.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;
using namespace Windows::System::Threading;

using namespace BridgeRT;


namespace AdapterLib
{
    //
    // MockAdapterValue.
    // Description:
    //  The class that implements BridgeRT::IAdapterValue.
    //
    MockAdapterValue::MockAdapterValue(
        String^ ObjectName,
        Object^ ParentObject,
        Object^ DefaultData // = nullptr
        )
        : name(ObjectName)
        , parent(ParentObject)
        , data(DefaultData)
    {
    }

    MockAdapterValue::MockAdapterValue(const MockAdapterValue^ Other)
        : name(Other->name)
        , parent(Other->parent)
        , data(Other->data)
    {
    }


    uint32 
    MockAdapterValue::Set(IAdapterValue^ Other)
    {
        this->name = Other->Name;
        this->data = Other->Data;

        // Simulate the COV signal when a value has changed
        MockAdapterProperty^ mockAdapterProperty = dynamic_cast<MockAdapterProperty^>(this->parent);
        if (mockAdapterProperty != nullptr)
        {
            mockAdapterProperty->NotifyCovSignal(this);
        }

        return ERROR_SUCCESS;
    }


    //
    // MockAdapterProperty.
    // Description:
    //  The class that implements BridgeRT::IAdapterProperty.
    //
  
    MockAdapterProperty::MockAdapterProperty(String^ Name, BridgeRT::IAdapterDevice^ ParentObject) 
        : name(Name)
        , parent(ParentObject)
        , mockDescPtr(nullptr)
        , interfaceHint(ref new String(L""))
    {
        // Used only for signature spec
    }

    MockAdapterProperty::MockAdapterProperty(const MOCK_PROPERTY_DESCRIPTOR* MockPropertyDescPtr, BridgeRT::IAdapterDevice^ ParentObject)
        : parent(ParentObject)
        , name(MockPropertyDescPtr->Name)
        , mockDescPtr(MockPropertyDescPtr)
    {
        MockAdapterDevice^ adapterDevice = dynamic_cast<MockAdapterDevice^>(ParentObject);
        std::wstring adapterPrefix(adapterDevice->Parent->ExposedAdapterPrefix->Data());
        std::wstring wszIfName = adapterPrefix + L"." + cAdapterName + L"." + MockPropertyDescPtr->InterfaceHint->Data();
        interfaceHint = ref new String(wszIfName.c_str());
        (void)this->Reset();
    }


    MockAdapterProperty::MockAdapterProperty(const MockAdapterProperty^ Other)
        : name(Other->name)
        , parent(Other->parent)
        , attributes(Other->attributes)
        , mockDescPtr(Other->mockDescPtr)
        , interfaceHint(Other->interfaceHint)
    {
    }


    MockAdapterAttribute^
    MockAdapterProperty::GetAttributeByName(String^ Name)
    {
        for (auto attr : this->attributes)
        {
            if (attr->Value->Name == Name)
            {
                return dynamic_cast<MockAdapterAttribute^>(attr);
            }
        }

        return nullptr;
    }


    uint32
    MockAdapterProperty::Set(IAdapterProperty^ Other)
    {
        this->name = Other->Name;

        auto otherAttributes = Other->Attributes;

        if (this->attributes.size() != otherAttributes->Size)
        {
            throw ref new InvalidArgumentException(L"Incompatible Adapter Properties");
        }

        for (uint32 attrInx = 0; attrInx < otherAttributes->Size; ++attrInx)
        {
            MockAdapterValue^ attr = static_cast<MockAdapterValue^>(this->attributes[attrInx]->Value);

            attr->Set(otherAttributes->GetAt(attrInx)->Value);
        }

        return ERROR_SUCCESS;
    }


    uint32
    MockAdapterProperty::Reset()
    {
        uint32 status = ERROR_SUCCESS;

        DSB_ASSERT(this->mockDescPtr != nullptr);

        this->attributes.clear();

        // Property name
        {
            MockAdapterAttribute^ attribute = ref new MockAdapterAttribute(AdapterLib::ToString(PROP_NAME),
                this,
                PropertyValue::CreateString(this->mockDescPtr->CurrentValue.Name));

            this->attributes.push_back(std::move(attribute));
        }

        // Property type
        {
            MockAdapterAttribute^ attribute = ref new MockAdapterAttribute(AdapterLib::ToString(PROP_TYPE),
                this,
                PropertyValue::CreateString(AdapterLib::ToString(this->mockDescPtr->PropertyType)));

            this->attributes.push_back(std::move(attribute));
        }

        // Property identifier
        {
            MockAdapterAttribute^ attribute = ref new MockAdapterAttribute(AdapterLib::ToString(PROP_ID),
                this,
                PropertyValue::CreateUInt32(this->mockDescPtr->Id));

            this->attributes.push_back(std::move(attribute));
        }

        // Property units
        {
            MockAdapterAttribute^ attribute = ref new MockAdapterAttribute(AdapterLib::ToString(PROP_UNITS),
                this,
                PropertyValue::CreateString(AdapterLib::ToString(this->mockDescPtr->Units)));

            this->attributes.push_back(std::move(attribute));
        }

        // Property value
        {
            MockAdapterAttribute^ attribute = ref new MockAdapterAttribute(AdapterLib::ToString(PROP_PRESENT_VALUE), this);

            switch (this->mockDescPtr->CurrentValue.ValueType)
            {
            case PropertyType::Boolean:
                attribute->Value->Data = PropertyValue::CreateBoolean(bool(this->mockDescPtr->CurrentValue.InitialValue.AsSimpleType != 0));
                break;
            case PropertyType::Double:
                attribute->Value->Data = PropertyValue::CreateDouble(this->mockDescPtr->CurrentValue.InitialValue.AsSimpleType);
                break;
            case PropertyType::UInt32:
                attribute->Value->Data = PropertyValue::CreateUInt32((unsigned int)(this->mockDescPtr->CurrentValue.InitialValue.AsSimpleType));
                break;
            case PropertyType::String:
                attribute->Value->Data = PropertyValue::CreateString(this->mockDescPtr->CurrentValue.InitialValue.AsString);
                break;

            default:
                status = ERROR_NOT_SUPPORTED;
                break;
            }

            //set the access
            attribute->Access = (mockDescPtr->PropertyAccess == AdapterPropertyAccessRO) ? E_ACCESS_TYPE::ACCESS_READ : E_ACCESS_TYPE::ACCESS_READWRITE;

            //set the COV behavior
            attribute->COVBehavior = SignalBehavior::Always;

            this->attributes.push_back(std::move(attribute));
        }

        return status;
    }


    void 
    MockAdapterProperty::NotifyCovSignal(IAdapterValue^ Attribute)
    {
        MockAdapterDevice^ device = dynamic_cast<MockAdapterDevice^>(this->parent);
        DSB_ASSERT(device != nullptr);

        device->SendSignal(Constants::CHANGE_OF_VALUE_SIGNAL, this, Attribute);
    }


    //
    // MockAdapterAttribute.
    // Description:
    //  The class that implements BridgeRT::IAdapterAttribute.
    //
    MockAdapterAttribute::MockAdapterAttribute(String^ ObjectName, Object^ ParentObject, Object^ DefaultData)
        : value(ref new MockAdapterValue(ObjectName, ParentObject, DefaultData))
    {
    }

    MockAdapterAttribute::MockAdapterAttribute(const MockAdapterAttribute^ other)
        : value(other->value)
        , annotations(other->annotations)
        , covBehavior(other->covBehavior)
        , access(other->access)
    {

    }


    //
    // MockAdapterMethod.
    // Description:
    //  The class that implements BridgeRT::IAdapterDevice.
    //
    MockAdapterMethod::MockAdapterMethod(String^ ObjectName, Object^ ParentObject)
        : name(ObjectName)
        , parent(ParentObject)
        , result(HRESULT_FROM_WIN32(ERROR_NOT_READY))
    {
    }


    MockAdapterMethod::MockAdapterMethod(const MockAdapterMethod^ Other)
        : name(Other->name)
        , parent(Other->parent)
        , description(Other->description)
        , inParams(Other->inParams)
        , outParams(Other->outParams)
        , result(Other->result)
    {
    }


    void
    MockAdapterMethod::InputParams::set(IAdapterValueVector^ Params)
    {
        if (Params->Size != this->inParams.size())
        {
            throw ref new InvalidArgumentException(L"Incompatible method input parameters");
        }

        for (uint32 paramInx = 0; paramInx < Params->Size; ++paramInx)
        {
            dynamic_cast<MockAdapterValue^>(this->inParams[paramInx])->Set(Params->GetAt(paramInx));
        }
    }

    
    MockAdapterValue^ 
    MockAdapterMethod::GetInputParamByName(Platform::String^ InputParamName)
    {
        for (auto inputParam : inParams)
        {
            if (inputParam->Name == InputParamName)
            {
                return dynamic_cast<MockAdapterValue^>(inputParam);
            }
        }

        return nullptr;
    }
    
    
    MockAdapterValue^ 
    MockAdapterMethod::GetOutputParamByName(Platform::String^ OutputParamName)
    {
        for (auto outputParam : outParams)
        {
            if (outputParam->Name == OutputParamName)
            {
                return dynamic_cast<MockAdapterValue^>(outputParam);
            }
        }

        return nullptr;
    }


    void
    MockAdapterMethod::setResult(HRESULT Hr)
    {
        this->result = Hr;
    }

    MockAdapterIcon::MockAdapterIcon(Windows::Storage::StorageFile^ srcFile, Platform::String^ srcUrl)
        : url(srcUrl)
        , srcImageFile(srcFile)

    {
        mimeType = srcFile->ContentType;
    }

    MockAdapterIcon::MockAdapterIcon(Platform::String^ srcUrl, Platform::String^ srcMimeType)
        : url(srcUrl)
        , mimeType(srcMimeType)
        , srcImageFile(nullptr)
    {
    }

    Platform::Array<BYTE>^ 
    MockAdapterIcon::GetImage()
    {
        Platform::Array<BYTE>^ imageData = nullptr;

        try
        {
            // Start a sequential read of the source image file
            auto readOp = srcImageFile->OpenSequentialReadAsync();
            auto readTask = create_task(readOp);
            auto stream = readTask.get();

            // Get the File Size
            auto getPropsOp = srcImageFile->GetBasicPropertiesAsync();
            auto getPropsTask = create_task(getPropsOp);
            auto props = getPropsTask.get();
            unsigned int fileSize = (unsigned int)(props->Size);

            // Create a holding buffer
            imageData = ref new Platform::Array<BYTE>(fileSize);

            // Load the Image Data
            auto dataReader = ref new DataReader(stream);
            auto loadOp = dataReader->LoadAsync(fileSize);
            auto loadTask = create_task(loadOp);
            loadTask.get();

            // Read image data into a byte array
            dataReader->ReadBytes(imageData);
        }
        catch (...)
        {
            imageData = nullptr;
        }

        return imageData;
    }

    //
    // MockAdapterDevice.
    // Description:
    //  The class that implements BridgeRT::IAdapterDevice.
    //
    MockAdapterDevice::MockAdapterDevice(Platform::String^ Name, MockAdapter^ ParentObject)
        : name(Name)
        , parent(ParentObject)
    {
        // Used only for signature spec

        this->createMethods();
        this->createSignals();
    }


    MockAdapterDevice::MockAdapterDevice(const MOCK_DEVICE_DESCRIPTOR* MockDeviceDescPtr, MockAdapter^ ParentObject)
        : parent(ParentObject)
        , name(MockDeviceDescPtr->Name)
        , vendor(MockDeviceDescPtr->VendorName)
        , model(MockDeviceDescPtr->Model)
        , firmwareVersion(MockDeviceDescPtr->Version)
        , serialNumber(MockDeviceDescPtr->SerialNumer)
        , description(MockDeviceDescPtr->Description)
        , controlPanelHandler(nullptr)
        , lightingServiceHandler (nullptr)
        , icon(nullptr)
    {
        for (int propInx = 0; propInx <= ARRAYSIZE(MockDeviceDescPtr->PropertyDescriptors); ++propInx)
        {
            const MOCK_PROPERTY_DESCRIPTOR* propDescPtr = &MockDeviceDescPtr->PropertyDescriptors[propInx];

            if (propDescPtr->Id == LAST_DESCRIPTOR_ID)
            {
                break;
            }

            MockAdapterProperty^ newProperty = ref new MockAdapterProperty(propDescPtr, this);

            this->properties.push_back(std::move(newProperty));
        }

        this->createMethods();
        this->createSignals();

        if (MockDeviceDescPtr->IconResourceName == nullptr)
        {
            return;
        }

        auto loadImg = StorageFile::GetFileFromApplicationUriAsync(ref new Uri(L"ms-appx:///AdapterLib/" + MockDeviceDescPtr->IconResourceName));
        auto loadTask = create_task(loadImg);
        loadTask.then([this](StorageFile^ file)
        {
            if (file == nullptr)
            {
                return;
            }
            icon = ref new MockAdapterIcon(file);
        }).wait();

    }


    MockAdapterDevice::~MockAdapterDevice()
    {
        this->heartbeatTimer->Cancel();
    }


    IAdapterMethodVector^
    MockAdapterDevice::Methods::get()
    {
        //
        // We allocate a new set of methods since the user
        // will change them before calling a method
        //
        std::vector<IAdapterMethod^> methodSet;

        for (auto method : this->methods)
        {
            MockAdapterMethod^ newMethod = ref new MockAdapterMethod(dynamic_cast<MockAdapterMethod^>(method));

            methodSet.push_back(newMethod);
        }

        return ref new AdapterMethodVector(methodSet);
    }


    _Use_decl_annotations_
    void MockAdapterDevice::SendSignal(
        String^ SignalName,
        IAdapterProperty^ Property,
        IAdapterValue^ Attribute
        )
    {
        //
        // Search for target signal
        //

        MockAdapterSignal^ targetSignal = nullptr;
        for (auto signal : this->signals)
        {
            if (signal->Name == SignalName)
            {
                targetSignal = dynamic_cast<MockAdapterSignal^>(signal);
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

        AutoLock sync(this->lock);

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

        MockAdapter^ adapter = dynamic_cast<MockAdapter^>(this->parent);
        DSB_ASSERT(adapter != nullptr);

        (void)adapter->NotifySignalListener(targetSignal);
    }


    _Use_decl_annotations_
    uint32
    MockAdapterDevice::MethodDispatch(
        IAdapterMethod^ Method,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        MockAdapterMethod^ mockMethod = dynamic_cast<MockAdapterMethod^>(Method);
        if (mockMethod == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        //
        // Dispatch to desired method
        //

        if (Method->Name == DEVICE_RESET_METHOD)
        {
            this->methodReset(Method);

            return WIN32_FROM_HRESULT(mockMethod->HResult);
        }
      
        return ERROR_NOT_SUPPORTED;
    }


    void
    MockAdapterDevice::createMethods()
    {
        // Reset method
        {
            MockAdapterMethod^ method = ref new MockAdapterMethod(DEVICE_RESET_METHOD, this);

            //
            // Method input parameters
            //
            method->addInputParam(
                ref new MockAdapterValue(
                    DEVICE_RESET__PROPERTY_HANDLE,
                    method,
                    ref new MockAdapterProperty(L"AdapterProperty", this) // For signature spec
                    )
                );

            //
            // Method output parameters
            // 
            // ...

            this->methods.push_back(std::move(method));
        }
    }


    void
    MockAdapterDevice::createSignals()
    {
        // Device arrival signal
        {
            MockAdapterSignal^ signal = ref new MockAdapterSignal(Constants::CHANGE_OF_VALUE_SIGNAL, this);

            //
            // Signal parameters
            // 
            {
                // Template objects for signal parameter signatures
                MockAdapterProperty^ tmpltProperty = ref new MockAdapterProperty(L"AdapterProperty", this);
                MockAdapterValue^ tmpltValue = ref new MockAdapterValue(L"AdapterValue", tmpltProperty);

                signal += ref new MockAdapterValue(Constants::COV__PROPERTY_HANDLE, signal, tmpltProperty);
                signal += ref new MockAdapterValue(Constants::COV__ATTRIBUTE_HANDLE, signal, tmpltValue);
            }

            this->signals.push_back(std::move(signal));
        }

        // Heartbeat signal
        {
            MockAdapterSignal^ signal = ref new MockAdapterSignal(HEARTBEAT_SIGNAL, this);
            this->signals.push_back(std::move(signal));

            // Fire the timer every 5s
            this->heartbeatTimer = ThreadPoolTimer::CreatePeriodicTimer(
                ref new TimerElapsedHandler(this, &MockAdapterDevice::heartbeatTimerElapsed), TimeSpan{ 50000000L });
        }
    }
    

    void
    MockAdapterDevice::heartbeatTimerElapsed(ThreadPoolTimer^ timer)
    {
        this->SendSignal(HEARTBEAT_SIGNAL, nullptr, nullptr);
    }


    _Use_decl_annotations_
    void
    MockAdapterDevice::methodReset(IAdapterMethod^ Method)
    {
        HRESULT hr = S_OK;
        IAdapterValue^ propHandleValue;
        MockAdapterProperty^ mockProperty;
        MockAdapterMethod^ mockMethod = dynamic_cast<MockAdapterMethod^>(Method);
        if (mockMethod == nullptr)
        {
            DSB_ASSERT(FALSE);
        }

        if (mockMethod->InputParams->Size != 1)
        {
            hr = E_INVALIDARG;
            goto done;
        }

        propHandleValue = mockMethod->InputParams->GetAt(0);
        DSB_ASSERT(propHandleValue != nullptr);

        if (propHandleValue->Data == nullptr)
        {
            // 
            // No property is given, means do a device level reset...
            //
            goto done;
        }

        mockProperty = dynamic_cast<MockAdapterProperty^>(propHandleValue->Data);
        if (mockProperty == nullptr)
        {
            hr = E_HANDLE;
            goto done;
        }

        // Reset the property value to 'factory defaults'
        hr = HRESULT_FROM_WIN32(mockProperty->Reset());

    done:

        mockMethod->setResult(hr);
    }


    _Use_decl_annotations_
    MockAdapterProperty^
    MockAdapterDevice::GetPropertyByName(Platform::String^ PropertyName)
    {
        for (auto adapterProperty : properties)
        {
            if (adapterProperty->Name == PropertyName)
            {
                return dynamic_cast<MockAdapterProperty^>(adapterProperty);
            }
        }

        return nullptr;
    }


    _Use_decl_annotations_
    MockAdapterMethod^ 
    MockAdapterDevice::GetMethodByName(Platform::String^ MethodName)
    {
        for (auto adapterMethod : methods)
        {
            if (adapterMethod->Name == MethodName)
            {
                return dynamic_cast<MockAdapterMethod^>(adapterMethod);
            }
        }

        return nullptr;
    }


    _Use_decl_annotations_
    MockAdapterSignal^ 
    MockAdapterDevice::GetSignalByName(_In_ Platform::String^ SignalName)
    {
        for (auto adapterSignal : signals)
        {
            if (adapterSignal->Name == SignalName)
            {
                return dynamic_cast<MockAdapterSignal^>(adapterSignal);
            }
        }

        return nullptr;
    }


    _Use_decl_annotations_
    void 
    MockAdapterDevice::AddProperty(BridgeRT::IAdapterProperty^ NewProperty)
    {
        this->properties.push_back(NewProperty);
    }


    _Use_decl_annotations_
    void 
    MockAdapterDevice::AddMethod(BridgeRT::IAdapterMethod^ NewMethod)
    {
        this->methods.push_back(NewMethod);
    }
    

    _Use_decl_annotations_
    void 
    MockAdapterDevice::AddSignal(BridgeRT::IAdapterSignal^ NewSignal)
    {
        this->signals.push_back(NewSignal);
    }

} // namespace AdapterLib
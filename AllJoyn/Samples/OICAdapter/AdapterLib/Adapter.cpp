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

//
// Module Name:
//
//      Adapter.cpp
//
// Abstract:
//
//      Adapter class implementation.
//
//      Adapter class implements the IAdapter interface.
//      When the DSB bridge component uses the adapter it instantiates an Adapter object
//      and uses it as the IAdapter.
//
//
#include "pch.h"
#include "Adapter.h"
#include "AdapterDevice.h"
#include "AdapterMethod.h"
#include "AdapterProperty.h"
#include "AdapterSignal.h"
#include "AdapterValue.h"

#include "util.h"
#include "ocpayload.h"
#include "octypes.h"

#include <string>
#include <algorithm>
#include <cctype>
#include <functional>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::System::Threading;

using namespace BridgeRT;

namespace AdapterLib
{
    //static member initialization
    static const string PLATFORM_DISCOVERY_QUERY = OC_RSRVD_PLATFORM_URI;
    static const string DEVICE_DISCOVERY_QUERY = OC_RSRVD_DEVICE_URI;
    static const string RESOURCE_DISCOVERY_QUERY = OC_RSRVD_WELL_KNOWN_URI;
    static const string RESOURCE_TYPE_SUBQUERY = string("?") + OC_RSRVD_RESOURCE_TYPE + string("=");
    static const string PRESENCE_QUERY = string("coap://") + OC_MULTICAST_PREFIX + OC_RSRVD_PRESENCE_URI;
    static const string RT_PRESENCE = OC_RSRVD_RESOURCE_TYPE_PRESENCE;

    static const int64 OCProcessTimeout = 100;            //in msec
    static const int64 DeviceDiscoveryTimeout = 60000;            //in msec
    static const int64 DeviceMonitorTimeout = 60000;            //in msec
    static const DWORD ResourceDiscoveryTimeout = 15000;     //in msecs

    static const int64 DevicePingTimeout = 300;            //in seconds

    static const GUID APPLICATION_GUID = { 0xc0d9e107,0x6ec8,0x4255,{0xb1,0xc1,0x05,0xae,0x3a,0x58,0xf8,0xad} };


    Adapter^ Adapter::adapterInstance = nullptr;

    Adapter::Adapter()
    {
        adapterInstance = this;

        Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
        Windows::ApplicationModel::PackageId^ packageId = package->Id;
        Windows::ApplicationModel::PackageVersion versionFromPkg = packageId->Version;

        m_vendor = ref new String(cVendor.c_str());
        m_adapterName = ref new String(cAdapterName.c_str());
        // the adapter prefix must be something like "com.mycompany" (only alpha num and dots)
        // it is used by the Device System Bridge as root string for all services and interfaces it exposes
        std::wstring adapterPrefix = cDomainPrefix + L"." + cVendor;
        std::transform(adapterPrefix.begin(), adapterPrefix.end(), adapterPrefix.begin(), std::tolower);
        m_exposedAdapterPrefix = ref new String(adapterPrefix.c_str());
        m_exposedApplicationGuid = Platform::Guid(APPLICATION_GUID);

        if (nullptr != package &&
            nullptr != packageId)
        {
            m_exposedApplicationName = packageId->Name;
            m_version = versionFromPkg.Major.ToString() + L"." + versionFromPkg.Minor.ToString() + L"." + versionFromPkg.Revision.ToString() + L"." + versionFromPkg.Build.ToString();
        }
        else
        {
            m_exposedApplicationName = L"DeviceSystemBridge";
            m_version = L"0.0.0.0";
        }

        CreateSignals();
    }


    Adapter::~Adapter()
    {
        Shutdown();
    }


    _Use_decl_annotations_
    uint32 Adapter::SetConfiguration(const Platform::Array<byte>^ ConfigurationData)
    {
        UNREFERENCED_PARAMETER(ConfigurationData);

        return ERROR_NOT_SUPPORTED;
    }


    _Use_decl_annotations_
    uint32 Adapter::GetConfiguration(Platform::Array<byte>^* ConfigurationDataPtr)
    {
        UNREFERENCED_PARAMETER(ConfigurationDataPtr);

        return ERROR_NOT_SUPPORTED;
    }


    uint32 Adapter::Initialize()
    {
        //Start the OCStackProcessing thread
        m_OICProcessThreadAction = ThreadPool::RunAsync(ref new WorkItemHandler([](IAsyncAction^ action)
        {
            OCStackResult status = OC_STACK_OK;
            {
                AutoLock sync(adapterInstance->m_ocStackLock);

            status = OCInit(NULL, 0, OC_CLIENT);
            if (status != OC_STACK_OK)
            {
                throw ref new FailureException();
            }
            adapterInstance->InitPlatformDiscovery();
            adapterInstance->InitPresenceRequest();
            }

            while (action->Status != AsyncStatus::Canceled)
            {
                {
                    AutoLock sync(adapterInstance->m_ocStackLock);
                OCProcess();
                }
                Sleep(OCProcessTimeout);
            }
        })
        );

        //Start the DeviceDiscovery timer
        TimeSpan ts;
        ts.Duration = DeviceDiscoveryTimeout * 10000LL;     //in 100nsec interval;
        m_DeviceDiscoveryTimer = ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler([this](ThreadPoolTimer^ timer)
        {
            InitPlatformDiscovery();
        }), ts);

        //Start the Device Monitor timer
        ts.Duration = DeviceMonitorTimeout * 10000LL;     //in 100nsec interval;
        m_DeviceMonitorTimer = ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler([this](ThreadPoolTimer^ timer)
        {
            MonitorDevices();
        }), ts);

        return ERROR_SUCCESS;
    }

    void Adapter::MonitorDevices()
    {
        AutoLock sync(m_lock);

        //if the last discovered time for the device is greater than DevicePingTimeout, remove it
        steady_clock::time_point tpNow = steady_clock::now();
        auto iter = m_devices.begin();
        while (iter != m_devices.end())
        {
            seconds secondsElapsed = duration_cast<seconds>(tpNow - dynamic_cast<AdapterDevice^>(*iter)->LastDiscoveredTime);
            if (secondsElapsed.count() >= DevicePingTimeout)
            {
                //remove the device
                NotifyDeviceRemoval(*iter);
                iter = m_devices.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    OCStackResult Adapter::InitPlatformDiscovery()
    {
        AutoLock sync(m_ocStackLock);
        OCCallbackData cbData{};

        cbData.cb = OnNotifyPlatform;
        cbData.context = reinterpret_cast<void*>(this);
        cbData.cd = nullptr;

        return OCDoResource(NULL, OC_REST_DISCOVER, PLATFORM_DISCOVERY_QUERY.c_str(), 0, 0, CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);
    }

    OCStackResult Adapter::InitDeviceDiscovery(PlatformInfo* pInfo)
    {
        AutoLock sync(m_ocStackLock);
        OCCallbackData cbData{};

        cbData.cb = OnNotifyDevice;
        cbData.context = reinterpret_cast<void*>(pInfo);

        return OCDoResource(NULL, OC_REST_DISCOVER, DEVICE_DISCOVERY_QUERY.c_str(), &pInfo->addr, 0, CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);
    }

    OCStackResult Adapter::InitResourceDiscovery(AdapterDevice^ pDevice)
    {
        AutoLock sync(m_ocStackLock);
        OCCallbackData cbData{};

        cbData.cb = OnNotifyResource;

        //Need to increase the ref count of the AdapterDevice before passing it as context. Otherwise the object will get deleted.
        IUnknown* pUnknown = reinterpret_cast<IUnknown*>(pDevice);
        pUnknown->AddRef();

        cbData.context = pUnknown;

        //release the ref count in the deleter
        cbData.cd = [](void* ctx)
        {
            IUnknown* pUnknown = reinterpret_cast<IUnknown*>(ctx);
            pUnknown->Release();
        };
        return OCDoResource(NULL, OC_REST_DISCOVER, RESOURCE_DISCOVERY_QUERY.c_str(), pDevice->DevAddr, 0, CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);
    }

    OCStackResult Adapter::InitGetRequest(ResourceContext* context, const std::string& query)
    {
        AutoLock sync(m_ocStackLock);
        OCCallbackData cbData{};

        cbData.cb = OnNotifyRepresentation;
        cbData.context = reinterpret_cast<void*>(context);
        
        return OCDoResource(NULL, OC_REST_GET, query.c_str(), context->pProperty->Parent->DevAddr, 0, CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);

    }

    OCStackResult Adapter::InitPostRequest(ResourceContext* context, const std::string& uri, OCPayload* payload)
    {
        AutoLock sync(m_ocStackLock);
        OCCallbackData cbData{};

        cbData.cb = OnNotifyRepresentation;
        cbData.context = reinterpret_cast<void*>(context);

        return OCDoResource(NULL, OC_REST_POST, uri.c_str(), context->pProperty->Parent->DevAddr, payload, CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);
    }

    OCStackResult Adapter::InitPutRequest(ResourceContext* context, const std::string& uri, OCPayload* payload)
    {
        AutoLock sync(m_ocStackLock);
        OCCallbackData cbData{};

        cbData.cb = OnNotifyRepresentation;
        cbData.context = reinterpret_cast<void*>(context);

        return OCDoResource(NULL, OC_REST_PUT, uri.c_str(), context->pProperty->Parent->DevAddr, payload, CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);
    }

    OCStackResult Adapter::InitObserveRequest(AdapterProperty^ pProperty)
    {
        AutoLock sync(m_ocStackLock);
        OCCallbackData cbData{};

        cbData.cb = OnNotifyObserve;
        cbData.context = reinterpret_cast<void*>(pProperty);
        OCDoHandle hdl = NULL;

        auto result = OCDoResource(&hdl, OC_REST_OBSERVE, pProperty->Uri.c_str(), pProperty->Parent->DevAddr, 0, CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);
        if (result == OC_STACK_OK)
        {
            pProperty->ObserverHandle = hdl;
        }
        
        return result;
    }

    OCStackResult Adapter::InitPresenceRequest()
    {
        AutoLock sync(m_ocStackLock);
        OCCallbackData cbData{};

        cbData.cb = OnNotifyPresence;
        cbData.context = NULL;

        return OCDoResource(NULL, OC_REST_PRESENCE, PRESENCE_QUERY.c_str(), 0, 0, CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);
     }

    OCStackApplicationResult Adapter::OnNotifyObserve(void* ctx, OCDoHandle /*handle*/, OCClientResponse * response)
    {
        AutoLock sync(adapterInstance->m_lock);

        if (response && response->payload
            && response->payload->type == PAYLOAD_TYPE_REPRESENTATION
            && response->result == OC_STACK_OK)
        {
            OCRepPayload* payload = nullptr;
            OCRepPayloadValue* val = nullptr;

            payload = (OCRepPayload*)response->payload;
            AdapterProperty^ pProperty = reinterpret_cast<AdapterProperty^>(ctx);
            if (payload && pProperty)
            {
                //find which value changed
                val = payload->values;
                while (val)
                {
                    auto pAttribute = pProperty->GetAttribute(val->name);
                    if (pAttribute && dynamic_cast<AdapterValue^>(pAttribute->Value) != *val)
                    {
                        //Change in value detected, update and notify
                        if (pProperty->Update(*val, pAttribute) == ERROR_SUCCESS)
                        {
                            IAdapterSignal^ signal = pProperty->Parent->GetSignal(Constants::CHANGE_OF_VALUE_SIGNAL);
                            if (signal)
                            {
                                signal->Params->GetAt(0)->Data = pProperty;

                                // Set the 'Attribute_Handle' signal parameter
                                signal->Params->GetAt(1)->Data = pAttribute->Value;

                                adapterInstance->NotifySignalListener(signal);
                            }
                        }
                    }
                    val = val->next;
                }
            }
        }
        return OC_STACK_KEEP_TRANSACTION;
    }

    OCStackApplicationResult Adapter::OnNotifyPlatform(void* /*ctx*/, OCDoHandle /*handle*/, OCClientResponse * response)
    {
        if (response && response->result == OC_STACK_OK)
        {
            OCPlatformPayload *payload = (OCPlatformPayload*)response->payload;

            if (payload && payload->base.type == PAYLOAD_TYPE_PLATFORM)
            {
                AutoLock sync(adapterInstance->m_lock);

                shared_ptr<PlatformInfo> pInfo{ new PlatformInfo{ payload->info, response->devAddr } };

                auto iter = find_if(adapterInstance->m_platforms.begin(), adapterInstance->m_platforms.end(), [&pInfo](shared_ptr<PlatformInfo>& info)
                {
                    return(*info == *pInfo);
                });

                if (iter == adapterInstance->m_platforms.end())
                {
                    adapterInstance->m_platforms.push_back(pInfo);
                    iter = prev(adapterInstance->m_platforms.end());
                }

                //auto pInfo = new PlatformInfo{ payload->info, response->devAddr };
                adapterInstance->InitDeviceDiscovery((*iter).get());
            }
        }
        return OC_STACK_KEEP_TRANSACTION;
    }

    OCStackApplicationResult Adapter::OnNotifyDevice(void* ctx, OCDoHandle /*handle*/, OCClientResponse * response)
    {
        if (response && response->result == OC_STACK_OK)
        {
            OCDevicePayload *payload = (OCDevicePayload*)response->payload;
            if (payload && payload->base.type == PAYLOAD_TYPE_DEVICE && payload->sid)
            {
                AutoLock sync(adapterInstance->m_lock);

                PlatformInfo *pInfo = reinterpret_cast<PlatformInfo*>(ctx);
                if (pInfo)
                {
                    try
                    {
                        AdapterDevice^ device = ref new AdapterDevice(*pInfo, *payload, adapterInstance);
                            adapterInstance->InitResourceDiscovery(device);
                        }
                    catch (NullReferenceException^)
                    {
                        //dont add the device to the list. ignore
                    }
                }
            }
        }
        return OC_STACK_KEEP_TRANSACTION;
    }

    OCStackApplicationResult Adapter::OnNotifyResource(void* ctx, OCDoHandle /*handle*/, OCClientResponse * response)
    {
        if (response && (response->result == OC_STACK_NO_RESOURCE || response->result == OC_STACK_OK))
        {
            OCDiscoveryPayload *payload = (OCDiscoveryPayload*)response->payload;
                AdapterDevice^ pDevice = reinterpret_cast<AdapterDevice^>(ctx);
                if (pDevice)
                {
                AutoLock sync(adapterInstance->m_lock);
                //check if the device already exist
                auto iter = find_if(adapterInstance->m_devices.begin(), adapterInstance->m_devices.end(), [pDevice](IAdapterDevice^ pDev)
                {
                    return (wstring(pDevice->SerialNumber->Data()) == pDev->SerialNumber->Data());
                });
                if (iter != adapterInstance->m_devices.end())
                {
                    //Device exist, see if resources are same. if there is a mismatch in the resources, we must remove the previous device representation
                    //Also if there are no resources remove the device
                    bool bRemove = false;
                    /*if (response->result == OC_STACK_NO_RESOURCE)
                    { 
                        bRemove = true;
                    }
                    else if (payload)
                    {
                        bRemove = !adapterInstance->DoDeviceResourcesMatch(dynamic_cast<AdapterDevice^>(*iter), payload->resources);
                    }*/
                    if (bRemove)
                    {
                        //remove the device
                        adapterInstance->NotifyDeviceRemoval(*iter);
                        adapterInstance->m_devices.erase(iter);
                    }
                    else
                    {
                        // the device representation match. update the last discovered time and leave
                        dynamic_cast<AdapterDevice^>(*iter)->LastDiscoveredTime = steady_clock::now();
                        goto leave;
                    }
                }
            }
            if (response->result == OC_STACK_OK)
            {
                    vector<shared_ptr<ResourceContext>> contextList;
                    OCResourcePayload* resource = payload->resources;

                    while (resource)
                    {
                        OCStringLL* strll = resource->types;
                        while (strll)
                        {
                            if (resource->uri && strll->value)
                            {
                                AdapterProperty^ pProperty = ref new AdapterProperty(resource->uri, strll->value, pDevice);
                                if (resource->bitmap & OC_OBSERVABLE)
                                {
                                    pProperty->Observable = true;
                                }
                                string query = resource->uri + RESOURCE_TYPE_SUBQUERY + strll->value;
                                auto pContext = new ResourceContext{};
                                pContext->hdl = CreateEvent(nullptr, TRUE, FALSE, nullptr);
                                pContext->pProperty = pProperty;
                                
                                adapterInstance->InitGetRequest(pContext, query);

                                contextList.push_back(shared_ptr<ResourceContext>(pContext, [](ResourceContext* pCtx)
                                {
                                    if (pCtx)
                                    {
                                        if (pCtx->hdl)
                                        {
                                            CloseHandle(pCtx->hdl);
                                            pCtx->hdl = nullptr;
                                        }
                                        delete pCtx;
                                    }
                                }));
                                
                            }
                            strll = strll->next;
                        }
                        resource = resource->next;
                    }
                    ThreadPool::RunAsync(ref new WorkItemHandler([pDevice, contextList](IAsyncAction^)
                    {
                        for (auto& pContext : contextList)
                        {
                            if ((WaitForSingleObject(pContext->hdl, ResourceDiscoveryTimeout) == WAIT_OBJECT_0) && (pContext->result == OC_STACK_OK))
                            {
                                AutoLock sync(adapterInstance->m_lock);
                                pDevice->AddProperty(pContext->pProperty);
                                if (pContext->pProperty->Observable)
                                {
                                    //Register for property change notifications
                                    adapterInstance->InitObserveRequest(pContext->pProperty);
                                }
                            }
                        }

                        {
                            AutoLock sync(adapterInstance->m_lock);
                        //check if the device already exist
                        auto iter = find_if(adapterInstance->m_devices.begin(), adapterInstance->m_devices.end(), [pDevice](IAdapterDevice^ pDev)
                        {
                            return (wstring(pDevice->SerialNumber->Data()) == pDev->SerialNumber->Data());
                        });
                        if (iter == adapterInstance->m_devices.end())
                        {
                        pDevice->LastDiscoveredTime = steady_clock::now();
                            adapterInstance->m_devices.push_back(pDevice);
                        }
                        }

                        adapterInstance->NotifyDeviceArrival(pDevice);
                    }));
                }
            }
    leave:
        return OC_STACK_DELETE_TRANSACTION;
    }

    OCStackApplicationResult Adapter::OnNotifyRepresentation(void* ctx, OCDoHandle /*handle*/, OCClientResponse * response)
    {
        OCStackResult result = OC_STACK_ERROR;
        OCRepPayload* payload = nullptr;
        ResourceContext* context = nullptr;

        AutoLock sync(adapterInstance->m_lock);
        if (response)
        {
            result = response->result;

            context = reinterpret_cast<ResourceContext*>(ctx);
            if (!context || !context->pProperty)
            {
                result = OC_STACK_INVALID_PARAM;
                context->result = result;
                return OC_STACK_DELETE_TRANSACTION;
            }

            if (result != OC_STACK_OK)
            {
                goto done;
            }

            payload = (OCRepPayload*)response->payload;
            if (!payload)
            {
                result = OC_STACK_ERROR;
                goto done;
            }

            result = adapterInstance->AddResources(payload, context->pProperty);
        }

    done:
        if (context)
        {
            context->result = result;
            if (context->hdl)
            {
                SetEvent(context->hdl); //indicates completion of the transaction
            }
        }
        return OC_STACK_DELETE_TRANSACTION;
    }

    OCStackApplicationResult Adapter::OnNotifyPresence(void* /*ctx*/, OCDoHandle /*handle*/, OCClientResponse * response)
    {
        if (response && response->result == OC_STACK_OK)
        {
            OCPresencePayload *payload = (OCPresencePayload*)response->payload;

            if (payload && payload->base.type == PAYLOAD_TYPE_PRESENCE)
            {
                if (payload->resourceType && string(payload->resourceType) == RT_PRESENCE)
                {
                    if (payload->trigger == OC_PRESENCE_TRIGGER_CREATE)
                    {
                        adapterInstance->InitPlatformDiscovery();
                    }
                }
            }
        }
        return OC_STACK_KEEP_TRANSACTION;
    }

    uint32
    Adapter::Shutdown()
    {
        if (m_DeviceMonitorTimer)
        {
            m_DeviceMonitorTimer->Cancel();
            m_DeviceMonitorTimer = nullptr;
        }

        if (m_DeviceDiscoveryTimer)
        {
            m_DeviceDiscoveryTimer->Cancel();
            m_DeviceDiscoveryTimer = nullptr;
        }

        if (m_OICProcessThreadAction)
        {
            m_OICProcessThreadAction->Cancel();
        }

        OCStop();

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::EnumDevices(
        ENUM_DEVICES_OPTIONS Options,
        IAdapterDeviceVector^* DeviceListPtr,
        IAdapterIoRequest^* RequestPtr
        )
    {
        uint32 status = ERROR_SUCCESS;

        UNREFERENCED_PARAMETER(Options);

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        if (DeviceListPtr == nullptr)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        {
            AutoLock sync(m_lock);

            //just return whatever list we have. the device will be notified as they arrive
            *DeviceListPtr = ref new AdapterDeviceVector(m_devices);
        }

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32
    Adapter::GetProperty(
        IAdapterProperty^ Property,
        IAdapterIoRequest^* RequestPtr
        )
    {
        uint32 status = ERROR_SUCCESS;
        string query;
        DWORD dwErr = 0;
        ResourceContext context{};

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        // sanity check
        AdapterProperty^ pProperty = dynamic_cast<AdapterProperty^>(Property);
        if (pProperty == nullptr)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        query = pProperty->Uri + RESOURCE_TYPE_SUBQUERY + pProperty->ResourceType;
        
        context.hdl = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        context.pProperty = pProperty;

        //Update the property
        InitGetRequest(&context, query);
        if ((dwErr = WaitForSingleObject(context.hdl, ResourceDiscoveryTimeout)) == WAIT_OBJECT_0)
        {
            status = Win32FromOCStackResult(context.result);
        }
        else
        {
            status = ERROR_TIMEOUT;
        }

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32
    Adapter::SetProperty(
        IAdapterProperty^ Property,
        IAdapterIoRequest^* RequestPtr
        )
    {
        uint32 status = ERROR_SUCCESS;
        ResourceContext context{};
        OCRepPayload* payload = nullptr;
        IPropertyValue ^propertyValue = nullptr;

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        // sanity check
        AdapterProperty^ pProperty = dynamic_cast<AdapterProperty^>(Property);
        if (pProperty == nullptr ||
            pProperty->Attributes == nullptr)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        payload = OCRepPayloadCreate();

        if (!payload)
        {
            status = ERROR_OUTOFMEMORY;
            goto done;
        }

        for (IAdapterAttribute^ pAttribute : pProperty->Attributes)
        {
            propertyValue = dynamic_cast<IPropertyValue ^>(pAttribute->Value->Data);
            if (nullptr == propertyValue)
            {
                status = ERROR_INVALID_PARAMETER;
                goto done;
            }

            status = AddToPayload(payload, ConvertTo<string>(pAttribute->Value->Name), propertyValue, dynamic_cast<AdapterAttribute^>(pAttribute)->GetDimensions());
            if (status != ERROR_SUCCESS)
            {
                goto done;
            }
        }

        context.hdl = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        context.pProperty = pProperty;

        //Update the property
        InitPutRequest(&context, pProperty->Uri, reinterpret_cast<OCPayload*>(payload));
        if (WaitForSingleObject(context.hdl, ResourceDiscoveryTimeout) == WAIT_OBJECT_0)
        {
            status = Win32FromOCStackResult(context.result);
        }
        else
        {
            status = ERROR_TIMEOUT;
        }
        CloseHandle(context.hdl);
        context.hdl = nullptr;

    done:
        if (status != ERROR_SUCCESS && payload)
        {
            OCPayloadDestroy(reinterpret_cast<OCPayload*>(payload));
        }
        return status;
    }


    _Use_decl_annotations_
    uint32
    Adapter::GetPropertyValue(
        IAdapterProperty^ Property,
        String^ AttributeName,
        IAdapterValue^* ValuePtr,
        IAdapterIoRequest^* RequestPtr
        )
    {
        uint32 status = ERROR_SUCCESS;

        // sanity check
        AdapterProperty^ pProperty = dynamic_cast<AdapterProperty^>(Property);
        if (ValuePtr == nullptr ||
            pProperty == nullptr ||
            pProperty->Attributes == nullptr)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        status = GetProperty(Property, RequestPtr);
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }
        
        // find corresponding attribute
        *ValuePtr = nullptr;
        for (auto attribute : pProperty->Attributes)
        {
            if (attribute->Value != nullptr &&
                attribute->Value->Name == AttributeName)
            {
                *ValuePtr = attribute->Value;
                status = ERROR_SUCCESS;
            }
        }

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32
    Adapter::SetPropertyValue(
        IAdapterProperty^ Property,
        IAdapterValue^ Value,
        IAdapterIoRequest^* RequestPtr
        )
    {
        uint32 status = ERROR_SUCCESS;
        ResourceContext context{};
        OCRepPayload* payload = nullptr;
        IPropertyValue ^propertyValue = nullptr;

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        // sanity check
        AdapterProperty^ pProperty = dynamic_cast<AdapterProperty^>(Property);
        AdapterValue^ pValue = dynamic_cast<AdapterValue^>(Value);
        AdapterAttribute^ pAttribute = pProperty->GetAttribute(Value->Name);
        if (pValue == nullptr ||
            pProperty == nullptr ||
            pAttribute == nullptr)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        propertyValue = dynamic_cast<IPropertyValue ^>(pValue->Data);
        if (nullptr == propertyValue)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        payload = OCRepPayloadCreate();

        if (!payload)
        {
            status = ERROR_OUTOFMEMORY;
            goto done;
        }

        status = AddToPayload(payload, ConvertTo<string>(pValue->Name), propertyValue, pAttribute->GetDimensions());
        
        context.hdl = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        context.pProperty = pProperty;

        //Update the property
        InitPostRequest(&context, pProperty->Uri, reinterpret_cast<OCPayload*>(payload));
        if (WaitForSingleObject(context.hdl, ResourceDiscoveryTimeout) == WAIT_OBJECT_0)
        {
            status = Win32FromOCStackResult(context.result);
        }
        else
        {
            status = ERROR_TIMEOUT;
        }
        CloseHandle(context.hdl);
        context.hdl = nullptr;

    done:
        return status;
    }

    OCStackResult Adapter::AddResources(OCRepPayload* payload, AdapterProperty^ pProperty, const std::wstring& namePrefix)
    {
        OCStackResult result = OC_STACK_OK;
        if (payload)
        {
            auto val = payload->values;
            wstring valName{};
            if (namePrefix.length() > 0)
            {
                valName = namePrefix + L".";
            }
            while (val)
            {
                wstring name = valName + ConvertTo<wstring, string>(val->name);

                if (val->type == OCREP_PROP_OBJECT)
                {
                    AddResources(val->obj, pProperty, name);
                }
                else if (val->type == OCREP_PROP_ARRAY && val->arr.type == OCREP_PROP_OBJECT)
                {
                    OCRepPayloadValueArray arr = val->arr;
                    size_t dimTotal = calcDimTotal(arr.dimensions);
                    if (dimTotal > 0)
                    {
                        for (size_t i = 0; i < dimTotal; ++i)
                        {
                            AddResources(arr.objArray[i], pProperty, name);
                        }
                    }
                }
                else
                {
                    pProperty->Add(*val, namePrefix);
                }
                val = val->next;
            }
        }
        return result;
    }

    uint32 Adapter::AddToPayload(OCRepPayload*& payload, const string& name, IPropertyValue ^value, const size_t dimensions[MAX_REP_ARRAY_DEPTH])
    {
        uint32 status = ERROR_SUCCESS;
        bool bValueAdded = false;

        if (!value || !payload)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        switch (value->Type)
        {
        case PropertyType::Boolean:
            bValueAdded = OCRepPayloadSetPropBool(payload, name.c_str(), value->GetBoolean());
            break;

        case PropertyType::Int64:
            bValueAdded = OCRepPayloadSetPropInt(payload, name.c_str(), value->GetInt64());
            break;

        case PropertyType::Double:
            bValueAdded = OCRepPayloadSetPropDouble(payload, name.c_str(), value->GetDouble());
            break;

        case PropertyType::String:
            bValueAdded = OCRepPayloadSetPropString(payload, name.c_str(), ConvertTo<string>(value->GetString()).c_str());
            break;

        case PropertyType::Int64Array:
        {
            Platform::Array<int64_t>^ valArray;
            value->GetInt64Array(&valArray);
            if (valArray && valArray->Length == calcDimTotal(dimensions))
            {
                bValueAdded = OCRepPayloadSetIntArray(payload, name.c_str(), valArray->Data, const_cast<size_t*>(dimensions));
            }
            break;
        }

        case PropertyType::DoubleArray:
        {
            Platform::Array<double>^ valArray;
            value->GetDoubleArray(&valArray);
            if (valArray && valArray->Length == calcDimTotal(dimensions))
            {
                bValueAdded = OCRepPayloadSetDoubleArray(payload, name.c_str(), valArray->Data, const_cast<size_t*>(dimensions));
            }
            break;
        }

        case PropertyType::BooleanArray:
        {
            Platform::Array<bool>^ valArray;
            value->GetBooleanArray(&valArray);
            if (valArray && valArray->Length == calcDimTotal(dimensions))
            {
                bValueAdded = OCRepPayloadSetBoolArray(payload, name.c_str(), valArray->Data, const_cast<size_t*>(dimensions));
            }
            break;
        }

        case PropertyType::StringArray:
        {
            Platform::Array<String^>^ valArray;
            value->GetStringArray(&valArray);
            if (valArray && valArray->Length == calcDimTotal(dimensions))
            {
                size_t i = 0;
                char** tempBuffer = nullptr;
                try
                {
                    tempBuffer = new char*[valArray->Length]{};
                    for (auto str : valArray)
                    {
                        tempBuffer[i] = new char[str->Length() + 1]{ 0 };
                        strcpy_s(tempBuffer[i], str->Length() + 1, ConvertTo<string>(str).c_str());
                        ++i;
                    }

                    bValueAdded = OCRepPayloadSetStringArray(payload, name.c_str(), const_cast<const char**>(tempBuffer), const_cast<size_t*>(dimensions));
                }
                catch (std::bad_alloc)
                {
                    //memory allocation failed. clean up the memory and return failure 
                }
                if (tempBuffer)
                {
                    //now delete temp buffer
                    for (i = 0; i < valArray->Length; ++i)
                    {
                        if (tempBuffer[i])
                        {
                            delete[] tempBuffer[i];
                        }
                    }
                    delete[] tempBuffer;
                }
            }
            break;
        }

        default:
            break;
        }

        if (!bValueAdded)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

    done:
        return status;
    }

    bool Adapter::DoDeviceResourcesMatch(AdapterDevice^ pDevice, OCResourcePayload* resource)
    {
        AutoLock sync(adapterInstance->m_lock);

        bool bMatch = true;
        vector<string> resourceUris;
        while (resource)
        {
            resourceUris.push_back(resource->uri);
            resource = resource->next;
        }
        if (pDevice->Properties->Size != resourceUris.size())
        {
            bMatch = false;
        }
        else
        {
            for (IAdapterProperty^ pProperty : pDevice->Properties)
            {
                if (find(resourceUris.begin(), resourceUris.end(), dynamic_cast<AdapterProperty^>(pProperty)->Uri) == resourceUris.end())
                {
                    bMatch = false;
                    break;
                }
            }
        }
        return bMatch;
    }

    _Use_decl_annotations_
    uint32
    Adapter::CallMethod(
        IAdapterMethod^ Method,
        IAdapterIoRequest^* RequestPtr
        )
    {
        UNREFERENCED_PARAMETER(Method);
        UNREFERENCED_PARAMETER(RequestPtr);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::RegisterSignalListener(
        IAdapterSignal^ Signal,
        IAdapterSignalListener^ Listener,
        Object^ ListenerContext
        )
    {
        if (Signal == nullptr || Listener == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        AutoLock sync(this->m_lock);

        m_signalListeners.insert(
            {
                Signal->GetHashCode(),
                SIGNAL_LISTENER_ENTRY(Signal, Listener, ListenerContext)
            }
        );

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::UnregisterSignalListener(
        IAdapterSignal^ Signal,
        IAdapterSignalListener^ Listener
        )
    {
        uint32 status = ERROR_NOT_FOUND;

        // Sync access to listeners list
        AutoLock sync(m_lock);

        // We use the hash code as the signal key
        int mmapkey = Signal->GetHashCode();

        // get all the listeners for the SignalHandle
        auto handlers = m_signalListeners.equal_range(mmapkey);

        for (auto iter = handlers.first; iter != handlers.second; ++iter)
        {
            if (iter->second.Listener == Listener)
            {
                // found it remove it
                m_signalListeners.erase(iter);

                status = ERROR_SUCCESS;
                break;
            }
        }

        return status;
    }


    _Use_decl_annotations_
    uint32
    Adapter::NotifySignalListener(
        IAdapterSignal^ Signal
        )
    {
        if (Signal == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        AutoLock sync(this->m_lock);

        auto handlerRange = m_signalListeners.equal_range(Signal->GetHashCode());

        std::vector<std::pair<int, SIGNAL_LISTENER_ENTRY>> handlers(
            handlerRange.first,
            handlerRange.second);

        for (auto iter = handlers.begin(); iter != handlers.end(); ++iter)
        {
            IAdapterSignalListener^ listener = iter->second.Listener;
            Object^ listenerContext = iter->second.Context;
            listener->AdapterSignalHandler(Signal, listenerContext);
        }

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::NotifyDeviceArrival(
        IAdapterDevice^ Device
        )
    {
        if (Device == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        IAdapterSignal^ deviceArrivalSignal = m_signals.at(DEVICE_ARRIVAL_SIGNAL_INDEX);
        deviceArrivalSignal->Params->GetAt(DEVICE_ARRIVAL_SIGNAL_PARAM_INDEX)->Data = Device;
        this->NotifySignalListener(deviceArrivalSignal);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::NotifyDeviceRemoval(
        IAdapterDevice^ Device
        )
    {
        if (Device == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        IAdapterSignal^ deviceRemovalSignal = m_signals.at(DEVICE_REMOVAL_SIGNAL_INDEX);
        deviceRemovalSignal->Params->GetAt(DEVICE_REMOVAL_SIGNAL_PARAM_INDEX)->Data = Device;
        this->NotifySignalListener(deviceRemovalSignal);

        return ERROR_SUCCESS;
    }


    void
    Adapter::CreateSignals()
    {
         //Device Arrival Signal
        AdapterSignal^ deviceArrivalSignal = ref new AdapterSignal(Constants::DEVICE_ARRIVAL_SIGNAL);
        deviceArrivalSignal += ref new AdapterValue(
            Constants::DEVICE_ARRIVAL__DEVICE_HANDLE,
            nullptr
            );

        // Device Removal Signal
        AdapterSignal^ deviceRemovalSignal = ref new AdapterSignal(Constants::DEVICE_REMOVAL_SIGNAL);
        deviceRemovalSignal += ref new AdapterValue(
            Constants::DEVICE_REMOVAL__DEVICE_HANDLE,
            nullptr
            );

        // Add Signals to Adapter Signals
        m_signals.push_back(deviceArrivalSignal);
        m_signals.push_back(deviceRemovalSignal);
    }
} // namespace AdapterLib
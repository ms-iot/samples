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
#include "AllJoynHelpers.h"
#include "AllJoynStatus.h"

using namespace Windows::Devices::AllJoyn;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;
using namespace Platform;
using namespace concurrency;
using namespace std;

namespace DeviceProviders
{
    const char * c_sessionlessSignalMatchString = "type='signal',sessionless='t'";

    AllJoynProvider::AllJoynProvider()
        : m_aboutListener(nullptr)
        , m_busAttachment(nullptr)
        , m_busListener(nullptr)
        , m_alljoynInitialized(false)
        , m_weakThis(this)
    {
        DEBUG_LIFETIME_IMPL(AllJoynProvider);
    }

    AllJoynProvider::~AllJoynProvider()
    {
        this->Shutdown();
    }

    AllJoynStatus^ AllJoynProvider::Start()
    {
        QStatus status = ER_OK;
        try
        {
            if (!m_alljoynInitialized)
            {
                status = alljoyn_init();
                if (ER_OK != status)
                {
                    goto leave;
                }
                m_alljoynInitialized = true;
            }

            if (nullptr == m_busAttachment)
            {
                m_busAttachment = alljoyn_busattachment_create(ALLJOYN_APPLICATION_NAME.c_str(), true);
                if (nullptr == m_busAttachment)
                {
                    status = ER_OUT_OF_MEMORY;
                    goto leave;
                }
                status = alljoyn_busattachment_start(m_busAttachment);
                if (ER_OK != status)
                {
                    goto leave;
                }
                status = alljoyn_busattachment_connect(m_busAttachment, nullptr);
                if (ER_OK != status)
                {
                    goto leave;
                }
                status = alljoyn_busattachment_addmatch(m_busAttachment, c_sessionlessSignalMatchString);
                if (ER_OK != status)
                {
                    goto leave;
                }
            }

            if (nullptr == m_aboutListener)
            {
                alljoyn_aboutlistener_callback aboutlistenerCallback = { AboutAnnounced };
                m_aboutListener = alljoyn_aboutlistener_create(&aboutlistenerCallback, &m_weakThis);
                if (nullptr == m_aboutListener)
                {
                    status = ER_OUT_OF_MEMORY;
                    goto leave;
                }
                alljoyn_busattachment_registeraboutlistener(m_busAttachment, m_aboutListener);
            }

            if (nullptr == m_busListener)
            {
                alljoyn_buslistener_callbacks buslistenerCallbacks = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
                m_busListener = alljoyn_buslistener_create(&buslistenerCallbacks, &m_weakThis);
                if (nullptr == m_aboutListener)
                {
                    status = ER_OUT_OF_MEMORY;
                    goto leave;
                }
                alljoyn_busattachment_registerbuslistener(m_busAttachment, m_busListener);
            }

            alljoyn_busattachment_whoimplements_interfaces(m_busAttachment, nullptr, 0);
        }
        catch (...)
        {
            status = QStatus::ER_FAIL;
        }

    leave:
        if (ER_OK != status)
        {
            this->Shutdown();
        }
        return ref new AllJoynStatus(status);
    }

    IVector<IService ^>^ AllJoynProvider::Services::get()
    {
        auto services = ref new Vector<IService^>();

        AutoLock lock(&m_servicesLock, true);
        for (auto& service : m_servicesMap)
        {
            services->Append(service.second);
        }
        return services;
    }

    void AllJoynProvider::Shutdown()
    {
        if (nullptr != m_busAttachment)
        {
            alljoyn_busattachment_cancelwhoimplements_interfaces(m_busAttachment, nullptr, 0);
        }

        if (nullptr != m_busListener)
        {
            alljoyn_busattachment_unregisterbuslistener(m_busAttachment, m_busListener);
            alljoyn_buslistener_destroy(m_busListener);
            m_busListener = nullptr;
        }

        if (nullptr != m_aboutListener)
        {
            alljoyn_busattachment_unregisteraboutlistener(m_busAttachment, m_aboutListener);
            alljoyn_aboutlistener_destroy(m_aboutListener);
            m_aboutListener = nullptr;
        }

        map<string, AllJoynService^> servicesCopy;
        {
            AutoLock lock(&m_servicesLock, true);
            servicesCopy = m_servicesMap;
            m_servicesMap.clear();
        }

        for (auto& service : servicesCopy)
        {
            RemoveService(service.second, alljoyn_sessionlostreason::ALLJOYN_SESSIONLOST_INVALID);
        }
        servicesCopy.clear();

        // stop and delete bus attachment
        if (nullptr != m_busAttachment)
        {
            alljoyn_busattachment_removematch(m_busAttachment, c_sessionlessSignalMatchString);
            alljoyn_busattachment_disconnect(m_busAttachment, nullptr);
            alljoyn_busattachment_stop(m_busAttachment);
            alljoyn_busattachment_join(m_busAttachment);
            alljoyn_busattachment_destroy(m_busAttachment);
            m_busAttachment = nullptr;
        }

        // shutdown AllJoyn if necessary
        if (m_alljoynInitialized)
        {
            alljoyn_shutdown();
            m_alljoynInitialized = false;
        }
    }

    IVector<IService^>^ AllJoynProvider::GetServicesWhichImplementInterface(Platform::String^ interfaceName)
    {
        auto services = ref new Vector<IService^>();

        AutoLock lock(&m_servicesLock, true);
        for (auto service : m_servicesMap)
        {
            if (service.second->ImplementsInterface(interfaceName))
            {
                services->Append(service.second);
            }
        }
        return services;
    }

    void AJ_CALL AllJoynProvider::AboutAnnounced(_In_ const void* context,
        _In_ const char* serviceName,
        _In_ uint16_t version,
        _In_ alljoyn_sessionport port,
        _In_ const alljoyn_msgarg objectDescriptionArg,
        _In_ const alljoyn_msgarg aboutDataArg)
    {
        UNREFERENCED_PARAMETER(version);

        AllJoynProvider ^provider = static_cast<const WeakReference *>(context)->Resolve<AllJoynProvider>();

        if (provider != nullptr)
        {
            auto objectDescriptionArgCopy = alljoyn_msgarg_copy(objectDescriptionArg);
            auto aboutDataArgCopy = alljoyn_msgarg_copy(aboutDataArg);
            string serviceNameString = serviceName;

            create_task([provider, objectDescriptionArgCopy, aboutDataArgCopy, serviceNameString, port]
            {
                AllJoynService ^ newService = nullptr;
                {
                    AutoLock lock(&provider->m_servicesLock, true);

                    auto iter = provider->m_servicesMap.find(serviceNameString);
                    if (iter != provider->m_servicesMap.end())
                    {
                        iter->second->Initialize(aboutDataArgCopy, objectDescriptionArgCopy);
                    }
                    else
                    {
                        newService = ref new AllJoynService(provider, serviceNameString, port);
                        newService->Initialize(aboutDataArgCopy, objectDescriptionArgCopy);

                        provider->m_servicesMap[serviceNameString] = newService;
                    }
                }
                if (newService)
                {
                    try
                    {
                        provider->ServiceJoined(provider, ref new ServiceJoinedEventArgs(newService));
                    }
                    catch (Exception^ ex)
                    {
                        OutputDebugString(ex->Message->Data());
                    }
                }
            });
        }
    }

    void AJ_CALL AllJoynProvider::NameOwnerChanged(const void* context, const char* busName, const char* previousOwner, const char* newOwner)
    {
        AllJoynProvider ^provider = static_cast<const WeakReference *>(context)->Resolve<AllJoynProvider>();

        if (provider != nullptr && newOwner == nullptr && strcmp(busName, previousOwner) == 0)
        {
            AllJoynService^ service = nullptr;
            {
                AutoLock lock(&provider->m_servicesLock, true);
                auto iter = provider->m_servicesMap.find(busName);
                if (iter != provider->m_servicesMap.end() && !iter->second->GetHasAnySessions())
                {
                    service = iter->second;
                }
            }

            if (service)
            {
                provider->RemoveService(service, alljoyn_sessionlostreason::ALLJOYN_SESSIONLOST_INVALID);
            }
        }
    }

    void AllJoynProvider::RemoveService(AllJoynService ^ service, alljoyn_sessionlostreason reason)
    {
        {
            AutoLock lock(&m_servicesLock, true);
            m_servicesMap.erase(AllJoynHelpers::PlatformToMultibyteStandardString(service->Name));
        }

        service->Shutdown();

        try
        {
            ServiceDropped(this, ref new ServiceDroppedEventArgs(service, static_cast<AllJoynSessionLostReason>(reason)));
        }
        catch (Exception^ ex)
        {
            OutputDebugString(ex->Message->Data());
        }
    }
}
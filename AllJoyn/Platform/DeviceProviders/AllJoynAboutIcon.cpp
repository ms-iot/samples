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
#include "AllJoynAboutIcon.h"
#include "AllJoynBusObject.h"
#include "AllJoynInterface.h"
#include "AllJoynHelpers.h"
#include "AllJoynProperty.h"
#include "AllJoynMethod.h"
#include "AllJoynStatus.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace std;
using namespace Concurrency;

namespace DeviceProviders
{
    string AllJoynAboutIcon::AboutIconObjectPath = "/About/DeviceIcon";
    String^ AllJoynAboutIcon::AboutIconInterface = L"org.alljoyn.Icon";

    AllJoynAboutIcon::AllJoynAboutIcon()
    {
    }

    AllJoynAboutIcon::~AllJoynAboutIcon()
    {
    }

    IAsyncAction^ AllJoynAboutIcon::InitializeAsync(AllJoynBusObject^ aboutIconObject)
    {
        if (!aboutIconObject)
        {
            return nullptr;
        }

        IInterface^ iconIface;
        try
        {
            iconIface = aboutIconObject->GetInterface(AboutIconInterface);
            if (!iconIface)
            {
                return nullptr;
            }
        }
        catch (...)
        {
            return nullptr;
        }

        std::vector<task<void>> tasks;

        auto prop = iconIface->GetProperty("Version");
        if (prop != nullptr)
        {
            tasks.push_back(create_task(prop->ReadValueAsync()).then([this](ReadValueResult^ readValueResult)
            {
                auto value = dynamic_cast<IBox<uint16>^>(readValueResult->Value);
                if (value != nullptr)
                {
                    m_version = value->Value;
                }
            }));
        }

        prop = iconIface->GetProperty("MimeType");
        if (prop != nullptr)
        {
            tasks.push_back(create_task(prop->ReadValueAsync()).then([this](ReadValueResult^ readValueResult)
            {
                m_mimeType = dynamic_cast<String^>(readValueResult->Value);
            }));
        }

        auto method = iconIface->GetMethod("GetContent");
        if (method != nullptr)
        {
            tasks.push_back(create_task(method->InvokeAsync(ref new Vector<Object^>())).then([this](InvokeMethodResult^ methodResult)
            {
                if (methodResult->Status->IsSuccess && methodResult->Values->Size == 1)
                {
                    auto vectorFromMethod = dynamic_cast<IVector<Object^>^>(methodResult->Values->GetAt(0));
                    if (vectorFromMethod != nullptr)
                    {
                        if (m_content == nullptr)
                        {
                            m_content = ref new Vector<uint8>();
                        }
                        else
                        {
                            m_content->Clear();
                        }

                        for (auto& elt : vectorFromMethod)
                        {
                            m_content->Append(dynamic_cast<IBox<uint8>^>(static_cast<Object^>(elt))->Value);
                        }
                    }
                }
            }));
        }

        method = iconIface->GetMethod("GetUrl");
        if (method != nullptr)
        {
            tasks.push_back(create_task(method->InvokeAsync(ref new Vector<Object^>())).then([this](InvokeMethodResult^ methodResult)
            {
                if (methodResult->Status->IsSuccess && methodResult->Values->Size == 1)
                {
                    m_url = dynamic_cast<String^>(methodResult->Values->GetAt(0));
                }
            }));
        }

        return create_async([tasks]()
        {
            when_all(begin(tasks), end(tasks)).wait();
        });
    }

    uint16 AllJoynAboutIcon::Version::get()
    {
        return m_version;
    }

    String^ AllJoynAboutIcon::MimeType::get()
    {
        return m_mimeType;
    }

    IVectorView<uint8>^ AllJoynAboutIcon::Content::get()
    {
        return m_content ? m_content->GetView() : nullptr;
    }

    String^ AllJoynAboutIcon::Url::get()
    {
        return m_url;
    }
}
